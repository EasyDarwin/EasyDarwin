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
    Contains:   Object store for RTSP server preferences.
    
    
    
*/

#ifndef __QTSSERVERPREFS_H__
#define __QTSSERVERPREFS_H__

#include "StrPtrLen.h"
#include "QTSSPrefs.h"
#include "XMLPrefsParser.h"

class QTSServerPrefs : public QTSSPrefs
{
    public:

        // INITIALIZE
        //
        // This function sets up the dictionary map. Must be called before instantiating
        // the first RTSPPrefs object.
    
        static void Initialize();

        QTSServerPrefs(XMLPrefsParser* inPrefsSource, Bool16 inWriteMissingPrefs);
        virtual ~QTSServerPrefs() {}
        
        //This is callable at any time, and is thread safe wrt to the accessors.
        //Pass in true if you want this function to update the prefs file if
        //any defaults need to be used. False otherwise
        void RereadServerPreferences(Bool16 inWriteMissingPrefs);
        
        //Individual accessor methods for preferences.

        //This is the real timeout
        UInt32  GetSessionTimeoutInSecs(){ return fSessionTimeoutInSecs; }
        
        //-1 means unlimited
        SInt32  GetMaxConnections()         { return fMaximumConnections; }
        SInt32  GetMaxKBitsBandwidth()      { return fMaxBandwidthInKBits; }       
        // for tcp buffer size scaling
        UInt32  GetMinTCPBufferSizeInBytes()            { return fMinTCPBufferSizeInBytes; }
        UInt32  GetMaxTCPBufferSizeInBytes()            { return fMaxTCPBufferSizeInBytes; }
        Float32 GetTCPSecondsToBuffer()                 { return fTCPSecondsToBuffer; }
        
		UInt16	GetCMSPort()							{ return fCMSPort; }
        
        //for debugging, mainly
        Bool16      ShouldServerBreakOnAssert()         { return fBreakOnAssert; }
        Bool16      IsAutoRestartEnabled()              { return fAutoRestart; }
        
        // For the compiled-in error logging module
        
        Bool16  IsErrorLogEnabled()             { return fErrorLogEnabled; }
        Bool16  IsScreenLoggingEnabled()        { return fScreenLoggingEnabled; }

        UInt32  GetMaxErrorLogBytes()           { return fErrorLogBytes; }
        UInt32  GetErrorRollIntervalInDays()    { return fErrorRollIntervalInDays; }
        UInt32  GetErrorLogVerbosity()          { return fErrorLogVerbosity; }
        void    SetErrorLogVerbosity(UInt32 verbosity)        { fErrorLogVerbosity = verbosity; }
        Bool16  GetAppendSrcAddrInTransport()   { return fAppendSrcAddrInTransport; }
        
        //
        UInt32  GetMaxRetransmitDelayInMsec()   { return fMaxRetransDelayInMsec; }
        Bool16  IsAckLoggingEnabled()           { return fIsAckLoggingEnabled; }
        UInt32  GetSendIntervalInMsec()         { return fSendIntervalInMsec; }
        UInt32  GetMaxSendAheadTimeInSecs()     { return fMaxSendAheadTimeInSecs; }
        Bool16  GetMSGDebugPrintfs()           { return fEnableMSGDebugPrintfs; }
        Bool16  GetCMSServerInfoEnabled()      { return fEnableCMSServerInfo; }
        
        Float32    GetOverbufferRate()                { return fOverbufferRate; }

		// force logs to close after each write (true or false)
        Bool16  GetCloseLogsOnWrite()           { return fCloseLogsOnWrite; }
        void    SetCloseLogsOnWrite(Bool16 closeLogsOnWrite);

        // Movie folder pref. If the path fits inside the buffer provided,
        // the path is copied into that buffer. Otherwise, a new buffer is allocated
        // and returned.
        //char*   GetMovieFolder(char* inBuffer, UInt32* ioLen);
        
        //
        // Transport addr pref. Caller must provide a buffer big enough for an IP addr
        void    GetTransportSrcAddr(StrPtrLen* ioBuf);
                
        // String preferences. Note that the pointers returned here is allocated
        // memory that you must delete!
        
        char*   GetErrorLogDir()
            { return this->GetStringPref(qtssPrefsErrorLogDir); }
        char*   GetErrorLogName()
            { return this->GetStringPref(qtssPrefsErrorLogName); }

		char*	GetServiceID()
			{ return this->GetStringPref(qtssPrefsServiceID); }

        char*   GetModuleDirectory()
            { return this->GetStringPref(qtssPrefsModuleFolder); }
        
