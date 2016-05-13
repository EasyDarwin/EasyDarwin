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
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSModuleUtils.cpp

    Contains:   Implements utility routines defined in QTSSModuleUtils.h.
                    
*/

#include "QTSSModuleUtils.h"
#include "QTSS_Private.h"

#include "StrPtrLen.h"
#include "OSArrayObjectDeleter.h"
#include "OSMemory.h"
#include "MyAssert.h"
#include "StringFormatter.h"
#include "ResizeableStringFormatter.h"
#include "QTAccessFile.h"
#include "StringParser.h"
#include "SafeStdLib.h"
#ifndef __Win32__
#include <netinet/in.h>
#endif

#ifdef __solaris__
#include <limits.h>
#endif

QTSS_TextMessagesObject     QTSSModuleUtils::sMessages = NULL;
QTSS_ServerObject           QTSSModuleUtils::sServer = NULL;
QTSS_StreamRef              QTSSModuleUtils::sErrorLog = NULL;
Bool16                      QTSSModuleUtils::sEnableRTSPErrorMsg = false;
QTSS_ErrorVerbosity         QTSSModuleUtils::sMissingPrefVerbosity = qtssMessageVerbosity;

void    QTSSModuleUtils::Initialize(QTSS_TextMessagesObject inMessages,
                                    QTSS_ServerObject inServer,
                                    QTSS_StreamRef inErrorLog)
{
    sMessages = inMessages;
    sServer = inServer;
    sErrorLog = inErrorLog;
}

QTSS_Error QTSSModuleUtils::ReadEntireFile(char* inPath, StrPtrLen* outData, QTSS_TimeVal inModDate, QTSS_TimeVal* outModDate)
{   
    
    QTSS_Object theFileObject = NULL;
    QTSS_Error theErr = QTSS_NoErr;
    
    outData->Ptr = NULL;
    outData->Len = 0;
    
    do { 

        // Use the QTSS file system API to read the file
        theErr = QTSS_OpenFileObject(inPath, 0, &theFileObject);
        if (theErr != QTSS_NoErr)
            break;
    
        UInt32 theParamLen = 0;
        QTSS_TimeVal* theModDate = NULL;
        theErr = QTSS_GetValuePtr(theFileObject, qtssFlObjModDate, 0, (void**)(void*)&theModDate, &theParamLen);
        Assert(theParamLen == sizeof(QTSS_TimeVal));
        if(theParamLen != sizeof(QTSS_TimeVal))
            break;
        if(outModDate != NULL)
            *outModDate = (QTSS_TimeVal)*theModDate;

        if(inModDate != -1) {   
            // If file hasn't been modified since inModDate, don't have to read the file
            if(*theModDate <= inModDate)
                break;
        }
        
        theParamLen = 0;
        UInt64* theLength = NULL;
        theErr = QTSS_GetValuePtr(theFileObject, qtssFlObjLength, 0, (void**)(void*)&theLength, &theParamLen);
        if (theParamLen != sizeof(UInt64))
            break;
        
		if (*theLength > kSInt32_Max)
			break;

        // Allocate memory for the file data
        outData->Ptr = NEW char[ (SInt32) (*theLength + 1) ];
        outData->Len = (SInt32) *theLength;
        outData->Ptr[outData->Len] = 0;
    
        // Read the data
        UInt32 recvLen = 0;
        theErr = QTSS_Read(theFileObject, outData->Ptr, outData->Len, &recvLen);
        if (theErr != QTSS_NoErr)
        {
            outData->Delete();
            break;
        }   
        Assert(outData->Len == recvLen);
    
    }while(false);
    
    // Close the file
    if(theFileObject != NULL) {
        theErr = QTSS_CloseFileObject(theFileObject);
    }
    
    return theErr;
}

