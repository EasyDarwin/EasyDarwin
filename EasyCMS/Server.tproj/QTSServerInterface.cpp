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
     File:       QTSServerInterface.cpp
     Contains:   Implementation of object defined in QTSServerInterface.h.
 */

 //INCLUDES:

#ifndef kVersionString
#endif
#include "QTSServerInterface.h"
#include "HTTPSessionInterface.h"

#include "HTTPProtocol.h"
#ifndef __MacOSX__
#include "revision.h"
#endif
#include "EasyUtil.h"

// STATIC DATA

UInt32                  QTSServerInterface::sServerAPIVersion = QTSS_API_VERSION;
QTSServerInterface*     QTSServerInterface::sServer = NULL;
#if __MacOSX__
StrPtrLen               QTSServerInterface::sServerNameStr("EasyCMS");
#else
StrPtrLen               QTSServerInterface::sServerNameStr("EasyCMS");
#endif

// kVersionString from revision.h, include with -i at project level
StrPtrLen               QTSServerInterface::sServerVersionStr(kVersionString);
StrPtrLen               QTSServerInterface::sServerBuildStr(kBuildString);
StrPtrLen               QTSServerInterface::sServerCommentStr(kCommentString);

StrPtrLen               QTSServerInterface::sServerPlatformStr(kPlatformNameString);
StrPtrLen               QTSServerInterface::sServerBuildDateStr(__DATE__ ", " __TIME__);
char                    QTSServerInterface::sServerHeader[kMaxServerHeaderLen];
StrPtrLen               QTSServerInterface::sServerHeaderPtr(sServerHeader, kMaxServerHeaderLen);

QTSSModule**            QTSServerInterface::sModuleArray[QTSSModule::kNumRoles];
UInt32                  QTSServerInterface::sNumModulesInRole[QTSSModule::kNumRoles];
OSQueue                 QTSServerInterface::sModuleQueue;
QTSSErrorLogStream      QTSServerInterface::sErrorLogStream;