        char*   GetRunUserName()
            { return this->GetStringPref(qtssPrefsRunUserName); }
        char*   GetRunGroupName()
            { return this->GetStringPref(qtssPrefsRunGroupName); }

        char*   GetPidFilePath()
            { return this->GetStringPref(qtssPrefsPidFile); }

        char*   GetStatsMonitorFileName()
            { return this->GetStringPref(qtssPrefsMonitorStatsFileName); }

        Bool16 ServerStatFileEnabled()      { return fEnableMonitorStatsFile; }
        UInt32 GetStatFileIntervalSec()     { return fStatsFileIntervalSeconds; }
                 
        UInt32  GetNumThreads()                   { return fNumThreads; } //short tasks threads
        UInt32  GetNumBlockingThreads()           { return fNumMsgThreads; } //return the number of threads that long tasks will be scheduled on -- RTSP processing for example.
        
        Bool16  GetDisableThinning()              { return fDisableThinning; }

        UInt16  GetMonitorLANPort()			{ return fMonitorLANPort; }       
        UInt16  GetMonitorWANPort()         { return fMonitorWANPort; }  
            
        char* GetMonitorLANIP()    { return this->GetStringPref(qtssPrefsMonitorLANIPAddr); }
        
        char* GetMonitorWANIP()     { return this->GetStringPref(qtssPrefsMonitorWANIPAddr); }

		char* GetCMSIP()			{ return this->GetStringPref(qtssPrefsCMSIPAddr); }
		char* GetSnapLocalPath()	{ return this->GetStringPref(qtssPrefsSnapLocalPath); }
		char* GetSnapWebPath()		{ return this->GetStringPref(qtssPrefsSnapWebPath); }
       
        Bool16 GetAllowGuestDefault()               { return fAllowGuestAuthorizeDefault; }

		char* GetRedisIP()			{return this->GetStringPref(qtssPrefsRedisIPAddr); }
        UInt16 GetRedisPort(){return fRedisPort;}


    private:
        UInt32      fSessionTimeoutInSecs;
        
        SInt32  fMaximumConnections;
        SInt32  fMaxBandwidthInKBits;
        
        Bool16  fBreakOnAssert;
        Bool16  fAutoRestart;
        
        UInt32  fErrorRollIntervalInDays;
        UInt32  fErrorLogBytes;
        UInt32  fErrorLogVerbosity;
        Bool16  fScreenLoggingEnabled;
        Bool16  fErrorLogEnabled;

        UInt32  fMinTCPBufferSizeInBytes;
        UInt32  fMaxTCPBufferSizeInBytes;
        Float32 fTCPSecondsToBuffer;

        UInt16  fCMSPort;
        Bool16  fAppendSrcAddrInTransport;

        UInt32  fMaxRetransDelayInMsec;
        Bool16  fIsAckLoggingEnabled;
        UInt32  fSendIntervalInMsec;
        UInt32  fMaxSendAheadTimeInSecs;

        Bool16  fAutoStart;
        Bool16  fEnableMSGDebugPrintfs;
        Bool16  fEnableCMSServerInfo;
        UInt32  fNumThreads;
        UInt32  fNumMsgThreads;
        
        Bool16  fEnableMonitorStatsFile;
        UInt32  fStatsFileIntervalSeconds;
    
        Float32    fOverbufferRate;
        
        Bool16  fCloseLogsOnWrite;
        
        Bool16 fDisableThinning;
        UInt16 fMonitorLANPort;    
        UInt16 fMonitorWANPort;
		UInt16 fRedisPort;

        char   fMonitorWANAddr[20];
        char   fMonitorLANAddr[20];
		char   fRedisAddr[20];
		char   fServiceID[64];
        Bool16 fAllowGuestAuthorizeDefault;
        
        enum
        {
            kAllowMultipleValues = 1,
            kDontAllowMultipleValues = 0
        };
        
        struct PrefInfo
        {
            UInt32  fAllowMultipleValues;
            char*   fDefaultValue;
            char**  fAdditionalDefVals; // For prefs with multiple default values
        };
            
        void SetupAttributes();

        // Returns the string preference with the specified ID. If there
        // was any problem, this will return an empty string.
        char* GetStringPref(QTSS_AttributeID inAttrID);
        
        static QTSSAttrInfoDict::AttrInfo   sAttributes[];
        static PrefInfo sPrefInfo[];
        
        // Prefs that have multiple default values (rtsp_ports) have
        // to be dealt with specially
        static char*    sAdditionalDefaultPorts[];   
};
#endif //__QTSSPREFS_H__