void    QTSSModuleUtils::SetupSupportedMethods(QTSS_Object inServer, QTSS_RTSPMethod* inMethodArray, UInt32 inNumMethods)
{
    // Report to the server that this module handles DESCRIBE, SETUP, PLAY, PAUSE, and TEARDOWN
    UInt32 theNumMethods = 0;
    (void)QTSS_GetNumValues(inServer, qtssSvrHandledMethods, &theNumMethods);
    
    for (UInt32 x = 0; x < inNumMethods; x++)
        (void)QTSS_SetValue(inServer, qtssSvrHandledMethods, theNumMethods++, (void*)&inMethodArray[x], sizeof(inMethodArray[x]));
}

void    QTSSModuleUtils::LogError(  QTSS_ErrorVerbosity inVerbosity,
                                    QTSS_AttributeID inTextMessage,
                                    UInt32 /*inErrNumber*/,
                                    char* inArgument,
                                    char* inArg2)
{
    static char* sEmptyArg = "";
    
    if (sMessages == NULL)
        return;
        
    // Retrieve the specified text message from the text messages dictionary.
    
    StrPtrLen theMessage;
    (void)QTSS_GetValuePtr(sMessages, inTextMessage, 0, (void**)(void*)&theMessage.Ptr, &theMessage.Len);
    if ((theMessage.Ptr == NULL) || (theMessage.Len == 0))
        (void)QTSS_GetValuePtr(sMessages, qtssMsgNoMessage, 0, (void**)(void*)&theMessage.Ptr, &theMessage.Len);

    if ((theMessage.Ptr == NULL) || (theMessage.Len == 0))
        return;
    
    // qtss_sprintf and ::strlen will crash if inArgument is NULL
    if (inArgument == NULL)
        inArgument = sEmptyArg;
    if (inArg2 == NULL)
        inArg2 = sEmptyArg;
    
    // Create a new string, and put the argument into the new string.
    
    UInt32 theMessageLen = theMessage.Len + ::strlen(inArgument) + ::strlen(inArg2);

    OSCharArrayDeleter theLogString(NEW char[theMessageLen + 1]);
    qtss_sprintf(theLogString.GetObject(), theMessage.Ptr, inArgument, inArg2);
    Assert(theMessageLen >= ::strlen(theLogString.GetObject()));
    
    (void)QTSS_Write(sErrorLog, theLogString.GetObject(), ::strlen(theLogString.GetObject()),
                        NULL, inVerbosity);
}

void QTSSModuleUtils::LogErrorStr( QTSS_ErrorVerbosity inVerbosity, char* inMessage) 
{  	
	if (inMessage == NULL) return;  
	(void)QTSS_Write(sErrorLog, inMessage, ::strlen(inMessage), NULL, inVerbosity);
}


