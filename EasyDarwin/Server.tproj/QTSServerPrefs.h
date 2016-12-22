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

	QTSServerPrefs(XMLPrefsParser* inPrefsSource, bool inWriteMissingPrefs);
	virtual ~QTSServerPrefs() {}

	//This is callable at any time, and is thread safe wrt to the accessors.
	//Pass in true if you want this function to update the prefs file if
	//any defaults need to be used. False otherwise
	void RereadServerPreferences(bool inWriteMissingPrefs);

	//Individual accessor methods for preferences.

	//Amount of idle time after which respective protocol sessions are timed out
	//(stored in seconds)

	//This is the value we advertise to clients (lower than the real one)
	UInt32  GetRTSPTimeoutInSecs() { return fRTSPTimeoutInSecs; }
	UInt32  GetRTPSessionTimeoutInSecs() { return fRTPSessionTimeoutInSecs; }
	StrPtrLen*  GetRTSPTimeoutAsString() { return &fRTSPTimeoutString; }

	//This is the real timeout
	UInt32  GetRTSPSessionTimeoutInSecs() { return fRTSPSessionTimeoutInSecs; }

	//-1 means unlimited
	SInt32  GetMaxConnections() { return fMaximumConnections; }
	SInt32  GetMaxKBitsBandwidth() { return fMaxBandwidthInKBits; }

	// Thinning algorithm parameters
	SInt32  GetDropAllPacketsTimeInMsec() { return fDropAllPacketsTimeInMsec; }
	SInt32  GetDropAllVideoPacketsTimeInMsec() { return fDropAllVideoPacketsTimeInMsec; }
	SInt32  GetThinAllTheWayTimeInMsec() { return fThinAllTheWayTimeInMsec; }
	SInt32  GetAlwaysThinTimeInMsec() { return fAlwaysThinTimeInMsec; }
	SInt32  GetStartThinningTimeInMsec() { return fStartThinningTimeInMsec; }
	SInt32  GetStartThickingTimeInMsec() { return fStartThickingTimeInMsec; }
	SInt32  GetThickAllTheWayTimeInMsec() { return fThickAllTheWayTimeInMsec; }
	UInt32  GetQualityCheckIntervalInMsec() { return fQualityCheckIntervalInMsec; }

	// for tcp buffer size scaling
	UInt32  GetMinTCPBufferSizeInBytes() { return fMinTCPBufferSizeInBytes; }
	UInt32  GetMaxTCPBufferSizeInBytes() { return fMaxTCPBufferSizeInBytes; }
	Float32 GetTCPSecondsToBuffer() { return fTCPSecondsToBuffer; }

	//for joining HTTP sessions from behind a round-robin DNS
	bool  GetDoReportHTTPConnectionAddress() { return fDoReportHTTPConnectionAddress; }

	//for debugging, mainly
	bool      ShouldServerBreakOnAssert() { return fBreakOnAssert; }
	bool      IsAutoRestartEnabled() { return fAutoRestart; }

	UInt32      GetTotalBytesUpdateTimeInSecs() { return fTBUpdateTimeInSecs; }
	UInt32      GetAvgBandwidthUpdateTimeInSecs() { return fABUpdateTimeInSecs; }
	UInt32      GetSafePlayDurationInSecs() { return fSafePlayDurationInSecs; }

	// For the compiled-in error logging module

	bool  IsErrorLogEnabled() { return fErrorLogEnabled; }
	bool  IsScreenLoggingEnabled() { return fScreenLoggingEnabled; }

	UInt32  GetMaxErrorLogBytes() { return fErrorLogBytes; }
	UInt32  GetErrorRollIntervalInDays() { return fErrorRollIntervalInDays; }
	UInt32  GetErrorLogVerbosity() { return fErrorLogVerbosity; }
	void    SetErrorLogVerbosity(UInt32 verbosity) { fErrorLogVerbosity = verbosity; }
	bool  GetAppendSrcAddrInTransport() { return fAppendSrcAddrInTransport; }

	//
	// For UDP retransmits
	UInt32  IsReliableUDPEnabled() { return fReliableUDP; }
	UInt32  GetMaxRetransmitDelayInMsec() { return fMaxRetransDelayInMsec; }
	bool  IsAckLoggingEnabled() { return fIsAckLoggingEnabled; }
	UInt32  GetRTCPPollIntervalInMsec() { return fRTCPPollIntervalInMsec; }
	UInt32  GetRTCPSocketRcvBufSizeinK() { return fRTCPSocketRcvBufSizeInK; }
	UInt32  GetSendIntervalInMsec() { return fSendIntervalInMsec; }
	UInt32  GetMaxSendAheadTimeInSecs() { return fMaxSendAheadTimeInSecs; }
	bool  IsSlowStartEnabled() { return fIsSlowStartEnabled; }
	bool  GetReliableUDPPrintfsEnabled() { return fReliableUDPPrintfs; }
	bool  GetRTSPDebugPrintfs() { return fEnableRTSPDebugPrintfs; }
	bool  GetRTSPServerInfoEnabled() { return fEnableRTSPServerInfo; }

	Float32    GetOverbufferRate() { return fOverbufferRate; }

	// RUDP window size
	UInt32  GetSmallWindowSizeInK() { return fSmallWindowSizeInK; }
	UInt32    GetMediumWindowSizeInK() { return fMediumWindowSizeInK; }
	UInt32  GetLargeWindowSizeInK() { return fLargeWindowSizeInK; }
	UInt32  GetWindowSizeThreshold() { return fWindowSizeThreshold; }
	UInt32    GetWindowSizeMaxThreshold() { return fWindowSizeMaxThreshold; }

	//
	// force logs to close after each write (true or false)
	bool  GetCloseLogsOnWrite() { return fCloseLogsOnWrite; }
	void    SetCloseLogsOnWrite(bool closeLogsOnWrite);

	//
	// Optionally require that reliable UDP content be in certain folders
	bool IsPathInsideReliableUDPDir(StrPtrLen* inPath);

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
	{
		return this->GetStringPref(qtssPrefsErrorLogDir);
	}
	char*   GetErrorLogName()
	{
		return this->GetStringPref(qtssPrefsErrorLogName);
	}

	char*   GetModuleDirectory()
	{
		return this->GetStringPref(qtssPrefsModuleFolder);
	}

	char*   GetAuthorizationRealm()
	{
		return this->GetStringPref(qtssPrefsDefaultAuthorizationRealm);
	}

	char*   GetRunUserName()
	{
		return this->GetStringPref(qtssPrefsRunUserName);
	}
	char*   GetRunGroupName()
	{
		return this->GetStringPref(qtssPrefsRunGroupName);
	}

	char*   GetPidFilePath()
	{
		return this->GetStringPref(qtssPrefsPidFile);
	}

	char*   GetStatsMonitorFileName()
	{
		return this->GetStringPref(qtssPrefsMonitorStatsFileName);
	}

	bool ServerStatFileEnabled() { return fEnableMonitorStatsFile; }
	UInt32 GetStatFileIntervalSec() { return fStatsFileIntervalSeconds; }
	bool CloudPlatformEnabled() { return fCloudPlatformEnabled; }
	QTSS_AuthScheme GetAuthScheme() { return fAuthScheme; }

	bool PacketHeaderPrintfsEnabled() { return fEnablePacketHeaderPrintfs; }
	bool PrintRTPHeaders() { return (bool)(fPacketHeaderPrintfOptions & kRTPALL); }
	bool PrintSRHeaders() { return (bool)(fPacketHeaderPrintfOptions & kRTCPSR); }
	bool PrintRRHeaders() { return (bool)(fPacketHeaderPrintfOptions & kRTCPRR); }
	bool PrintAPPHeaders() { return (bool)(fPacketHeaderPrintfOptions & kRTCPAPP); }
	bool PrintACKHeaders() { return (bool)(fPacketHeaderPrintfOptions & kRTCPACK); }

	UInt32 DeleteSDPFilesInterval() { return fsdp_file_delete_interval_seconds; }

	UInt32  GetNumThreads() { return fNumThreads; } //short tasks threads
	UInt32  GetNumBlockingThreads() { return fNumRTSPThreads; } //return the number of threads that long tasks will be scheduled on -- RTSP processing for example.

	bool  GetDisableThinning() { return fDisableThinning; }

	UInt16  GetDefaultStreamQuality() { return fDefaultStreamQuality; }
	bool  GetUDPMonitorEnabled() { return fUDPMonitorEnabled; }
	UInt16  GetUDPMonitorVideoPort() { return fUDPMonitorVideoPort; }
	UInt16  GetUDPMonitorAudioPort() { return fUDPMonitorAudioPort; }

	char* GetMonitorDestIP() { return this->GetStringPref(qtssPrefsUDPMonitorDestIPAddr); }

	char* GetMonitorSrcIP() { return this->GetStringPref(qtssPrefsUDPMonitorSourceIPAddr); }

	bool GetAllowGuestDefault() { return fAllowGuestAuthorizeDefault; }

	UInt16 GetServiceLanPort() { return fServiceLANPort; }
	UInt16 GetServiceWanPort() { return fServiceWANPort; }

	char* GetServiceWANIP() { return this->GetStringPref(easyPrefsServiceWANIPAddr); }
	UInt16 GetRTSPWANPort() const {	return fRTSPWANPort; }
	UINT16 GetRTMPWANPort()	const { return fRTMPWANPort; }

	char* GetNginxRootFolder() { return this->GetStringPref(qtssPrefsNginxRootFolder); }
	char* GetNginxWebPath() { return this->GetStringPref(easyPrefsNginxWebPath); }

