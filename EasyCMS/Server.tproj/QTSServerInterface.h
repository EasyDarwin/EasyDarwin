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
	Website: http://www.EasyDarwin.org
*/
/*
    File:       QTSServerInterface.h

    Contains:   This object defines an interface for getting and setting server-wide
                attributes, and storing global server resources.
                
                There can be only one of these objects per process, so there
                is a static accessor.
*/


#ifndef __QTSSERVERINTERFACE_H__
#define __QTSSERVERINTERFACE_H__

#include "QTSS.h"
#include "QTSSDictionary.h"
#include "QTSServerPrefs.h"
#include "QTSSMessages.h"
#include "QTSSModule.h"
#include "atomic.h"

#include "OSMutex.h"
#include "Task.h"
#include "TCPListenerSocket.h"
#include "ResizeableStringFormatter.h"
#include <EasyProtocol.h>

using namespace EasyDarwin::Protocol;

// OSRefTable;
class QTSServerPrefs;
class QTSSMessages;

// This object also functions as our assert logger
class QTSSErrorLogStream : public QTSSStream, public AssertLogger
{
    public:
    
        // This QTSSStream is used by modules to write to the error log
    
        QTSSErrorLogStream() {}
        virtual ~QTSSErrorLogStream() {}
        
        virtual QTSS_Error  Write(void* inBuffer, UInt32 inLen, UInt32* outLenWritten, UInt32 inFlags);
        virtual void        LogAssert(char* inMessage);
};

class QTSServerInterface : public QTSSDictionary
{
    public:
    
        //Initialize must be called right off the bat to initialize dictionary resources
        static void     Initialize();

        //
        // CONSTRUCTOR / DESTRUCTOR
        
        QTSServerInterface();
        virtual ~QTSServerInterface() {}
        
        //
        //
        // STATISTICS MANIPULATION
        // These functions are how the server keeps its statistics current
        
        void                AlterCurrentServiceSessionCount(SInt32 inDifference)
            { OSMutexLocker locker(&fMutex); fNumServiceSessions += inDifference; }
            
		void            IncrementTotalLate(SInt64 milliseconds)
           {    OSMutexLocker locker(&fMutex); 
                fTotalLate += milliseconds;
                if (milliseconds > fCurrentMaxLate) fCurrentMaxLate = milliseconds;
                if (milliseconds > fMaxLate) fMaxLate = milliseconds;
           }
           
        void            IncrementTotalQuality(SInt32 level)
           { OSMutexLocker locker(&fMutex); fTotalQuality += level; }
           
        void            IncrementNumThinned(SInt32 inDifference)
           { OSMutexLocker locker(&fMutex); fNumThinned += inDifference; }

        void            ClearTotalLate()
           { OSMutexLocker locker(&fMutex); fTotalLate = 0;  }
        void            ClearCurrentMaxLate()
           { OSMutexLocker locker(&fMutex); fCurrentMaxLate = 0;  }
        void            ClearTotalQuality()
           { OSMutexLocker locker(&fMutex); fTotalQuality = 0;  }

		void 			InitNumThreads(UInt32 numThreads) {  fNumThreads = numThreads; }
        //
        // ACCESSORS
        
        QTSS_ServerState    GetServerState()        { return fServerState; }
        UInt32              GetNumServiceSessions()    { return fNumServiceSessions; }
        
        Float32             GetCPUPercent()         { return fCPUPercent; }
        Bool16              SigIntSet()             { return fSigInt; }
        Bool16				SigTermSet()			{ return fSigTerm; }
		
        UInt32              GetDebugLevel()                     { return fDebugLevel; }
        UInt32              GetDebugOptions()                   { return fDebugOptions; }
        void                SetDebugLevel(UInt32 debugLevel)    { fDebugLevel = debugLevel; }
        void                SetDebugOptions(UInt32 debugOptions){ fDebugOptions = debugOptions; }
        
        SInt64          GetMaxLate()                { return fMaxLate; };
        SInt64          GetTotalLate()              { return fTotalLate; };
        SInt64          GetCurrentMaxLate()         { return fCurrentMaxLate; };
        SInt64          GetTotalQuality()           { return fTotalQuality; };
        SInt32          GetNumThinned()             { return fNumThinned; };
        UInt32          GetNumThreads()             { return fNumThreads; };

        //
        //
        // GLOBAL OBJECTS REPOSITORY
        // This object is in fact global, so there is an accessor for it as well.
        
        static QTSServerInterface*  GetServer()         { return sServer; }
        
        //Allows you to map RTP session IDs (strings) to actual RTP session objects
        OSRefTable*         GetDeviceSessionMap()          { return fDeviceSessionMap; }

		EasyNVRs &GetRegisterNVRs() { return fRegisterNVRs; }

        QTSServerPrefs*     GetPrefs()                  { return fSrvrPrefs; }
        QTSSMessages*       GetMessages()               { return fSrvrMessages; }
        
        // SERVER NAME & VERSION
        
        static StrPtrLen&   GetServerName()             { return sServerNameStr; }
        static StrPtrLen&   GetServerVersion()          { return sServerVersionStr; }
        static StrPtrLen&   GetServerPlatform()         { return sServerPlatformStr; }
        static StrPtrLen&   GetServerBuildDate()        { return sServerBuildDateStr; }
        static StrPtrLen&   GetServerHeader()           { return sServerHeaderPtr; }
        static StrPtrLen&   GetServerBuild()            { return sServerBuildStr; }
        static StrPtrLen&   GetServerComment()          { return sServerCommentStr; }
        