void QTSSModuleUtils::LogPrefErrorStr( QTSS_ErrorVerbosity inVerbosity, char*  preference, char* inMessage)
{  	
	if (inMessage == NULL || preference == NULL) 
	{  Assert(0);
	   return;  
	}
	char buffer[1024];
	
	qtss_snprintf(buffer,sizeof(buffer), "Server preference %s %s",  preference, inMessage);
   
	(void)QTSS_Write(sErrorLog, buffer, ::strlen(buffer), NULL, inVerbosity);
}

                        
char* QTSSModuleUtils::GetFullPath( QTSS_RTSPRequestObject inRequest,
                                    QTSS_AttributeID whichFileType,
                                    UInt32* outLen,
                                    StrPtrLen* suffix)
{
    Assert(outLen != NULL);
    
	(void)QTSS_LockObject(inRequest);
    // Get the proper file path attribute. This may return an error if
    // the file type is qtssFilePathTrunc attr, because there may be no path
    // once its truncated. That's ok. In that case, we just won't append a path.
    StrPtrLen theFilePath;
    (void)QTSS_GetValuePtr(inRequest, whichFileType, 0, (void**)(void*)&theFilePath.Ptr, &theFilePath.Len);
		
    StrPtrLen theRootDir;
    QTSS_Error theErr = QTSS_GetValuePtr(inRequest, qtssRTSPReqRootDir, 0, (void**)(void*)&theRootDir.Ptr, &theRootDir.Len);
	Assert(theErr == QTSS_NoErr);


    //trim off extra / characters before concatenating
    // so root/ + /path instead of becoming root//path  is now root/path  as it should be.
    
	if (theRootDir.Len && theRootDir.Ptr[theRootDir.Len -1] == kPathDelimiterChar
	    && theFilePath.Len  && theFilePath.Ptr[0] == kPathDelimiterChar)
	{
	    char *thePathEnd = &(theFilePath.Ptr[theFilePath.Len]);
	    while (theFilePath.Ptr != thePathEnd)
	    {
	        if (*theFilePath.Ptr != kPathDelimiterChar)
	            break;
	            
	        theFilePath.Ptr ++;
	        theFilePath.Len --;
	    }
	}

    //construct a full path out of the root dir path for this request,
    //and the url path.
    *outLen = theFilePath.Len + theRootDir.Len + 2;
    if (suffix != NULL)
        *outLen += suffix->Len;
    
    char* theFullPath = NEW char[*outLen];
    
    //write all the pieces of the path into this new buffer.
    StringFormatter thePathFormatter(theFullPath, *outLen);
    thePathFormatter.Put(theRootDir);
    thePathFormatter.Put(theFilePath);
    if (suffix != NULL)
        thePathFormatter.Put(*suffix);
    thePathFormatter.PutTerminator();

    *outLen = *outLen - 2;
	
	(void)QTSS_UnlockObject(inRequest);
	
    return theFullPath;
}

