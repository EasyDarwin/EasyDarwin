/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 */
/*
    File:       QTSSRawFileModule.cpp

    Contains:   Implementation of Raw File module

    

*/

#include "QTSSRawFileModule.h"
#include "OSHeaders.h"
#include "StrPtrLen.h"
#include "OSArrayObjectDeleter.h"
#include "QTSSModuleUtils.h"
#include "OSMemory.h"
#include "ev.h"
#include "QTSSMemoryDeleter.h"

#define RAWFILE_FILE_ASYNC 1
#define RAW_FILE_DEBUGGING 0


// ATTRIBUTES IDs

static QTSS_AttributeID sStateAttr                  = qtssIllegalAttrID;
static QTSS_AttributeID sFileAttr                   = qtssIllegalAttrID;
static QTSS_AttributeID sFileBufferAttr             = qtssIllegalAttrID;
static QTSS_AttributeID sReadOffsetAttr             = qtssIllegalAttrID;
static QTSS_AttributeID sWriteOffsetAttr            = qtssIllegalAttrID;

// STATIC DATA

static StrPtrLen    sRawSuffix(".raw");

static const UInt32 kReadingBufferState = 0;
static const UInt32 kWritingBufferState = 1;

// FUNCTIONS

static QTSS_Error QTSSRawFileModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error Preprocess(QTSS_StandardRTSP_Params* inParams);



QTSS_Error QTSSRawFileModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSRawFileModuleDispatch);
}


QTSS_Error QTSSRawFileModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_RTSPPreProcessor_Role:
            return Preprocess(&inParams->rtspPreProcessorParams);
    }
    return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{
    (void)QTSS_AddRole(QTSS_RTSPPreProcessor_Role);

    static char*        sStateName          =   "QTSSRawFileModuleState";
    static char*        sFileName           =   "QTSSRawFileModuleFile";
    static char*        sFileBufferName     =   "QTSSRawFileModuleFileBuffer";
    static char*        sReadOffsetName     =   "QTSSRawFileModuleReadOffset";
    static char*        sWriteOffsetName    =   "QTSSRawFileModuleWriteOffset";

    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sStateName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sStateName, &sStateAttr);
    
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sFileName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sFileName, &sFileAttr);

    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sFileBufferName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sFileBufferName, &sFileBufferAttr);

    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sReadOffsetName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sReadOffsetName, &sReadOffsetAttr);

    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sWriteOffsetName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sWriteOffsetName, &sWriteOffsetAttr);

    // Tell the server our name!
    static char* sModuleName = "QTSSRawFileModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}


QTSS_Error Preprocess(QTSS_StandardRTSP_Params* inParams)
{
    static UInt32 sFileBufSize = 32768;
    static UInt32 sInitialState = kReadingBufferState;
    static UInt32 sZero = 0;
    
    UInt32 theLen = 0;
    UInt32* theStateP = NULL;
    QTSS_Error theErr = QTSS_NoErr;

    QTSS_Object theFile = NULL;
    
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sStateAttr, 0, (void**)&theStateP, &theLen);
    if ((theStateP == NULL) || (theLen != sizeof(UInt32)))
    {
        // Initial state. We haven't started sending the file yet, so
        // check to see if this is our request, and if it is, set everything up.
        
        // Only operate if this is a DESCRIBE
        QTSS_RTSPMethod* theMethod = NULL;
        UInt32 theLen = 0;
        if ((QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqMethod, 0,
                (void**)&theMethod, &theLen) != QTSS_NoErr) || (theLen != sizeof(QTSS_RTSPMethod)))
        {
            Assert(0);
            return QTSS_RequestFailed;
        }

        if (*theMethod != qtssDescribeMethod)
            return QTSS_RequestFailed;

        // Check to see if this is a raw file request
        char* theFilePath = NULL;          
        (void)QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqLocalPath, 0, &theFilePath);
		QTSSCharArrayDeleter theFilePathDeleter(theFilePath);
        theLen = ::strlen(theFilePath);
		
        // Copy the full path, and append a ".raw"
        OSCharArrayDeleter rawPath(NEW char[theLen + sRawSuffix.Len + 4]);
        ::memcpy(rawPath.GetObject(), theFilePath, theLen);
        ::strcpy(rawPath.GetObject() + theLen, sRawSuffix.Ptr);
        
#if RAWFILE_FILE_ASYNC
        theErr = QTSS_OpenFileObject(rawPath.GetObject(), qtssOpenFileAsync, &theFile);
