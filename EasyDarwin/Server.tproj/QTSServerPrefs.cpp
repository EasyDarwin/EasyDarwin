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
#include <QTSSModuleUtils.h>

#ifndef __Win32__
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


char* QTSServerPrefs::sAdditionalDefaultPorts[] =
{
	"8554",
	NULL
};

char* QTSServerPrefs::sRTP_Header_Players[] =
{
	"vlc",
	NULL
};

char* QTSServerPrefs::sAdjust_Bandwidth_Players[] =
{
	"vlc",
	NULL
};

char* QTSServerPrefs::sNo_Pause_Time_Adjustment_Players[] =
{
	"vlc",
	"EasyPlayer",
	NULL
};

char* QTSServerPrefs::sRTP_Start_Time_Players[] =
{
	NULL
};

char* QTSServerPrefs::sDisable_Rate_Adapt_Players[] =
{
	NULL
};

char* QTSServerPrefs::sFixed_Target_Time_Players[] =
{
	NULL
};

char* QTSServerPrefs::sDisable_Thinning_Players[] =
{
	NULL
};

QTSServerPrefs::PrefInfo QTSServerPrefs::sPrefInfo[] =
{
	{ kDontAllowMultipleValues, "0",        NULL                    },  //rtsp_timeout
	{ kDontAllowMultipleValues, "180",      NULL                    },  //real_rtsp_timeout
	{ kDontAllowMultipleValues,	"120",		NULL					},	//rtp_timeout
	{ kDontAllowMultipleValues, "1000",     NULL                    },  //maximum_connections
	{ kDontAllowMultipleValues, "102400",   NULL                    },  //maximum_bandwidth
	{ kDontAllowMultipleValues,	DEFAULTPATHS_MOVIES_DIR, NULL       },	//movie_folder
	{ kAllowMultipleValues,     "0",        NULL                    },  //bind_ip_addr
	{ kDontAllowMultipleValues, "false",    NULL                    },  //break_on_assert
	{ kDontAllowMultipleValues, "true",     NULL                    },  //auto_restart
	{ kDontAllowMultipleValues, "1",        NULL                    },  //total_bytes_update
	{ kDontAllowMultipleValues, "60",       NULL                    },  //average_bandwidth_update
	{ kDontAllowMultipleValues, "600",      NULL                    },  //safe_play_duration
	{ kDontAllowMultipleValues,	DEFAULTPATHS_SSM_DIR,	NULL		},	//module_folder
	{ kDontAllowMultipleValues, "Error",    NULL                    },  //error_logfile_name
	{ kDontAllowMultipleValues,	DEFAULTPATHS_LOG_DIR,	NULL		},	//error_logfile_dir
	{ kDontAllowMultipleValues, "7",        NULL                    },  //error_logfile_interval
	{ kDontAllowMultipleValues, "256000",   NULL                    },  //error_logfile_size
	{ kDontAllowMultipleValues, "2",        NULL                    },  //error_logfile_verbosity
	{ kDontAllowMultipleValues, "true",     NULL                    },  //screen_logging
	{ kDontAllowMultipleValues, "true",     NULL                    },  //error_logging
	{ kDontAllowMultipleValues, "1750",     NULL                    },  //drop_all_video_delay
	{ kDontAllowMultipleValues, "0",        NULL                    },  //start_thinning_delay
	{ kDontAllowMultipleValues, "64",       NULL                    },  //large_window_size
	{ kDontAllowMultipleValues, "200",      NULL                    },  //window_size_threshold
	{ kDontAllowMultipleValues, "8192",     NULL                    },  //min_tcp_buffer_size
	{ kDontAllowMultipleValues,	"200000",	NULL					},	//max_tcp_buffer_size
	{ kDontAllowMultipleValues, ".5",       NULL                    },  //tcp_seconds_to_buffer
	{ kDontAllowMultipleValues, "false",    NULL                    },  //do_report_http_connection_ip_address
	{ kDontAllowMultipleValues, "Streaming Server", NULL            },  //default_authorization_realm
#ifndef __Win32__
	{ kDontAllowMultipleValues, "qtss",     NULL                    },  //run_user_name
	{ kDontAllowMultipleValues, "qtss",     NULL                    },  //run_group_name
#else
	{ kDontAllowMultipleValues, "",         NULL                    },  //run_user_name
	{ kDontAllowMultipleValues, "",         NULL                    },  //run_group_name
#endif
	{ kDontAllowMultipleValues, "false",    NULL                    },  //append_source_addr_in_transport
	{ kAllowMultipleValues,     "554",      sAdditionalDefaultPorts },  //rtsp_ports
	{ kDontAllowMultipleValues, "500",      NULL                    },  //max_retransmit_delay
	{ kDontAllowMultipleValues, "24",       NULL                    },  //small_window_size
	{ kDontAllowMultipleValues, "false",    NULL                    },  //ack_logging_enabled
	{ kDontAllowMultipleValues, "100",      NULL                    },  //rtcp_poll_interval
	{ kDontAllowMultipleValues, "768",      NULL                    },  //rtcp_rcv_buf_size
	{ kDontAllowMultipleValues, "50",       NULL                    },  //send_interval
	{ kDontAllowMultipleValues, "-2000",    NULL                    },  //thick_all_the_way_delay
	{ kDontAllowMultipleValues, "",         NULL                    },  //alt_transport_src_ipaddr
	{ kDontAllowMultipleValues, "25",       NULL                    },  //max_send_ahead_time
	{ kDontAllowMultipleValues, "true",     NULL                    },  //reliable_udp_slow_start
	{ kDontAllowMultipleValues, "false",    NULL                    },  //enable_cloud_platform
	{ kDontAllowMultipleValues, "digest",   NULL                    },  //authentication_scheme
	{ kDontAllowMultipleValues, "10",       NULL                    },  //sdp_file_delete_interval_seconds
	{ kDontAllowMultipleValues, "false",    NULL                    },  //auto_start
	{ kDontAllowMultipleValues, "true",     NULL                    },  //reliable_udp
	{ kAllowMultipleValues,	DEFAULTPATHS_DIRECTORY_SEPARATOR,	NULL},	//reliable_udp_dirs (set all dirs)
	{ kDontAllowMultipleValues, "false",    NULL                    },  //reliable_udp_printfs
	{ kDontAllowMultipleValues, "2500",     NULL                    },  //drop_all_packets_delay
	{ kDontAllowMultipleValues, "1500",     NULL                    },  //thin_all_the_way_delay
	{ kDontAllowMultipleValues, "750",      NULL                    },  //always_thin_delay
	{ kDontAllowMultipleValues, "250",      NULL                    },  //start_thicking_delay
	{ kDontAllowMultipleValues, "1000",     NULL                    },  //quality_check_interval
	{ kDontAllowMultipleValues, "false",    NULL                    },  //RTSP_error_message
	{ kDontAllowMultipleValues, "false",    NULL                    },  //RTSP_debug_printfs
#if __MacOSX__
	{ kDontAllowMultipleValues, "false",     NULL                    },  //enable_monitor_stats_file
#else
	{ kDontAllowMultipleValues, "false",    NULL                    },  //enable_monitor_stats_file
#endif
	{ kDontAllowMultipleValues, "10",        NULL                   },  //monitor_stats_file_interval_seconds
	{ kDontAllowMultipleValues, "server_status",        NULL        },   //monitor_stats_file_name
	{ kDontAllowMultipleValues, "false",    NULL                    },  //enable_packet_header_printfs
	{ kDontAllowMultipleValues, "rtp;rr;sr;app;ack;",NULL           },  //packet_header_printf_options
	{ kDontAllowMultipleValues, "2.0",		NULL					},	//overbuffer_rate
	{ kDontAllowMultipleValues,	"48",		NULL					},	//medium_window_size
	{ kDontAllowMultipleValues,	"1000",		NULL					},	//window_size_max_threshold
	{ kDontAllowMultipleValues, "true",     NULL                    },  //RTSP_server_info
	{ kDontAllowMultipleValues, "0",        NULL                    },  //run_num_threads
	{ kDontAllowMultipleValues, DEFAULTPATHS_PID_DIR PLATFORM_SERVER_BIN_NAME ".pid",	NULL	},	//pid_file
	{ kDontAllowMultipleValues, "false",    NULL                    },   //force_logs_close_on_write
	{ kDontAllowMultipleValues, "false",    NULL                    },   //disable_thinning
	{ kAllowMultipleValues,     "Android",    sRTP_Header_Players     },  //player_requires_rtp_header_info
	{ kAllowMultipleValues,     "Android",    sAdjust_Bandwidth_Players     },  //player_requires_bandwidth_adjustment
	{ kAllowMultipleValues,     "Android",    sNo_Pause_Time_Adjustment_Players     },  //player_requires_no_pause_time_adjustment
	{ kDontAllowMultipleValues, "true",     NULL                     }, //enable_3gpp_protocol
	{ kDontAllowMultipleValues, "true",     NULL                     }, //enable_3gpp_protocol_rate_adaptation
	{ kDontAllowMultipleValues, "1",        NULL                     }, //3gpp_protocol_rate_adaptation_report_frequency
	{ kDontAllowMultipleValues, "0",        NULL                     }, //default_stream_quality
	{ kAllowMultipleValues,     "Real",     sRTP_Start_Time_Players  }, //player_requires_rtp_start_time_adjust
	{ kDontAllowMultipleValues, "false",    NULL                     }, //enable_3gpp_debug_printfs
	{ kDontAllowMultipleValues, "false",    NULL                     }, //enable_udp_monitor_stream
	{ kDontAllowMultipleValues, "5002",     NULL                     }, //udp_monitor_video_stream
	{ kDontAllowMultipleValues, "5004",     NULL                     }, //udp_monitor_audio_stream
	{ kDontAllowMultipleValues, "127.0.0.1",NULL                     }, //udp_monitor_dest_ip
	{ kDontAllowMultipleValues, "0.0.0.0",  NULL                     }, //udp_monitor_src_ip
	{ kDontAllowMultipleValues, "true",     NULL                     }, //enable_allow_guest_default
	{ kDontAllowMultipleValues, "1",        NULL                     },  //run_num_rtsp_threads
	{ kAllowMultipleValues,     "",         sDisable_Rate_Adapt_Players }, //player_requires_disable_3gpp_rate_adapt
	{ kAllowMultipleValues,     "",         sFixed_Target_Time_Players  }, //player_requires_3gpp_target_time
	{ kDontAllowMultipleValues, "3000",     NULL                        }, //3gpp_target_time_milliseconds
	{ kAllowMultipleValues,     "",         sDisable_Thinning_Players   }, //player_requires_disable_thinning

	{ kDontAllowMultipleValues, "8080",     NULL                        }  //http_service_port   

};