QTSS_Error	QTSSModuleUtils::SendHTTPErrorResponse( QTSS_RTSPRequestObject inRequest,
													QTSS_SessionStatusCode inStatusCode,
                                                    Bool16 inKillSession,
                                                    char *errorMessage)
{
    static Bool16 sFalse = false;
    
    //set status code for access log
    (void)QTSS_SetValue(inRequest, qtssRTSPReqStatusCode, 0, &inStatusCode, sizeof(inStatusCode));

    if (inKillSession) // tell the server to end the session
        (void)QTSS_SetValue(inRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));
    
    ResizeableStringFormatter theErrorMessage(NULL, 0); //allocates and deletes memory
    ResizeableStringFormatter bodyMessage(NULL,0); //allocates and deletes memory

    char messageLineBuffer[64]; // used for each line
    static const int maxMessageBufferChars = sizeof(messageLineBuffer) -1;
    messageLineBuffer[maxMessageBufferChars] = 0; // guarantee termination

    // ToDo: put in a more meaningful http error message for each error. Not required by spec.
    // ToDo: maybe use the HTTP protcol class static error strings.
    char* errorMsg = "error"; 

    DateBuffer theDate;
    DateTranslator::UpdateDateBuffer(&theDate, 0); // get the current GMT date and time

    UInt32 realCode = 0;
    UInt32 len = sizeof(realCode);
    (void) QTSS_GetValue(inRequest, qtssRTSPReqRealStatusCode, 0,  (void*)&realCode,&len);

    char serverHeaderBuffer[64]; // the qtss Server: header field
    len = sizeof(serverHeaderBuffer) -1; // leave room for terminator
    (void) QTSS_GetValue(sServer, qtssSvrRTSPServerHeader, 0,  (void*)serverHeaderBuffer,&len);
    serverHeaderBuffer[len] = 0; // terminate.
 
    qtss_snprintf(messageLineBuffer,maxMessageBufferChars, "HTTP/1.1 %"_U32BITARG_" %s",realCode, errorMsg);
    theErrorMessage.Put(messageLineBuffer,::strlen(messageLineBuffer));
    theErrorMessage.PutEOL();

    theErrorMessage.Put(serverHeaderBuffer,::strlen(serverHeaderBuffer));
    theErrorMessage.PutEOL();
 
    qtss_snprintf(messageLineBuffer,maxMessageBufferChars, "Date: %s",theDate.GetDateBuffer());
    theErrorMessage.Put(messageLineBuffer,::strlen(messageLineBuffer));
    theErrorMessage.PutEOL();
 
    Bool16 addBody =  (errorMessage != NULL && ::strlen(errorMessage) != 0); // body error message so add body headers
    if (addBody) // body error message so add body headers
    {
        // first create the html body
        static const StrPtrLen htmlBodyStart("<html><body>\n");
        bodyMessage.Put(htmlBodyStart.Ptr,htmlBodyStart.Len);
 
        //<h1>errorMessage</h1>\n
        static const StrPtrLen hStart("<h1>");
        bodyMessage.Put(hStart.Ptr,hStart.Len);

        bodyMessage.Put(errorMessage,::strlen(errorMessage));

        static const StrPtrLen hTerm("</h1>\n");
        bodyMessage.Put(hTerm.Ptr,hTerm.Len);
 
        static const StrPtrLen htmlBodyTerm("</body></html>\n");
        bodyMessage.Put(htmlBodyTerm.Ptr,htmlBodyTerm.Len);

        // write body headers
        static const StrPtrLen bodyHeaderType("Content-Type: text/html");
        theErrorMessage.Put(bodyHeaderType.Ptr,bodyHeaderType.Len);
        theErrorMessage.PutEOL();

        qtss_snprintf(messageLineBuffer,maxMessageBufferChars, "Content-Length: %"_U32BITARG_"", bodyMessage.GetBytesWritten());
        theErrorMessage.Put(messageLineBuffer,::strlen(messageLineBuffer));        
        theErrorMessage.PutEOL();
    }

    static const StrPtrLen headerClose("Connection: close");
    theErrorMessage.Put(headerClose.Ptr,headerClose.Len);
    theErrorMessage.PutEOL();

    theErrorMessage.PutEOL();  // terminate headers with empty line

    if (addBody) // add html body
    {
        theErrorMessage.Put(bodyMessage.GetBufPtr(),bodyMessage.GetBytesWritten());
    }

    //
    // Now that we've formatted the message into the temporary buffer,
    // write it out to the request stream and the Client Session object
    (void)QTSS_Write(inRequest, theErrorMessage.GetBufPtr(), theErrorMessage.GetBytesWritten(), NULL, 0);
    (void)QTSS_SetValue(inRequest, qtssRTSPReqRespMsg, 0, theErrorMessage.GetBufPtr(), theErrorMessage.GetBytesWritten());
    
    return QTSS_RequestFailed;
}

QTSS_ModulePrefsObject QTSSModuleUtils::GetModulePrefsObject(QTSS_ModuleObject inModObject)
{
    QTSS_ModulePrefsObject thePrefsObject = NULL;
    UInt32 theLen = sizeof(thePrefsObject);
    QTSS_Error theErr = QTSS_GetValue(inModObject, qtssModPrefs, 0, &thePrefsObject, &theLen);
    Assert(theErr == QTSS_NoErr);
    
    return thePrefsObject;
}

QTSS_Object QTSSModuleUtils::GetModuleAttributesObject(QTSS_ModuleObject inModObject)
{
    QTSS_Object theAttributesObject = NULL;
    UInt32 theLen = sizeof(theAttributesObject);
    QTSS_Error theErr = QTSS_GetValue(inModObject, qtssModAttributes, 0, &theAttributesObject, &theLen);
    Assert(theErr == QTSS_NoErr);
    
    return theAttributesObject;
}

QTSS_ModulePrefsObject QTSSModuleUtils::GetModuleObjectByName(const StrPtrLen& inModuleName)
{
    QTSS_ModuleObject theModule = NULL;
    UInt32 theLen = sizeof(theModule);
    
    for (int x = 0; QTSS_GetValue(sServer, qtssSvrModuleObjects, x, &theModule, &theLen) == QTSS_NoErr; x++)
    {
        Assert(theModule != NULL);
        Assert(theLen == sizeof(theModule));
        
        StrPtrLen theName;
        QTSS_Error theErr = QTSS_GetValuePtr(theModule, qtssModName, 0, (void**)(void*)&theName.Ptr, &theName.Len);
        Assert(theErr == QTSS_NoErr);
        
        if (inModuleName.Equal(theName))
            return theModule;
            
#if DEBUG
        theModule = NULL;
        theLen = sizeof(theModule);
#endif
    }
    return NULL;
}

