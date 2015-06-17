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
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSS_Private.c

    Contains:   Code for stub library and stub callback functions.    
*/

#include <stdlib.h>
#include "SafeStdLib.h"
#ifndef __Win32__
#include <sys/types.h>
#include <sys/uio.h>
#endif

#include "QTSS.h"
#include "QTSS_Private.h"

static QTSS_CallbacksPtr    sCallbacks = NULL;
static QTSS_StreamRef       sErrorLogStream = NULL;

QTSS_Error _stublibrary_main(void* inPrivateArgs, QTSS_DispatchFuncPtr inDispatchFunc)
{
    QTSS_PrivateArgsPtr theArgs = (QTSS_PrivateArgsPtr)inPrivateArgs;
    
    // Setup

    sCallbacks = theArgs->inCallbacks;
    sErrorLogStream = theArgs->inErrorLogStream;
    
    // Send requested information back to the server
    
    theArgs->outStubLibraryVersion = QTSS_API_VERSION;
    theArgs->outDispatchFunction = inDispatchFunc;
    
    return QTSS_NoErr;
}

// STUB FUNCTION DEFINITIONS

void*           QTSS_New(FourCharCode inMemoryIdentifier, UInt32 inSize)
{
    return (void *) ((QTSS_CallbackPtrProcPtr) sCallbacks->addr [kNewCallback]) (inMemoryIdentifier, inSize);
}

void            QTSS_Delete(void* inMemory)
{
    (sCallbacks->addr [kDeleteCallback]) (inMemory);
}

SInt64          QTSS_Milliseconds(void)
{
    SInt64 outMilliseconds = 0;
    (sCallbacks->addr [kMillisecondsCallback]) (&outMilliseconds);
    return outMilliseconds;
}

time_t          QTSS_MilliSecsTo1970Secs(SInt64 inQTSS_MilliSeconds)
{
    time_t outSeconds = 0;
    (sCallbacks->addr [kConvertToUnixTimeCallback]) (&inQTSS_MilliSeconds, &outSeconds);
    return outSeconds;
}

// STARTUP ROUTINES
    
QTSS_Error  QTSS_AddRole(QTSS_Role inRole)
{
    return (sCallbacks->addr [kAddRoleCallback]) (inRole);  
}

// DICTIONARY ROUTINES

QTSS_Error  QTSS_CreateObjectType(QTSS_ObjectType* outType)
{
    return (sCallbacks->addr [kCreateObjectTypeCallback]) (outType);    
}

QTSS_Error  QTSS_AddAttribute(QTSS_ObjectType inType, const char* inTag, void* inUnused)
{
    return (sCallbacks->addr [kAddAttributeCallback]) (inType, inTag, inUnused);    
}

QTSS_Error  QTSS_AddStaticAttribute(QTSS_ObjectType inObjectType, char* inAttrName, void* inUnused, QTSS_AttrDataType inAttrDataType)
{
    return (sCallbacks->addr [kAddStaticAttributeCallback]) (inObjectType, inAttrName, inUnused, inAttrDataType);   
}

QTSS_Error  QTSS_AddInstanceAttribute(QTSS_Object inObject, char* inAttrName, void* inUnused, QTSS_AttrDataType inAttrDataType)
{
    return (sCallbacks->addr [kAddInstanceAttributeCallback]) (inObject, inAttrName, inUnused, inAttrDataType); 
}

QTSS_Error QTSS_RemoveInstanceAttribute(QTSS_Object inObject, QTSS_AttributeID inID)
{
    return (sCallbacks->addr [kRemoveInstanceAttributeCallback]) (inObject, inID);  
}

QTSS_Error  QTSS_IDForAttr(QTSS_ObjectType inType, const char* inTag, QTSS_AttributeID* outID)
{
    return (sCallbacks->addr [kIDForTagCallback]) (inType, inTag, outID);   
}

QTSS_Error QTSS_GetAttrInfoByIndex(QTSS_Object inObject, UInt32 inIndex, QTSS_Object* outAttrInfoObject)
{
    return (sCallbacks->addr [kGetAttrInfoByIndexCallback]) (inObject, inIndex, outAttrInfoObject); 
}

QTSS_Error QTSS_GetAttrInfoByID(QTSS_Object inObject, QTSS_AttributeID inAttrID, QTSS_Object* outAttrInfoObject)
{
    return (sCallbacks->addr [kGetAttrInfoByIDCallback]) (inObject, inAttrID, outAttrInfoObject);   
}

