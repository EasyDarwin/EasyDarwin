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
    { kDontAllowMultipleValues, "90",      NULL                     },  //0 session_timeout

    { kDontAllowMultipleValues, "1000",     NULL                    },  //1 maximum_connections
    { kDontAllowMultipleValues, "102400",   NULL                    },  //2 maximum_bandwidth
	{ kDontAllowMultipleValues,	"127.0.0.1",NULL					},	//3 cms_ip_addr
    { kAllowMultipleValues,     "0",        NULL                    },  //4 bind_ip_addr
    { kDontAllowMultipleValues, "false",    NULL                    },  //5 break_on_assert
    { kDontAllowMultipleValues, "true",     NULL                    },  //6 auto_restart

	{ kDontAllowMultipleValues,	DEFAULTPATHS_SSM_DIR,	NULL		},	//7 module_folder
    { kDontAllowMultipleValues, "Error",    NULL                    },  //8 error_logfile_name
	{ kDontAllowMultipleValues,	DEFAULTPATHS_LOG_DIR,	NULL		},	//9 error_logfile_dir
    { kDontAllowMultipleValues, "7",        NULL                    },  //10 error_logfile_interval
    { kDontAllowMultipleValues, "256000",   NULL                    },  //11 error_logfile_size
    { kDontAllowMultipleValues, "2",        NULL                    },  //12 error_logfile_verbosity
    { kDontAllowMultipleValues, "true",     NULL                    },  //13 screen_logging
    { kDontAllowMultipleValues, "true",     NULL                    },  //14 error_logging
    { kDontAllowMultipleValues, "CMS000",   NULL                    },  //15 service_id
    { kDontAllowMultipleValues, "127.0.0.1",NULL					},  //16 redis_ip_addr
    { kDontAllowMultipleValues, "6379",     NULL					},	//17 redis_port
    { kDontAllowMultipleValues, "./snap/",      NULL                },  //18 snap_local_path
    { kDontAllowMultipleValues, "http://snap.easydarwin.org/", NULL },  //19 snap_web_path
    { kDontAllowMultipleValues, "false",    NULL                    },  //20 auto_start
    { kDontAllowMultipleValues, "false",    NULL                    },  //21 MSG_debug_printfs
    { kDontAllowMultipleValues, "false",    NULL                    },  //22 enable_monitor_stats_file
    { kDontAllowMultipleValues, "10",       NULL                    },  //23 monitor_stats_file_interval_seconds
    { kDontAllowMultipleValues, "server_status",        NULL        },  //24 monitor_stats_file_name
	{ kDontAllowMultipleValues, "0",        NULL                    },  //25 run_num_threads
    { kDontAllowMultipleValues, DEFAULTPATHS_PID_DIR "easycms" ".pid",	NULL	},	//26 pid_file
    { kDontAllowMultipleValues, "false",    NULL                    },  //27 force_logs_close_on_write
    { kDontAllowMultipleValues, "10000",    NULL                     }, //28 monitor_lan_port
    { kDontAllowMultipleValues, "10000",    NULL                     }, //29 monitor_wan_port
    { kDontAllowMultipleValues, "127.0.0.1",NULL                     }, //30 monitor_lan_ip
    { kDontAllowMultipleValues, "0.0.0.0",  NULL                     }, //31 monitor_wan_ip
    { kDontAllowMultipleValues, "2",        NULL                     }  //32 run_num_msg_threads
};
 
