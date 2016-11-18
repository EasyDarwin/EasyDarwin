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
	 File:       GenerateXMLPrefs.h

	 Contains:   Routine that updates a QTSS 1.x 2.x PrefsSource to the new XMLPrefsSource.

 */

#include "GenerateXMLPrefs.h"
#include "QTSSDataConverter.h"
#include "QTSS.h"

struct PrefConversionInfo
{
	char*               fPrefName;
	char*               fModuleName;
	QTSS_AttrDataType   fPrefType;
};

static const PrefConversionInfo kPrefs[] =
{
	{ "rtsp_timeout",                   NULL,           qtssAttrDataTypeUInt32 },
	{ "rtsp_session_timeout",              NULL,           qtssAttrDataTypeUInt32 },
	{ "rtp_timeout",                    NULL,           qtssAttrDataTypeUInt32 },
	{ "maximum_connections",            NULL,           qtssAttrDataTypeSInt32 },
	{ "maximum_bandwidth",              NULL,           qtssAttrDataTypeSInt32 },
	{ "movie_folder",                   NULL,           qtssAttrDataTypeCharArray },
	{ "bind_ip_addr",                   NULL,           qtssAttrDataTypeCharArray },
	{ "break_on_assert",                NULL,           qtssAttrDataTypeBool16 },
	{ "auto_restart",                   NULL,           qtssAttrDataTypeBool16 },
	{ "total_bytes_update",             NULL,           qtssAttrDataTypeUInt32 },
	{ "average_bandwidth_update",       NULL,           qtssAttrDataTypeUInt32 },
	{ "safe_play_duration",                     NULL,   qtssAttrDataTypeUInt32 },
	{ "module_folder",                          NULL,   qtssAttrDataTypeCharArray },
	{ "error_logfile_name",                     NULL,   qtssAttrDataTypeCharArray },
	{ "error_logfile_dir",                      NULL,   qtssAttrDataTypeCharArray },
	{ "error_logfile_interval",                 NULL,   qtssAttrDataTypeUInt32 },
	{ "error_logfile_size",                     NULL,   qtssAttrDataTypeUInt32 },
	{ "error_logfile_verbosity",                NULL,   qtssAttrDataTypeUInt32 },
	{ "screen_logging",                         NULL,   qtssAttrDataTypeBool16 },
	{ "error_logging",                          NULL,   qtssAttrDataTypeBool16 },
	{ "drop_all_video_delay",                   NULL,   qtssAttrDataTypeSInt32 },
	{ "start_thinning_delay",                   NULL,   qtssAttrDataTypeSInt32 },
	{ "large_window_size",                      NULL,   qtssAttrDataTypeSInt32 },
	{ "window_size_threshold",                  NULL,   qtssAttrDataTypeSInt32 },
	{ "min_tcp_buffer_size",                    NULL,   qtssAttrDataTypeUInt32 },
	{ "max_tcp_buffer_size",                    NULL,   qtssAttrDataTypeUInt32 },
	{ "tcp_seconds_to_buffer",                  NULL,   qtssAttrDataTypeFloat32 },
	{ "do_report_http_connection_ip_address",   NULL,   qtssAttrDataTypeBool16 },
	{ "default_authorization_realm",            NULL,   qtssAttrDataTypeCharArray },
	{ "run_user_name",                          NULL,   qtssAttrDataTypeCharArray },
	{ "run_group_name",                         NULL,   qtssAttrDataTypeCharArray },
	{ "append_source_addr_in_transport",        NULL,   qtssAttrDataTypeBool16 },
	{ "rtsp_port",                              NULL,   qtssAttrDataTypeUInt16 },

	{ "request_logging",                        "QTSSAccessLogModule",  qtssAttrDataTypeBool16 },
	{ "request_logfile_name",                   "QTSSAccessLogModule",  qtssAttrDataTypeCharArray },
	{ "request_logfile_dir",                    "QTSSAccessLogModule",  qtssAttrDataTypeCharArray },
	{ "request_logfile_size",                   "QTSSAccessLogModule",  qtssAttrDataTypeUInt32 },
	{ "request_logfile_interval",               "QTSSAccessLogModule",  qtssAttrDataTypeUInt32 },

	{ "history_update_interval",                "QTSSSvrControlModule", qtssAttrDataTypeUInt32 },

	{ "buffer_seconds",                         "QTSSFileModule",   qtssAttrDataTypeUInt32 },
	{ "sdp_url",                                "QTSSFileModule",   qtssAttrDataTypeCharArray },
	{ "admin_email",                            "QTSSFileModule",   qtssAttrDataTypeCharArray },
	{ "max_advance_send_time",                  "QTSSFileModule",   qtssAttrDataTypeUInt32 },

	{ "reflector_delay",                        "QTSSReflectorModule",  qtssAttrDataTypeUInt32 },
	{ "reflector_bucket_size",                  "QTSSReflectorModule",  qtssAttrDataTypeUInt32 },

	{ "web_stats_url",                          "QTSSWebStatsModule",   qtssAttrDataTypeCharArray },

	{ "loss_thin_tolerance",                    "QTSSFlowControlModule",    qtssAttrDataTypeUInt32 },
	{ "num_losses_to_thin",                     "QTSSFlowControlModule",    qtssAttrDataTypeUInt32 },
	{ "loss_thick_tolerance",                   "QTSSFlowControlModule",    qtssAttrDataTypeUInt32 },
	{ "num_losses_to_thick",                    "QTSSFlowControlModule",    qtssAttrDataTypeUInt32 },
	{ "num_worses_to_thin",                     "QTSSFlowControlModule",    qtssAttrDataTypeUInt32 },

	{ "relay_stats_url",                        "QTSSRelayModule",  qtssAttrDataTypeCharArray },
	{ "relay_prefs_file",                       "QTSSRelayModule",  qtssAttrDataTypeCharArray },

	{ "num_conns_per_ip_addr",                  "QTSSSpamDefenseModule",    qtssAttrDataTypeUInt32 },

	{ "modAccess_usersfilepath",                "QTSSAccessModule", qtssAttrDataTypeCharArray },
	{ "modAccess_groupsfilepath",               "QTSSAccessModule", qtssAttrDataTypeCharArray },
	{ "modAccess_qtaccessfilename",             "QTSSAccessModule", qtssAttrDataTypeCharArray },

	//
	// This element will be used if the pref is something we don't know about.
	// Just have unknown prefs default to be server prefs with a type of char
	{ NULL,                                     NULL,               qtssAttrDataTypeCharArray }
};

