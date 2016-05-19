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
    File:       QTSServerInterface.cpp

    Contains:   Implementation of object defined in QTSServerInterface.h.
    
    

*/

//INCLUDES:

#ifndef kVersionString
#include "revision.h"
#endif
#include "QTSServerInterface.h"

#include "RTPSessionInterface.h"
#include "OSRef.h"
#include "UDPSocketPool.h"
#include "RTSPProtocol.h"
#include "RTPPacketResender.h"
#include "revision.h"


// STATIC DATA

UInt32                  QTSServerInterface::sServerAPIVersion = QTSS_API_VERSION;
QTSServerInterface*     QTSServerInterface::sServer = NULL;
#if __MacOSX__
StrPtrLen               QTSServerInterface::sServerNameStr("EasyDarwin");
#else
StrPtrLen               QTSServerInterface::sServerNameStr("EasyDarwin");
#endif

// kVersionString from revision.h, include with -i at project level
StrPtrLen               QTSServerInterface::sServerVersionStr(kVersionString);
StrPtrLen               QTSServerInterface::sServerBuildStr(kBuildString);
StrPtrLen               QTSServerInterface::sServerCommentStr(kCommentString);

StrPtrLen               QTSServerInterface::sServerPlatformStr(kPlatformNameString);
StrPtrLen               QTSServerInterface::sServerBuildDateStr(__DATE__ ", "__TIME__);
char                    QTSServerInterface::sServerHeader[kMaxServerHeaderLen];
StrPtrLen               QTSServerInterface::sServerHeaderPtr(sServerHeader, kMaxServerHeaderLen);

ResizeableStringFormatter       QTSServerInterface::sPublicHeaderFormatter(NULL, 0);
StrPtrLen                       QTSServerInterface::sPublicHeaderStr;

QTSSModule**            QTSServerInterface::sModuleArray[QTSSModule::kNumRoles];
UInt32                  QTSServerInterface::sNumModulesInRole[QTSSModule::kNumRoles];
OSQueue                 QTSServerInterface::sModuleQueue;
QTSSErrorLogStream      QTSServerInterface::sErrorLogStream;