QTSSAttrInfoDict::AttrInfo  QTSServerPrefs::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
	/* 0 */ { "rtsp_timeout",                           NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 1 */ { "real_rtsp_timeout",                      NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 2 */ { "rtp_timeout",                            NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 3 */ { "maximum_connections",                    NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 4 */ { "maximum_bandwidth",                      NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 5 */ { "movie_folder",                           NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
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
	/* 20 */ { "drop_all_video_delay",                  NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 21 */ { "start_thinning_delay",                  NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 22 */ { "large_window_size",                     NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 23 */ { "window_size_threshold",                 NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 24 */ { "min_tcp_buffer_size",                   NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 25 */ { "max_tcp_buffer_size",                   NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 26 */ { "tcp_seconds_to_buffer",                 NULL,                   qtssAttrDataTypeFloat32,    qtssAttrModeRead | qtssAttrModeWrite },
	/* 27 */ { "do_report_http_connection_ip_address",  NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 28 */ { "default_authorization_realm",           NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 29 */ { "run_user_name",                         NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 30 */ { "run_group_name",                        NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 31 */ { "append_source_addr_in_transport",       NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 32 */ { "rtsp_port",                             NULL,                   qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 33 */ { "max_retransmit_delay",                  NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 34 */ { "small_window_size",                     NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 35 */ { "ack_logging_enabled",                   NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 36 */ { "rtcp_poll_interval",                    NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 37 */ { "rtcp_rcv_buf_size",                     NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 38 */ { "send_interval",                         NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 39 */ { "thick_all_the_way_delay",               NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 40 */ { "alt_transport_src_ipaddr",              NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 41 */ { "max_send_ahead_time",                   NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 42 */ { "reliable_udp_slow_start",               NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 43 */ { "enable_cloud_platform",                 NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 44 */ { "authentication_scheme",                 NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 45 */ { "sdp_file_delete_interval_seconds",      NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 46 */ { "auto_start",                            NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 47 */ { "reliable_udp",                          NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 48 */ { "reliable_udp_dirs",                     NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 49 */ { "reliable_udp_printfs",                  NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 50 */ { "drop_all_packets_delay",                NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 51 */ { "thin_all_the_way_delay",                NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 52 */ { "always_thin_delay",                     NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 53 */ { "start_thicking_delay",                  NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 54 */ { "quality_check_interval",                NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 55 */ { "RTSP_error_message",                    NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 56 */ { "RTSP_debug_printfs",                    NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 57 */ { "enable_monitor_stats_file",             NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 58 */ { "monitor_stats_file_interval_seconds",   NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 59 */ { "monitor_stats_file_name",               NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 60 */ { "enable_packet_header_printfs",          NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 61 */ { "packet_header_printf_options",          NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 62 */ { "overbuffer_rate",						NULL,					qtssAttrDataTypeFloat32,	qtssAttrModeRead | qtssAttrModeWrite },
	/* 63 */ { "medium_window_size",					NULL,					qtssAttrDataTypeUInt32,		qtssAttrModeRead | qtssAttrModeWrite },
	/* 64 */ { "window_size_max_threshold",				NULL,					qtssAttrDataTypeUInt32,		qtssAttrModeRead | qtssAttrModeWrite },
	/* 65 */ { "RTSP_server_info",                      NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 66 */ { "run_num_threads",                       NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 67 */ { "pid_file",								NULL,					qtssAttrDataTypeCharArray,	qtssAttrModeRead | qtssAttrModeWrite },
	/* 68 */ { "force_logs_close_on_write",             NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 69 */ { "disable_thinning",                      NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 70 */ { "player_requires_rtp_header_info",		NULL,					qtssAttrDataTypeCharArray,	qtssAttrModeRead | qtssAttrModeWrite },
	/* 71 */ { "player_requires_bandwidth_adjustment",	NULL,					qtssAttrDataTypeCharArray,	qtssAttrModeRead | qtssAttrModeWrite },
	/* 72 */ { "player_requires_no_pause_time_adjustment",	NULL,				qtssAttrDataTypeCharArray,	qtssAttrModeRead | qtssAttrModeWrite },
	/* 73 */ { "enable_3gpp_protocol",                  NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 74 */ { "enable_3gpp_protocol_rate_adaptation",  NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 75 */ { "3gpp_protocol_rate_adaptation_report_frequency",  NULL,         qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 76 */ { "default_stream_quality",                NULL,                   qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 77 */ { "player_requires_rtp_start_time_adjust", NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 78 */ { "enable_3gpp_debug_printfs",  NULL,                              qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 79 */ { "enable_udp_monitor_stream",  NULL,                              qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 80 */ { "udp_monitor_video_port",  NULL,                                 qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 81 */ { "udp_monitor_audio_port",  NULL,                                 qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 82 */ { "udp_monitor_dest_ip",  NULL,                                    qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 83 */ { "udp_monitor_src_ip",  NULL,                                     qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 84 */ { "enable_allow_guest_default",  NULL,                             qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 85 */ { "run_num_rtsp_threads",  NULL,                                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 86 */ { "player_requires_disable_3gpp_rate_adapt",   NULL,               qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 87 */ { "player_requires_3gpp_target_time",   NULL,                      qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 88 */ { "3gpp_target_time_milliseconds",   NULL,                         qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 89 */ { "player_requires_disable_thinning", NULL,                        qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },

	/* 90 */ { "http_service_port",					NULL,                       qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite }

};


QTSServerPrefs::QTSServerPrefs(XMLPrefsParser* inPrefsSource, Bool16 inWriteMissingPrefs)
	: QTSSPrefs(inPrefsSource, NULL, QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kPrefsDictIndex), false),
	fRTSPTimeoutInSecs(0),
	fRTSPTimeoutString(fRTSPTimeoutBuf, 0),
	fRealRTSPTimeoutInSecs(0),
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
	fDropAllPacketsTimeInMsec(0),
	fDropAllVideoPacketsTimeInMsec(0),
	fThinAllTheWayTimeInMsec(0),
	fAlwaysThinTimeInMsec(0),
	fStartThinningTimeInMsec(0),
	fStartThickingTimeInMsec(0),
	fThickAllTheWayTimeInMsec(0),
	fQualityCheckIntervalInMsec(0),
	fMinTCPBufferSizeInBytes(0),
	fMaxTCPBufferSizeInBytes(0),
	fTCPSecondsToBuffer(0),
	fDoReportHTTPConnectionAddress(false),
	fAppendSrcAddrInTransport(false),
	fSmallWindowSizeInK(0),
	fMediumWindowSizeInK(0),
	fLargeWindowSizeInK(0),
	fWindowSizeThreshold(0),
	fWindowSizeMaxThreshold(0),
	fMaxRetransDelayInMsec(0),
	fIsAckLoggingEnabled(false),
	fRTCPPollIntervalInMsec(0),
	fRTCPSocketRcvBufSizeInK(0),
	fIsSlowStartEnabled(false),
	fSendIntervalInMsec(0),
	fMaxSendAheadTimeInSecs(0),
	fCloudPlatformEnabled(false),
	fAuthScheme(qtssAuthDigest),
	fsdp_file_delete_interval_seconds(10),
	fAutoStart(false),
	fReliableUDP(true),
	fReliableUDPPrintfs(false),
	fEnableRTSPErrMsg(false),
	fEnableRTSPDebugPrintfs(false),
	fEnableRTSPServerInfo(true),
	fNumThreads(0),
	fNumRTSPThreads(0),
#if __MacOSX__
	fEnableMonitorStatsFile(false),
#else
	fEnableMonitorStatsFile(false),
#endif 
	fStatsFileIntervalSeconds(10),
	fOverbufferRate(0.0),
	fEnablePacketHeaderPrintfs(false),
	fPacketHeaderPrintfOptions(kRTPALL | kRTCPSR | kRTCPRR | kRTCPAPP | kRTCPACK),
	fCloseLogsOnWrite(false),
	fDisableThinning(false),
	//
	f3gppProtocolEnabled(true),
	f3gppProtocolRateAdaptationEnabled(true),
	f3gppProtocolRateAdaptationReportFrequency(1),
	fDefaultStreamQuality(0),
	f3gppDebugPrintfsEnabled(false),
	fUDPMonitorEnabled(false),
	fUDPMonitorVideoPort(0),
	fUDPMonitorAudioPort(0),
	fAllowGuestAuthorizeDefault(true),
	f3GPPRateAdaptTargetTime(0),
	fHTTPServicePort(8080)
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
	this->SetVal(qtssPrefsRTSPTimeout, &fRTSPTimeoutInSecs, sizeof(fRTSPTimeoutInSecs));
	this->SetVal(qtssPrefsRealRTSPTimeout, &fRealRTSPTimeoutInSecs, sizeof(fRealRTSPTimeoutInSecs));
	this->SetVal(qtssPrefsRTPTimeout, &fRTPTimeoutInSecs, sizeof(fRTPTimeoutInSecs));
	this->SetVal(qtssPrefsMaximumConnections, &fMaximumConnections, sizeof(fMaximumConnections));
	this->SetVal(qtssPrefsMaximumBandwidth, &fMaxBandwidthInKBits, sizeof(fMaxBandwidthInKBits));
	this->SetVal(qtssPrefsBreakOnAssert, &fBreakOnAssert, sizeof(fBreakOnAssert));
	this->SetVal(qtssPrefsAutoRestart, &fAutoRestart, sizeof(fAutoRestart));
	this->SetVal(qtssPrefsTotalBytesUpdate, &fTBUpdateTimeInSecs, sizeof(fTBUpdateTimeInSecs));
	this->SetVal(qtssPrefsAvgBandwidthUpdate, &fABUpdateTimeInSecs, sizeof(fABUpdateTimeInSecs));
	this->SetVal(qtssPrefsSafePlayDuration, &fSafePlayDurationInSecs, sizeof(fSafePlayDurationInSecs));

	this->SetVal(qtssPrefsErrorRollInterval, &fErrorRollIntervalInDays, sizeof(fErrorRollIntervalInDays));
	this->SetVal(qtssPrefsMaxErrorLogSize, &fErrorLogBytes, sizeof(fErrorLogBytes));
	this->SetVal(qtssPrefsErrorLogVerbosity, &fErrorLogVerbosity, sizeof(fErrorLogVerbosity));
	this->SetVal(qtssPrefsScreenLogging, &fScreenLoggingEnabled, sizeof(fScreenLoggingEnabled));
	this->SetVal(qtssPrefsErrorLogEnabled, &fErrorLogEnabled, sizeof(fErrorLogEnabled));

	this->SetVal(qtssPrefsMinTCPBufferSizeInBytes, &fMinTCPBufferSizeInBytes, sizeof(fMinTCPBufferSizeInBytes));
	this->SetVal(qtssPrefsMaxTCPBufferSizeInBytes, &fMaxTCPBufferSizeInBytes, sizeof(fMaxTCPBufferSizeInBytes));
	this->SetVal(qtssPrefsTCPSecondsToBuffer, &fTCPSecondsToBuffer, sizeof(fTCPSecondsToBuffer));

	this->SetVal(qtssPrefsDoReportHTTPConnectionAddress, &fDoReportHTTPConnectionAddress, sizeof(fDoReportHTTPConnectionAddress));
	this->SetVal(qtssPrefsSrcAddrInTransport, &fAppendSrcAddrInTransport, sizeof(fAppendSrcAddrInTransport));

	this->SetVal(qtssPrefsSmallWindowSizeInK, &fSmallWindowSizeInK, sizeof(fSmallWindowSizeInK));
	this->SetVal(qtssPrefsMediumWindowSizeInK, &fMediumWindowSizeInK, sizeof(fMediumWindowSizeInK));
	this->SetVal(qtssPrefsLargeWindowSizeInK, &fLargeWindowSizeInK, sizeof(fLargeWindowSizeInK));
	this->SetVal(qtssPrefsWindowSizeThreshold, &fWindowSizeThreshold, sizeof(fWindowSizeThreshold));
	this->SetVal(qtssPrefsWindowSizeMaxThreshold, &fWindowSizeMaxThreshold, sizeof(fWindowSizeMaxThreshold));

	this->SetVal(qtssPrefsMaxRetransDelayInMsec, &fMaxRetransDelayInMsec, sizeof(fMaxRetransDelayInMsec));
	this->SetVal(qtssPrefsAckLoggingEnabled, &fIsAckLoggingEnabled, sizeof(fIsAckLoggingEnabled));
	this->SetVal(qtssPrefsRTCPPollIntervalInMsec, &fRTCPPollIntervalInMsec, sizeof(fRTCPPollIntervalInMsec));
	this->SetVal(qtssPrefsRTCPSockRcvBufSizeInK, &fRTCPSocketRcvBufSizeInK, sizeof(fRTCPSocketRcvBufSizeInK));
	this->SetVal(qtssPrefsSendInterval, &fSendIntervalInMsec, sizeof(fSendIntervalInMsec));
	this->SetVal(qtssPrefsMaxAdvanceSendTimeInSec, &fMaxSendAheadTimeInSecs, sizeof(fMaxSendAheadTimeInSecs));
	this->SetVal(qtssPrefsReliableUDPSlowStart, &fIsSlowStartEnabled, sizeof(fIsSlowStartEnabled));
	this->SetVal(qtssPrefsEnableCloudPlatform, &fCloudPlatformEnabled, sizeof(fCloudPlatformEnabled));
	this->SetVal(qtssPrefsDeleteSDPFilesInterval, &fsdp_file_delete_interval_seconds, sizeof(fsdp_file_delete_interval_seconds));
	this->SetVal(qtssPrefsAutoStart, &fAutoStart, sizeof(fAutoStart));
	this->SetVal(qtssPrefsReliableUDP, &fReliableUDP, sizeof(fReliableUDP));
	this->SetVal(qtssPrefsReliableUDPPrintfs, &fReliableUDPPrintfs, sizeof(fReliableUDPPrintfs));

	this->SetVal(qtssPrefsDropAllPacketsDelayInMsec, &fDropAllPacketsTimeInMsec, sizeof(fDropAllPacketsTimeInMsec));
	this->SetVal(qtssPrefsDropVideoAllPacketsDelayInMsec, &fDropAllVideoPacketsTimeInMsec, sizeof(fDropAllVideoPacketsTimeInMsec));
	this->SetVal(qtssPrefsThinAllTheWayDelayInMsec, &fThinAllTheWayTimeInMsec, sizeof(fThinAllTheWayTimeInMsec));
	this->SetVal(qtssPrefsAlwaysThinDelayInMsec, &fAlwaysThinTimeInMsec, sizeof(fAlwaysThinTimeInMsec));
	this->SetVal(qtssPrefsStartThinningDelayInMsec, &fStartThinningTimeInMsec, sizeof(fStartThinningTimeInMsec));
	this->SetVal(qtssPrefsStartThickingDelayInMsec, &fStartThickingTimeInMsec, sizeof(fStartThickingTimeInMsec));
	this->SetVal(qtssPrefsThickAllTheWayDelayInMsec, &fThickAllTheWayTimeInMsec, sizeof(fThickAllTheWayTimeInMsec));
	this->SetVal(qtssPrefsQualityCheckIntervalInMsec, &fQualityCheckIntervalInMsec, sizeof(fQualityCheckIntervalInMsec));
	this->SetVal(qtssPrefsEnableRTSPErrorMessage, &fEnableRTSPErrMsg, sizeof(fEnableRTSPErrMsg));
	this->SetVal(qtssPrefsEnableRTSPDebugPrintfs, &fEnableRTSPDebugPrintfs, sizeof(fEnableRTSPDebugPrintfs));
	this->SetVal(qtssPrefsEnableRTSPServerInfo, &fEnableRTSPServerInfo, sizeof(fEnableRTSPServerInfo));
	this->SetVal(qtssPrefsRunNumThreads, &fNumThreads, sizeof(fNumThreads));
	this->SetVal(qtssPrefsEnableMonitorStatsFile, &fEnableMonitorStatsFile, sizeof(fEnableMonitorStatsFile));
	this->SetVal(qtssPrefsMonitorStatsFileIntervalSec, &fStatsFileIntervalSeconds, sizeof(fStatsFileIntervalSeconds));

	this->SetVal(qtssPrefsEnablePacketHeaderPrintfs, &fEnablePacketHeaderPrintfs, sizeof(fEnablePacketHeaderPrintfs));
	this->SetVal(qtssPrefsCloseLogsOnWrite, &fCloseLogsOnWrite, sizeof(fCloseLogsOnWrite));
	this->SetVal(qtssPrefsOverbufferRate, &fOverbufferRate, sizeof(fOverbufferRate));
	this->SetVal(qtssPrefsDisableThinning, &fDisableThinning, sizeof(fDisableThinning));

	this->SetVal(qtssPrefsEnable3gppProtocol, &f3gppProtocolEnabled, sizeof(f3gppProtocolEnabled));
	this->SetVal(qtssPrefsEnable3gppProtocolRateAdapt, &f3gppProtocolRateAdaptationEnabled, sizeof(f3gppProtocolRateAdaptationEnabled));
	this->SetVal(qtssPrefs3gppRateAdaptReportFrequency, &f3gppProtocolRateAdaptationReportFrequency, sizeof(f3gppProtocolRateAdaptationReportFrequency)); //3gpp_protocol_rate_adaptation_report_frequency
	this->SetVal(qtssPrefsDefaultStreamQuality, &fDefaultStreamQuality, sizeof(fDefaultStreamQuality)); //default_stream_quality
	this->SetVal(qtssPrefsEnable3gppDebugPrintfs, &f3gppDebugPrintfsEnabled, sizeof(f3gppDebugPrintfsEnabled));
	this->SetVal(qtssPrefsEnableUDPMonitor, &fUDPMonitorEnabled, sizeof(fUDPMonitorEnabled));
	this->SetVal(qtssPrefsUDPMonitorAudioPort, &fUDPMonitorVideoPort, sizeof(fUDPMonitorVideoPort));
	this->SetVal(qtssPrefsUDPMonitorVideoPort, &fUDPMonitorAudioPort, sizeof(fUDPMonitorAudioPort));
	this->SetVal(qtssPrefsUDPMonitorDestIPAddr, &fUDPMonitorDestAddr, sizeof(fUDPMonitorDestAddr));
	this->SetVal(qtssPrefsUDPMonitorSourceIPAddr, &fUDPMonitorSrcAddr, sizeof(fUDPMonitorSrcAddr));
	this->SetVal(qtssPrefsEnableAllowGuestDefault, &fAllowGuestAuthorizeDefault, sizeof(fAllowGuestAuthorizeDefault)); //enable_allow_guest_authorize_default
	this->SetVal(qtssPrefsNumRTSPThreads, &fNumRTSPThreads, sizeof(fNumRTSPThreads));
	this->SetVal(qtssPrefs3GPPTargetTime, &f3GPPRateAdaptTargetTime, sizeof(f3GPPRateAdaptTargetTime));

	this->SetVal(easyPrefsHTTPServicePort, &fHTTPServicePort, sizeof(fHTTPServicePort));
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
		ContainerRef pref = fPrefsSource->GetPrefRefByName(server, theMap->GetAttrName(x));
		char* thePrefValue = NULL;
		if (pref != NULL)
			thePrefValue = fPrefsSource->GetPrefValueByRef(pref, 0, &thePrefName,
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
				QTSSModuleUtils::LogError(QTSSModuleUtils::GetMisingPrefLogVerbosity(),
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
					this->SetPrefValue(x, y + 1, sPrefInfo[x].fAdditionalDefVals[y], sAttributes[x].fAttrDataType);
			}

			if (inWriteMissingPrefs)
			{
				//
				// Add this value into the file, cuz we need it.
				pref = fPrefsSource->AddPref(server, sAttributes[x].fAttrName, QTSSDataConverter::TypeToTypeString(sAttributes[x].fAttrDataType));
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
				QTSSModuleUtils::LogError(qtssWarningVerbosity,
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
					this->SetPrefValue(x, z + 1, sPrefInfo[x].fAdditionalDefVals[z], sAttributes[x].fAttrDataType);
			}

			if (inWriteMissingPrefs)
			{
				//
				// Remove it out of the file and add in the default.
				fPrefsSource->RemovePref(pref);
				pref = fPrefsSource->AddPref(server, sAttributes[x].fAttrName, QTSSDataConverter::TypeToTypeString(sAttributes[x].fAttrDataType));
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
	this->UpdatePrintfOptions();
	QTSSModuleUtils::SetEnableRTSPErrorMsg(fEnableRTSPErrMsg);

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

char*   QTSServerPrefs::GetMovieFolder(char* inBuffer, UInt32* ioLen)
{
	OSMutexLocker locker(&fPrefsMutex);

	// Get the movie folder attribute
	StrPtrLen* theMovieFolder = this->GetValue(qtssPrefsMovieFolder);

	// If the movie folder path fits inside the provided buffer, copy it there
	if (theMovieFolder->Len < *ioLen)
		::memcpy(inBuffer, theMovieFolder->Ptr, theMovieFolder->Len);
	else
	{
		// Otherwise, allocate a buffer to store the path
		inBuffer = NEW char[theMovieFolder->Len + 2];
		::memcpy(inBuffer, theMovieFolder->Ptr, theMovieFolder->Len);
	}
	inBuffer[theMovieFolder->Len] = 0;
	*ioLen = theMovieFolder->Len;
	return inBuffer;
}

Bool16 QTSServerPrefs::IsPathInsideReliableUDPDir(StrPtrLen* inPath)
{
	OSMutexLocker locker(&fPrefsMutex);

	QTSS_Error theErr = QTSS_NoErr;
	for (UInt32 x = 0; theErr == QTSS_NoErr; x++)
	{
		StrPtrLen theReliableUDPDir;
		theErr = this->GetValuePtr(qtssPrefsReliableUDPDirs, x, (void**)&theReliableUDPDir.Ptr, &theReliableUDPDir.Len, true);

		if (theErr != QTSS_NoErr)
			return false;

		if (inPath->NumEqualIgnoreCase(theReliableUDPDir.Ptr, theReliableUDPDir.Len))
			return true;
	}
	Assert(0);
	return false;
}

void QTSServerPrefs::UpdatePrintfOptions()
{
	StrPtrLen* theOptions = this->GetValue(qtssPrefsPacketHeaderPrintfOptions);
	if (theOptions == NULL || theOptions->Len == 0)
		return;

	fPacketHeaderPrintfOptions = 0;
	if (theOptions->FindStringIgnoreCase("rtp"))
		fPacketHeaderPrintfOptions |= kRTPALL;
	if (theOptions->FindStringIgnoreCase("sr"))
		fPacketHeaderPrintfOptions |= kRTCPSR;
	if (theOptions->FindStringIgnoreCase("rr"))
		fPacketHeaderPrintfOptions |= kRTCPRR;
	if (theOptions->FindStringIgnoreCase("app"))
		fPacketHeaderPrintfOptions |= kRTCPAPP;
	if (theOptions->FindStringIgnoreCase("ack"))
		fPacketHeaderPrintfOptions |= kRTCPACK;

}

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