#else
        theErr = QTSS_OpenFileObject(rawPath.GetObject(), qtssOpenFileAsync, &theFile);
#endif
        
        // If the file doesn't exist, and if this is a path with a '.raw' at the end,
        // check to see if the path without the extra .raw exists
        if (theErr != QTSS_NoErr)
        {
            theFile = NULL;
            rawPath.GetObject()[theLen] = '\0';
            
            if (theLen > sRawSuffix.Len)
            {
                StrPtrLen comparer((theFilePath + theLen) - sRawSuffix.Len, sRawSuffix.Len);
                if (comparer.Equal(sRawSuffix))
                {
#if RAWFILE_FILE_ASYNC
                    theErr = QTSS_OpenFileObject(rawPath.GetObject(), qtssOpenFileAsync, &theFile);
#else
                    theErr = QTSS_OpenFileObject(rawPath.GetObject(), kOpenFileNoFlags, &theFile);
#endif
                }
            }
        }

        // If the file doesn't exist, we should probably return a 404 not found.
        if (theErr != QTSS_NoErr)
            return QTSS_RequestFailed;

        // Before sending any response, set keep alive to off for this connection
        // Regardless of what the client sends, the server always closes the connection after sending the file
        static Bool16 sFalse = false;
        (void)QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));

        // We have a real file. Setup all the dictionary values we need
        (void)QTSS_SetValue(inParams->inRTSPSession, sFileAttr, 0, &theFile, sizeof(theFile));

        // Create a buffer to store data.
        char* theFileBuffer = NEW char[sFileBufSize];
        (void)QTSS_SetValue(inParams->inRTSPSession, sFileBufferAttr, 0, &theFileBuffer, sizeof(theFileBuffer));

        // Store our initial state
        (void)QTSS_SetValue(inParams->inRTSPSession, sStateAttr, 0, &sInitialState, sizeof(sInitialState));
        theStateP = &sInitialState; // so we can proceed normally

        (void)QTSS_SetValue(inParams->inRTSPSession, sReadOffsetAttr, 0, &sZero, sizeof(sZero));
        (void)QTSS_SetValue(inParams->inRTSPSession, sWriteOffsetAttr, 0, &sZero, sizeof(sZero));
    }
    
    // Get our attributes
    char** theFileBufferP = NULL;
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sFileBufferAttr, 0, (void**)&theFileBufferP, &theLen);
    Assert(theFileBufferP != NULL);
    Assert(theLen == sizeof(char*));
    
    QTSS_Object* theFileP = NULL;
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sFileAttr, 0, (void**)&theFileP, &theLen);
    Assert(theFileP != NULL);
    Assert(theLen == sizeof(QTSS_Object));
    
    UInt32* theReadOffsetP = NULL;
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sReadOffsetAttr, 0, (void**)&theReadOffsetP, &theLen);
    Assert(theReadOffsetP != NULL);
    Assert(theLen == sizeof(UInt32));

    UInt32* theWriteOffsetP = NULL;
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sWriteOffsetAttr, 0, (void**)&theWriteOffsetP, &theLen);
    Assert(theWriteOffsetP != NULL);
    Assert(theLen == sizeof(UInt32));
    
    UInt32 theReadOffset = *theReadOffsetP;
    UInt32 theWriteOffset = *theWriteOffsetP;
    UInt32 theState = *theStateP;

    Bool16 isBlocked = false;
    
    // Get the length of the file onto the stack
    theLen = sizeof(UInt64);
    UInt64 theFileLength = 0;
    theErr = QTSS_GetValue(*theFileP, qtssFlObjLength, 0, (void*)&theFileLength, &theLen);
    Assert(theErr == QTSS_NoErr);
    Assert(theLen == sizeof(UInt64));

    // Get the offset in the file onto the stack
    theLen = sizeof(UInt64);
    UInt64 theOffset = 0;
    theErr = QTSS_GetValue(*theFileP, qtssFlObjPosition, 0, (void*)&theOffset, &theLen);
    Assert(theErr == QTSS_NoErr);
    Assert(theLen == sizeof(UInt64));

    while (!isBlocked)
    {
        // If we have less than the full buffer size left to go in the file,
        // down adjust our buffer size to be the amount of data remaining in the file
        UInt32 theBufferSize = sFileBufSize;
        if ((theFileLength - theOffset) < sFileBufSize)
            theBufferSize = (UInt32) (theFileLength - theOffset);

        switch (theState)
        {
            case kReadingBufferState:
            {
                // Read as much data as possible out of the file
                UInt32 theRecvLen = 0;
                (void)QTSS_Read(*theFileP,
                                (*theFileBufferP) + theReadOffset,
                                    theBufferSize - theReadOffset,
                                    &theRecvLen);
                theReadOffset += theRecvLen;
                theOffset += theRecvLen;
#if RAW_FILE_DEBUGGING
                qtss_printf("Got %"_U32BITARG_" bytes back from file read. Now at: %"_64BITARG_"u\n", theRecvLen, theOffset);
#endif
                if (theReadOffset < theBufferSize)
                {
#if RAW_FILE_DEBUGGING
                    qtss_printf("Flow controlled on file. Waiting for read event\n");
#endif
                    isBlocked = true;
                    break;
                }
                
                theReadOffset = 0;
                Assert(theWriteOffset == 0);
                theState = kWritingBufferState;
            }
            case kWritingBufferState:
            {
                UInt32 theWrittenLen = 0;
                
                // for debugging purposes, construct an IOVec out of this data
                iovec theVec[5];
                UInt32 units = (theBufferSize - theWriteOffset) /4;
                UInt32 offset = theWriteOffset;
                theVec[1].iov_base = (*theFileBufferP) + offset;
                theVec[1].iov_len = units;
                offset += units;
                theVec[2].iov_base = (*theFileBufferP) + offset;
                theVec[2].iov_len = units;
                offset += units;
                theVec[3].iov_base = (*theFileBufferP) + offset;
                theVec[3].iov_len = units;
                offset += units;
                theVec[4].iov_base = (*theFileBufferP) + offset;
                theVec[4].iov_len = theBufferSize - offset;
                
                (void)QTSS_WriteV(  inParams->inRTSPSession,
                                    theVec, 5,
                                    theBufferSize - theWriteOffset,
                                    &theWrittenLen);
                theWriteOffset += theWrittenLen;
#if RAW_FILE_DEBUGGING
                qtss_printf("Got %"_U32BITARG_" bytes back from socket write.\n", theWrittenLen);
#endif
                if (theWriteOffset < theBufferSize)
                {
#if RAW_FILE_DEBUGGING
                    qtss_printf("Flow controlled on socket. Waiting for write event.\n");
#endif
                    isBlocked = true;
                    break;
                }
                
                // Check to see if we're done. If we are, delete stuff and return
                if (theOffset == theFileLength)
                {
#if RAW_FILE_DEBUGGING
                    qtss_printf("File transfer complete\n");
#endif
                    return QTSS_NoErr;
                }
                
                theWriteOffset = 0;
                Assert(theReadOffset == 0);
                theState = kReadingBufferState;
            }
        }
    }
    
    Assert(isBlocked);
    
    // We've reached a blocking condition for some reason.
    // Save our state, request an event, and return.
    (void)QTSS_SetValue(inParams->inRTSPSession, sReadOffsetAttr, 0, &theReadOffset, sizeof(theReadOffset));
    (void)QTSS_SetValue(inParams->inRTSPSession, sWriteOffsetAttr, 0, &theWriteOffset, sizeof(theWriteOffset));
    (void)QTSS_SetValue(inParams->inRTSPSession, sStateAttr, 0, &theState, sizeof(theState));

    // If we're reading, wait for the file to become readable
    if (theState == kReadingBufferState)
        (void)QTSS_RequestEvent(*theFileP, QTSS_ReadableEvent);
    // If we're writing, wait for the socket to become writable
    else
        (void)QTSS_RequestEvent(inParams->inRTSPSession, QTSS_WriteableEvent);
    return QTSS_NoErr;
}

QTSS_Error CloseRTSPSession(QTSS_RTSPSession_Params* inParams)
{
    // In this role, the allocated resources are deleted before closing the RTSP session
    UInt32 theLen = 0;
    
    // Get our file buffer pointer and delete it
    char** theFileBufferP = NULL;                   // File buffer pointer
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sFileBufferAttr, 0, (void**)&theFileBufferP, &theLen);
    if (theFileBufferP != NULL) 
        delete [] *theFileBufferP;
    
    QTSS_Object* theFileP = NULL;
    // Get our file pointer and delete it
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sFileAttr, 0, (void**)&theFileP, &theLen);
    Assert(theFileP != NULL);
    Assert(theLen == sizeof(QTSS_Object));
    (void)QTSS_CloseFileObject(*theFileP);

    return QTSS_NoErr;
}