QTSS_Error QTSS_GetAttrInfoByName(QTSS_Object inObject, char* inAttrName, QTSS_Object* outAttrInfoObject)
{
    return (sCallbacks->addr [kGetAttrInfoByNameCallback]) (inObject, inAttrName, outAttrInfoObject);   
}

QTSS_Error  QTSS_GetValuePtr (QTSS_Object inDictionary, QTSS_AttributeID inID, UInt32 inIndex, void** outBuffer, UInt32* outLen)
{
    return (sCallbacks->addr [kGetAttributePtrByIDCallback]) (inDictionary, inID, inIndex, outBuffer, outLen);  
}

QTSS_Error  QTSS_GetValue (QTSS_Object inDictionary, QTSS_AttributeID inID, UInt32 inIndex, void* ioBuffer, UInt32* ioLen)
{
    return (sCallbacks->addr [kGetAttributeByIDCallback]) (inDictionary, inID, inIndex, ioBuffer, ioLen);   
}

QTSS_Error QTSS_GetValueAsString (QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex, char** outString)
{
    return (sCallbacks->addr [kGetValueAsStringCallback]) (inObject, inID, inIndex, outString); 
}

QTSS_Error  QTSS_TypeStringToType(const char* inTypeString, QTSS_AttrDataType* outType)
{
    return (sCallbacks->addr [kTypeStringToTypeCallback]) (inTypeString, outType);  
}

QTSS_Error  QTSS_TypeToTypeString(const QTSS_AttrDataType inType, char** outTypeString)
{
    return (sCallbacks->addr [kTypeToTypeStringCallback]) (inType, outTypeString);  
}

QTSS_Error  QTSS_StringToValue(const char* inValueAsString, const QTSS_AttrDataType inType, void* ioBuffer, UInt32* ioBufSize)
{
    return (sCallbacks->addr [kStringToValueCallback]) (inValueAsString, inType, ioBuffer, ioBufSize);  
}

QTSS_Error  QTSS_ValueToString(const void* inValue, const UInt32 inValueLen, const QTSS_AttrDataType inType, char** outString)
{
    return (sCallbacks->addr [kValueToStringCallback]) (inValue, inValueLen, inType, outString);    
}

QTSS_Error  QTSS_SetValue (QTSS_Object inDictionary, QTSS_AttributeID inID,UInt32 inIndex,  const void* inBuffer,  UInt32 inLen)
{
    return (sCallbacks->addr [kSetAttributeByIDCallback]) (inDictionary, inID, inIndex, inBuffer, inLen);   
}

QTSS_Error  QTSS_SetValuePtr (QTSS_Object inDictionary, QTSS_AttributeID inID, const void* inBuffer,  UInt32 inLen)
{
    return (sCallbacks->addr [kSetAttributePtrCallback]) (inDictionary, inID, inBuffer, inLen); 
}

QTSS_Error  QTSS_CreateObjectValue (QTSS_Object inDictionary, QTSS_AttributeID inID, QTSS_ObjectType inType, UInt32* outIndex, QTSS_Object* outCreatedObject)
{
    return (sCallbacks->addr [kCreateObjectValueCallback]) (inDictionary, inID, inType, outIndex, outCreatedObject);    
}

QTSS_Error  QTSS_GetNumValues (QTSS_Object inObject, QTSS_AttributeID inID, UInt32* outNumValues)
{
    return (sCallbacks->addr [kGetNumValuesCallback]) (inObject, inID, outNumValues);   
}

QTSS_Error  QTSS_GetNumAttributes (QTSS_Object inObject, UInt32* outNumValues)
{
    return (sCallbacks->addr [kGetNumAttributesCallback]) (inObject, outNumValues); 
}

QTSS_Error  QTSS_RemoveValue (QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex)
{
    return (sCallbacks->addr [kRemoveValueCallback]) (inObject, inID, inIndex); 
}

// STREAM ROUTINES

QTSS_Error  QTSS_Write(QTSS_StreamRef inStream, const void* inBuffer, UInt32 inLen, UInt32* outLenWritten, UInt32 inFlags)
{
    return (sCallbacks->addr [kWriteCallback]) (inStream, inBuffer, inLen, outLenWritten, inFlags); 
}

QTSS_Error  QTSS_WriteV(QTSS_StreamRef inStream, iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength, UInt32* outLenWritten)
{
    return (sCallbacks->addr [kWriteVCallback]) (inStream, inVec, inNumVectors, inTotalLength, outLenWritten);  
}

