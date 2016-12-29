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
#include "OSMemory.h"
#include "QTSSDataConverter.h"
#include "defaultPaths.h"
#include "QTSSRollingLog.h"
#include "OS.h"

#ifndef __Win32__
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


QTSServerPrefs::PrefInfo QTSServerPrefs::sPrefInfo[] =
{
	{ kDontAllowMultipleValues, "90",      nullptr                     },  //0 http_session_timeout
	{ kDontAllowMultipleValues, "1000",     nullptr                    },  //1 maximum_connections
	{ kAllowMultipleValues,     "0",        nullptr                    },  //2 bind_ip_addr
	{ kDontAllowMultipleValues, "false",    nullptr                    },  //3 break_on_assert
	{ kDontAllowMultipleValues, "true",     nullptr                    },  //4 auto_restart
	{ kDontAllowMultipleValues,	DEFAULTPATHS_SSM_DIR,	nullptr		},	//5 module_folder
	{ kDontAllowMultipleValues, "Error",    nullptr                    },  //6 error_logfile_name
	{ kDontAllowMultipleValues,	DEFAULTPATHS_LOG_DIR,	nullptr		},	//7 error_logfile_dir
	{ kDontAllowMultipleValues, "7",        nullptr                    },  //8 error_logfile_interval
	{ kDontAllowMultipleValues, "256000",   nullptr                    },  //9 error_logfile_size
	{ kDontAllowMultipleValues, "2",        nullptr                    },  //10 error_logfile_verbosity
	{ kDontAllowMultipleValues, "true",     nullptr                    },  //11 screen_logging
	{ kDontAllowMultipleValues, "true",     nullptr                    },  //12 error_logging
	{ kDontAllowMultipleValues, "./snap/",	nullptr					},  //13 snap_local_path
	{ kDontAllowMultipleValues, "http://snap.easydarwin.org/", nullptr },  //14 snap_web_path
	{ kDontAllowMultipleValues, "false",    nullptr                    },  //15 auto_start
	{ kDontAllowMultipleValues, "false",    nullptr                    },  //16 MSG_debug_printfs
	{ kDontAllowMultipleValues, "false",    nullptr                    },  //17 enable_monitor_stats_file
	{ kDontAllowMultipleValues, "10",       nullptr                    },  //18 monitor_stats_file_interval_seconds
	{ kDontAllowMultipleValues, "server_status",        nullptr        },  //19 monitor_stats_file_name
	{ kDontAllowMultipleValues, "0",        nullptr                    },  //20 run_num_threads
	{ kDontAllowMultipleValues, DEFAULTPATHS_PID_DIR "easycms" ".pid",	nullptr	},	//21 pid_file
	{ kDontAllowMultipleValues, "false",    nullptr                    },  //22 force_logs_close_on_write
	{ kDontAllowMultipleValues, "10000",    nullptr                     }, //23 service_lan_port
	{ kDontAllowMultipleValues, "10000",    nullptr                     }, //24 service_wan_port
	{ kDontAllowMultipleValues, "0.0.0.0",  nullptr                     }, //25 service_wan_ip
	{ kDontAllowMultipleValues, "2",        nullptr                     }  //26 run_num_msg_threads
};

