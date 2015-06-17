/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
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
        
        //Amount of idle time after which respective protocol sessions are timed out
        //(stored in seconds)
        
        //This is the value we advertise to clients (lower than the real one)
        UInt32  GetRTSPTimeoutInSecs()  { return fRTSPTimeoutInSecs; }
        UInt32  GetRTPTimeoutInSecs()   { return fRTPTimeoutInSecs; }
        StrPtrLen*  GetRTSPTimeoutAsString() { return &fRTSPTimeoutString; }
        
        //This is the real timeout
        UInt32  GetSessionTimeoutInSecs(){ return fSessionTimeoutInSecs; }
        
        //-1 means unlimited
        SInt32  GetMaxConnections()         { return fMaximumConnections; }
        SInt32  GetMaxKBitsBandwidth()      { return fMaxBandwidthInKBits; }
        
        // Thinning algorithm parameters
        SInt32  GetDropAllPacketsTimeInMsec()           { return fDropAllPacketsTimeInMsec; }
        SInt32  GetThinAllTheWayTimeInMsec()            { return fThinAllTheWayTimeInMsec; }
        SInt32  GetAlwaysThinTimeInMsec()               { return fAlwaysThinTimeInMsec; }
        SInt32  GetStartThinningTimeInMsec()            { return fStartThinningTimeInMsec; }
        SInt32  GetStartThickingTimeInMsec()            { return fStartThickingTimeInMsec; }
        SInt32  GetThickAllTheWayTimeInMsec()           { return fThickAllTheWayTimeInMsec; }
        UInt32  GetQualityCheckIntervalInMsec()         { return fQualityCheckIntervalInMsec; }
                
        // for tcp buffer size scaling
        UInt32  GetMinTCPBufferSizeInBytes()            { return fMinTCPBufferSizeInBytes; }
        UInt32  GetMaxTCPBufferSizeInBytes()            { return fMaxTCPBufferSizeInBytes; }
        Float32 GetTCPSecondsToBuffer()                 { return fTCPSecondsToBuffer; }
        
		UInt16	GetCMSPort()							{ return fCMSPort; }
        
        //for debugging, mainly
        Bool16      ShouldServerBreakOnAssert()         { return fBreakOnAssert; }
        Bool16      IsAutoRestartEnabled()              { return fAutoRestart; }

        UInt32      GetTotalBytesUpdateTimeInSecs()     { return fTBUpdateTimeInSecs; }
        UInt32      GetAvgBandwidthUpdateTimeInSecs()   { return fABUpdateTimeInSecs; }
        UInt32      GetSafePlayDurationInSecs()         { return fSafePlayDurationInSecs; }
        
        // For the compiled-in error logging module
        
        Bool16  IsErrorLogEnabled()             { return fErrorLogEnabled; }
        Bool16  IsScreenLoggingEnabled()        { return fScreenLoggingEnabled; }

        UInt32  GetMaxErrorLogBytes()           { return fErrorLogBytes; }
        UInt32  GetErrorRollIntervalInDays()    { return fErrorRollIntervalInDays; }
        UInt32  GetErrorLogVerbosity()          { return fErrorLogVerbosity; }
        void    SetErrorLogVerbosity(UInt32 verbosity)        { fErrorLogVerbosity = verbosity; }
        Bool16  GetAppendSrcAddrInTransport()   { return fAppendSrcAddrInTransport; }
        
        //
        // For UDP retransmits
        UInt32  IsReliableUDPEnabled()          { return fReliableUDP; }
        UInt32  GetMaxRetransmitDelayInMsec()   { return fMaxRetransDelayInMsec; }
        Bool16  IsAckLoggingEnabled()           { return fIsAckLoggingEnabled; }
        UInt32  GetSendIntervalInMsec()         { return fSendIntervalInMsec; }
        UInt32  GetMaxSendAheadTimeInSecs()     { return fMaxSendAheadTimeInSecs; }
        Bool16  IsSlowStartEnabled()            { return fIsSlowStartEnabled; }
        Bool16  GetReliableUDPPrintfsEnabled()  { return fReliableUDPPrintfs; }
        Bool16  GetMSGDebugPrintfs()           { return fEnableMSGDebugPrintfs; }
        Bool16  GetRTSPServerInfoEnabled()      { return fEnableRTSPServerInfo; }
        
        Float32    GetOverbufferRate()                { return fOverbufferRate; }
        
        // RUDP window size
        UInt32  GetSmallWindowSizeInK()         { return fSmallWindowSizeInK; }
        UInt32    GetMediumWindowSizeInK()        { return fMediumWindowSizeInK; }
        UInt32  GetLargeWindowSizeInK()         { return fLargeWindowSizeInK; }
        UInt32  GetWindowSizeThreshold()        { return fWindowSizeThreshold; }
         UInt32    GetWindowSizeMaxThreshold()        { return fWindowSizeMaxThreshold; }
       
        //
        // force logs to close after each write (true or false)
        Bool16  GetCloseLogsOnWrite()           { return fCloseLogsOnWrite; }
        void    SetCloseLogsOnWrite(Bool16 closeLogsOnWrite);
        
        //
        // Optionally require that reliable UDP content be in certain folders
        Bool16 IsPathInsideReliableUDPDir(StrPtrLen* inPath);

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
        Bool16  AutoDeleteSDPFiles()        { return fauto_delete_sdp_files; }
        QTSS_AuthScheme GetAuthScheme()     { return fAuthScheme; }
        
        Bool16 PacketHeaderPrintfsEnabled() { return fEnablePacketHeaderPrintfs; }
        Bool16 PrintRTPHeaders()    { return (Bool16) (fPacketHeaderPrintfOptions & kRTPALL); }
        Bool16 PrintSRHeaders()     { return (Bool16) (fPacketHeaderPrintfOptions & kRTCPSR); }
        Bool16 PrintRRHeaders()     { return (Bool16) (fPacketHeaderPrintfOptions & kRTCPRR); }
        Bool16 PrintAPPHeaders()    { return (Bool16) (fPacketHeaderPrintfOptions & kRTCPAPP); }
        Bool16 PrintACKHeaders()    { return (Bool16) (fPacketHeaderPrintfOptions & kRTCPACK); }

        UInt32 DeleteSDPFilesInterval()           { return fsdp_file_delete_interval_seconds; }
                
        UInt32  GetNumThreads()                   { return fNumThreads; } //short tasks threads
        UInt32  GetNumBlockingThreads()           { return fNumMsgThreads; } //return the number of threads that long tasks will be scheduled on -- RTSP processing for example.
        
        Bool16  GetDisableThinning()              { return fDisableThinning; }
        
        Bool16  Get3GPPEnabled()                  { return f3gppProtocolEnabled; }
        Bool16  Get3GPPRateAdaptationEnabled()    { return f3gppProtocolRateAdaptationEnabled; }
        UInt16  Get3GPPRateAdaptReportFrequency() { return f3gppProtocolRateAdaptationReportFrequency; }
        UInt16  GetDefaultStreamQuality()         { return fDefaultStreamQuality; }       
        Bool16  Get3GPPDebugPrintfs()             { return f3gppDebugPrintfsEnabled; }
        Bool16  GetUDPMonitorEnabled()            { return fUDPMonitorEnabled; }
        UInt16  GetMonitorLANPort()			{ return fMonitorLANPort; }       
        UInt16  GetMonitorWANPort()         { return fMonitorWANPort; }  
		UInt16	GetRedisPort()				{ return fRedisPort; }
            
        char* GetMonitorLANIP()    { return this->GetStringPref(qtssPrefsMonitorLANIPAddr); }
        
        char* GetMonitorWANIP()     { return this->GetStringPref(qtssPrefsMonitorWANIPAddr); }

		char* GetCMSIP()			{ return this->GetStringPref(qtssPrefsCMSIPAddr); }

		char* GetRedisIP()			{ return this->GetStringPref(qtssPrefsRedisIPAddr); }

		char* GetSnapLocalPath()	{ return this->GetStringPref(qtssPrefsSnapLocalPath); }
		char* GetSnapWebPath()		{ return this->GetStringPref(qtssPrefsSnapWebPath); }
       
        Bool16 GetAllowGuestDefault()               { return fAllowGuestAuthorizeDefault; }
        
    private:

        UInt32      fRTSPTimeoutInSecs;
        char        fRTSPTimeoutBuf[20];
        StrPtrLen   fRTSPTimeoutString;
        UInt32      fSessionTimeoutInSecs;
        UInt32      fRTPTimeoutInSecs;
        
        SInt32  fMaximumConnections;
        SInt32  fMaxBandwidthInKBits;
        
        Bool16  fBreakOnAssert;
        Bool16  fAutoRestart;
        UInt32  fTBUpdateTimeInSecs;
        UInt32  fABUpdateTimeInSecs;
        UInt32  fSafePlayDurationInSecs;
        
        UInt32  fErrorRollIntervalInDays;
        UInt32  fErrorLogBytes;
        UInt32  fErrorLogVerbosity;
        Bool16  fScreenLoggingEnabled;
        Bool16  fErrorLogEnabled;
        
        SInt32  fDropAllPacketsTimeInMsec;
        SInt32  fThinAllTheWayTimeInMsec;
        SInt32  fAlwaysThinTimeInMsec;
        SInt32  fStartThinningTimeInMsec;
        SInt32  fStartThickingTimeInMsec;
        SInt32  fThickAllTheWayTimeInMsec;
        UInt32  fQualityCheckIntervalInMsec;

        UInt32  fMinTCPBufferSizeInBytes;
        UInt32  fMaxTCPBufferSizeInBytes;
        Float32 fTCPSecondsToBuffer;

        UInt16  fCMSPort;
        Bool16  fAppendSrcAddrInTransport;

        UInt32  fSmallWindowSizeInK;
        UInt32  fMediumWindowSizeInK;
        UInt32  fLargeWindowSizeInK;
        UInt32  fWindowSizeThreshold;
        UInt32  fWindowSizeMaxThreshold;

        UInt32  fMaxRetransDelayInMsec;
        Bool16  fIsAckLoggingEnabled;
        Bool16  fIsSlowStartEnabled;
        UInt32  fSendIntervalInMsec;
        UInt32  fMaxSendAheadTimeInSecs;
        Bool16  fauto_delete_sdp_files;
        QTSS_AuthScheme fAuthScheme;
        UInt32  fsdp_file_delete_interval_seconds;
        Bool16  fAutoStart;
        Bool16  fReliableUDP;
        Bool16  fReliableUDPPrintfs;
        Bool16  fEnableRTSPErrMsg;
        Bool16  fEnableMSGDebugPrintfs;
        Bool16  fEnableRTSPServerInfo;
        UInt32  fNumThreads;
        UInt32  fNumMsgThreads;
        
        Bool16  fEnableMonitorStatsFile;
        UInt32  fStatsFileIntervalSeconds;
    
        Float32    fOverbufferRate;
        
        Bool16  fEnablePacketHeaderPrintfs;
        UInt32  fPacketHeaderPrintfOptions;
        Bool16  fCloseLogsOnWrite;
        
        Bool16 fDisableThinning;
        
        Bool16 f3gppProtocolEnabled;
        Bool16 f3gppProtocolRateAdaptationEnabled;
        UInt16 f3gppProtocolRateAdaptationReportFrequency;
        UInt16 fDefaultStreamQuality;
        Bool16 f3gppDebugPrintfsEnabled;
        Bool16 fUDPMonitorEnabled;
        UInt16 fMonitorLANPort;    
        UInt16 fMonitorWANPort;
		UInt16 fRedisPort;

        char   fMonitorWANAddr[20];
        char   fMonitorLANAddr[20];
		char   fRedisAddr[20];
		char   fServiceID[64];
        Bool16 fAllowGuestAuthorizeDefault;

        enum //fPacketHeaderPrintfOptions
        {
            kRTPALL = 1 << 0,
            kRTCPSR = 1 << 1,
            kRTCPRR = 1 << 2,
            kRTCPAPP = 1<< 3,
            kRTCPACK = 1<< 4
        };
        
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
        void UpdateAuthScheme();
        void UpdatePrintfOptions();
        //
        // Returns the string preference with the specified ID. If there
        // was any problem, this will return an empty string.
        char* GetStringPref(QTSS_AttributeID inAttrID);
        
        static QTSSAttrInfoDict::AttrInfo   sAttributes[];
        static PrefInfo sPrefInfo[];
        
        // Prefs that have multiple default values (rtsp_ports) have
        // to be dealt with specially
        static char*    sAdditionalDefaultPorts[];
        
        // Player prefs
        static char*    sRTP_Header_Players[];
        static char*    sAdjust_Bandwidth_Players[];
        static char*    sNo_Adjust_Pause_Time_Players[];
        static char*    sNo_Pause_Time_Adjustment_Players[];
       
};
#endif //__QTSSPREFS_H__