void    QTSSModuleUtils::GetAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, 
                                                void* ioBuffer, void* inDefaultValue, UInt32 inBufferLen)
{
    //
    // Check to make sure this attribute is the right type. If it's not, this will coerce
    // it to be the right type. This also returns the id of the attribute
    QTSS_AttributeID theID = QTSSModuleUtils::CheckAttributeDataType(inObject, inAttributeName, inType, inDefaultValue, inBufferLen);

    //
    // Get the attribute value.
    QTSS_Error theErr = QTSS_GetValue(inObject, theID, 0, ioBuffer, &inBufferLen);
    
    //
    // Caller should KNOW how big this attribute is
    Assert(theErr != QTSS_NotEnoughSpace);
    
    if (theErr != QTSS_NoErr)
    {
        //
        // If we couldn't get the attribute value for whatever reason, just use the
        // default if it was provided.
        ::memcpy(ioBuffer, inDefaultValue, inBufferLen);

        if (inBufferLen > 0)
        {
            //
            // Log an error for this pref only if there was a default value provided.
            char* theValueAsString = NULL;
            theErr = QTSS_ValueToString(inDefaultValue, inBufferLen, inType, &theValueAsString);
            Assert(theErr == QTSS_NoErr);
            OSCharArrayDeleter theValueStr(theValueAsString);
            QTSSModuleUtils::LogError(  sMissingPrefVerbosity, 
                                        qtssServerPrefMissing,
                                        0,
                                        inAttributeName,
                                        theValueStr.GetObject());
        }
        
        //
        // Create an entry for this attribute                           
        QTSSModuleUtils::CreateAttribute(inObject, inAttributeName, inType, inDefaultValue, inBufferLen);
    }
}

char*   QTSSModuleUtils::GetStringAttribute(QTSS_Object inObject, char* inAttributeName, char* inDefaultValue)
{
    UInt32 theDefaultValLen = 0;
    if (inDefaultValue != NULL)
        theDefaultValLen = ::strlen(inDefaultValue);
    
    //
    // Check to make sure this attribute is the right type. If it's not, this will coerce
    // it to be the right type
    QTSS_AttributeID theID = QTSSModuleUtils::CheckAttributeDataType(inObject, inAttributeName, qtssAttrDataTypeCharArray, inDefaultValue, theDefaultValLen);

    char* theString = NULL;
    (void)QTSS_GetValueAsString(inObject, theID, 0, &theString);
    if (theString != NULL)
        return theString;
    
    //
    // If we get here the attribute must be missing, so create it and log
    // an error.
    
    QTSSModuleUtils::CreateAttribute(inObject, inAttributeName, qtssAttrDataTypeCharArray, inDefaultValue, theDefaultValLen);
    
    //
    // Return the default if it was provided. Only log an error if the default value was provided
    if (theDefaultValLen > 0)
    {
        QTSSModuleUtils::LogError(  sMissingPrefVerbosity,
                                    qtssServerPrefMissing,
                                    0,
                                    inAttributeName,
                                    inDefaultValue);
    }
    
    if (inDefaultValue != NULL)
    {
        //
        // Whether to return the default value or not from this function is dependent
        // solely on whether the caller passed in a non-NULL pointer or not.
        // This ensures that if the caller wants an empty-string returned as a default
        // value, it can do that.
        theString = NEW char[theDefaultValLen + 1];
        ::strcpy(theString, inDefaultValue);
        return theString;
    }
    return NULL;
}

