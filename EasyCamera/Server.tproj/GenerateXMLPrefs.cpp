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
    /* 0 */ { "connection_timeout",             NULL,           qtssAttrDataTypeUInt32 },
    /* 1 */ { "cms_addr",						NULL,           qtssAttrDataTypeCharArray },
    /* 2 */ { "cms_port",						NULL,           qtssAttrDataTypeUInt32 },
	
	/* 3 */ { "local_camera_addr",				NULL,			qtssAttrDataTypeCharArray },
	/* 4 */ { "local_camera_port",				NULL,			qtssAttrDataTypeUInt16 },
    /* 5 */ { "serial_number",					NULL,           qtssAttrDataTypeCharArray },
    /* 6 */ { "camera_stream_type",				NULL,           qtssAttrDataTypeUInt32 },

    /* 7 */ { "maximum_bandwidth",              NULL,           qtssAttrDataTypeSInt32 },


    /* 8 */ { "rtsp_server_addr",               NULL,			qtssAttrDataTypeCharArray },
    /* 9 */ { "rtsp_server_port",				NULL,			qtssAttrDataTypeUInt16 },

    /* 10 */ { "error_logfile_name",             NULL,			qtssAttrDataTypeCharArray },
    /* 11 */ { "error_logfile_dir",              NULL,			qtssAttrDataTypeCharArray },
    /* 12 */ { "error_logfile_interval",        NULL,			qtssAttrDataTypeUInt32 },
    /* 13 */ { "error_logfile_size",			NULL,			qtssAttrDataTypeUInt32 },
    /* 14 */ { "error_logfile_verbosity",		NULL,			qtssAttrDataTypeUInt32 },
    /* 15 */ { "screen_logging",				NULL,			qtssAttrDataTypeBool16 },
    /* 16 */ { "error_logging",					NULL,			qtssAttrDataTypeBool16 },

    /* 17 */ { "run_user_name",					NULL,			qtssAttrDataTypeCharArray },
    /* 18 */ { "run_password",					NULL,			qtssAttrDataTypeCharArray },
    //
    // This element will be used if the pref is something we don't know about.
    // Just have unknown prefs default to be server prefs with a type of char
    { NULL,                                     NULL,			qtssAttrDataTypeCharArray }
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