QTSSAttrInfoDict::AttrInfo  QTSServerPrefs::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
	/* 0 */ { "http_session_timeout",						nullptr,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 1 */ { "maximum_connections",                    nullptr,                   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 2 */ { "bind_ip_addr",                           nullptr,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 3 */ { "break_on_assert",                        nullptr,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 4 */ { "auto_restart",                           nullptr,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 5 */ { "module_folder",                          nullptr,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 6 */ { "error_logfile_name",                     nullptr,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 7 */ { "error_logfile_dir",                      nullptr,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 8 */ { "error_logfile_interval",                nullptr,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 9 */ { "error_logfile_size",                    nullptr,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 10 */ { "error_logfile_verbosity",               nullptr,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 11 */ { "screen_logging",                        nullptr,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 12 */ { "error_logging",                         nullptr,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 13 */ { "snap_local_path",						nullptr,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 14 */ { "snap_web_path",							nullptr,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 15 */ { "auto_start",                            nullptr,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 16 */ { "MSG_debug_printfs",						nullptr,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 17 */ { "enable_monitor_stats_file",             nullptr,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 18 */ { "monitor_stats_file_interval_seconds",   nullptr,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 19 */ { "monitor_stats_file_name",               nullptr,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 20 */ { "run_num_threads",                       nullptr,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 21 */ { "pid_file",								nullptr,					qtssAttrDataTypeCharArray,	qtssAttrModeRead | qtssAttrModeWrite },
	/* 22 */ { "force_logs_close_on_write",             nullptr,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 23 */ { "service_lan_port",						nullptr,					qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 24 */ { "service_wan_port",						nullptr,					qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 25 */ { "service_wan_ip",						nullptr,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
	/* 26 */ { "run_num_msg_threads",					nullptr,					qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite }
};

QTSServerPrefs::QTSServerPrefs(XMLPrefsParser* inPrefsSource, bool inWriteMissingPrefs)
	: QTSSPrefs(inPrefsSource, nullptr, QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kPrefsDictIndex), false),
	fSessionTimeoutInSecs(0),
	fMaximumConnections(0),
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
	fEnableMonitorStatsFile(false),
	fStatsFileIntervalSeconds(10),
	fCloseLogsOnWrite(false),
	fMonitorLANPort(0),
	fMonitorWANPort(0)
{
	setupAttributes();
	RereadServerPreferences(inWriteMissingPrefs);

	char jpgDir[512] = { 0 };
	qtss_sprintf(jpgDir, "%s", this->GetSnapLocalPath());
	OS::RecursiveMakeDir(jpgDir);

}

void QTSServerPrefs::Initialize()
{
	for (UInt32 x = 0; x < qtssPrefsNumParams; x++)
		QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kPrefsDictIndex)->
		SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
			sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);
}


void QTSServerPrefs::setupAttributes()
{
	this->SetVal(qtssPrefsSessionTimeout, &fSessionTimeoutInSecs, sizeof(fSessionTimeoutInSecs));

	this->SetVal(qtssPrefsMaximumConnections, &fMaximumConnections, sizeof(fMaximumConnections));
	this->SetVal(qtssPrefsBreakOnAssert, &fBreakOnAssert, sizeof(fBreakOnAssert));
	this->SetVal(qtssPrefsAutoRestart, &fAutoRestart, sizeof(fAutoRestart));

	this->SetVal(qtssPrefsErrorRollInterval, &fErrorRollIntervalInDays, sizeof(fErrorRollIntervalInDays));
	this->SetVal(qtssPrefsMaxErrorLogSize, &fErrorLogBytes, sizeof(fErrorLogBytes));
	this->SetVal(qtssPrefsErrorLogVerbosity, &fErrorLogVerbosity, sizeof(fErrorLogVerbosity));
	this->SetVal(qtssPrefsScreenLogging, &fScreenLoggingEnabled, sizeof(fScreenLoggingEnabled));
	this->SetVal(qtssPrefsErrorLogEnabled, &fErrorLogEnabled, sizeof(fErrorLogEnabled));

	this->SetVal(qtssPrefsAutoStart, &fAutoStart, sizeof(fAutoStart));

	this->SetVal(qtssPrefsEnableMSGDebugPrintfs, &fEnableMSGDebugPrintfs, sizeof(fEnableMSGDebugPrintfs));
	this->SetVal(qtssPrefsRunNumThreads, &fNumThreads, sizeof(fNumThreads));
	this->SetVal(qtssPrefsEnableMonitorStatsFile, &fEnableMonitorStatsFile, sizeof(fEnableMonitorStatsFile));
	this->SetVal(qtssPrefsMonitorStatsFileIntervalSec, &fStatsFileIntervalSeconds, sizeof(fStatsFileIntervalSeconds));

	this->SetVal(qtssPrefsCloseLogsOnWrite, &fCloseLogsOnWrite, sizeof(fCloseLogsOnWrite));

	this->SetVal(qtssPrefsServiceLANPort, &fMonitorLANPort, sizeof(fMonitorLANPort));
	this->SetVal(qtssPrefsServiceWANPort, &fMonitorWANPort, sizeof(fMonitorWANPort));
	this->SetVal(qtssPrefsServiceWANIPAddr, &fMonitorWANAddr, sizeof(fMonitorWANAddr));

	this->SetVal(qtssPrefsNumMsgThreads, &fNumMsgThreads, sizeof(fNumMsgThreads));
}



void QTSServerPrefs::RereadServerPreferences(bool inWriteMissingPrefs)
{
	OSMutexLocker locker(&fPrefsMutex);
	QTSSDictionaryMap* theMap = QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kPrefsDictIndex);

	for (UInt32 x = 0; x < theMap->GetNumAttrs(); x++)
	{
		//
		// Look for a pref in the file that matches each pref in the dictionary
		char* thePrefTypeStr = nullptr;
		char* thePrefName = nullptr;

		ContainerRef server = fPrefsSource->GetRefForServer();
		ContainerRef pref = fPrefsSource->GetPrefRefByName(server, theMap->GetAttrName(x));
		char* thePrefValue = nullptr;
		if (pref != nullptr)
			thePrefValue = fPrefsSource->GetPrefValueByRef(pref, 0, &thePrefName,
			(char**)&thePrefTypeStr);

		if ((thePrefValue == nullptr) && (x < qtssPrefsNumParams)) // Only generate errors for server prefs
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

			this->setPrefValue(x, 0, sPrefInfo[x].fDefaultValue, sAttributes[x].fAttrDataType);
			if (sPrefInfo[x].fAdditionalDefVals != nullptr)
			{
				//
				// Add additional default values if they exist
				for (UInt32 y = 0; sPrefInfo[x].fAdditionalDefVals[y] != nullptr; y++)
					this->setPrefValue(x, y + 1, sPrefInfo[x].fAdditionalDefVals[y], sAttributes[x].fAttrDataType);
			}

			if (inWriteMissingPrefs)
			{
				//
				// Add this value into the file, cuz we need it.
				pref = fPrefsSource->AddPref(server, sAttributes[x].fAttrName, QTSSDataConverter::TypeToTypeString(sAttributes[x].fAttrDataType));
				fPrefsSource->AddPrefValue(pref, sPrefInfo[x].fDefaultValue);

				if (sPrefInfo[x].fAdditionalDefVals != nullptr)
				{
					for (UInt32 a = 0; sPrefInfo[x].fAdditionalDefVals[a] != nullptr; a++)
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

			this->setPrefValue(x, 0, sPrefInfo[x].fDefaultValue, sAttributes[x].fAttrDataType);
			if (sPrefInfo[x].fAdditionalDefVals != nullptr)
			{
				//
				// Add additional default values if they exist
				for (UInt32 z = 0; sPrefInfo[x].fAdditionalDefVals[z] != nullptr; z++)
					this->setPrefValue(x, z + 1, sPrefInfo[x].fAdditionalDefVals[z], sAttributes[x].fAttrDataType);
			}

			if (inWriteMissingPrefs)
			{
				//
				// Remove it out of the file and add in the default.
				fPrefsSource->RemovePref(pref);
				pref = fPrefsSource->AddPref(server, sAttributes[x].fAttrName, QTSSDataConverter::TypeToTypeString(sAttributes[x].fAttrDataType));
				fPrefsSource->AddPrefValue(pref, sPrefInfo[x].fDefaultValue);
				if (sPrefInfo[x].fAdditionalDefVals != nullptr)
				{
					for (UInt32 b = 0; sPrefInfo[x].fAdditionalDefVals[b] != nullptr; b++)
						fPrefsSource->AddPrefValue(pref, sPrefInfo[x].fAdditionalDefVals[b]);
				}
			}
			continue;
		}

		UInt32 theNumValues = 0;
		if ((x < qtssPrefsNumParams) && (!sPrefInfo[x].fAllowMultipleValues))
			theNumValues = 1;

		this->setPrefValuesFromFileWithRef(pref, x, theNumValues);
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
//        inBuffer = new char[theMovieFolder->Len + 2];
//        ::memcpy(inBuffer, theMovieFolder->Ptr, theMovieFolder->Len);
//    }
//    inBuffer[theMovieFolder->Len] = 0;
//    *ioLen = theMovieFolder->Len;
//    return inBuffer;
//}

char* QTSServerPrefs::getStringPref(QTSS_AttributeID inAttrID)
{
	StrPtrLen theBuffer;
	(void)this->GetValue(inAttrID, 0, nullptr, &theBuffer.Len);
	theBuffer.Ptr = new char[theBuffer.Len + 1];
	theBuffer.Ptr[0] = '\0';

	if (theBuffer.Len > 0)
	{
		QTSS_Error theErr = this->GetValue(inAttrID, 0, theBuffer.Ptr, &theBuffer.Len);
		if (theErr == QTSS_NoErr)
			theBuffer.Ptr[theBuffer.Len] = 0;
	}
	return theBuffer.Ptr;
}

void QTSServerPrefs::SetCloseLogsOnWrite(bool closeLogsOnWrite)
{
	QTSSRollingLog::SetCloseOnWrite(closeLogsOnWrite);
	fCloseLogsOnWrite = closeLogsOnWrite;
}