private:

	UInt32      fRTSPTimeoutInSecs;
	char        fRTSPTimeoutBuf[20];
	StrPtrLen   fRTSPTimeoutString;
	UInt32      fRTSPSessionTimeoutInSecs;
	UInt32      fRTPSessionTimeoutInSecs;

	SInt32  fMaximumConnections;
	SInt32  fMaxBandwidthInKBits;

	bool  fBreakOnAssert;
	bool  fAutoRestart;
	UInt32  fTBUpdateTimeInSecs;
	UInt32  fABUpdateTimeInSecs;
	UInt32  fSafePlayDurationInSecs;

	UInt32  fErrorRollIntervalInDays;
	UInt32  fErrorLogBytes;
	UInt32  fErrorLogVerbosity;
	bool  fScreenLoggingEnabled;
	bool  fErrorLogEnabled;

	SInt32  fDropAllPacketsTimeInMsec;
	SInt32  fDropAllVideoPacketsTimeInMsec;
	SInt32  fThinAllTheWayTimeInMsec;
	SInt32  fAlwaysThinTimeInMsec;
	SInt32  fStartThinningTimeInMsec;
	SInt32  fStartThickingTimeInMsec;
	SInt32  fThickAllTheWayTimeInMsec;
	UInt32  fQualityCheckIntervalInMsec;

	UInt32  fMinTCPBufferSizeInBytes;
	UInt32  fMaxTCPBufferSizeInBytes;
	Float32 fTCPSecondsToBuffer;

	bool  fDoReportHTTPConnectionAddress;
	bool  fAppendSrcAddrInTransport;

	UInt32  fSmallWindowSizeInK;
	UInt32  fMediumWindowSizeInK;
	UInt32  fLargeWindowSizeInK;
	UInt32  fWindowSizeThreshold;
	UInt32  fWindowSizeMaxThreshold;

	UInt32  fMaxRetransDelayInMsec;
	bool  fIsAckLoggingEnabled;
	UInt32  fRTCPPollIntervalInMsec;
	UInt32  fRTCPSocketRcvBufSizeInK;
	bool  fIsSlowStartEnabled;
	UInt32  fSendIntervalInMsec;
	UInt32  fMaxSendAheadTimeInSecs;

	bool  fCloudPlatformEnabled;

	QTSS_AuthScheme fAuthScheme;
	UInt32  fsdp_file_delete_interval_seconds;
	bool  fAutoStart;
	bool  fReliableUDP;
	bool  fReliableUDPPrintfs;
	bool  fEnableRTSPErrMsg;
	bool  fEnableRTSPDebugPrintfs;
	bool  fEnableRTSPServerInfo;
	UInt32  fNumThreads;
	UInt32  fNumRTSPThreads;

	UInt16	fServiceLANPort;
	UInt16	fServiceWANPort;

	bool  fEnableMonitorStatsFile;
	UInt32  fStatsFileIntervalSeconds;

	Float32	fOverbufferRate;

	bool   fEnablePacketHeaderPrintfs;
	UInt32 fPacketHeaderPrintfOptions;
	bool   fCloseLogsOnWrite;

	bool   fDisableThinning;
	UInt16 fDefaultStreamQuality;
	bool   fUDPMonitorEnabled;
	UInt16 fUDPMonitorVideoPort;
	UInt16 fUDPMonitorAudioPort;
	char   fUDPMonitorDestAddr[20];
	char   fUDPMonitorSrcAddr[20];
	bool   fAllowGuestAuthorizeDefault;

	char   fRTSPWANAddr[20];
	UInt16 fRTSPWANPort;
	UINT16 fRTMPWANPort;

	enum //fPacketHeaderPrintfOptions
	{
		kRTPALL = 1 << 0,
		kRTCPSR = 1 << 1,
		kRTCPRR = 1 << 2,
		kRTCPAPP = 1 << 3,
		kRTCPACK = 1 << 4
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
	static char*    sRTP_Start_Time_Players[];
	static char*    sDisable_Rate_Adapt_Players[];
	static char*    sFixed_Target_Time_Players[];
	static char*    sDisable_Thinning_Players[];

};
#endif //__QTSSPREFS_H__