int GenerateAllXMLPrefs(FilePrefsSource* inPrefsSource, XMLPrefsParser* inXMLPrefs)
{
	for (UInt32 x = 0; x < inPrefsSource->GetNumKeys(); x++)
	{
		//
		// Get the name of this pref
		char* thePrefName = inPrefsSource->GetKeyAtIndex(x);

		//
		// Find the information corresponding to this pref in the above array
		UInt32 y = 0;
		for (; kPrefs[y].fPrefName != NULL; y++)
			if (::strcmp(thePrefName, kPrefs[y].fPrefName) == 0)
				break;

		char* theTypeString = (char*)QTSSDataConverter::TypeToTypeString(kPrefs[y].fPrefType);
		ContainerRef module = inXMLPrefs->GetRefForModule(kPrefs[y].fModuleName);
		ContainerRef pref = inXMLPrefs->AddPref(module, thePrefName, theTypeString);

		char* theValue = inPrefsSource->GetValueAtIndex(x);

		static char* kTrue = "true";
		static char* kFalse = "false";

		//
		// If the pref is a bool, the new pref format uses "true" & "false",
		// the old one uses "enabled" and "disabled", so we have to explicitly convert.
		if (kPrefs[y].fPrefType == qtssAttrDataTypeBool16)
		{
			if (::strcmp(theValue, "enabled") == 0)
				theValue = kTrue;
			else
				theValue = kFalse;
		}
		inXMLPrefs->AddPrefValue(pref, theValue);
	}

	return inXMLPrefs->WritePrefsFile();
}

int GenerateStandardXMLPrefs(PrefsSource* inPrefsSource, XMLPrefsParser* inXMLPrefs)
{
	char thePrefBuf[1024];

	for (UInt32 x = 0; kPrefs[x].fPrefName != NULL; x++)
	{
		char* theTypeString = (char*)QTSSDataConverter::TypeToTypeString(kPrefs[x].fPrefType);
		ContainerRef module = inXMLPrefs->GetRefForModule(kPrefs[x].fModuleName);
		ContainerRef pref = inXMLPrefs->AddPref(module, kPrefs[x].fPrefName, theTypeString);

		for (UInt32 y = 0; true; y++)
		{
			if (inPrefsSource->GetValueByIndex(kPrefs[x].fPrefName, y, thePrefBuf) == 0)
				break;

			//
			// If the pref is a bool, the new pref format uses "true" & "false",
			// the old one uses "enabled" and "disabled", so we have to explicitly convert.
			if (kPrefs[x].fPrefType == qtssAttrDataTypeBool16)
			{
				if (::strcmp(thePrefBuf, "enabled") == 0)
					::strcpy(thePrefBuf, "true");
				else
					::strcpy(thePrefBuf, "false");
			}
			inXMLPrefs->AddPrefValue(pref, thePrefBuf);
		}
	}

	return inXMLPrefs->WritePrefsFile();
}