QTSS_Error  QTSS_Flush(QTSS_StreamRef inStream)
{
    return (sCallbacks->addr [kFlushCallback]) (inStream);  
}

QTSS_Error  QTSS_Read(QTSS_StreamRef inRef, void* ioBuffer, UInt32 inBufLen, UInt32* outLengthRead)
{
    return (sCallbacks->addr [kReadCallback]) (inRef, ioBuffer, inBufLen, outLengthRead);       
}

QTSS_Error  QTSS_Seek(QTSS_StreamRef inRef, UInt64 inNewPosition)
{
    return (sCallbacks->addr [kSeekCallback]) (inRef, inNewPosition);
}

QTSS_Error  QTSS_Advise(QTSS_StreamRef inRef, UInt64 inPosition, UInt32 inAdviseSize)
{
    return (sCallbacks->addr [kAdviseCallback]) (inRef, inPosition, inAdviseSize);      
}



// SERVICE ROUTINES

QTSS_Error QTSS_AddService(const char* inServiceName, QTSS_ServiceFunctionPtr inFunctionPtr)
{
    return (sCallbacks->addr [kAddServiceCallback]) (inServiceName, inFunctionPtr);     
}

QTSS_Error QTSS_IDForService(const char* inTag, QTSS_ServiceID* outID)
{
    return (sCallbacks->addr [kIDForServiceCallback]) (inTag, outID);   
}

QTSS_Error QTSS_DoService(QTSS_ServiceID inID, QTSS_ServiceFunctionArgsPtr inArgs)
{
    return (sCallbacks->addr [kDoServiceCallback]) (inID, inArgs);  
}

// RTSP ROUTINES

QTSS_Error QTSS_SendRTSPHeaders(QTSS_RTSPRequestObject inRef)
{
    return (sCallbacks->addr [kSendRTSPHeadersCallback]) (inRef);       
}

QTSS_Error QTSS_AppendRTSPHeader(QTSS_RTSPRequestObject inRef, QTSS_RTSPHeader inHeader, const char* inValue, UInt32 inValueLen)
{
    return (sCallbacks->addr [kAppendRTSPHeadersCallback]) (inRef, inHeader, inValue, inValueLen);      
}

QTSS_Error QTSS_SendStandardRTSPResponse(QTSS_RTSPRequestObject inRTSPRequest, QTSS_Object inRTPInfo, UInt32 inFlags)
{
    return (sCallbacks->addr [kSendStandardRTSPCallback]) (inRTSPRequest, inRTPInfo, inFlags);      
}

// RTP ROUTINES

QTSS_Error QTSS_AddRTPStream(QTSS_ClientSessionObject inClientSession, QTSS_RTSPRequestObject inRTSPRequest, QTSS_RTPStreamObject* outStream, QTSS_AddStreamFlags inFlags)
{
    return (sCallbacks->addr [kAddRTPStreamCallback]) (inClientSession, inRTSPRequest, outStream, inFlags);     
}

QTSS_Error QTSS_Play(QTSS_ClientSessionObject inClientSession, QTSS_RTSPRequestObject inRTSPRequest, QTSS_PlayFlags inPlayFlags)
{
    return (sCallbacks->addr [kPlayCallback]) (inClientSession, inRTSPRequest, inPlayFlags);        
}

QTSS_Error QTSS_Pause(QTSS_ClientSessionObject inClientSession)
{
    return (sCallbacks->addr [kPauseCallback]) (inClientSession);       
}

QTSS_Error QTSS_Teardown(QTSS_ClientSessionObject inClientSession)
{
    return (sCallbacks->addr [kTeardownCallback]) (inClientSession);        
}

QTSS_Error QTSS_RefreshTimeOut(QTSS_ClientSessionObject inClientSession)
{
    return (sCallbacks->addr [kRefreshTimeOutCallback]) (inClientSession);
}


// FILE SYSTEM ROUTINES

QTSS_Error  QTSS_OpenFileObject(char* inPath, QTSS_OpenFileFlags inFlags, QTSS_Object* outFileObject)
{
    return (sCallbacks->addr [kOpenFileObjectCallback]) (inPath, inFlags, outFileObject);       
}

QTSS_Error  QTSS_CloseFileObject(QTSS_Object inFileObject)
{
    return (sCallbacks->addr [kCloseFileObjectCallback]) (inFileObject);        
}

// SOCKET ROUTINES

