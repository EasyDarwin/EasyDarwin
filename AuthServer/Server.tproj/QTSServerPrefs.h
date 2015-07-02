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

        QTSServerPrefs(XMLPrefsParser* inPrefsSource, Bool16 inWriteMissingPrefs);
        virtual ~QTSServerPrefs() {}
        
        //This is callable at any time, and is thread safe wrt to the accessors.
        //Pass in true if you want this function to update the prefs file if
        //any defaults need to be used. False otherwise
        void RereadServerPreferences(Bool16 inWriteMissingPrefs);
        
        //Individual accessor methods for preferences.
        
        //Amount of idle time after which respective protocol sessions are timed out
        //(stored in seconds)
        
        //This is the real timeout
        UInt32  GetSessionTimeoutInSecs(){ return fSessionTimeoutInSecs; }
        
        //-1 means unlimited
        SInt32  GetMaxConnections()         { return fMaximumConnections; }
        
        //for debugging, mainly
        Bool16      ShouldServerBreakOnAssert()         { return fBreakOnAssert; }

        // For the compiled-in error logging module
        Bool16  IsErrorLogEnabled()             { return fErrorLogEnabled; }
        Bool16  IsScreenLoggingEnabled()        { return fScreenLoggingEnabled; }

        UInt32  GetMaxErrorLogBytes()           { return fErrorLogBytes; }
        UInt32  GetErrorRollIntervalInDays()    { return fErrorRollIntervalInDays; }
        UInt32  GetErrorLogVerbosity()          { return fErrorLogVerbosity; }
        void      SetErrorLogVerbosity(UInt32 verbosity)        { fErrorLogVerbosity = verbosity; }
        Bool16  GetMSGDebugPrintfs()           { return fEnableMSGDebugPrintfs; }
        //
        // force logs to close after each write (true or false)
        Bool16  GetCloseLogsOnWrite()           { return fCloseLogsOnWrite; }
        void    SetCloseLogsOnWrite(Bool16 closeLogsOnWrite);
                
        // String preferences. Note that the pointers returned here is allocated
        // memory that you must delete!
        char*   GetErrorLogDir()
            { return this->GetStringPref(qtssPrefsErrorLogDir); }
        char*   GetErrorLogName()
            { return this->GetStringPref(qtssPrefsErrorLogName); }


        char*   GetRunUserName()
            { return this->GetStringPref(qtssPrefsRunUserName); }
        char*   GetRunGroupName()
            { return this->GetStringPref(qtssPrefsRunGroupName); }

        char*   GetPidFilePath()
            { return this->GetStringPref(qtssPrefsPidFile); }

        char*   GetStatsMonitorFileName()
            { return this->GetStringPref(qtssPrefsMonitorStatsFileName); }

        Bool16 ServerStatFileEnabled()      { return fEnableMonitorStatsFile; }
        UInt32 GetStatFileIntervalSec()     { return fStatsFileIntervalSeconds; }
                
        UInt32  GetNumThreads()                   { return fNumThreads; } //short tasks threads
        UInt32  GetNumBlockingThreads() { return fNumMsgThreads; } //return the number of threads that long tasks will be scheduled on -- RTSP processing for example.
    private:

        UInt32  fSessionTimeoutInSecs;
        SInt32  fMaximumConnections;
        Bool16  fBreakOnAssert;

        UInt32  fErrorRollIntervalInDays;
        UInt32  fErrorLogBytes;
        UInt32  fErrorLogVerbosity;
        Bool16  fScreenLoggingEnabled;
        Bool16  fErrorLogEnabled;
        
        Bool16  fEnableMSGDebugPrintfs;
        UInt32  fNumThreads;
        UInt32  fNumMsgThreads;
        
        Bool16  fEnableMonitorStatsFile;
        UInt32  fStatsFileIntervalSeconds;
    
        Bool16  fCloseLogsOnWrite;

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

        // Returns the string preference with the specified ID. If there
        // was any problem, this will return an empty string.
        char* GetStringPref(QTSS_AttributeID inAttrID);
        
        static QTSSAttrInfoDict::AttrInfo   sAttributes[];
        static PrefInfo sPrefInfo[];
};
#endif //__QTSSPREFS_H__
