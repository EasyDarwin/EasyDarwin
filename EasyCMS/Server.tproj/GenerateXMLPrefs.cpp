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
	/* 0 */ { "session_timeout",						NULL,                   qtssAttrDataTypeUInt32 },
	/* 1 */ { "maximum_connections",                    NULL,                   qtssAttrDataTypeSInt32 },

	/* 4 */ { "bind_ip_addr",                           NULL,                   qtssAttrDataTypeCharArray },
    /* 5 */ { "break_on_assert",                        NULL,                   qtssAttrDataTypeBool16 },
    /* 6 */ { "auto_restart",                           NULL,                   qtssAttrDataTypeBool16 },
	/* 7 */ { "module_folder",                          NULL,                   qtssAttrDataTypeCharArray },
    /* 8 */ { "error_logfile_name",                     NULL,                   qtssAttrDataTypeCharArray },
    /* 9 */ { "error_logfile_dir",                      NULL,                   qtssAttrDataTypeCharArray },
    /* 10 */ { "error_logfile_interval",                NULL,                   qtssAttrDataTypeUInt32 },
    /* 11 */ { "error_logfile_size",                    NULL,                   qtssAttrDataTypeUInt32 },
    /* 12 */ { "error_logfile_verbosity",               NULL,                   qtssAttrDataTypeUInt32 },
    /* 13 */ { "screen_logging",                        NULL,                   qtssAttrDataTypeBool16 },
    /* 14 */ { "error_logging",                         NULL,                   qtssAttrDataTypeBool16 },
    /* 15 */ { "service_id",							NULL,                   qtssAttrDataTypeCharArray },
    /* 16 */ { "redis_ip_addr",							NULL,                   qtssAttrDataTypeCharArray },
	/* 17 */ { "redis_port",							NULL,                   qtssAttrDataTypeUInt16 },
    /* 18 */ { "snap_local_path",						NULL,                   qtssAttrDataTypeCharArray },
    /* 19 */ { "snap_web_path",							NULL,                   qtssAttrDataTypeCharArray },
    /* 20 */ { "auto_start",                            NULL,                   qtssAttrDataTypeBool16 },
    /* 21 */ { "MSG_debug_printfs",						NULL,                   qtssAttrDataTypeBool16 },
    /* 22 */ { "enable_monitor_stats_file",             NULL,                   qtssAttrDataTypeBool16 },
    /* 23 */ { "monitor_stats_file_interval_seconds",   NULL,                   qtssAttrDataTypeUInt32 },
    /* 24 */ { "monitor_stats_file_name",               NULL,                   qtssAttrDataTypeCharArray },
	/* 25 */ { "run_num_threads",                       NULL,                   qtssAttrDataTypeUInt32 },
	/* 26 */ { "pid_file",								NULL,					qtssAttrDataTypeCharArray },
    /* 27 */ { "force_logs_close_on_write",             NULL,                   qtssAttrDataTypeBool16 },
	/* 28 */ { "monitor_lan_port",						NULL,					qtssAttrDataTypeUInt16 },
    /* 29 */ { "monitor_wan_port",						NULL,					qtssAttrDataTypeUInt16 },
    /* 30 */ { "monitor_lan_ip",						NULL,                   qtssAttrDataTypeCharArray },
    /* 31 */ { "monitor_wan_ip",						NULL,                   qtssAttrDataTypeCharArray },
    /* 32 */ { "run_num_msg_threads",					NULL,					qtssAttrDataTypeUInt32 },

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
        for ( ; kPrefs[y].fPrefName != NULL; y++)
            if (::strcmp(thePrefName, kPrefs[y].fPrefName) == 0)
                break;
        
        char* theTypeString = (char*)QTSSDataConverter::TypeToTypeString(kPrefs[y].fPrefType);
        ContainerRef module = inXMLPrefs->GetRefForModule(kPrefs[y].fModuleName);
        ContainerRef pref = inXMLPrefs->AddPref(module, thePrefName, theTypeString);

        char* theValue = inPrefsSource->GetValueAtIndex(x);
            
        static char* kTrue = "true";
        static char* kFalse = "false";
        
        //
        // If the pref is a Bool16, the new pref format uses "true" & "false",
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
            // If the pref is a Bool16, the new pref format uses "true" & "false",
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
