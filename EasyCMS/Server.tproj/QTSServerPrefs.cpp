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
    File:       QTSSPrefs.cpp

    Contains:   Implements class defined in QTSSPrefs.h.

    Change History (most recent first):
    
*/

#include "QTSServerPrefs.h"
#include "MyAssert.h"
#include "OSMemory.h"
#include "QTSSDataConverter.h"
#include "defaultPaths.h"
#include "QTSSRollingLog.h"
 
#ifndef __Win32__
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
 

QTSServerPrefs::PrefInfo QTSServerPrefs::sPrefInfo[] =
{
    { kDontAllowMultipleValues, "0",        NULL                    },  //0 rtsp_timeout
    { kDontAllowMultipleValues, "90",      NULL                    },  //1 session_timeout
	{ kDontAllowMultipleValues,	"120",		NULL					},	//2 rtp_timeout
    { kDontAllowMultipleValues, "1000",     NULL                    },  //3 maximum_connections
    { kDontAllowMultipleValues, "102400",   NULL                    },  //4 maximum_bandwidth
	{ kDontAllowMultipleValues,	"127.0.0.1",NULL					},	//5 cms_ip_addr
    { kAllowMultipleValues,     "0",        NULL                    },  //6 bind_ip_addr
    { kDontAllowMultipleValues, "false",    NULL                    },  //7 break_on_assert
    { kDontAllowMultipleValues, "true",     NULL                    },  //8 auto_restart
    { kDontAllowMultipleValues, "1",        NULL                    },  //9 total_bytes_update
    { kDontAllowMultipleValues, "60",       NULL                    },  //10 average_bandwidth_update
    { kDontAllowMultipleValues, "600",      NULL                    },  //11 safe_play_duration
	{ kDontAllowMultipleValues,	DEFAULTPATHS_SSM_DIR,	NULL		},	//12 module_folder
    { kDontAllowMultipleValues, "Error",    NULL                    },  //13 error_logfile_name
	{ kDontAllowMultipleValues,	DEFAULTPATHS_LOG_DIR,	NULL		},	//14 error_logfile_dir
    { kDontAllowMultipleValues, "7",        NULL                    },  //15 error_logfile_interval
    { kDontAllowMultipleValues, "256000",   NULL                    },  //16 error_logfile_size
    { kDontAllowMultipleValues, "2",        NULL                    },  //17 error_logfile_verbosity
    { kDontAllowMultipleValues, "true",     NULL                    },  //18 screen_logging
    { kDontAllowMultipleValues, "true",     NULL                    },  //19 error_logging
    { kDontAllowMultipleValues, "CMS000",   NULL                    },  //20 service_id
    { kDontAllowMultipleValues, "0",        NULL                    },  //21 start_thinning_delay
    { kDontAllowMultipleValues, "64",       NULL                    },  //22 large_window_size
    { kDontAllowMultipleValues, "200",      NULL                    },  //23 window_size_threshold
    { kDontAllowMultipleValues, "8192",     NULL                    },  //24 min_tcp_buffer_size
	{ kDontAllowMultipleValues,	"200000",	NULL					},	//25 max_tcp_buffer_size
    { kDontAllowMultipleValues, ".5",       NULL                    },  //26 tcp_seconds_to_buffer
    { kDontAllowMultipleValues, "60000",	NULL                    },  //27 cms_port
    { kDontAllowMultipleValues, "127.0.0.1",NULL					},  //28 redis_ip_addr
#ifndef __Win32__
    { kDontAllowMultipleValues, "qtss",     NULL                    },  //29 run_user_name
    { kDontAllowMultipleValues, "qtss",     NULL                    },  //30 run_group_name
#else
    { kDontAllowMultipleValues, "",         NULL                    },  //29 run_user_name
    { kDontAllowMultipleValues, "",         NULL                    },  //30 run_group_name
#endif
    { kDontAllowMultipleValues, "false",    NULL                    },  //31 append_source_addr_in_transport
    { kDontAllowMultipleValues, "6379",     NULL					},	//32 redis_port
    { kDontAllowMultipleValues, "500",      NULL                    },  //33 max_retransmit_delay
    { kDontAllowMultipleValues, "24",       NULL                    },  //34 small_window_size
    { kDontAllowMultipleValues, "false",    NULL                    },  //35 ack_logging_enabled
    { kDontAllowMultipleValues, "./snap/",      NULL                    },  //36 snap_local_path
    { kDontAllowMultipleValues, "http://cms.easydarwin.org/",      NULL                    },  //37 snap_web_path
    { kDontAllowMultipleValues, "50",       NULL                    },  //38 send_interval
    { kDontAllowMultipleValues, "-2000",    NULL                    },  //39 thick_all_the_way_delay
    { kDontAllowMultipleValues, "",         NULL                    },  //40 alt_transport_src_ipaddr
    { kDontAllowMultipleValues, "25",       NULL                    },  //41 max_send_ahead_time
    { kDontAllowMultipleValues, "digest",   NULL                    },  //42 authentication_scheme
    { kDontAllowMultipleValues, "false",    NULL                    },  //43 auto_start

    { kDontAllowMultipleValues, "false",    NULL                    },  //44 MSG_debug_printfs
#if __MacOSX__
    { kDontAllowMultipleValues, "false",     NULL                    },  //45 enable_monitor_stats_file
#else
    { kDontAllowMultipleValues, "false",    NULL                    },  //45 enable_monitor_stats_file
#endif
    { kDontAllowMultipleValues, "10",        NULL                   },  //46 monitor_stats_file_interval_seconds
    { kDontAllowMultipleValues, "server_status",        NULL        },  //47 monitor_stats_file_name


	{ kDontAllowMultipleValues, "2.0",		NULL					},	//48 overbuffer_rate
	{ kDontAllowMultipleValues,	"48",		NULL					},	//49 medium_window_size
	{ kDontAllowMultipleValues,	"1000",		NULL					},	//50 window_size_max_threshold
    { kDontAllowMultipleValues, "true",     NULL                    },  //51 CMS_server_info
	{ kDontAllowMultipleValues, "0",        NULL                    },  //52 run_num_threads
    { kDontAllowMultipleValues, DEFAULTPATHS_PID_DIR PLATFORM_SERVER_BIN_NAME ".pid",	NULL	},	//53 pid_file
    { kDontAllowMultipleValues, "false",    NULL                    },   //54 force_logs_close_on_write
    { kDontAllowMultipleValues, "false",    NULL                    },   //55 disable_thinning
    { kDontAllowMultipleValues, "10000",    NULL                     }, //56 monitor_lan_port
    { kDontAllowMultipleValues, "10000",    NULL                     }, //57 monitor_wan_port
    { kDontAllowMultipleValues, "127.0.0.1",NULL                     }, //58 monitor_lan_ip
    { kDontAllowMultipleValues, "0.0.0.0",  NULL                     }, //59 monitor_wan_ip
    { kDontAllowMultipleValues, "true",     NULL                     }, //60 enable_allow_guest_default
    { kDontAllowMultipleValues, "2",        NULL                     }, //61 run_num_msg_threads
	{ kDontAllowMultipleValues , "121.41.73.249", NULL				 }, //62 dss_ip
	{ kDontAllowMultipleValues , "554",		NULL					 }	//63 dss_port
};
 