QTSSAttrInfoDict::AttrInfo  QTSServerPrefs::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0 */ { "session_timeout",						NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 1 */ { "maximum_connections",                    NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 2 */ { "maximum_bandwidth",                      NULL,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 3 */ { "cms_ip_addr",							NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 4 */ { "bind_ip_addr",                           NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 5 */ { "break_on_assert",                        NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 6 */ { "auto_restart",                           NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 7 */ { "module_folder",                          NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 8 */ { "error_logfile_name",                     NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 9 */ { "error_logfile_dir",                      NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 10 */ { "error_logfile_interval",                NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 11 */ { "error_logfile_size",                    NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 12 */ { "error_logfile_verbosity",               NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 13 */ { "screen_logging",                        NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 14 */ { "error_logging",                         NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 15 */ { "service_id",							NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 16 */ { "redis_ip_addr",							NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 17 */ { "redis_port",							NULL,                   qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 18 */ { "snap_local_path",						NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 19 */ { "snap_web_path",							NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 20 */ { "auto_start",                            NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 21 */ { "MSG_debug_printfs",						NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 22 */ { "enable_monitor_stats_file",             NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 23 */ { "monitor_stats_file_interval_seconds",   NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 24 */ { "monitor_stats_file_name",               NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 25 */ { "run_num_threads",                       NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 26 */ { "pid_file",								NULL,					qtssAttrDataTypeCharArray,	qtssAttrModeRead | qtssAttrModeWrite },
    /* 27 */ { "force_logs_close_on_write",             NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 28 */ { "monitor_lan_port",						NULL,					qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 29 */ { "monitor_wan_port",						NULL,					qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 30 */ { "monitor_lan_ip",						NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 31 */ { "monitor_wan_ip",						NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 32 */ { "run_num_msg_threads",					NULL,					qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite }
};

QTSServerPrefs::QTSServerPrefs(XMLPrefsParser* inPrefsSource, Bool16 inWriteMissingPrefs)
:   QTSSPrefs(inPrefsSource, NULL, QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kPrefsDictIndex), false),
    fSessionTimeoutInSecs(0),
    fMaximumConnections(0),
    fMaxBandwidthInKBits(0),
    fBreakOnAssert(false),
    fAutoRestart(false),
    fErrorRollIntervalInDays(0),
    fErrorLogBytes(0),
    fErrorLogVerbosity(0),
    fScreenLoggingEnabled(true),
    fErrorLogEnabled(false),
    fAutoStart(false),
    fEnableMSGDebugPrintfs(false),
    fNumThreads(0),
    fNumMsgThreads(0),
#if __MacOSX__
    fEnableMonitorStatsFile(false),
#else
    fEnableMonitorStatsFile(false),
#endif 
    fStatsFileIntervalSeconds(10),
    fCloseLogsOnWrite(false),
	fMonitorLANPort(0),
	fMonitorWANPort(0)
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
    this->SetVal(qtssPrefsSessionTimeout,	&fSessionTimeoutInSecs,    sizeof(fSessionTimeoutInSecs));

    this->SetVal(qtssPrefsMaximumConnections,&fMaximumConnections,      sizeof(fMaximumConnections));
    this->SetVal(qtssPrefsMaximumBandwidth, &fMaxBandwidthInKBits,      sizeof(fMaxBandwidthInKBits));
    this->SetVal(qtssPrefsBreakOnAssert,    &fBreakOnAssert,            sizeof(fBreakOnAssert));
    this->SetVal(qtssPrefsAutoRestart,      &fAutoRestart,              sizeof(fAutoRestart));

    this->SetVal(qtssPrefsErrorRollInterval, &fErrorRollIntervalInDays, sizeof(fErrorRollIntervalInDays));
    this->SetVal(qtssPrefsMaxErrorLogSize,  &fErrorLogBytes,            sizeof(fErrorLogBytes));
    this->SetVal(qtssPrefsErrorLogVerbosity, &fErrorLogVerbosity,       sizeof(fErrorLogVerbosity));
    this->SetVal(qtssPrefsScreenLogging,    &fScreenLoggingEnabled,     sizeof(fScreenLoggingEnabled));
    this->SetVal(qtssPrefsErrorLogEnabled,  &fErrorLogEnabled,          sizeof(fErrorLogEnabled));

    this->SetVal(qtssPrefsAutoStart,                &fAutoStart,                sizeof(fAutoStart));

    this->SetVal(qtssPrefsEnableMSGDebugPrintfs,		&fEnableMSGDebugPrintfs,		sizeof(fEnableMSGDebugPrintfs));
    this->SetVal(qtssPrefsRunNumThreads,                &fNumThreads,                   sizeof(fNumThreads));
    this->SetVal(qtssPrefsEnableMonitorStatsFile,       &fEnableMonitorStatsFile,       sizeof(fEnableMonitorStatsFile));
    this->SetVal(qtssPrefsMonitorStatsFileIntervalSec,  &fStatsFileIntervalSeconds,     sizeof(fStatsFileIntervalSeconds));

    this->SetVal(qtssPrefsCloseLogsOnWrite,             &fCloseLogsOnWrite,             sizeof(fCloseLogsOnWrite));
	
    this->SetVal(qtssPrefsMonitorLANPort,				&fMonitorLANPort,          sizeof(fMonitorLANPort));
    this->SetVal(qtssPrefsMonitorWANPort,				&fMonitorWANPort,          sizeof(fMonitorWANPort));
    this->SetVal(qtssPrefsMonitorLANIPAddr,				&fMonitorLANAddr,           sizeof(fMonitorLANAddr));
    this->SetVal(qtssPrefsMonitorWANIPAddr,				&fMonitorWANAddr,            sizeof(fMonitorWANAddr));

	this->SetVal(qtssPrefsServiceID,					&fServiceID,            sizeof(fServiceID));

	this->SetVal(qtssPrefsRedisIPAddr,					&fRedisAddr,            sizeof(fRedisAddr));
	this->SetVal(qtssPrefsRedisPorts,					&fRedisPort,          sizeof(fRedisPort));
    this->SetVal(qtssPrefsNumMsgThreads,				&fNumMsgThreads,               sizeof(fNumMsgThreads));
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

    QTSSRollingLog::SetCloseOnWrite(fCloseLogsOnWrite);
    //
    // In case we made any changes, write out the prefs file
    (void)fPrefsSource->WritePrefsFile();
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

