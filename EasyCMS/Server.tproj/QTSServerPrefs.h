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
    Contains:   Object store for HTTP server preferences.
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
    static void Initialize();

    QTSServerPrefs(XMLPrefsParser* inPrefsSource, Bool16 inWriteMissingPrefs);
    virtual ~QTSServerPrefs() {}

    //This is callable at any time, and is thread safe wrt to the accessors.
    //Pass in true if you want this function to update the prefs file if
    //any defaults need to be used. False otherwise
    void RereadServerPreferences(Bool16 inWriteMissingPrefs);

    //Individual accessor methods for preferences.

    //This is the real timeout
    UInt32  GetSessionTimeoutInSecs() { return fSessionTimeoutInSecs; }

    //-1 means unlimited
    SInt32  GetMaxConnections() { return fMaximumConnections; }

    //for debugging, mainly
    Bool16      ShouldServerBreakOnAssert() { return fBreakOnAssert; }
    Bool16      IsAutoRestartEnabled() { return fAutoRestart; }

    // For the compiled-in error logging module

    Bool16  IsErrorLogEnabled() { return fErrorLogEnabled; }
    Bool16  IsScreenLoggingEnabled() { return fScreenLoggingEnabled; }

    UInt32  GetMaxErrorLogBytes() { return fErrorLogBytes; }
    UInt32  GetErrorRollIntervalInDays() { return fErrorRollIntervalInDays; }
    UInt32  GetErrorLogVerbosity() { return fErrorLogVerbosity; }
    void    SetErrorLogVerbosity(UInt32 verbosity) { fErrorLogVerbosity = verbosity; }

    Bool16  GetMSGDebugPrintfs() { return fEnableMSGDebugPrintfs; }

    // force logs to close after each write (true or false)
    Bool16  GetCloseLogsOnWrite() { return fCloseLogsOnWrite; }
    void    SetCloseLogsOnWrite(Bool16 closeLogsOnWrite);

    // String preferences. Note that the pointers returned here is allocated
    // memory that you must delete!

    char*   GetErrorLogDir()
    {
        return this->getStringPref(qtssPrefsErrorLogDir);
    }
    char*   GetErrorLogName()
    {
        return this->getStringPref(qtssPrefsErrorLogName);
    }

    char*	GetServiceID()
    {
        return this->getStringPref(qtssPrefsServiceID);
    }

    char*   GetModuleDirectory()
    {
        return this->getStringPref(qtssPrefsModuleFolder);
    }

    char*   GetPidFilePath()
    {
        return this->getStringPref(qtssPrefsPidFile);
    }

    char*   GetStatsMonitorFileName()
    {
        return this->getStringPref(qtssPrefsMonitorStatsFileName);
    }

    Bool16 ServerStatFileEnabled() { return fEnableMonitorStatsFile; }
    UInt32 GetStatFileIntervalSec() { return fStatsFileIntervalSeconds; }

    UInt32  GetNumThreads() { return fNumThreads; }
    UInt32  GetNumBlockingThreads() { return fNumMsgThreads; }

    UInt16  GetMonitorLANPort() { return fMonitorLANPort; }
    UInt16  GetMonitorWANPort() { return fMonitorWANPort; }

    char* GetMonitorLANIP() { return this->getStringPref(qtssPrefsMonitorLANIPAddr); }
    char* GetMonitorWANIP() { return this->getStringPref(qtssPrefsMonitorWANIPAddr); }

    char* GetSnapLocalPath() { return this->getStringPref(qtssPrefsSnapLocalPath); }
    char* GetSnapWebPath() { return this->getStringPref(qtssPrefsSnapWebPath); }
private:
    void setupAttributes();

    // Returns the string preference with the specified ID. If there
    // was any problem, this will return an empty string.
    char* getStringPref(QTSS_AttributeID inAttrID);

    UInt32      fSessionTimeoutInSecs;

    SInt32  fMaximumConnections;

    Bool16  fBreakOnAssert;
    Bool16  fAutoRestart;

    UInt32  fErrorRollIntervalInDays;
    UInt32  fErrorLogBytes;
    UInt32  fErrorLogVerbosity;
    Bool16  fScreenLoggingEnabled;
    Bool16  fErrorLogEnabled;

    Bool16  fAutoStart;
    Bool16  fEnableMSGDebugPrintfs;

    UInt32  fNumThreads;
    UInt32  fNumMsgThreads;

    Bool16  fEnableMonitorStatsFile;
    UInt32  fStatsFileIntervalSeconds;

    Bool16  fCloseLogsOnWrite;

    UInt16 fMonitorLANPort;
    UInt16 fMonitorWANPort;

    char   fMonitorWANAddr[20];
    char   fMonitorLANAddr[20];
    char   fServiceID[64];

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

    static QTSSAttrInfoDict::AttrInfo   sAttributes[];
    static PrefInfo sPrefInfo[];
};
#endif //__QTSSPREFS_H__
