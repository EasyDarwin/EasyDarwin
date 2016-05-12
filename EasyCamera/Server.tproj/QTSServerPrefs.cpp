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
 
#ifndef __Win32__
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#endif

QTSServerPrefs::PrefInfo QTSServerPrefs::sPrefInfo[] =
{
    { kDontAllowMultipleValues, "0",        NULL                    },  //connection_timeout

    { kDontAllowMultipleValues, "Error",    NULL                    },  //error_logfile_name
	{ kDontAllowMultipleValues,	DEFAULTPATHS_LOG_DIR,	NULL		},	//error_logfile_dir
    { kDontAllowMultipleValues, "7",        NULL                    },  //error_logfile_interval
    { kDontAllowMultipleValues, "10240",   NULL                    },  //error_logfile_size
    { kDontAllowMultipleValues, "2",        NULL                    },  //error_logfile_verbosity
    { kDontAllowMultipleValues, "true",     NULL                    },  //screen_logging
    { kDontAllowMultipleValues, "true",     NULL                    },  //error_logging

	{ kDontAllowMultipleValues, "3",        NULL                    },  //run_num_threads
    { kDontAllowMultipleValues, DEFAULTPATHS_PID_DIR PLATFORM_SERVER_BIN_NAME ".pid",	NULL	},	//pid_file
    { kDontAllowMultipleValues, "false",    NULL                    },   //force_logs_close_on_write

    { kDontAllowMultipleValues, "1",        NULL                     }  //run_num_blocking_threads
   
};

QTSSAttrInfoDict::AttrInfo  QTSServerPrefs::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0 */ { "connection_timeout",						NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },

	/* 1 */ { "error_logfile_name",						NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 2 */ { "error_logfile_dir",						NULL,                   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModeWrite },
    /* 3 */ { "error_logfile_interval",					NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 4 */ { "error_logfile_size",						NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 5 */ { "error_logfile_verbosity",				NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 6 */ { "screen_logging",							NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
    /* 7 */ { "error_logging",							NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },

	/* 8 */ { "run_num_threads",						NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 9 */ { "pid_file",								NULL,					qtssAttrDataTypeCharArray,	qtssAttrModeRead | qtssAttrModeWrite },
    /* 10 */{ "force_logs_close_on_write",				NULL,                   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModeWrite },
	/* 11 */{ "run_num_blocking_threads",				NULL,                   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite }
};


QTSServerPrefs::QTSServerPrefs(XMLPrefsParser* inPrefsSource, Bool16 inWriteMissingPrefs)
:   QTSSPrefs(inPrefsSource, NULL, QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kPrefsDictIndex), false),
    fConnectionTimeoutInSecs(0),
    fErrorRollIntervalInDays(0),
    fErrorLogBytes(0),
    fErrorLogVerbosity(0),
    fScreenLoggingEnabled(true),
    fErrorLogEnabled(false),
    fNumThreads(0),
    fNumBlockingThreads(0),
    fCloseLogsOnWrite(false)
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
    this->SetVal(qtssPrefsConnectionTimeout,&fConnectionTimeoutInSecs,        sizeof(fConnectionTimeoutInSecs));

    this->SetVal(qtssPrefsErrorRollInterval, &fErrorRollIntervalInDays, sizeof(fErrorRollIntervalInDays));
    this->SetVal(qtssPrefsMaxErrorLogSize,  &fErrorLogBytes,            sizeof(fErrorLogBytes));
    this->SetVal(qtssPrefsErrorLogVerbosity, &fErrorLogVerbosity,       sizeof(fErrorLogVerbosity));
    this->SetVal(qtssPrefsScreenLogging,    &fScreenLoggingEnabled,     sizeof(fScreenLoggingEnabled));
    this->SetVal(qtssPrefsErrorLogEnabled,  &fErrorLogEnabled,          sizeof(fErrorLogEnabled));

    this->SetVal(qtssPrefsRunNumThreads,	&fNumThreads,                   sizeof(fNumThreads));

    this->SetVal(qtssPrefsCloseLogsOnWrite,	&fCloseLogsOnWrite,             sizeof(fCloseLogsOnWrite));
    this->SetVal(qtssPrefsNumBlockingThreads,	&fNumBlockingThreads,               sizeof(fNumBlockingThreads));
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

//Bool16 QTSServerPrefs::GetCMSIP(char* outCMSIP)
//{
//	if(outCMSIP == NULL)
//		return false;
//
//#ifndef _WIN32
//    typedef struct hostent HOSTENT;
//    signal(SIGPIPE, SIG_IGN);
//#endif
//	HOSTENT *host_entry = ::gethostbyname(this->GetStringPref(qtssPrefsCMSIPAddr));
//    char ip[20] = {0};
//    if (host_entry == NULL)
//    {
//        switch (h_errno)
//        {
//        case HOST_NOT_FOUND:
//            fputs("The host was not found.\n", stderr);
//            break;
//        case NO_ADDRESS:
//            fputs("The name is valid but it has no address.\n", stderr);
//            break;
//        case NO_RECOVERY:
//            fputs("A non-recoverable name server error occurred.\n", stderr);
//            break;
//        case TRY_AGAIN:
//            fputs("The name server is temporarily unavailable.", stderr);
//            break;
//        }
//        return false;
//    }
//    else
//    {
//        sprintf(outCMSIP, "%s", inet_ntoa(*((struct in_addr *) host_entry->h_addr_list[0])));
//    }
//
//	return true;
//}