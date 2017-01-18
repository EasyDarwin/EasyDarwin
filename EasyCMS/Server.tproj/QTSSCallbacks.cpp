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
     Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
     Github: https://github.com/EasyDarwin
     WEChat: EasyDarwin
     Website: http://www.EasyDarwin.org
 */
 /*
     File:       QTSSCallbacks.cpp

     Contains:   Implements QTSS Callback functions.


 */

#include "QTSSCallbacks.h"
#include "QTSSDictionary.h"
#include "QTSSStream.h"
#include "HTTPSession.h"
#include "OS.h"
#include "QTSSFile.h"
#include "QTSSSocket.h"
#include "QTSServerInterface.h"
#include "QTSSDataConverter.h"
#include "QTSSModule.h"

#include <errno.h>

#define __QTSSCALLBACKS_DEBUG__ 0
#define debug_printf if (__QTSSCALLBACKS_DEBUG__) qtss_printf

void* QTSSCallbacks::QTSS_New(FourCharCode /*inMemoryIdentifier*/, UInt32 inSize)
{
    //
    // This callback is now deprecated because the server no longer uses FourCharCodes
    // for memory debugging. For clients that still use it, the default, non-debugging
    // version of New is used.

    //return OSMemory::New(inSize, inMemoryIdentifier, false);

	auto temp = new int[inSize];

    return temp;
}

void QTSSCallbacks::QTSS_Delete(void* inMemory)
{
	delete[] inMemory;
}

void QTSSCallbacks::QTSS_Milliseconds(SInt64* outMilliseconds)
{
    if (outMilliseconds != nullptr)
        *outMilliseconds = OS::Milliseconds();
}

void QTSSCallbacks::QTSS_ConvertToUnixTime(SInt64 *inQTSS_MilliSecondsPtr, time_t* outSecondsPtr)
{
    if ((nullptr != outSecondsPtr) && (nullptr != inQTSS_MilliSecondsPtr))
        *outSecondsPtr = OS::TimeMilli_To_UnixTimeSecs(*inQTSS_MilliSecondsPtr);
}

QTSS_Error QTSSCallbacks::QTSS_AddRole(QTSS_Role inRole)
{
    QTSS_ModuleState* theState = static_cast<QTSS_ModuleState*>(OSThread::GetMainThreadData());
    if (OSThread::GetCurrent() != nullptr)
        theState = static_cast<QTSS_ModuleState*>(OSThread::GetCurrent()->GetThreadData());

    // Roles can only be added before modules have had their Initialize role invoked.
    if ((theState == nullptr) || (theState->curRole != QTSS_Register_Role))
        return QTSS_OutOfState;

    return theState->curModule->AddRole(inRole);
}