void    QTSSModuleUtils::GetIOAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType,
                            void* ioDefaultResultBuffer, UInt32 inBufferLen)
{
    char *defaultBuffPtr = NEW char[inBufferLen];
    ::memcpy(defaultBuffPtr,ioDefaultResultBuffer,inBufferLen);
    QTSSModuleUtils::GetAttribute(inObject, inAttributeName, inType, ioDefaultResultBuffer, defaultBuffPtr, inBufferLen);
    delete [] defaultBuffPtr;

}
                            

QTSS_AttributeID QTSSModuleUtils::GetAttrID(QTSS_Object inObject, char* inAttributeName)
{
    //
    // Get the attribute ID of this attribute.
    QTSS_Object theAttrInfo = NULL;
    QTSS_Error theErr = QTSS_GetAttrInfoByName(inObject, inAttributeName, &theAttrInfo);
    if (theErr != QTSS_NoErr)
        return qtssIllegalAttrID;

    QTSS_AttributeID theID = qtssIllegalAttrID; 
    UInt32 theLen = sizeof(theID);
    theErr = QTSS_GetValue(theAttrInfo, qtssAttrID, 0, &theID, &theLen);
    Assert(theErr == QTSS_NoErr);

    return theID;
}

QTSS_AttributeID QTSSModuleUtils::CheckAttributeDataType(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen)
{
    //
    // Get the attribute type of this attribute.
    QTSS_Object theAttrInfo = NULL;
    QTSS_Error theErr = QTSS_GetAttrInfoByName(inObject, inAttributeName, &theAttrInfo);
    if (theErr != QTSS_NoErr)
        return qtssIllegalAttrID;

    QTSS_AttrDataType theAttributeType = qtssAttrDataTypeUnknown;
    UInt32 theLen = sizeof(theAttributeType);
    theErr = QTSS_GetValue(theAttrInfo, qtssAttrDataType, 0, &theAttributeType, &theLen);
    Assert(theErr == QTSS_NoErr);
    
    QTSS_AttributeID theID = qtssIllegalAttrID; 
    theLen = sizeof(theID);
    theErr = QTSS_GetValue(theAttrInfo, qtssAttrID, 0, &theID, &theLen);
    Assert(theErr == QTSS_NoErr);

    if (theAttributeType != inType)
    {
        char* theValueAsString = NULL;
        theErr = QTSS_ValueToString(inDefaultValue, inBufferLen, inType, &theValueAsString);
        Assert(theErr == QTSS_NoErr);
        OSCharArrayDeleter theValueStr(theValueAsString);
        QTSSModuleUtils::LogError(  qtssWarningVerbosity,
                                    qtssServerPrefWrongType,
                                    0,
                                    inAttributeName,
                                    theValueStr.GetObject());
                                    
        theErr = QTSS_RemoveInstanceAttribute( inObject, theID );
        Assert(theErr == QTSS_NoErr);
        return  QTSSModuleUtils::CreateAttribute(inObject, inAttributeName, inType, inDefaultValue, inBufferLen);
    }
    return theID;
}

QTSS_AttributeID QTSSModuleUtils::CreateAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen)
{
    QTSS_Error theErr = QTSS_AddInstanceAttribute(inObject, inAttributeName, NULL, inType);
    Assert((theErr == QTSS_NoErr) || (theErr == QTSS_AttrNameExists));
    
    QTSS_AttributeID theID = QTSSModuleUtils::GetAttrID(inObject, inAttributeName);
    Assert(theID != qtssIllegalAttrID);
        
    //
    // Caller can pass in NULL for inDefaultValue, in which case we don't add the default
    if (inDefaultValue != NULL)
    {
        theErr = QTSS_SetValue(inObject, theID, 0, inDefaultValue, inBufferLen);
        Assert(theErr == QTSS_NoErr);
    }
    return theID;
}