QTSSAttrInfoDict::AttrInfo  QTSServerInterface::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0  */ { "qtssServerAPIVersion",          NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 1  */ { "qtssSvrDefaultDNSName",         NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead },
    /* 2  */ { "qtssSvrDefaultIPAddr",          NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 3  */ { "qtssSvrServerName",             NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 4  */ { "qtssSvrServerVersion",      NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 5  */ { "qtssSvrServerBuildDate",    NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 6  */ { "qtssSvrHTTPPorts",              NULL,   qtssAttrDataTypeUInt16,     qtssAttrModeRead },
    /* 7  */ { "qtssSvrHTTPServerHeader",       NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 8  */ { "qtssSvrState",					NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite  },
    /* 9  */ { "qtssSvrIsOutOfDescriptors",     isOutOfDescriptors,     qtssAttrDataTypeBool16, qtssAttrModeRead },
    /* 10 */ { "qtssCurrentSessionCount",		NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },

    /* 11 */ { "qtssSvrHandledMethods",         NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe  },
    /* 12 */ { "qtssSvrModuleObjects",          NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 13 */ { "qtssSvrStartupTime",            NULL,   qtssAttrDataTypeTimeVal,    qtssAttrModeRead },
    /* 14 */ { "qtssSvrGMTOffsetInHrs",         NULL,   qtssAttrDataTypeSInt32,     qtssAttrModeRead },
    /* 15 */ { "qtssSvrDefaultIPAddrStr",       NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead },

    /* 16 */ { "qtssSvrPreferences",            NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead | qtssAttrModeInstanceAttrAllowed},
    /* 17 */ { "qtssSvrMessages",               NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead },
    /* 18 */ { "qtssSvrClientSessions",         NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead },
    /* 19 */ { "qtssSvrCurrentTimeMilliseconds",currentUnixTimeMilli,   qtssAttrDataTypeTimeVal,qtssAttrModeRead},
    /* 20 */ { "qtssSvrCPULoadPercent",         NULL,   qtssAttrDataTypeFloat32,    qtssAttrModeRead},

    /* 21 */ { "qtssSvrConnectedUsers",         NULL, qtssAttrDataTypeQTSS_Object,      qtssAttrModeRead | qtssAttrModeWrite },
    /* 22 */ { "qtssSvrServerBuild",           NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 23 */ { "qtssSvrServerPlatform",        NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 24 */ { "qtssSvrHTTPServerComment",     NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 25 */ { "qtssSvrNumThinned",            NULL,   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },

    /* 26 */ { "qtssSvrNumThreads",            NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe }
};

void    QTSServerInterface::Initialize()
{
    for (UInt32 x = 0; x < qtssSvrNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServerDictIndex)->
        SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
            sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);

    //Write out a premade server header
    StringFormatter serverFormatter(sServerHeaderPtr.Ptr, kMaxServerHeaderLen);
    serverFormatter.Put(HTTPProtocol::GetHeaderString(httpServerHeader)->Ptr, HTTPProtocol::GetHeaderString(httpServerHeader)->Len);
    serverFormatter.Put(": ");
    serverFormatter.Put(sServerNameStr);
    serverFormatter.PutChar('/');
    serverFormatter.Put(sServerVersionStr);
    serverFormatter.PutChar(' ');

    serverFormatter.PutChar('(');
    serverFormatter.Put("Build/");
    serverFormatter.Put(sServerBuildStr);
    serverFormatter.Put("; ");
    serverFormatter.Put("Platform/");
    serverFormatter.Put(sServerPlatformStr);
    serverFormatter.PutChar(';');

    if (sServerCommentStr.Len > 0)
    {
        serverFormatter.PutChar(' ');
        serverFormatter.Put(sServerCommentStr);
    }
    serverFormatter.PutChar(')');


    sServerHeaderPtr.Len = serverFormatter.GetCurrentOffset();
    Assert(sServerHeaderPtr.Len < kMaxServerHeaderLen);
}

QTSServerInterface::QTSServerInterface()
    : QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServerDictIndex), &fMutex),
    fSrvrPrefs(NULL),
    fSrvrMessages(NULL),
    fServerState(qtssStartingUpState),
    fDefaultIPAddr(0),
    fListeners(NULL),
    fNumListeners(0),
    fStartupTime_UnixMilli(0),
    fGMTOffset(0),
    fNumHTTPSessions(0),
    fCPUPercent(0),
    fCPUTimeUsedInSec(0),
    fSigInt(false),
    fSigTerm(false),
    fDebugLevel(0),
    fDebugOptions(0),
    fNumThinned(0),
    fNumThreads(0)
{
    for (UInt32 y = 0; y < QTSSModule::kNumRoles; y++)
    {
        sModuleArray[y] = NULL;
        sNumModulesInRole[y] = 0;
    }

    this->SetVal(qtssSvrState, &fServerState, sizeof(fServerState));
    this->SetVal(qtssServerAPIVersion, &sServerAPIVersion, sizeof(sServerAPIVersion));
    this->SetVal(qtssSvrDefaultIPAddr, &fDefaultIPAddr, sizeof(fDefaultIPAddr));
    this->SetVal(qtssSvrServerName, sServerNameStr.Ptr, sServerNameStr.Len);
    this->SetVal(qtssSvrServerVersion, sServerVersionStr.Ptr, sServerVersionStr.Len);
    this->SetVal(qtssSvrServerBuildDate, sServerBuildDateStr.Ptr, sServerBuildDateStr.Len);
    this->SetVal(qtssSvrHTTPServerHeader, sServerHeaderPtr.Ptr, sServerHeaderPtr.Len);
    this->SetVal(qtssCurrentSessionCount, &fNumHTTPSessions, sizeof(fNumHTTPSessions));
    this->SetVal(qtssSvrStartupTime, &fStartupTime_UnixMilli, sizeof(fStartupTime_UnixMilli));
    this->SetVal(qtssSvrGMTOffsetInHrs, &fGMTOffset, sizeof(fGMTOffset));
    this->SetVal(qtssSvrCPULoadPercent, &fCPUPercent, sizeof(fCPUPercent));

    this->SetVal(qtssSvrServerBuild, sServerBuildStr.Ptr, sServerBuildStr.Len);
    this->SetVal(qtssSvrHTTPServerComment, sServerCommentStr.Ptr, sServerCommentStr.Len);
    this->SetVal(qtssSvrServerPlatform, sServerPlatformStr.Ptr, sServerPlatformStr.Len);

    this->SetVal(qtssSvrNumThinned, &fNumThinned, sizeof(fNumThinned));
    this->SetVal(qtssSvrNumThreads, &fNumThreads, sizeof(fNumThreads));

    qtss_sprintf(fCloudServiceNodeID, "%s", EasyUtil::GetUUID().c_str());

    sServer = this;

}

void QTSServerInterface::LogError(QTSS_ErrorVerbosity inVerbosity, char* inBuffer)
{
    QTSS_RoleParams theParams;
    theParams.errorParams.inVerbosity = inVerbosity;
    theParams.errorParams.inBuffer = inBuffer;

    for (UInt32 x = 0; x < QTSServerInterface::GetNumModulesInRole(QTSSModule::kErrorLogRole); x++)
        (void)QTSServerInterface::GetModule(QTSSModule::kErrorLogRole, x)->CallDispatch(QTSS_ErrorLog_Role, &theParams);

    // If this is a fatal error, set the proper attribute in the RTSPServer dictionary
    if ((inVerbosity == qtssFatalVerbosity) && (sServer != NULL))
    {
        QTSS_ServerState theState = qtssFatalErrorState;
        (void)sServer->SetValue(qtssSvrState, 0, &theState, sizeof(theState));
    }
}

void QTSServerInterface::SetValueComplete(UInt32 inAttrIndex, QTSSDictionaryMap* inMap,
    UInt32 inValueIndex, void* inNewValue, UInt32 inNewValueLen)
{
    if (inAttrIndex == qtssSvrState)
    {
        Assert(inNewValueLen == sizeof(QTSS_ServerState));

        //
        // Invoke the server state change role
        QTSS_RoleParams theParams;
        theParams.stateChangeParams.inNewState = *(QTSS_ServerState*)inNewValue;

        static QTSS_ModuleState sStateChangeState = { NULL, 0, NULL, false };
        if (OSThread::GetCurrent() == NULL)
            OSThread::SetMainThreadData(&sStateChangeState);
        else
            OSThread::GetCurrent()->SetThreadData(&sStateChangeState);

        UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kStateChangeRole);
        {
            for (UInt32 theCurrentModule = 0; theCurrentModule < numModules; theCurrentModule++)
            {
                QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kStateChangeRole, theCurrentModule);
                (void)theModule->CallDispatch(QTSS_StateChange_Role, &theParams);
            }
        }

        //
        // Make sure to clear out the thread data
        if (OSThread::GetCurrent() == NULL)
            OSThread::SetMainThreadData(NULL);
        else
            OSThread::GetCurrent()->SetThreadData(NULL);
    }
}

void* QTSServerInterface::currentUnixTimeMilli(QTSSDictionary* inServer, UInt32* outLen)
{
    QTSServerInterface* theServer = (QTSServerInterface*)inServer;
    theServer->fCurrentTime_UnixMilli = OS::TimeMilli_To_UnixTimeMilli(OS::Milliseconds());

    // Return the result
    *outLen = sizeof(theServer->fCurrentTime_UnixMilli);
    return &theServer->fCurrentTime_UnixMilli;
}

void* QTSServerInterface::isOutOfDescriptors(QTSSDictionary* inServer, UInt32* outLen)
{
    QTSServerInterface* theServer = (QTSServerInterface*)inServer;

    theServer->fIsOutOfDescriptors = false;
    for (UInt32 x = 0; x < theServer->fNumListeners; x++)
    {
        if (theServer->fListeners[x]->IsOutOfDescriptors())
        {
            theServer->fIsOutOfDescriptors = true;
            break;
        }
    }
    // Return the result
    *outLen = sizeof(theServer->fIsOutOfDescriptors);
    return &theServer->fIsOutOfDescriptors;
}

QTSS_Error QTSSErrorLogStream::Write(void* inBuffer, UInt32 inLen, UInt32* outLenWritten, UInt32 inFlags)
{
    // For the error log stream, the flags are considered to be the verbosity
    // of the error.
    if (inFlags >= qtssIllegalVerbosity)
        inFlags = qtssMessageVerbosity;

    QTSServerInterface::LogError(inFlags, (char*)inBuffer);
    if (outLenWritten != NULL)
        *outLenWritten = inLen;

    return QTSS_NoErr;
}

void QTSSErrorLogStream::LogAssert(char* inMessage)
{
    QTSServerInterface::LogError(qtssAssertVerbosity, inMessage);
}