QTSSAttrInfoDict::AttrInfo  QTSServerPrefs::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0 */ { "rtsp_timeout",                           NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 1 */ { "session_timeout",						NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 2 */ { "rtp_timeout",                            NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 3 */ { "maximum_connections",                    NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 4 */ { "maximum_bandwidth",                      NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 5 */ { "cms_ip_addr",							NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 6 */ { "bind_ip_addr",                           NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 7 */ { "break_on_assert",                        NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 8 */ { "auto_restart",                           NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 9 */ { "total_bytes_update",                     NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 10 */ { "average_bandwidth_update",              NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 11 */ { "safe_play_duration",                    NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 12 */ { "module_folder",                         NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 13 */ { "error_logfile_name",                    NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 14 */ { "error_logfile_dir",                     NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 15 */ { "error_logfile_interval",                NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 16 */ { "error_logfile_size",                    NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 17 */ { "error_logfile_verbosity",               NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 18 */ { "screen_logging",                        NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 19 */ { "error_logging",                         NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 20 */ { "service_id",							NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 21 */ { "start_thinning_delay",                  NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 22 */ { "large_window_size",                     NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 23 */ { "window_size_threshold",                 NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 24 */ { "min_tcp_buffer_size",                   NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 25 */ { "max_tcp_buffer_size",                   NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 26 */ { "tcp_seconds_to_buffer",                 NULL,                   qtssAttrDataTypeFloat32,    qtssAttrModeRead | qtssAttrModeWrite },
    /* 27 */ { "cms_port",								NULL,					qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 28 */ { "redis_ip_addr",							NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 29 */ { "run_user_name",                         NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 30 */ { "run_group_name",                        NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 31 */ { "append_source_addr_in_transport",       NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 32 */ { "redis_port",							NULL,                   qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 33 */ { "max_retransmit_delay",                  NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 34 */ { "small_window_size",                     NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 35 */ { "ack_logging_enabled",                   NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 36 */ { "snap_local_path",						NULL,                   qtssAttrDataTypeCharArray,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 37 */ { "snap_web_path",							NULL,                   qtssAttrDataTypeCharArray,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 38 */ { "send_interval",                         NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 39 */ { "thick_all_the_way_delay",               NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 40 */ { "alt_transport_src_ipaddr",              NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 41 */ { "max_send_ahead_time",                   NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },

	/* 42 */ { "authentication_scheme",                 NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 43 */ { "auto_start",                            NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 44 */ { "MSG_debug_printfs",						NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 45 */ { "enable_monitor_stats_file",             NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 46 */ { "monitor_stats_file_interval_seconds",   NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 47 */ { "monitor_stats_file_name",               NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },

	/* 48 */ { "overbuffer_rate",						NULL,					qtssAttrDataTypeFloat32,	qtssAttrModeRead | qtssAttrModeWrite },
	/* 49 */ { "medium_window_size",					NULL,					qtssAttrDataTypeUInt32,		qtssAttrModeRead | qtssAttrModeWrite },
	/* 50 */ { "window_size_max_threshold",				NULL,					qtssAttrDataTypeUInt32,		qtssAttrModeRead | qtssAttrModeWrite },
    /* 51 */ { "CMS_server_info",                      NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 52 */ { "run_num_threads",                       NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 53 */ { "pid_file",								NULL,					qtssAttrDataTypeCharArray,	qtssAttrModeRead | qtssAttrModeWrite },
    /* 54 */ { "force_logs_close_on_write",             NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 55 */ { "disable_thinning",                      NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 56 */ { "monitor_lan_port",		NULL,									qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 57 */ { "monitor_wan_port",		NULL,									qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 58 */ { "monitor_lan_ip",		NULL,                                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 59 */ { "monitor_wan_ip",		NULL,                                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 60 */ { "enable_allow_guest_default",  NULL,                             qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 61 */ { "run_num_msg_threads",  NULL,									qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 62 */ {"dss_ip",					NULL,									qtssAttrDataTypeCharArray,	qtssAttrModeRead | qtssAttrModeWrite },
	/* 63 */ {"dss_port",				NULL,									qtssAttrDataTypeUInt16,		qtssAttrModeRead | qtssAttrModeWrite }
};


QTSServerPrefs::QTSServerPrefs(XMLPrefsParser* inPrefsSource, Bool16 inWriteMissingPrefs)
:   QTSSPrefs(inPrefsSource, NULL, QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kPrefsDictIndex), false),
    fRTSPTimeoutInSecs(0),
    fRTSPTimeoutString(fRTSPTimeoutBuf, 0),
    fSessionTimeoutInSecs(0),
    fRTPTimeoutInSecs(0),
    fMaximumConnections(0),
    fMaxBandwidthInKBits(0),
    fBreakOnAssert(false),
    fAutoRestart(false),
    fTBUpdateTimeInSecs(0),
    fABUpdateTimeInSecs(0),
    fSafePlayDurationInSecs(0),
    fErrorRollIntervalInDays(0),
    fErrorLogBytes(0),
    fErrorLogVerbosity(0),
    fScreenLoggingEnabled(true),
    fErrorLogEnabled(false),
    fMinTCPBufferSizeInBytes(0),
    fMaxTCPBufferSizeInBytes(0),
    fTCPSecondsToBuffer(0),
    fCMSPort(0),
    fAppendSrcAddrInTransport(false),
    fSmallWindowSizeInK(0),
	fMediumWindowSizeInK(0),
    fLargeWindowSizeInK(0),
    fWindowSizeThreshold(0),
	fWindowSizeMaxThreshold(0),
    fMaxRetransDelayInMsec(0),
    fIsAckLoggingEnabled(false),
    fSendIntervalInMsec(0),
    fMaxSendAheadTimeInSecs(0),
    fAuthScheme(qtssAuthDigest),
    fAutoStart(false),
    fEnableMSGDebugPrintfs(false),
    fEnableCMSServerInfo(true),
    fNumThreads(0),
    fNumMsgThreads(0),
#if __MacOSX__
    fEnableMonitorStatsFile(false),
#else
    fEnableMonitorStatsFile(false),
#endif 
    fStatsFileIntervalSeconds(10),
	fOverbufferRate(0.0),
    fCloseLogsOnWrite(false),
    fDisableThinning(false),
	fMonitorLANPort(0),
	fMonitorWANPort(0),
	fAllowGuestAuthorizeDefault(true)
{
    SetupAttributes();
    RereadServerPreferences(inWriteMissingPrefs);
}

void QTSServerPrefs::Initialize()
{
    for (UInt32 x = 0; x < qtssPrefsNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kPrefsDictIndex)->
            SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
                            sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);
}


void QTSServerPrefs::SetupAttributes()
{
    this->SetVal(qtssPrefsRTSPTimeout,      &fRTSPTimeoutInSecs,        sizeof(fRTSPTimeoutInSecs));
    this->SetVal(qtssPrefsSessionTimeout,	&fSessionTimeoutInSecs,    sizeof(fSessionTimeoutInSecs));
    this->SetVal(qtssPrefsRTPTimeout,       &fRTPTimeoutInSecs,         sizeof(fRTPTimeoutInSecs));
    this->SetVal(qtssPrefsMaximumConnections,&fMaximumConnections,      sizeof(fMaximumConnections));
    this->SetVal(qtssPrefsMaximumBandwidth, &fMaxBandwidthInKBits,      sizeof(fMaxBandwidthInKBits));
    this->SetVal(qtssPrefsBreakOnAssert,    &fBreakOnAssert,            sizeof(fBreakOnAssert));
    this->SetVal(qtssPrefsAutoRestart,      &fAutoRestart,              sizeof(fAutoRestart));
    this->SetVal(qtssPrefsTotalBytesUpdate, &fTBUpdateTimeInSecs,       sizeof(fTBUpdateTimeInSecs));
    this->SetVal(qtssPrefsAvgBandwidthUpdate,&fABUpdateTimeInSecs,      sizeof(fABUpdateTimeInSecs));
    this->SetVal(qtssPrefsSafePlayDuration, &fSafePlayDurationInSecs,   sizeof(fSafePlayDurationInSecs));

    this->SetVal(qtssPrefsErrorRollInterval, &fErrorRollIntervalInDays, sizeof(fErrorRollIntervalInDays));
    this->SetVal(qtssPrefsMaxErrorLogSize,  &fErrorLogBytes,            sizeof(fErrorLogBytes));
    this->SetVal(qtssPrefsErrorLogVerbosity, &fErrorLogVerbosity,       sizeof(fErrorLogVerbosity));
    this->SetVal(qtssPrefsScreenLogging,    &fScreenLoggingEnabled,     sizeof(fScreenLoggingEnabled));
    this->SetVal(qtssPrefsErrorLogEnabled,  &fErrorLogEnabled,          sizeof(fErrorLogEnabled));

    this->SetVal(qtssPrefsMinTCPBufferSizeInBytes,  &fMinTCPBufferSizeInBytes,  sizeof(fMinTCPBufferSizeInBytes));
    this->SetVal(qtssPrefsMaxTCPBufferSizeInBytes,  &fMaxTCPBufferSizeInBytes,  sizeof(fMaxTCPBufferSizeInBytes));
    this->SetVal(qtssPrefsTCPSecondsToBuffer,   &fTCPSecondsToBuffer,           sizeof(fTCPSecondsToBuffer));

    this->SetVal(qtssPrefsCMSPort,				&fCMSPort,						sizeof(fCMSPort));
    this->SetVal(qtssPrefsSrcAddrInTransport,   &fAppendSrcAddrInTransport, sizeof(fAppendSrcAddrInTransport));

    this->SetVal(qtssPrefsSmallWindowSizeInK,   &fSmallWindowSizeInK,       sizeof(fSmallWindowSizeInK));
	this->SetVal(qtssPrefsMediumWindowSizeInK, 	&fMediumWindowSizeInK,   sizeof(fMediumWindowSizeInK));
    this->SetVal(qtssPrefsLargeWindowSizeInK,   &fLargeWindowSizeInK,       sizeof(fLargeWindowSizeInK));
    this->SetVal(qtssPrefsWindowSizeThreshold,  &fWindowSizeThreshold,      sizeof(fWindowSizeThreshold));
	this->SetVal(qtssPrefsWindowSizeMaxThreshold,	&fWindowSizeMaxThreshold,   sizeof(fWindowSizeMaxThreshold));

    this->SetVal(qtssPrefsMaxRetransDelayInMsec,    &fMaxRetransDelayInMsec,    sizeof(fMaxRetransDelayInMsec));
    this->SetVal(qtssPrefsAckLoggingEnabled,        &fIsAckLoggingEnabled,      sizeof(fIsAckLoggingEnabled));
    this->SetVal(qtssPrefsSendInterval,             &fSendIntervalInMsec,       sizeof(fSendIntervalInMsec));
    this->SetVal(qtssPrefsMaxAdvanceSendTimeInSec,  &fMaxSendAheadTimeInSecs,   sizeof(fMaxSendAheadTimeInSecs));
    this->SetVal(qtssPrefsAutoStart,                &fAutoStart,                sizeof(fAutoStart));

    this->SetVal(qtssPrefsEnableMSGDebugPrintfs,		&fEnableMSGDebugPrintfs,		sizeof(fEnableMSGDebugPrintfs));
    this->SetVal(qtssPrefsEnableCMSServerInfo,         &fEnableCMSServerInfo,         sizeof(fEnableCMSServerInfo));
    this->SetVal(qtssPrefsRunNumThreads,                &fNumThreads,                   sizeof(fNumThreads));
    this->SetVal(qtssPrefsEnableMonitorStatsFile,       &fEnableMonitorStatsFile,       sizeof(fEnableMonitorStatsFile));
    this->SetVal(qtssPrefsMonitorStatsFileIntervalSec,  &fStatsFileIntervalSeconds,     sizeof(fStatsFileIntervalSeconds));

    this->SetVal(qtssPrefsCloseLogsOnWrite,             &fCloseLogsOnWrite,             sizeof(fCloseLogsOnWrite));
	this->SetVal(qtssPrefsOverbufferRate,				&fOverbufferRate,				sizeof(fOverbufferRate));
    this->SetVal(qtssPrefsDisableThinning,              &fDisableThinning,              sizeof(fDisableThinning));
	
    this->SetVal(qtssPrefsMonitorLANPort,				&fMonitorLANPort,          sizeof(fMonitorLANPort));
    this->SetVal(qtssPrefsMonitorWANPort,				&fMonitorWANPort,          sizeof(fMonitorWANPort));
    this->SetVal(qtssPrefsMonitorLANIPAddr,				&fMonitorLANAddr,           sizeof(fMonitorLANAddr));
    this->SetVal(qtssPrefsMonitorWANIPAddr,				&fMonitorWANAddr,            sizeof(fMonitorWANAddr));

	this->SetVal(qtssPrefsServiceID,					&fServiceID,            sizeof(fServiceID));

	this->SetVal(qtssPrefsRedisIPAddr,					&fRedisAddr,            sizeof(fRedisAddr));
	this->SetVal(qtssPrefsRedisPorts,					&fRedisPort,          sizeof(fRedisPort));
	
    this->SetVal(qtssPrefsEnableAllowGuestDefault,      &fAllowGuestAuthorizeDefault,   sizeof(fAllowGuestAuthorizeDefault)); //enable_allow_guest_authorize_default
    this->SetVal(qtssPrefsNumMsgThreads,				&fNumMsgThreads,               sizeof(fNumMsgThreads));
	
	this->SetVal(qtssPrefsDssIP, &fDssIP, sizeof(fDssIP));
	this->SetVal(qtssPrefsDssPort, &fDssPort, sizeof(fDssPort));
}



void QTSServerPrefs::RereadServerPreferences(Bool16 inWriteMissingPrefs)
{
    OSMutexLocker locker(&fPrefsMutex);
    QTSSDictionaryMap* theMap = QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kPrefsDictIndex);
    
    for (UInt32 x = 0; x < theMap->GetNumAttrs(); x++)
    {
        //
        // Look for a pref in the file that matches each pref in the dictionary
        char* thePrefTypeStr = NULL;
        char* thePrefName = NULL;
        
        ContainerRef server = fPrefsSource->GetRefForServer();
        ContainerRef pref = fPrefsSource->GetPrefRefByName( server, theMap->GetAttrName(x) );
        char* thePrefValue = NULL;
        if (pref != NULL)
            thePrefValue = fPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,
                                                                    (char**)&thePrefTypeStr);
        
        if ((thePrefValue == NULL) && (x < qtssPrefsNumParams)) // Only generate errors for server prefs
        {
            //
            // There is no pref, use the default and log an error
            if (::strlen(sPrefInfo[x].fDefaultValue) > 0)
            {
                //
                // Only log this as an error if there is a default (an empty string
                // doesn't count). If there is no default, we will constantly print
                // out an error message...
                QTSSModuleUtils::LogError(  QTSSModuleUtils::GetMisingPrefLogVerbosity(),
                                            qtssServerPrefMissing,
                                            0,
                                            sAttributes[x].fAttrName,
                                            sPrefInfo[x].fDefaultValue);
            }
            
            this->SetPrefValue(x, 0, sPrefInfo[x].fDefaultValue, sAttributes[x].fAttrDataType);
            if (sPrefInfo[x].fAdditionalDefVals != NULL)
            {
                //
                // Add additional default values if they exist
                for (UInt32 y = 0; sPrefInfo[x].fAdditionalDefVals[y] != NULL; y++)
                    this->SetPrefValue(x, y+1, sPrefInfo[x].fAdditionalDefVals[y], sAttributes[x].fAttrDataType);
            }
            
            if (inWriteMissingPrefs)
            {
                //
                // Add this value into the file, cuz we need it.
                pref = fPrefsSource->AddPref( server, sAttributes[x].fAttrName, QTSSDataConverter::TypeToTypeString(sAttributes[x].fAttrDataType));
                fPrefsSource->AddPrefValue(pref, sPrefInfo[x].fDefaultValue);
                
                if (sPrefInfo[x].fAdditionalDefVals != NULL)
                {
                    for (UInt32 a = 0; sPrefInfo[x].fAdditionalDefVals[a] != NULL; a++)
                        fPrefsSource->AddPrefValue(pref, sPrefInfo[x].fAdditionalDefVals[a]);
                }
            }
            continue;
        }
        
        QTSS_AttrDataType theType = QTSSDataConverter::TypeStringToType(thePrefTypeStr);
        
        if ((x < qtssPrefsNumParams) && (theType != sAttributes[x].fAttrDataType)) // Only generate errors for server prefs
        {
            //
            // The pref in the file has the wrong type, use the default and log an error
            
            if (::strlen(sPrefInfo[x].fDefaultValue) > 0)
            {
                //
                // Only log this as an error if there is a default (an empty string
                // doesn't count). If there is no default, we will constantly print
                // out an error message...
                QTSSModuleUtils::LogError(  qtssWarningVerbosity,
                                            qtssServerPrefWrongType,
                                            0,
                                            sAttributes[x].fAttrName,
                                            sPrefInfo[x].fDefaultValue);
            }
            
            this->SetPrefValue(x, 0, sPrefInfo[x].fDefaultValue, sAttributes[x].fAttrDataType);
            if (sPrefInfo[x].fAdditionalDefVals != NULL)
            {
                //
                // Add additional default values if they exist
                for (UInt32 z = 0; sPrefInfo[x].fAdditionalDefVals[z] != NULL; z++)
                    this->SetPrefValue(x, z+1, sPrefInfo[x].fAdditionalDefVals[z], sAttributes[x].fAttrDataType);
            }

            if (inWriteMissingPrefs)
            {
                //
                // Remove it out of the file and add in the default.
                fPrefsSource->RemovePref(pref);
                pref = fPrefsSource->AddPref( server, sAttributes[x].fAttrName, QTSSDataConverter::TypeToTypeString(sAttributes[x].fAttrDataType));
                fPrefsSource->AddPrefValue(pref, sPrefInfo[x].fDefaultValue);
                if (sPrefInfo[x].fAdditionalDefVals != NULL)
                {
                    for (UInt32 b = 0; sPrefInfo[x].fAdditionalDefVals[b] != NULL; b++)
                        fPrefsSource->AddPrefValue(pref, sPrefInfo[x].fAdditionalDefVals[b]);
                }
            }
            continue;
        }
        
        UInt32 theNumValues = 0;
        if ((x < qtssPrefsNumParams) && (!sPrefInfo[x].fAllowMultipleValues))
            theNumValues = 1;
            
        this->SetPrefValuesFromFileWithRef(pref, x, theNumValues);
    }
    
    //
    // Do any special pref post-processing
    this->UpdateAuthScheme();
    
    QTSSRollingLog::SetCloseOnWrite(fCloseLogsOnWrite);
    //
    // In case we made any changes, write out the prefs file
    (void)fPrefsSource->WritePrefsFile();
}

void    QTSServerPrefs::UpdateAuthScheme()
{
    static StrPtrLen sNoAuthScheme("none");
    static StrPtrLen sBasicAuthScheme("basic");
    static StrPtrLen sDigestAuthScheme("digest");
    
    // Get the auth scheme attribute
    StrPtrLen* theAuthScheme = this->GetValue(qtssPrefsAuthenticationScheme);
    
    if (theAuthScheme->Equal(sNoAuthScheme))
        fAuthScheme = qtssAuthNone;
    else if (theAuthScheme->Equal(sBasicAuthScheme))
        fAuthScheme = qtssAuthBasic;
    else if (theAuthScheme->Equal(sDigestAuthScheme))
        fAuthScheme = qtssAuthDigest;
}

//char*   QTSServerPrefs::GetMovieFolder(char* inBuffer, UInt32* ioLen)
//{
//    OSMutexLocker locker(&fPrefsMutex);
//
//    // Get the movie folder attribute
//    StrPtrLen* theMovieFolder = this->GetValue(qtssPrefsCMSIPAddr);
//
//    // If the movie folder path fits inside the provided buffer, copy it there
//    if (theMovieFolder->Len < *ioLen)
//        ::memcpy(inBuffer, theMovieFolder->Ptr, theMovieFolder->Len);
//    else
//    {
//        // Otherwise, allocate a buffer to store the path
//        inBuffer = NEW char[theMovieFolder->Len + 2];
//        ::memcpy(inBuffer, theMovieFolder->Ptr, theMovieFolder->Len);
//    }
//    inBuffer[theMovieFolder->Len] = 0;
//    *ioLen = theMovieFolder->Len;
//    return inBuffer;
//}

void QTSServerPrefs::GetTransportSrcAddr(StrPtrLen* ioBuf)
{
    OSMutexLocker locker(&fPrefsMutex);

    // Get the movie folder attribute
    StrPtrLen* theTransportAddr = this->GetValue(qtssPrefsAltTransportIPAddr);

    // If the movie folder path fits inside the provided buffer, copy it there
    if ((theTransportAddr->Len > 0) && (theTransportAddr->Len < ioBuf->Len))
    {
        ::memcpy(ioBuf->Ptr, theTransportAddr->Ptr, theTransportAddr->Len);
        ioBuf->Len = theTransportAddr->Len;
    }
    else
        ioBuf->Len = 0;
}

char* QTSServerPrefs::GetStringPref(QTSS_AttributeID inAttrID)
{
    StrPtrLen theBuffer;
    (void)this->GetValue(inAttrID, 0, NULL, &theBuffer.Len);
    theBuffer.Ptr = NEW char[theBuffer.Len + 1];
    theBuffer.Ptr[0] = '\0';
    
    if (theBuffer.Len > 0)
    {
        QTSS_Error theErr = this->GetValue(inAttrID, 0, theBuffer.Ptr, &theBuffer.Len);
        if (theErr == QTSS_NoErr)
            theBuffer.Ptr[theBuffer.Len] = 0;
    }
    return theBuffer.Ptr;
}

void QTSServerPrefs::SetCloseLogsOnWrite(Bool16 closeLogsOnWrite) 
{
    QTSSRollingLog::SetCloseOnWrite(closeLogsOnWrite);
    fCloseLogsOnWrite = closeLogsOnWrite; 
}