QTSS_Error  QTSS_CreateStreamFromSocket(int inFileDesc, QTSS_StreamRef* outStream)
{
    return (sCallbacks->addr [kCreateSocketStreamCallback]) (inFileDesc, outStream);        
}

QTSS_Error  QTSS_DestroySocketStream(QTSS_StreamRef inStream)
{
    return (sCallbacks->addr [kDestroySocketStreamCallback]) (inStream);        

}

// ASYNC I/O STREAM ROUTINES

QTSS_Error  QTSS_RequestEvent(QTSS_StreamRef inStream, QTSS_EventType inEventMask)
{
    return (sCallbacks->addr [kRequestEventCallback]) (inStream, inEventMask);      
}

QTSS_Error  QTSS_SignalStream(QTSS_StreamRef inStream)
{
    return (sCallbacks->addr [kSignalStreamCallback]) (inStream);       
}

QTSS_Error  QTSS_SetIdleTimer(SInt64 inIdleMsec)
{
    return (sCallbacks->addr [kSetIdleTimerCallback]) (inIdleMsec);     
}

QTSS_Error  QTSS_SetIntervalRoleTimer(SInt64 inIdleMsec)
{
    return (sCallbacks->addr [kSetIntervalRoleTimerCallback]) (inIdleMsec);     
}

QTSS_Error  QTSS_RequestGlobalLock()
{
    return (sCallbacks->addr [kRequestGlobalLockCallback])  ();
}

// SYNCH GLOBAL MULTIPLE READERS/SINGLE WRITER ROUTINES

Bool16  QTSS_IsGlobalLocked()
{
    return (Bool16) (sCallbacks->addr [kIsGlobalLockedCallback])  ();
}

QTSS_Error  QTSS_GlobalUnLock()
{
    return (sCallbacks->addr [kUnlockGlobalLock])  ();
}

QTSS_Error  QTSS_LockObject(QTSS_Object inObject)
{
    return (sCallbacks->addr [kLockObjectCallback])  (inObject);
}

QTSS_Error  QTSS_UnlockObject(QTSS_Object inObject)
{
    return (sCallbacks->addr [kUnlockObjectCallback])  (inObject);
}

// AUTHENTICATION AND AUTHORIZATION ROUTINE
QTSS_Error  QTSS_Authenticate(  const char* inAuthUserName, 
                                const char* inAuthResourceLocalPath, 
                                const char* inAuthMoviesDir, 
                                QTSS_ActionFlags inAuthRequestAction, 
                                QTSS_AuthScheme inAuthScheme, 
                                QTSS_RTSPRequestObject ioAuthRequestObject)
{
    return (sCallbacks->addr [kAuthenticateCallback]) (inAuthUserName, inAuthResourceLocalPath, inAuthMoviesDir, inAuthRequestAction, inAuthScheme, ioAuthRequestObject);
}

QTSS_Error	QTSS_Authorize(QTSS_RTSPRequestObject inAuthRequestObject, char** outAuthRealm, Bool16* outAuthUserAllowed)
{
    return (sCallbacks->addr [kAuthorizeCallback]) (inAuthRequestObject, outAuthRealm, outAuthUserAllowed);
}

void  QTSS_LockStdLib()
{
   (sCallbacks->addr [kLockStdLibCallback])  ();
}

void  QTSS_UnlockStdLib()
{
    (sCallbacks->addr [kUnlockStdLibCallback])  ();
}

//DispatchCenter消息中心调用
QTSS_Error	QTSS_SendHTTPPacket(QTSS_RTSPSessionObject inServiceSession, char* inValue, UInt32 inValueLen, Bool16 connectionClose, Bool16 decrement)
{
	return (sCallbacks->addr[kSendHTTPPacketCallback]) (inServiceSession, inValue, inValueLen, connectionClose, decrement);
}

QTSS_Error	QTSS_RegDevSession(QTSS_RTSPSessionObject inServiceSession, char* inValue, UInt32 inValueLen)
{
	return (sCallbacks->addr[kRegDevSessionCallback]) (inServiceSession, inValue, inValueLen);
}

QTSS_Error	QTSS_UpdateDevRedis(QTSS_RTSPSessionObject inServiceSession)
{
	return (sCallbacks->addr[kUpdateDevRedisCallback]) (inServiceSession);
}

QTSS_Error	QTSS_UpdateDevSnap(QTSS_RTSPSessionObject inServiceSession, const char* inSnapTime, const char* inSnapJpg)
{
	return (sCallbacks->addr[kUpdateDevSnapCallback]) (inServiceSession, inSnapTime, inSnapJpg);
}
    