QTSS_Error QTSSCallbacks::QTSS_LockObject(QTSS_Object inDictionary)
{
    if (inDictionary == nullptr)
        return QTSS_BadArgument;

    static_cast<QTSSDictionary*>(inDictionary)->GetMutex()->Lock();
    static_cast<QTSSDictionary*>(inDictionary)->SetLocked(true);
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_UnlockObject(QTSS_Object inDictionary)
{
    if (inDictionary == nullptr)
        return QTSS_BadArgument;

    static_cast<QTSSDictionary*>(inDictionary)->SetLocked(false);
    static_cast<QTSSDictionary*>(inDictionary)->GetMutex()->Unlock();

    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_CreateObjectType(QTSS_ObjectType* outType)
{
    QTSS_ObjectType type = QTSSDictionaryMap::CreateNewMap();
    if (type == 0)
        return QTSS_RequestFailed;

    *outType = type;
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_AddAttribute(QTSS_ObjectType inType, const char* inName, void* inUnused)
{
    //
    // This call is deprecated, make the new call with sensible default arguments
    return QTSSCallbacks::QTSS_AddStaticAttribute(inType, inName, inUnused, qtssAttrDataTypeUnknown);
}

QTSS_Error QTSSCallbacks::QTSS_AddStaticAttribute(QTSS_ObjectType inObjectType, const char* inAttrName, void* inUnused, QTSS_AttrDataType inAttrDataType)
{
    Assert(inUnused == nullptr);
    QTSS_ModuleState* theState = static_cast<QTSS_ModuleState*>(OSThread::GetMainThreadData());
    if (OSThread::GetCurrent() != nullptr)
        theState = static_cast<QTSS_ModuleState*>(OSThread::GetCurrent()->GetThreadData());

    // Static attributes can only be added before modules have had their Initialize role invoked.
    if ((theState == nullptr) || (theState->curRole != QTSS_Register_Role))
        return QTSS_OutOfState;

    UInt32 theDictionaryIndex = QTSSDictionaryMap::GetMapIndex(inObjectType);
    if (theDictionaryIndex == QTSSDictionaryMap::kIllegalDictionary)
        return QTSS_BadArgument;

    QTSSDictionaryMap* theMap = QTSSDictionaryMap::GetMap(theDictionaryIndex);
    return theMap->AddAttribute(inAttrName, nullptr, inAttrDataType, qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe);
}

QTSS_Error QTSSCallbacks::QTSS_AddInstanceAttribute(QTSS_Object inObject, const char* inAttrName, void* inUnused, QTSS_AttrDataType inAttrDataType)
{
    Assert(inUnused == nullptr);
    if ((inObject == nullptr) || (inAttrName == nullptr))
        return QTSS_BadArgument;

    return static_cast<QTSSDictionary*>(inObject)->AddInstanceAttribute(inAttrName, nullptr, inAttrDataType, qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModeDelete | qtssAttrModePreempSafe);
}

QTSS_Error QTSSCallbacks::QTSS_RemoveInstanceAttribute(QTSS_Object inObject, QTSS_AttributeID inID)
{
    if (inObject == nullptr || (inID == qtssIllegalAttrID))
        return QTSS_BadArgument;

    return static_cast<QTSSDictionary*>(inObject)->RemoveInstanceAttribute(inID);
}

QTSS_Error QTSSCallbacks::QTSS_IDForAttr(QTSS_ObjectType inType, const char* inName, QTSS_AttributeID* outID)
{
    if (outID == nullptr)
        return QTSS_BadArgument;

    UInt32 theDictionaryIndex = QTSSDictionaryMap::GetMapIndex(inType);
    if (theDictionaryIndex == QTSSDictionaryMap::kIllegalDictionary)
        return QTSS_BadArgument;

    return QTSSDictionaryMap::GetMap(theDictionaryIndex)->GetAttrID(inName, outID);
}

QTSS_Error QTSSCallbacks::QTSS_GetAttrInfoByIndex(QTSS_Object inObject, UInt32 inIndex, QTSS_Object* outAttrInfoObject)
{
    if (inObject == nullptr)
        return QTSS_BadArgument;

    return static_cast<QTSSDictionary*>(inObject)->GetAttrInfoByIndex(inIndex, reinterpret_cast<QTSSAttrInfoDict**>(outAttrInfoObject));
}

QTSS_Error QTSSCallbacks::QTSS_GetAttrInfoByID(QTSS_Object inObject, QTSS_AttributeID inAttrID, QTSS_Object* outAttrInfoObject)
{
    if (inObject == nullptr || (inAttrID == qtssIllegalAttrID))
        return QTSS_BadArgument;

    return static_cast<QTSSDictionary*>(inObject)->GetAttrInfoByID(inAttrID, reinterpret_cast<QTSSAttrInfoDict**>(outAttrInfoObject));
}

QTSS_Error QTSSCallbacks::QTSS_GetAttrInfoByName(QTSS_Object inObject, const char* inAttrName, QTSS_Object* outAttrInfoObject)
{
    if (inObject == nullptr)
        return QTSS_BadArgument;

    return static_cast<QTSSDictionary*>(inObject)->GetAttrInfoByName(inAttrName, reinterpret_cast<QTSSAttrInfoDict**>(outAttrInfoObject));
}


QTSS_Error QTSSCallbacks::QTSS_GetValuePtr(QTSS_Object inDictionary, QTSS_AttributeID inID, UInt32 inIndex, void** outBuffer, UInt32* outLen)
{
    if ((inDictionary == nullptr) || (outBuffer == nullptr) || (outLen == nullptr) || (inID == qtssIllegalAttrID))
        return QTSS_BadArgument;
    return static_cast<QTSSDictionary*>(inDictionary)->GetValuePtr(inID, inIndex, outBuffer, outLen);
}


QTSS_Error QTSSCallbacks::QTSS_GetValue(QTSS_Object inDictionary, QTSS_AttributeID inID, UInt32 inIndex, void* ioBuffer, UInt32* ioLen)
{
    if (inDictionary == nullptr || (inID == qtssIllegalAttrID))
        return QTSS_BadArgument;
    return static_cast<QTSSDictionary*>(inDictionary)->GetValue(inID, inIndex, ioBuffer, ioLen);
}

QTSS_Error QTSSCallbacks::QTSS_GetValueAsString(QTSS_Object inDictionary, QTSS_AttributeID inID, UInt32 inIndex, char** outString)
{
    if (inDictionary == nullptr)
        return QTSS_BadArgument;
    return static_cast<QTSSDictionary*>(inDictionary)->GetValueAsString(inID, inIndex, outString);
}

QTSS_Error QTSSCallbacks::QTSS_TypeToTypeString(const QTSS_AttrDataType inType, char** outTypeString)
{
    if (outTypeString == nullptr)
        return QTSS_BadArgument;

    *outTypeString = QTSSDataConverter::TypeToTypeString(inType);
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_TypeStringToType(char* inTypeString, QTSS_AttrDataType* outType)
{
    if ((inTypeString == nullptr) || (outType == nullptr))
        return QTSS_BadArgument;

    *outType = QTSSDataConverter::TypeStringToType(inTypeString);
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_StringToValue(char* inValueAsString, const QTSS_AttrDataType inType, void* ioBuffer, UInt32* ioBufSize)
{
    return QTSSDataConverter::StringToValue(inValueAsString, inType, ioBuffer, ioBufSize);
}

QTSS_Error QTSSCallbacks::QTSS_ValueToString(void* inValue, const UInt32 inValueLen, const QTSS_AttrDataType inType, char** outString)
{
    if ((inValue == nullptr) || (outString == nullptr))
        return QTSS_BadArgument;

    *outString = QTSSDataConverter::ValueToString(inValue, inValueLen, inType);
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_SetValue(QTSS_Object inDictionary, QTSS_AttributeID inID, UInt32 inIndex, const void* inBuffer, UInt32 inLen)
{
    if ((inDictionary == nullptr) || ((inBuffer == nullptr) && (inLen > 0)) || (inID == qtssIllegalAttrID))
        return QTSS_BadArgument;
    return static_cast<QTSSDictionary*>(inDictionary)->SetValue(inID, inIndex, inBuffer, inLen);
}

QTSS_Error QTSSCallbacks::QTSS_SetValuePtr(QTSS_Object inDictionary, QTSS_AttributeID inID, const void* inBuffer, UInt32 inLen)
{
    if ((inDictionary == nullptr) || ((inBuffer == nullptr) && (inLen > 0)))
        return QTSS_BadArgument;
    return static_cast<QTSSDictionary*>(inDictionary)->SetValuePtr(inID, inBuffer, inLen);
}

QTSS_Error QTSSCallbacks::QTSS_CreateObject(QTSS_Object inDictionary, QTSS_AttributeID inID, QTSS_ObjectType inType, UInt32* outIndex, QTSS_Object* outCreatedObject)
{
    if ((inDictionary == nullptr) || (outCreatedObject == nullptr) || (outIndex == nullptr) || (inID == qtssIllegalAttrID))
        return QTSS_BadArgument;

    QTSSDictionaryMap* theMap = nullptr;
    if (inType != qtssDynamicObjectType)
    {
        UInt32 theDictionaryIndex = QTSSDictionaryMap::GetMapIndex(inType);
        if (theDictionaryIndex == QTSSDictionaryMap::kIllegalDictionary)
            return QTSS_BadArgument;

        theMap = QTSSDictionaryMap::GetMap(theDictionaryIndex);
    }

    return static_cast<QTSSDictionary*>(inDictionary)->CreateObjectValue(inID, outIndex, reinterpret_cast<QTSSDictionary**>(outCreatedObject), theMap);
}

QTSS_Error QTSSCallbacks::QTSS_GetNumValues(QTSS_Object inObject, QTSS_AttributeID inID, UInt32* outNumValues)
{
    if ((inObject == nullptr) || (outNumValues == nullptr) || (inID == qtssIllegalAttrID))
        return QTSS_BadArgument;

    *outNumValues = static_cast<QTSSDictionary*>(inObject)->GetNumValues(inID);
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_GetNumAttributes(QTSS_Object inObject, UInt32* outNumValues)
{

    if (outNumValues == nullptr)
        return QTSS_BadArgument;

    if (inObject == nullptr)
        return QTSS_BadArgument;

    OSMutexLocker locker(static_cast<QTSSDictionary*>(inObject)->GetMutex());

    QTSSDictionaryMap* theMap;
    *outNumValues = 0;

    // Get the Static Attribute count
    theMap = static_cast<QTSSDictionary*>(inObject)->GetDictionaryMap();
    if (theMap != nullptr)
        *outNumValues += theMap->GetNumNonRemovedAttrs();
    // Get the Instance Attribute count
    theMap = static_cast<QTSSDictionary*>(inObject)->GetInstanceDictMap();
    if (theMap != nullptr)
        *outNumValues += theMap->GetNumNonRemovedAttrs();

    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_RemoveValue(QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex)
{
    if (inObject == nullptr)
        return QTSS_BadArgument;

    return static_cast<QTSSDictionary*>(inObject)->RemoveValue(inID, inIndex);
}

QTSS_Error QTSSCallbacks::QTSS_Write(QTSS_StreamRef inStream, void* inBuffer, UInt32 inLen, UInt32* outLenWritten, UInt32 inFlags)
{
    if (inStream == nullptr)
        return QTSS_BadArgument;
    QTSS_Error theErr = static_cast<QTSSStream*>(inStream)->Write(inBuffer, inLen, outLenWritten, inFlags);

    // Server internally propogates POSIX errorcodes such as EAGAIN and ENOTCONN up to this
    // level. The API guarentees that no POSIX errors get returned, so we have QTSS_Errors
    // to replace them. So we have to replace them here.
    if (theErr == EAGAIN)
        return QTSS_WouldBlock;
    else if (theErr > 0)
        return QTSS_NotConnected;
    else
        return theErr;
}

QTSS_Error QTSSCallbacks::QTSS_WriteV(QTSS_StreamRef inStream, iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength, UInt32* outLenWritten)
{
    if (inStream == nullptr)
        return QTSS_BadArgument;
    QTSS_Error theErr = static_cast<QTSSStream*>(inStream)->WriteV(inVec, inNumVectors, inTotalLength, outLenWritten);

    // Server internally propogates POSIX errorcodes such as EAGAIN and ENOTCONN up to this
    // level. The API guarentees that no POSIX errors get returned, so we have QTSS_Errors
    // to replace them. So we have to replace them here.
    if (theErr == EAGAIN)
        return QTSS_WouldBlock;
    else if (theErr > 0)
        return QTSS_NotConnected;
    else
        return theErr;
}

QTSS_Error QTSSCallbacks::QTSS_Flush(QTSS_StreamRef inStream)
{
    if (inStream == nullptr)
        return QTSS_BadArgument;
    QTSS_Error theErr = static_cast<QTSSStream*>(inStream)->Flush();

    // Server internally propogates POSIX errorcodes such as EAGAIN and ENOTCONN up to this
    // level. The API guarentees that no POSIX errors get returned, so we have QTSS_Errors
    // to replace them. So we have to replace them here.
    if (theErr == EAGAIN)
        return QTSS_WouldBlock;
    else if (theErr > 0)
        return QTSS_NotConnected;
    else
        return theErr;
}

QTSS_Error QTSSCallbacks::QTSS_Read(QTSS_StreamRef inStream, void* ioBuffer, UInt32 inBufLen, UInt32* outLengthRead)
{
    if ((inStream == nullptr) || (ioBuffer == nullptr))
        return QTSS_BadArgument;
    QTSS_Error theErr = static_cast<QTSSStream*>(inStream)->Read(ioBuffer, inBufLen, outLengthRead);

    // Server internally propogates POSIX errorcodes such as EAGAIN and ENOTCONN up to this
    // level. The API guarentees that no POSIX errors get returned, so we have QTSS_Errors
    // to replace them. So we have to replace them here.
    if (theErr == EAGAIN)
        return QTSS_WouldBlock;
    else if (theErr > 0)
        return QTSS_NotConnected;
    else
        return theErr;
}

QTSS_Error QTSSCallbacks::QTSS_Seek(QTSS_StreamRef inStream, UInt64 inNewPosition)
{
    if (inStream == nullptr)
        return QTSS_BadArgument;
    return static_cast<QTSSStream*>(inStream)->Seek(inNewPosition);
}

QTSS_Error  QTSSCallbacks::QTSS_Advise(QTSS_StreamRef inStream, UInt64 inPosition, UInt32 inAdviseSize)
{
    if (inStream == nullptr)
        return QTSS_BadArgument;
    return static_cast<QTSSStream*>(inStream)->Advise(inPosition, inAdviseSize);
}

QTSS_Error QTSSCallbacks::QTSS_OpenFileObject(char* inPath, QTSS_OpenFileFlags inFlags, QTSS_Object* outFileObject)
{
    if ((inPath == nullptr) || (outFileObject == nullptr))
        return QTSS_BadArgument;

    //
    // Create a new file object
    QTSSFile* theNewFile = new QTSSFile();
    QTSS_Error theErr = theNewFile->Open(inPath, inFlags);

    if (theErr != QTSS_NoErr)
        delete theNewFile; // No module wanted to open the file.
    else
        *outFileObject = theNewFile;

    return theErr;
}

QTSS_Error QTSSCallbacks::QTSS_CloseFileObject(QTSS_Object inFileObject)
{
    if (inFileObject == nullptr)
        return QTSS_BadArgument;

    QTSSFile* theFile = static_cast<QTSSFile*>(inFileObject);

    theFile->Close();
    delete theFile;
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_CreateStreamFromSocket(int inFileDesc, QTSS_StreamRef* outStream)
{
    if (outStream == nullptr)
        return QTSS_BadArgument;

    if (inFileDesc < 0)
        return QTSS_BadArgument;

    //
    // Create a new socket object
    *outStream = static_cast<QTSS_StreamRef>(new QTSSSocket(inFileDesc));
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_DestroySocketStream(QTSS_StreamRef inStream)
{
    if (inStream == nullptr)
        return QTSS_BadArgument;

    //
    // Note that the QTSSSocket destructor will call close on its file descriptor.
    // Calling module should not also close the file descriptor! (This is noted in the API)
    QTSSSocket* theSocket = static_cast<QTSSSocket*>(inStream);
    delete theSocket;
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_AddService(const char* inServiceName, QTSS_ServiceFunctionPtr inFunctionPtr)
{
    QTSS_ModuleState* theState = static_cast<QTSS_ModuleState*>(OSThread::GetMainThreadData());
    if (OSThread::GetCurrent() != nullptr)
        theState = static_cast<QTSS_ModuleState*>(OSThread::GetCurrent()->GetThreadData());

    // This may happen if this callback is occurring on module-created thread
    if (theState == nullptr)
        return QTSS_OutOfState;

    // Roles can only be added before modules have had their Initialize role invoked.
    if (theState->curRole != QTSS_Register_Role)
        return QTSS_OutOfState;

    return QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServiceDictIndex)->
        AddAttribute(inServiceName, reinterpret_cast<QTSS_AttrFunctionPtr>(inFunctionPtr), qtssAttrDataTypeUnknown, qtssAttrModeRead);
}

QTSS_Error QTSSCallbacks::QTSS_IDForService(const char* inTag, QTSS_ServiceID* outID)
{
    return QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServiceDictIndex)->
        GetAttrID(inTag, outID);
}

QTSS_Error QTSSCallbacks::QTSS_DoService(QTSS_ServiceID inID, QTSS_ServiceFunctionArgsPtr inArgs)
{
    // Make sure that the service ID is in fact valid

    QTSSDictionaryMap* theMap = QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServiceDictIndex);
    SInt32 theIndex = theMap->ConvertAttrIDToArrayIndex(inID);
    if (theIndex < 0)
        return QTSS_IllegalService;

    // Get the service function 
    QTSS_ServiceFunctionPtr theFunction = reinterpret_cast<QTSS_ServiceFunctionPtr>(theMap->GetAttrFunction(theIndex));

    // Invoke it, return the result.    
    return (theFunction)(inArgs);
}


QTSS_Error QTSSCallbacks::QTSS_RequestEvent(QTSS_StreamRef inStream, QTSS_EventType inEventMask)
{
    // First thing to do is to alter the thread's module state to reflect the fact
    // that an event is outstanding.
    QTSS_ModuleState* theState = static_cast<QTSS_ModuleState*>(OSThread::GetMainThreadData());
    if (OSThread::GetCurrent() != nullptr)
        theState = static_cast<QTSS_ModuleState*>(OSThread::GetCurrent()->GetThreadData());

    if (theState == nullptr)
        return QTSS_RequestFailed;

    if (theState->curTask == nullptr)
        return QTSS_OutOfState;
    ;
    theState->eventRequested = true;

    // Now, tell this stream to be ready for the requested event
    QTSSStream* theStream = static_cast<QTSSStream*>(inStream);
    theStream->SetTask(theState->curTask);
    theStream->RequestEvent(inEventMask);
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_SignalStream(QTSS_StreamRef inStream)
{
    if (inStream == nullptr)
        return QTSS_BadArgument;

    QTSSStream* theStream = static_cast<QTSSStream*>(inStream);
    if (theStream->GetTask() != nullptr)
        theStream->GetTask()->Signal(Task::kReadEvent);
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_SetIdleTimer(SInt64 inMsecToWait)
{
    QTSS_ModuleState* theState = static_cast<QTSS_ModuleState*>(OSThread::GetMainThreadData());
    if (OSThread::GetCurrent() != nullptr)
        theState = static_cast<QTSS_ModuleState*>(OSThread::GetCurrent()->GetThreadData());

    // This may happen if this callback is occurring on module-created thread
    if (theState == nullptr)
        return QTSS_RequestFailed;

    if (theState->curTask == nullptr)
        return QTSS_OutOfState;

    theState->eventRequested = true;
    theState->idleTime = inMsecToWait;
    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_SetIdleRoleTimer(SInt64 inMsecToWait)
{

    QTSS_ModuleState* theState = static_cast<QTSS_ModuleState*>(OSThread::GetMainThreadData());
    if (OSThread::GetCurrent() != nullptr)
        theState = static_cast<QTSS_ModuleState*>(OSThread::GetCurrent()->GetThreadData());

    // This may happen if this callback is occurring on module-created thread
    if (theState == nullptr)
        return QTSS_RequestFailed;

    if (theState->curModule == nullptr)
        return QTSS_RequestFailed;


    QTSSModule* theModule = theState->curModule;
    QTSS_ModuleState* thePrivateModuleState = theModule->GetModuleState();
    thePrivateModuleState->idleTime = inMsecToWait;
    theModule->Signal(Task::kUpdateEvent);

    return QTSS_NoErr;
}

QTSS_Error QTSSCallbacks::QTSS_RequestLockedCallback()
{
    QTSS_ModuleState* theState = static_cast<QTSS_ModuleState*>(OSThread::GetMainThreadData());
    if (OSThread::GetCurrent() != nullptr)
        theState = static_cast<QTSS_ModuleState*>(OSThread::GetCurrent()->GetThreadData());

    // This may happen if this callback is occurring on module-created thread
    if (theState == nullptr)
        return QTSS_RequestFailed;

    if (theState->curTask == nullptr)
        return QTSS_OutOfState;

    theState->globalLockRequested = true; //x

    return QTSS_NoErr;
}

bool QTSSCallbacks::QTSS_IsGlobalLocked()
{
    QTSS_ModuleState* theState = static_cast<QTSS_ModuleState*>(OSThread::GetMainThreadData());
    if (OSThread::GetCurrent() != nullptr)
        theState = static_cast<QTSS_ModuleState*>(OSThread::GetCurrent()->GetThreadData());

    // This may happen if this callback is occurring on module-created thread
    if (theState == nullptr)
        return false;

    if (theState->curTask == nullptr)
        return false;

    return theState->isGlobalLocked;
}

QTSS_Error QTSSCallbacks::QTSS_UnlockGlobalLock()
{
    QTSS_ModuleState* theState = static_cast<QTSS_ModuleState*>(OSThread::GetMainThreadData());
    if (OSThread::GetCurrent() != nullptr)
        theState = static_cast<QTSS_ModuleState*>(OSThread::GetCurrent()->GetThreadData());

    // This may happen if this callback is occurring on module-created thread
    if (theState == nullptr)
        return QTSS_RequestFailed;

    if (theState->curTask == nullptr)
        return QTSS_OutOfState;

    reinterpret_cast<Task*>(OSThread::GetCurrent())->GlobalUnlock();

    theState->globalLockRequested = false;
    theState->isGlobalLocked = false;

    return QTSS_NoErr;
}

void QTSSCallbacks::QTSS_LockStdLib()
{
    OS::GetStdLibMutex()->Lock();
}

void QTSSCallbacks::QTSS_UnlockStdLib()
{
    OS::GetStdLibMutex()->Unlock();
}

QTSS_Error QTSSCallbacks::Easy_SendMsg(Easy_HTTPSessionObject inHTTPSession, char* inMsg, UInt32 inMsgLen, bool connectionClose, bool decrement)
{
    if (inHTTPSession == nullptr)
        return QTSS_BadArgument;

    HTTPSession* session = static_cast<HTTPSession*>(inHTTPSession);
    string theValue(inMsg, inMsgLen);

    return session->SendHTTPPacket(theValue, connectionClose, decrement);
}