QTSSAttrInfoDict::AttrInfo  QTSServerInterface::sConnectedUserAttributes[] = 
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0  */ { "qtssConnectionType",                    NULL,   qtssAttrDataTypeCharArray,      qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 1  */ { "qtssConnectionCreateTimeInMsec",        NULL,   qtssAttrDataTypeTimeVal,        qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 2  */ { "qtssConnectionTimeConnectedInMsec",     TimeConnected,  qtssAttrDataTypeTimeVal,        qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 3  */ { "qtssConnectionBytesSent",               NULL,   qtssAttrDataTypeUInt32,         qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 4  */ { "qtssConnectionMountPoint",              NULL,   qtssAttrDataTypeCharArray,      qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 5  */ { "qtssConnectionHostName",                NULL,   qtssAttrDataTypeCharArray,      qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe } ,

    /* 6  */ { "qtssConnectionSessRemoteAddrStr",       NULL,   qtssAttrDataTypeCharArray,      qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 7  */ { "qtssConnectionSessLocalAddrStr",        NULL,   qtssAttrDataTypeCharArray,      qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    
    /* 8  */ { "qtssConnectionCurrentBitRate",          NULL,   qtssAttrDataTypeUInt32,         qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 9  */ { "qtssConnectionPacketLossPercent",       NULL,   qtssAttrDataTypeFloat32,        qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    // this last parameter is a workaround for the current dictionary implementation.  For qtssConnectionTimeConnectedInMsec above we have a param
    // retrieval function.  This needs storage to keep the value returned, but if it sets its own param then the function no longer gets called.
    /* 10 */ { "qtssConnectionTimeStorage",             NULL,   qtssAttrDataTypeTimeVal,        qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
};


QTSSAttrInfoDict::AttrInfo  QTSServerInterface::sAttributes[] = 
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0  */ { "qtssServerAPIVersion",          NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 1  */ { "qtssSvrDefaultDNSName",         NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead },
    /* 2  */ { "qtssSvrDefaultIPAddr",          NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 3  */ { "qtssSvrServerName",             NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 4  */ { "qtssRTSPSvrServerVersion",      NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 5  */ { "qtssRTSPSvrServerBuildDate",    NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 6  */ { "qtssSvrRTSPPorts",              NULL,   qtssAttrDataTypeUInt16,     qtssAttrModeRead },
    /* 7  */ { "qtssSvrRTSPServerHeader",       NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 8  */ { "qtssSvrState",              NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite  },
    /* 9  */ { "qtssSvrIsOutOfDescriptors",     IsOutOfDescriptors,     qtssAttrDataTypeBool16, qtssAttrModeRead },
    /* 10 */ { "qtssRTSPCurrentSessionCount",   NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 11 */ { "qtssRTSPHTTPCurrentSessionCount",NULL,  qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 12 */ { "qtssRTPSvrNumUDPSockets",       GetTotalUDPSockets,     qtssAttrDataTypeUInt32, qtssAttrModeRead },
    /* 13 */ { "qtssRTPSvrCurConn",             NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 14 */ { "qtssRTPSvrTotalConn",           NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 15 */ { "qtssRTPSvrCurBandwidth",        NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 16 */ { "qtssRTPSvrTotalBytes",          NULL,   qtssAttrDataTypeUInt64,     qtssAttrModeRead },
    /* 17 */ { "qtssRTPSvrAvgBandwidth",        NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 18 */ { "qtssRTPSvrCurPackets",          NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 19 */ { "qtssRTPSvrTotalPackets",        NULL,   qtssAttrDataTypeUInt64,     qtssAttrModeRead },
    /* 20 */ { "qtssSvrHandledMethods",         NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe  },
    /* 21 */ { "qtssSvrModuleObjects",          NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 22 */ { "qtssSvrStartupTime",            NULL,   qtssAttrDataTypeTimeVal,    qtssAttrModeRead },
    /* 23 */ { "qtssSvrGMTOffsetInHrs",         NULL,   qtssAttrDataTypeSInt32,     qtssAttrModeRead },
    /* 24 */ { "qtssSvrDefaultIPAddrStr",       NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead },
    /* 25 */ { "qtssSvrPreferences",            NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead | qtssAttrModeInstanceAttrAllowed},
    /* 26 */ { "qtssSvrMessages",               NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead },
    /* 27 */ { "qtssSvrClientSessions",         NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead },
    /* 28 */ { "qtssSvrCurrentTimeMilliseconds",CurrentUnixTimeMilli,   qtssAttrDataTypeTimeVal,qtssAttrModeRead},
    /* 29 */ { "qtssSvrCPULoadPercent",         NULL,   qtssAttrDataTypeFloat32,    qtssAttrModeRead},
    /* 30 */ { "qtssSvrNumReliableUDPBuffers",  GetNumUDPBuffers,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 31 */ { "qtssSvrReliableUDPWastageInBytes",GetNumWastedBytes, qtssAttrDataTypeUInt32,        qtssAttrModeRead },
    /* 32 */ { "qtssSvrConnectedUsers",         NULL, qtssAttrDataTypeQTSS_Object,      qtssAttrModeRead | qtssAttrModeWrite },
    /* 33 */ { "qtssMP3SvrCurConn",             NULL, qtssAttrDataTypeUInt32,       qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 34 */ { "qtssMP3SvrTotalConn",           NULL, qtssAttrDataTypeUInt32,       qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 35 */ { "qtssMP3SvrCurBandwidth",        NULL, qtssAttrDataTypeUInt32,       qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 36 */ { "qtssMP3SvrTotalBytes",          NULL, qtssAttrDataTypeUInt64,       qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 37 */ { "qtssMP3SvrAvgBandwidth",        NULL, qtssAttrDataTypeUInt32,       qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },

    /* 38  */ { "qtssSvrServerBuild",           NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 39  */ { "qtssSvrServerPlatform",        NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 40  */ { "qtssSvrRTSPServerComment",     NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
	/* 41  */ { "qtssSvrNumThinned",            NULL,   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 42  */ { "qtssSvrNumThreads",            NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe }
    
};

void    QTSServerInterface::Initialize()
{
    for (UInt32 x = 0; x < qtssSvrNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServerDictIndex)->
            SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
                sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);

    for (UInt32 y = 0; y < qtssConnectionNumParams; y++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kQTSSConnectedUserDictIndex)->
            SetAttribute(y, sConnectedUserAttributes[y].fAttrName, sConnectedUserAttributes[y].fFuncPtr,
                sConnectedUserAttributes[y].fAttrDataType, sConnectedUserAttributes[y].fAttrPermission);

    //Write out a premade server header
    StringFormatter serverFormatter(sServerHeaderPtr.Ptr, kMaxServerHeaderLen);
    serverFormatter.Put(RTSPProtocol::GetHeaderString(qtssServerHeader));
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
 :  QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServerDictIndex), &fMutex),
    fSocketPool(NULL),
    fRTPMap(NULL),
	fHLSMap(NULL),
	fReflectorSessionMap(NULL),
    fSrvrPrefs(NULL),
    fSrvrMessages(NULL),
    fServerState(qtssStartingUpState),
    fDefaultIPAddr(0),
    fListeners(NULL),
    fNumListeners(0),
    fStartupTime_UnixMilli(0),
    fGMTOffset(0),
    fNumRTSPSessions(0),
    fNumRTSPHTTPSessions(0),
    fNumRTPSessions(0),
    fNumRTPPlayingSessions(0),
    fTotalRTPSessions(0),
    fTotalRTPBytes(0),
    fTotalRTPPackets(0),
    fTotalRTPPacketsLost(0),
    fPeriodicRTPBytes(0),
    fPeriodicRTPPacketsLost(0),
    fPeriodicRTPPackets(0),
    fCurrentRTPBandwidthInBits(0),
    fAvgRTPBandwidthInBits(0),
    fRTPPacketsPerSecond(0),
    fCPUPercent(0),
    fCPUTimeUsedInSec(0),
    fUDPWastageInBytes(0),
    fNumUDPBuffers(0),
    fNumMP3Sessions(0),
    fTotalMP3Sessions(0),
    fCurrentMP3BandwidthInBits(0),
    fTotalMP3Bytes(0),
    fAvgMP3BandwidthInBits(0),
    fSigInt(false),
    fSigTerm(false),
    fDebugLevel(0),
    fDebugOptions(0),    
    fMaxLate(0),
    fTotalLate(0),
    fCurrentMaxLate(0),
    fTotalQuality(0),
    fNumThinned(0),
    fNumThreads(0)
{
    for (UInt32 y = 0; y < QTSSModule::kNumRoles; y++)
    {
        sModuleArray[y] = NULL;
        sNumModulesInRole[y] = 0;
    }

    this->SetVal(qtssSvrState,              &fServerState,              sizeof(fServerState));
    this->SetVal(qtssServerAPIVersion,      &sServerAPIVersion,         sizeof(sServerAPIVersion));
    this->SetVal(qtssSvrDefaultIPAddr,      &fDefaultIPAddr,            sizeof(fDefaultIPAddr));
    this->SetVal(qtssSvrServerName,         sServerNameStr.Ptr,         sServerNameStr.Len);
    this->SetVal(qtssSvrServerVersion,      sServerVersionStr.Ptr,      sServerVersionStr.Len);
    this->SetVal(qtssSvrServerBuildDate,    sServerBuildDateStr.Ptr,    sServerBuildDateStr.Len);
    this->SetVal(qtssSvrRTSPServerHeader,   sServerHeaderPtr.Ptr,       sServerHeaderPtr.Len);
    this->SetVal(qtssRTSPCurrentSessionCount, &fNumRTSPSessions,        sizeof(fNumRTSPSessions));
    this->SetVal(qtssRTSPHTTPCurrentSessionCount, &fNumRTSPHTTPSessions,sizeof(fNumRTSPHTTPSessions));
    this->SetVal(qtssRTPSvrCurConn,         &fNumRTPSessions,           sizeof(fNumRTPSessions));
    this->SetVal(qtssRTPSvrTotalConn,       &fTotalRTPSessions,         sizeof(fTotalRTPSessions));
    this->SetVal(qtssRTPSvrCurBandwidth,    &fCurrentRTPBandwidthInBits,sizeof(fCurrentRTPBandwidthInBits));
    this->SetVal(qtssRTPSvrTotalBytes,      &fTotalRTPBytes,            sizeof(fTotalRTPBytes));
    this->SetVal(qtssRTPSvrAvgBandwidth,    &fAvgRTPBandwidthInBits,    sizeof(fAvgRTPBandwidthInBits));
    this->SetVal(qtssRTPSvrCurPackets,      &fRTPPacketsPerSecond,      sizeof(fRTPPacketsPerSecond));
    this->SetVal(qtssRTPSvrTotalPackets,    &fTotalRTPPackets,          sizeof(fTotalRTPPackets));
    this->SetVal(qtssSvrStartupTime,        &fStartupTime_UnixMilli,    sizeof(fStartupTime_UnixMilli));
    this->SetVal(qtssSvrGMTOffsetInHrs,     &fGMTOffset,                sizeof(fGMTOffset));
    this->SetVal(qtssSvrCPULoadPercent,     &fCPUPercent,               sizeof(fCPUPercent));
    this->SetVal(qtssMP3SvrCurConn,         &fNumMP3Sessions,           sizeof(fNumMP3Sessions));
    this->SetVal(qtssMP3SvrTotalConn,       &fTotalMP3Sessions,         sizeof(fTotalMP3Sessions));
    this->SetVal(qtssMP3SvrCurBandwidth,    &fCurrentMP3BandwidthInBits,sizeof(fCurrentMP3BandwidthInBits));
    this->SetVal(qtssMP3SvrTotalBytes,      &fTotalMP3Bytes,            sizeof(fTotalMP3Bytes));
    this->SetVal(qtssMP3SvrAvgBandwidth,    &fAvgMP3BandwidthInBits,    sizeof(fAvgMP3BandwidthInBits));

    this->SetVal(qtssSvrServerBuild,        sServerBuildStr.Ptr,    sServerBuildStr.Len);
    this->SetVal(qtssSvrRTSPServerComment,  sServerCommentStr.Ptr,  sServerCommentStr.Len);
    this->SetVal(qtssSvrServerPlatform,     sServerPlatformStr.Ptr, sServerPlatformStr.Len);

    this->SetVal(qtssSvrNumThinned,         &fNumThinned,               sizeof(fNumThinned));
    this->SetVal(qtssSvrNumThreads,         &fNumThreads,               sizeof(fNumThreads));
    

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

void QTSServerInterface::KillAllRTPSessions()
{
    OSMutexLocker locker(fRTPMap->GetMutex());
    for (OSRefHashTableIter theIter(fRTPMap->GetHashTable()); !theIter.IsDone(); theIter.Next())
    {
        OSRef* theRef = theIter.GetCurrent();
        RTPSessionInterface* theSession = (RTPSessionInterface*)theRef->GetObject();
        theSession->Signal(Task::kKillEvent);
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


RTPStatsUpdaterTask::RTPStatsUpdaterTask()
:   Task(), fLastBandwidthTime(0), fLastBandwidthAvg(0), fLastBytesSent(0), fLastTotalMP3Bytes(0)
{
    this->SetTaskName("RTPStatsUpdaterTask");
    this->Signal(Task::kStartEvent);
}

Float32 RTPStatsUpdaterTask::GetCPUTimeInSeconds()
{
    // This function returns the total number of seconds that the
    // process running RTPStatsUpdaterTask() has been executing as
    // a user process.
    Float32 cpuTimeInSec = 0.0;
#ifdef __Win32__
    // The Win32 way of getting the time for this process
    HANDLE hProcess = GetCurrentProcess();
    SInt64 createTime, exitTime, kernelTime, userTime;
    if(GetProcessTimes(hProcess, (LPFILETIME)&createTime, (LPFILETIME)&exitTime, (LPFILETIME)&kernelTime, (LPFILETIME)&userTime))
    {
        // userTime is in 10**-7 seconds since Jan.1, 1607.
        // (What type of computers did they use in 1607?)
        cpuTimeInSec = (Float32) (userTime/10000000.0);
    }
    else
    {
        // This should never happen!!!
        Assert(0);
        cpuTimeInSec = 0.0;
    }
#else
    // The UNIX way of getting the time for this process
    clock_t cpuTime = clock();
    cpuTimeInSec = (Float32) cpuTime / CLOCKS_PER_SEC;
#endif
    return cpuTimeInSec;
}

SInt64 RTPStatsUpdaterTask::Run()
{

    QTSServerInterface* theServer = QTSServerInterface::sServer;
    
    // All of this must happen atomically wrt dictionary values we are manipulating
    OSMutexLocker locker(&theServer->fMutex);
    
    //First update total bytes. This must be done because total bytes is a 64 bit number,
    //so no atomic functions can apply.
    //
    // NOTE: The line below is not thread safe on non-PowerPC platforms. This is
    // because the fPeriodicRTPBytes variable is being manipulated from within an
    // atomic_add. On PowerPC, assignments are atomic, so the assignment below is ok.
    // On a non-PowerPC platform, the following would be thread safe:
    //unsigned int periodicBytes = atomic_add(&theServer->fPeriodicRTPBytes, 0);
    unsigned int periodicBytes = theServer->fPeriodicRTPBytes;
    (void)atomic_sub(&theServer->fPeriodicRTPBytes, periodicBytes);
    theServer->fTotalRTPBytes += periodicBytes;
    
    // Same deal for packet totals
    unsigned int periodicPackets = theServer->fPeriodicRTPPackets;
    (void)atomic_sub(&theServer->fPeriodicRTPPackets, periodicPackets);
    theServer->fTotalRTPPackets += periodicPackets;
    
    // ..and for lost packet totals
    unsigned int periodicPacketsLost = theServer->fPeriodicRTPPacketsLost;
    (void)atomic_sub(&theServer->fPeriodicRTPPacketsLost, periodicPacketsLost);
    theServer->fTotalRTPPacketsLost += periodicPacketsLost;
    
    SInt64 curTime = OS::Milliseconds();
    
    //for cpu percent
        Float32 cpuTimeInSec = GetCPUTimeInSeconds();
    
    //also update current bandwidth statistic
    if (fLastBandwidthTime != 0)
    {
        Assert(curTime > fLastBandwidthTime);
        UInt32 delta = (UInt32)(curTime - fLastBandwidthTime);
		// Prevent divide by zero errror
		if (delta < 1000) {
			WarnV(delta >= 1000, "delta < 1000");
			(void)this->GetEvents();//we must clear the event mask!
			return theServer->GetPrefs()->GetTotalBytesUpdateTimeInSecs() * 1000;
		}

        UInt32 packetsPerSecond = periodicPackets;
        UInt32 theTime = delta / 1000;
 
        packetsPerSecond /= theTime;
        Assert(packetsPerSecond >= 0);
        theServer->fRTPPacketsPerSecond = packetsPerSecond;
        UInt32 additionalBytes = 28 * packetsPerSecond; // IP headers = 20 + UDP headers = 8
        UInt32 headerBits = 8 * additionalBytes;
        headerBits  /= theTime;
      
        Float32 bits = periodicBytes * 8;
        bits /= theTime;
        theServer->fCurrentRTPBandwidthInBits = (UInt32) (bits + headerBits);
         
        // okay let's do it for MP3 bytes now
        bits = (Float32)(((SInt64)theServer->fTotalMP3Bytes - fLastTotalMP3Bytes) * 8);
        bits /= theTime;
        theServer->fCurrentMP3BandwidthInBits = (UInt32)bits;


        //do the computation for cpu percent
        Float32 diffTime = cpuTimeInSec - theServer->fCPUTimeUsedInSec;
        theServer->fCPUPercent = (diffTime/theTime) * 100;  
		
		UInt32 numProcessors = OS::GetNumProcessors();
		
		if (numProcessors > 1)
			theServer->fCPUPercent /= numProcessors;
    }
    
    fLastTotalMP3Bytes = (SInt64)theServer->fTotalMP3Bytes;
    fLastBandwidthTime = curTime;
    // We use a running average for avg. bandwidth calculations
    theServer->fAvgMP3BandwidthInBits = (theServer->fAvgMP3BandwidthInBits
            + theServer->fCurrentMP3BandwidthInBits)/2; 
    
    //for cpu percent
    theServer->fCPUTimeUsedInSec    = cpuTimeInSec; 
    
    //also compute average bandwidth, a much more smooth value. This is done with
    //the fLastBandwidthAvg, a timestamp of the last time we did an average, and
    //fLastBytesSent, the number of bytes sent when we last did an average.
    if ((fLastBandwidthAvg != 0) && (curTime > (fLastBandwidthAvg +
        (theServer->GetPrefs()->GetAvgBandwidthUpdateTimeInSecs() * 1000))))
    {
        UInt32 delta = (UInt32)(curTime - fLastBandwidthAvg);
        SInt64 bytesSent = theServer->fTotalRTPBytes - fLastBytesSent;
        Assert(bytesSent >= 0);
        
        //do the bandwidth computation using floating point divides
        //for accuracy and speed.
        Float32 bits = (Float32)(bytesSent * 8);
        Float32 theAvgTime = (Float32)delta;
        theAvgTime /= 1000;
        bits /= theAvgTime;
        Assert(bits >= 0);
        theServer->fAvgRTPBandwidthInBits = (UInt32)bits;

        fLastBandwidthAvg = curTime;
        fLastBytesSent = theServer->fTotalRTPBytes;
        
        //if the bandwidth is above the bandwidth setting, disconnect 1 user by sending them
        //a BYE RTCP packet.
        SInt32 maxKBits = theServer->GetPrefs()->GetMaxKBitsBandwidth();
        if ((maxKBits > -1) && (theServer->fAvgRTPBandwidthInBits > ((UInt32)maxKBits * 1024)))
        {
            //we need to make sure that all of this happens atomically wrt the session map
            OSMutexLocker locker(theServer->GetRTPSessionMap()->GetMutex());
            RTPSessionInterface* theSession = this->GetNewestSession(theServer->fRTPMap);
            if (theSession != NULL)
                if ((curTime - theSession->GetSessionCreateTime()) <
                        theServer->GetPrefs()->GetSafePlayDurationInSecs() * 1000)
                    theSession->Signal(Task::kKillEvent);
        }
    }
    else if (fLastBandwidthAvg == 0)
    {
        fLastBandwidthAvg = curTime;
        fLastBytesSent = theServer->fTotalRTPBytes;
    }
    
    (void)this->GetEvents();//we must clear the event mask!
    return theServer->GetPrefs()->GetTotalBytesUpdateTimeInSecs() * 1000;
}

RTPSessionInterface* RTPStatsUpdaterTask::GetNewestSession(OSRefTable* inRTPSessionMap)
{
    //Caller must lock down the RTP session map
    SInt64 theNewestPlayTime = 0;
    RTPSessionInterface* theNewestSession = NULL;
    
    //use the session map to iterate through all the sessions, finding the most
    //recently connected client
    for (OSRefHashTableIter theIter(inRTPSessionMap->GetHashTable()); !theIter.IsDone(); theIter.Next())
    {
        OSRef* theRef = theIter.GetCurrent();
        RTPSessionInterface* theSession = (RTPSessionInterface*)theRef->GetObject();
        Assert(theSession->GetSessionCreateTime() > 0);
        if (theSession->GetSessionCreateTime() > theNewestPlayTime)
        {
            theNewestPlayTime = theSession->GetSessionCreateTime();
            theNewestSession = theSession;
        }
    }
    return theNewestSession;
}



void* QTSServerInterface::CurrentUnixTimeMilli(QTSSDictionary* inServer, UInt32* outLen)
{
    QTSServerInterface* theServer = (QTSServerInterface*)inServer;
    theServer->fCurrentTime_UnixMilli = OS::TimeMilli_To_UnixTimeMilli(OS::Milliseconds()); 
    
    // Return the result
    *outLen = sizeof(theServer->fCurrentTime_UnixMilli);
    return &theServer->fCurrentTime_UnixMilli;
}

void* QTSServerInterface::GetTotalUDPSockets(QTSSDictionary* inServer, UInt32* outLen)
{
    QTSServerInterface* theServer = (QTSServerInterface*)inServer;
    // Multiply by 2 because this is returning the number of socket *pairs*
    theServer->fTotalUDPSockets = theServer->fSocketPool->GetSocketQueue()->GetLength() * 2;
    
    // Return the result
    *outLen = sizeof(theServer->fTotalUDPSockets);
    return &theServer->fTotalUDPSockets;
}

void* QTSServerInterface::IsOutOfDescriptors(QTSSDictionary* inServer, UInt32* outLen)
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

void* QTSServerInterface::GetNumUDPBuffers(QTSSDictionary* inServer, UInt32* outLen)
{
    // This param retrieval function must be invoked each time it is called,
    // because whether we are out of descriptors or not is continually changing
    QTSServerInterface* theServer = (QTSServerInterface*)inServer;
    
    theServer->fNumUDPBuffers = RTPPacketResender::GetNumRetransmitBuffers();

    // Return the result
    *outLen = sizeof(theServer->fNumUDPBuffers);
    return &theServer->fNumUDPBuffers;
}

void* QTSServerInterface::GetNumWastedBytes(QTSSDictionary* inServer, UInt32* outLen)
{
    // This param retrieval function must be invoked each time it is called,
    // because whether we are out of descriptors or not is continually changing
    QTSServerInterface* theServer = (QTSServerInterface*)inServer;
    
    theServer->fUDPWastageInBytes = RTPPacketResender::GetWastedBufferBytes();

    // Return the result
    *outLen = sizeof(theServer->fUDPWastageInBytes);
    return &theServer->fUDPWastageInBytes;  
}

void* QTSServerInterface::TimeConnected(QTSSDictionary* inConnection, UInt32* outLen)
{
    SInt64 connectTime;
    void* result;
    UInt32 len = sizeof(connectTime);
    inConnection->GetValue(qtssConnectionCreateTimeInMsec, 0, &connectTime, &len);
    SInt64 timeConnected = OS::Milliseconds() - connectTime;
    *outLen = sizeof(timeConnected);
    inConnection->SetValue(qtssConnectionTimeStorage, 0, &timeConnected, sizeof(connectTime));
    inConnection->GetValuePtr(qtssConnectionTimeStorage, 0, &result, outLen);

    // Return the result
    return result;
}


QTSS_Error  QTSSErrorLogStream::Write(void* inBuffer, UInt32 inLen, UInt32* outLenWritten, UInt32 inFlags)
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

//redis,add,begin
bool QTSServerInterface::ConRedis()//连接redis服务器
{
	if(fIfConSucess)
		return true;

	struct timeval timeout = { 0, 500000 }; // 0.5 seconds
	fRedisCon = redisConnectWithTimeout(fRedisIP, fRedisPort, timeout);
	if (fRedisCon == NULL || fRedisCon->err)
	{
		if (fRedisCon) {
			qtss_printf("INFO:Connect redis failed,%s\n", fRedisCon->errstr);
			redisFree(fRedisCon);
		} else {
			qtss_printf("INFO:Connect redis failed,can't allocate redis context\n");
		}
		fIfConSucess = false;
	}
	else
	{
		fIfConSucess = true;
		struct timeval timeoutEx = { 1, 0 }; // 1seconds,设置socket接收和发送超时
		redisSetTimeout(fRedisCon,timeoutEx);

		RedisInit();//可能在这个函数的执行过程中，redis连接又失败了，所以真正的连接失败或者成功要看fIfConSucess
		qtss_printf("INFO:Connect redis sucess\n");
	}
	return fIfConSucess;
}

bool QTSServerInterface::RedisCommand(const char * strCommand)//执行一些不关心结果的命令，只要知道成功或者失败，一般为写
{
	OSMutexLocker mutexLock(&fRedisMutex);

	if(!ConRedis())//每一次执行命令之前都先连接redis,如果当前redis还没有成功连接
		return false;
	redisReply* reply = (redisReply*)redisCommand(fRedisCon,strCommand);
	//需要注意的是，如果返回的对象是NULL，则表示客户端和服务器之间出现严重错误，必须重新链接。
	if (NULL == reply) 
	{
		redisFree(fRedisCon);
		fIfConSucess = false;
		return false;
	}
	//qtss_printf("%s\n",strCommand);
	//qtss_printf("%s\n",reply->str);
	freeReplyObject(reply);
	return true;
}

bool QTSServerInterface::RedisChangeRtpMum()//更改rtp数
{
	char chTemp[128] = {0};
	sprintf(chTemp,"hset %s:%d_Info RTP %d",fEasyDarWInIP,fEasyDarWInPort,fNumRTPSessions);//hset对RTP属性进行覆盖更新
	return RedisCommand(chTemp);
}

bool QTSServerInterface::RedisAddPushName(const char * strPushNmae)
{
	char chTemp[128] = {0};
	sprintf(chTemp,"sadd %s:%d_PushName %s",fEasyDarWInIP,fEasyDarWInPort,strPushNmae);//将推流名称加入到set中
	return RedisCommand(chTemp);
}

bool QTSServerInterface::RedisDelPushName(const char * strPushNmae)
{
	char chTemp[128] = {0};
	sprintf(chTemp,"srem %s:%d_PushName %s",fEasyDarWInIP,fEasyDarWInPort,strPushNmae);//将推流名称从set中移除
	return RedisCommand(chTemp);
}

bool QTSServerInterface::RedisTTL()//注意当网络在一段时间很差时可能会因为超时时间达到而key被删除，这时应该重新设置该key
{
	OSMutexLocker mutexLock(&fRedisMutex);

	bool bReval = false;
	if(!ConRedis())//每一次执行命令之前都先连接redis,如果当前redis还没有成功连接
		return false;

	char chTemp[128] = {0};//注意128位是否足够
	sprintf(chTemp,"expire  %s:%d_Live 15",fEasyDarWInIP,fEasyDarWInPort);//更改超时时间

	redisReply* reply = (redisReply*)redisCommand(fRedisCon,chTemp);
	//需要注意的是，如果返回的对象是NULL，则表示客户端和服务器之间出现严重错误，必须重新链接。
	if (NULL == reply) 
	{
		redisFree(fRedisCon);
		fIfConSucess = false;
		return false;
	}

	if( (reply->type == REDIS_REPLY_INTEGER) && (reply->integer == 1) )//正常情况
	{
		bReval = true;
	}
	else if( (reply->type == REDIS_REPLY_INTEGER) && (reply->integer == 0) )//说明当前key已经不存在了,那么我们需要重新生成该key
	{
		sprintf(chTemp,"setex %s:%d_Live 15 1",fEasyDarWInIP,fEasyDarWInPort);
		bReval = RedisCommand(chTemp);
	}
	else//其他情况，这是不可能发生的。实际上是有可能发生的，经测试，如果redis没有写的权限，就会发生。
	{
		Assert(0);
	}
	freeReplyObject(reply);
	return bReval;
}

bool QTSServerInterface::RedisJudgeSessionID(const char * strSessionID)//验证SessionID的合法性
{
	//算法描述，删除指定sessionID对应的key，如果成功删除，表明SessionID存在，验证通过，否则验证失败
	OSMutexLocker mutexLocker(&fRedisMutex);

	if(!ConRedis())
		return false;

	bool bReval = false;
	char chTemp[128] = {0};
	sprintf(chTemp,"del SessionID_%s",strSessionID);//如果key存在则返回整数类型1，否则返回整数类型0

	redisReply* reply = (redisReply*)redisCommand(fRedisCon,chTemp);
	//需要注意的是，如果返回的对象是NULL，则表示客户端和服务器之间出现严重错误，必须重新链接。
	//这里只是举例说明，简便起见，后面的命令就不再做这样的判断了。
	if (NULL == reply) 
	{
		redisFree(fRedisCon);
		fIfConSucess = false;
		return false;
	}
	if( (reply->type == REDIS_REPLY_INTEGER) && (reply->integer == 1) )
		bReval = true;
	else
		bReval = false;

	freeReplyObject(reply);
	return bReval;
}

bool QTSServerInterface::RedisInit()//连接redis成功之后调用该函数执行一些初始化的工作
{
	//每一次与redis连接后，都应该清除上一次的数据存储，使用覆盖或者直接清除的方式,串行命令使用管线更加高效
	char chTemp[128] = {0};
	char chPassword[]="~ziguangwulian~iguangwulian~guangwulian~uangwulian";
	do 
	{
		//1,redis密码认证
		sprintf(chTemp,"auth %s",chPassword);
		if(redisAppendCommand(fRedisCon,chTemp) != REDIS_OK)
			break;
		
		//2,EasyDarWin唯一信息存储(覆盖上一次的存储)
		sprintf(chTemp,"sadd EasyDarWinName %s:%d",fEasyDarWInIP,fEasyDarWInPort);
		if(redisAppendCommand(fRedisCon,chTemp) != REDIS_OK)
			break;

		//3,EasyDarWin属性存储,设置多个filed使用hmset，单个使用hset(覆盖上一次的存储)
		sprintf(chTemp,"hmset %s:%d_Info IP %s PORT %d RTP %d",fEasyDarWInIP,fEasyDarWInPort,fEasyDarWInIP,fEasyDarWInPort,fNumRTPSessions);
		if(redisAppendCommand(fRedisCon,chTemp) != REDIS_OK)
			break;
		
		//4,清除推流名称存储
		sprintf(chTemp,"del %s:%d_PushName",fEasyDarWInIP,fEasyDarWInPort);
		if(redisAppendCommand(fRedisCon,chTemp) != REDIS_OK)
			break;

		char* strAllPushName;
		OSMutex *mutexMap = fReflectorSessionMap->GetMutex();
		int iPos = 0,iLen = 0;
		
		mutexMap->Lock();
		strAllPushName = new char[fReflectorSessionMap->GetNumRefsInTable()*128 + 1];//为每一个推流名称分配128字节的内存
		memset(strAllPushName,0,fReflectorSessionMap->GetNumRefsInTable()*128 + 1);//内存初始化为0
		for (OSRefHashTableIter theIter(fReflectorSessionMap->GetHashTable()); !theIter.IsDone(); theIter.Next())
		{
			OSRef* theRef			=	theIter.GetCurrent();
			StrPtrLen* strPushName  =	theRef->GetString();

			char *ppos1 = strstr(strPushName->Ptr,"/");//"./Movies/12345678/0001.sdp"->"12345678/0001.sdp"
			if(ppos1 != NULL)
			{
				char *ppos2 = strstr(ppos1+1,"/");
				if(ppos2 != NULL)
				{
					iLen = strPushName->Len-(ppos2-strPushName->Ptr+1);
					strAllPushName[iPos] = ' ';
					memcpy(strAllPushName+iPos+1,ppos2+1,iLen);
					iPos += 1+iLen;
				}
			}
			/*
			string strTemp(strPushName->Ptr,strPushName->Len);
			int npos=strTemp.find('\\');
			if(npos!=string::npos)
			{
				strAllPushName=strAllPushName+' '+strTemp.substr(npos+1);
			}*/
		}   
		mutexMap->Unlock();

		char *chNewTemp = new char[strlen(strAllPushName)+128];//注意，这里不能再使用chTemp，因为长度不确定，可能导致缓冲区溢出
		
		//5,推流名称存储
		sprintf(chNewTemp,"sadd %s:%d_PushName%s",fEasyDarWInIP,fEasyDarWInPort,strAllPushName);
		if(redisAppendCommand(fRedisCon,chNewTemp) != REDIS_OK)
		{
			delete[] chNewTemp;
			delete[] strAllPushName;
			break;
		}
		delete[] chNewTemp;
		delete[] strAllPushName;
		
		//6,保活，设置15秒，这之后当前CMS已经开始提供服务了
		sprintf(chTemp,"setex %s:%d_Live 15 1",fEasyDarWInIP,fEasyDarWInPort);
		if(redisAppendCommand(fRedisCon,chTemp) != REDIS_OK)
			break;

		bool bBreak = false;
		redisReply* reply = NULL;
		for(int i=0;i<6;i++)
		{
			if(REDIS_OK != redisGetReply(fRedisCon,(void**)&reply))
			{
				bBreak = true;
				//freeReplyObject(reply);//经测试，如果在redisGetReply时，连接关闭，freeReplyObject会引起崩溃，因为reply为空
				break;
			}
			freeReplyObject(reply);
		}
		if(bBreak)//说明redisGetReply出现了错误
			break;
		return true;
	} while (0);
	//走到这说明出现了错误，需要进行重连,重连操作再下一次执行命令时进行,在这仅仅是置标志位
	redisFree(fRedisCon);
	fIfConSucess = false;

	return false;
}
//redis,add,end