QTSS_ActionFlags QTSSModuleUtils::GetRequestActions(QTSS_RTSPRequestObject theRTSPRequest)
{
    // Don't touch write requests
    QTSS_ActionFlags action = qtssActionFlagsNoFlags;
    UInt32 len = sizeof(QTSS_ActionFlags);
    QTSS_Error theErr = QTSS_GetValue(theRTSPRequest, qtssRTSPReqAction, 0, (void*)&action, &len);
    Assert(theErr == QTSS_NoErr);
    Assert(len == sizeof(QTSS_ActionFlags));
    return action;
}

char* QTSSModuleUtils::GetLocalPath_Copy(QTSS_RTSPRequestObject theRTSPRequest)
{   char*   pathBuffStr = NULL;
    QTSS_Error theErr = QTSS_GetValueAsString(theRTSPRequest, qtssRTSPReqLocalPath, 0, &pathBuffStr);
    Assert(theErr == QTSS_NoErr);
    return pathBuffStr;
}

char* QTSSModuleUtils::GetMoviesRootDir_Copy(QTSS_RTSPRequestObject theRTSPRequest)
{   char*   movieRootDirStr = NULL;
    QTSS_Error theErr = QTSS_GetValueAsString(theRTSPRequest,qtssRTSPReqRootDir, 0, &movieRootDirStr);
    Assert(theErr == QTSS_NoErr);
    return movieRootDirStr;
}

QTSS_UserProfileObject QTSSModuleUtils::GetUserProfileObject(QTSS_RTSPRequestObject theRTSPRequest)
{   QTSS_UserProfileObject theUserProfile = NULL;
    UInt32 len = sizeof(QTSS_UserProfileObject);
    QTSS_Error theErr = QTSS_GetValue(theRTSPRequest, qtssRTSPReqUserProfile, 0, (void*)&theUserProfile, &len);
    Assert(theErr == QTSS_NoErr);
    return theUserProfile;
}

char *QTSSModuleUtils::GetUserName_Copy(QTSS_UserProfileObject inUserProfile)
{
    char*   username = NULL;    
    (void) QTSS_GetValueAsString(inUserProfile, qtssUserName, 0, &username);
    return username;
}

char**  QTSSModuleUtils::GetGroupsArray_Copy(QTSS_UserProfileObject inUserProfile, UInt32 *outNumGroupsPtr)
{
    Assert(NULL != outNumGroupsPtr);

    char** outGroupCharPtrArray = NULL;
    *outNumGroupsPtr = 0;
    
    if (NULL == inUserProfile)
        return NULL;
    
    QTSS_Error theErr = QTSS_GetNumValues (inUserProfile,qtssUserGroups, outNumGroupsPtr);
    if (theErr != QTSS_NoErr || *outNumGroupsPtr == 0)
        return NULL;
        
    outGroupCharPtrArray = NEW char*[*outNumGroupsPtr]; // array of char *
    UInt32 len = 0;
    for (UInt32 index = 0; index < *outNumGroupsPtr; index++)
    {   outGroupCharPtrArray[index] = NULL;
        QTSS_GetValuePtr(inUserProfile, qtssUserGroups, index,(void **) &outGroupCharPtrArray[index], &len);
    }   

    return outGroupCharPtrArray;
}

Bool16 QTSSModuleUtils::UserInGroup(QTSS_UserProfileObject inUserProfile, char* inGroup, UInt32 inGroupLen)
{
	if (NULL == inUserProfile || NULL == inGroup  ||  inGroupLen == 0) 
		return false;
		
	char *userName = NULL;
	UInt32 len = 0;
	QTSS_GetValuePtr(inUserProfile, qtssUserName, 0, (void **)(void*)&userName, &len);
	if (len == 0 || userName == NULL || userName[0] == 0) // no user to check
		return false;

	UInt32 numGroups = 0;
	QTSS_GetNumValues (inUserProfile,qtssUserGroups, &numGroups);
	if (numGroups == 0) // no groups to check
		return false;

	Bool16 result = false;
	char* userGroup = NULL;
	StrPtrLenDel userGroupStr; //deletes pointer in destructor
	
	for (UInt32 index = 0; index < numGroups; index++)
	{  
		userGroup = NULL;
		QTSS_GetValueAsString(inUserProfile, qtssUserGroups, index, &userGroup); //allocates string
		userGroupStr.Delete();
		userGroupStr.Set(userGroup);					
		if(userGroupStr.Equal(inGroup))
		{	
			result = true;
			break;
		}
	}   

	return result;
	
}