        //断开所有session连接
        void                RemoveAllDeviceSession();
        
        //
        // SIGINT - to interrupt the server, set this flag and the server will shut down
        void                SetSigInt()                 { fSigInt = true; }

       // SIGTERM - to kill the server, set this flag and the server will shut down
        void                SetSigTerm()                 { fSigTerm = true; }
        
        // MODULE STORAGE
        
        // All module objects are stored here, and are accessable through
        // these routines.
        
        // Returns the number of modules that act in a given role
        static UInt32       GetNumModulesInRole(QTSSModule::RoleIndex inRole)
                { Assert(inRole < QTSSModule::kNumRoles); return sNumModulesInRole[inRole]; }
        
        // Allows the caller to iterate over all modules that act in a given role           
        static QTSSModule*  GetModule(QTSSModule::RoleIndex inRole, UInt32 inIndex)
				{   Assert(inRole < QTSSModule::kNumRoles);
					Assert(inIndex < sNumModulesInRole[inRole]);
					if (inRole >= QTSSModule::kNumRoles) //index out of bounds, shouldn't happen
					{    return NULL;
					}
					if (inIndex >= sNumModulesInRole[inRole]) //index out of bounds, shouldn't happen
					{   return NULL;
					}
					return sModuleArray[inRole][inIndex];
				}

        //
        // We need to override this. This is how we implement the QTSS_StateChange_Role
        virtual void    SetValueComplete(UInt32 inAttrIndex, QTSSDictionaryMap* inMap,
									UInt32 inValueIndex, void* inNewValue, UInt32 inNewValueLen);
        
        //
        // ERROR LOGGING
        
        // Invokes the error logging modules with some data
        static void     LogError(QTSS_ErrorVerbosity inVerbosity, char* inBuffer);
        
        // Returns the error log stream
        static QTSSErrorLogStream* GetErrorLogStream() { return &sErrorLogStream; }

        // LOCKING DOWN THE SERVER OBJECT
        OSMutex*        GetServerObjectMutex() { return &fMutex; }

    protected:

        // Setup by the derived RTSPServer object
        
        //所有服务单元Hash表，以ServiceType区分不同功能单元
        OSRefTable*                 fDeviceSessionMap;
        
		EasyNVRs fRegisterNVRs;

        QTSServerPrefs*             fSrvrPrefs;
        QTSSMessages*               fSrvrMessages;

        QTSServerPrefs*				fStubSrvrPrefs;
        QTSSMessages*				fStubSrvrMessages;

        QTSS_ServerState            fServerState;
        UInt32                      fDefaultIPAddr;
        
        // Array of pointers to TCPListenerSockets.
        TCPListenerSocket**         fListeners;
        UInt32                      fNumListeners; // Number of elements in the array
        
        // startup time
        SInt64              fStartupTime_UnixMilli;
        SInt32              fGMTOffset;

        //
        // MODULE DATA
        
        static QTSSModule**             sModuleArray[QTSSModule::kNumRoles];
        static UInt32                   sNumModulesInRole[QTSSModule::kNumRoles];
        static OSQueue                  sModuleQueue;
        static QTSSErrorLogStream       sErrorLogStream;

		char fDMSServiceID[QTSS_MAX_SESSION_ID_LENGTH];

    private:
    
        enum
        {
            kMaxServerHeaderLen = 1000
        };

        static void* TimeConnected(QTSSDictionary* inConnection, UInt32* outLen);

        static UInt32       sServerAPIVersion;
        static StrPtrLen    sServerNameStr;
        static StrPtrLen    sServerVersionStr;
        static StrPtrLen    sServerBuildStr;
        static StrPtrLen    sServerCommentStr;
        static StrPtrLen    sServerPlatformStr;
        static StrPtrLen    sServerBuildDateStr;
        static char         sServerHeader[kMaxServerHeaderLen];
        static StrPtrLen    sServerHeaderPtr;

        OSMutex             fMutex;

        UInt32              fNumServiceSessions;
        
        Float32             fCPUPercent;
        Float32             fCPUTimeUsedInSec;              
        
        // are we out of descriptors?
        Bool16              fIsOutOfDescriptors;
        
        // Storage for current time attribute
        SInt64              fCurrentTime_UnixMilli;
       
        Bool16              fSigInt;
        Bool16              fSigTerm;

        UInt32              fDebugLevel;
        UInt32              fDebugOptions;
        

        SInt64          fMaxLate;
        SInt64          fTotalLate;
        SInt64          fCurrentMaxLate;
        SInt64          fTotalQuality;
        SInt32          fNumThinned;
        UInt32          fNumThreads;
 
        // Param retrieval functions
        static void* CurrentUnixTimeMilli(QTSSDictionary* inServer, UInt32* outLen);
        static void* IsOutOfDescriptors(QTSSDictionary* inServer, UInt32* outLen);
        
        static QTSServerInterface*  sServer;
        static QTSSAttrInfoDict::AttrInfo   sAttributes[];
        static QTSSAttrInfoDict::AttrInfo   sConnectedUserAttributes[];

		char* GetDMSServiceID() { return fDMSServiceID; }
};

#endif // __QTSSERVERINTERFACE_H__