Bool16 QTSSModuleUtils::AddressInList(QTSS_Object inObject, QTSS_AttributeID listID, StrPtrLen *inAddressPtr)
{
    StrPtrLenDel strDeleter;
    char*   theAttributeString = NULL;    
    IPComponentStr inAddress(inAddressPtr);
    IPComponentStr addressFromList;
    
    if (!inAddress.Valid())
        return false;

    UInt32 numValues = 0;
    (void) QTSS_GetNumValues(inObject, listID, &numValues);
    
    for (UInt32 index = 0; index < numValues; index ++)
    { 
        strDeleter.Delete();
        (void) QTSS_GetValueAsString(inObject, listID, index, &theAttributeString);
        strDeleter.Set(theAttributeString);
 
        addressFromList.Set(&strDeleter);
        if (addressFromList.Equal(&inAddress))
            return true;
    }

    return false;
}

Bool16 QTSSModuleUtils::FindStringInAttributeList(QTSS_Object inObject, QTSS_AttributeID listID, StrPtrLen *inStrPtr)
{
    StrPtrLenDel tempString;
     
    if (NULL == inStrPtr || NULL == inStrPtr->Ptr || 0 == inStrPtr->Len)
        return false;

    UInt32 numValues = 0;
    (void) QTSS_GetNumValues(inObject, listID, &numValues);
    
    for (UInt32 index = 0; index < numValues; index ++)
    { 
        tempString.Delete();
        (void) QTSS_GetValueAsString(inObject, listID, index, &tempString.Ptr);
        tempString.Set(tempString.Ptr);
		if (tempString.Ptr == NULL)
			return false;
			
        if (tempString.Equal(StrPtrLen("*",1)))
            return true;
        
        if (inStrPtr->FindString(tempString.Ptr))
            return true;
            
   }

    return false;
}

IPComponentStr IPComponentStr::sLocalIPCompStr("127.0.0.*");

IPComponentStr::IPComponentStr(char *theAddressPtr)
{
    StrPtrLen sourceStr(theAddressPtr);
     (void) this->Set(&sourceStr);    
}

IPComponentStr::IPComponentStr(StrPtrLen *sourceStrPtr)
{
    (void) this->Set(sourceStrPtr);    
}

Bool16 IPComponentStr::Set(StrPtrLen *theAddressStrPtr)
{
    fIsValid = false;
   
    StringParser IP_Paser(theAddressStrPtr);
    StrPtrLen *piecePtr = &fAddressComponent[0];

    while (IP_Paser.GetDataRemaining() > 0) 
    {
        IP_Paser.ConsumeUntil(piecePtr,'.');    
        if (piecePtr->Len == 0) 
            break;
        
        IP_Paser.ConsumeLength(NULL, 1);
        if (piecePtr == &fAddressComponent[IPComponentStr::kNumComponents -1])
        {
           fIsValid = true;
           break;
        }
        
        piecePtr++;
    };
     
    return fIsValid;
}


Bool16 IPComponentStr::Equal(IPComponentStr *testAddressPtr)
{
    if (testAddressPtr == NULL) 
        return false;
    
    if ( !this->Valid() || !testAddressPtr->Valid() )
        return false;

    for (UInt16 component= 0 ; component < IPComponentStr::kNumComponents ; component ++)
    {
        StrPtrLen *allowedPtr = this->GetComponent(component);
        StrPtrLen *testPtr = testAddressPtr->GetComponent(component);
                       
        if ( testPtr->Equal("*") || allowedPtr->Equal("*") )
            continue;
            
        if (!testPtr->Equal(*allowedPtr) ) 
            return false; 
    };  
    
    return true;
}


