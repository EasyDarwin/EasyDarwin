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

#ifndef __MP3BroadcasterLog_h__
#define __MP3BroadcasterLog_h__

#include "QTSSRollingLog.h"
#include "StrPtrLen.h"
#include <string.h>


class MP3BroadcasterLog : public QTSSRollingLog
{
    enum { eLogMaxBytes = 0, eLogMaxDays = 0 };
    
    public:
        MP3BroadcasterLog( char* defaultPath, char* logName, Bool16 enabled );
        virtual ~MP3BroadcasterLog() {}
    
        virtual char* GetLogName() 
        {   // RTSPRollingLog wants to see a "new'd" copy of the file name
            char*   name = new char[strlen( mLogFileName ) + 1 ];
            
            if ( name )
                ::strcpy( name, mLogFileName );

            return name;
        }       
        
        virtual char* GetLogDir() 
        {   // RTSPRollingLog wants to see a "new'd" copy of the file name
            char *name = new char[strlen( mDirPath ) + 1 ];
            
            if ( name )
                ::strcpy( name, mDirPath );

            return name;
        }       
                                                                                
        virtual UInt32 GetRollIntervalInDays() { return eLogMaxDays; /* we dont' roll*/ }
                                            
        virtual UInt32 GetMaxLogBytes() {  return eLogMaxBytes; /* we dont' roll*/ }
        
        void    LogInfo( const char* infoStr );
        void    LogMediaError( const char* path, const char* errStr, const char* messageStr);
        void    LogMediaData( const char* song, const char* title, const char* artist, const char* album, 
                                UInt32 duration, SInt16 result);

        bool    WantsLogging() { return mWantsLogging; }
        const char* LogFileName() { return mLogFileName; }
        const char* LogDirName() { return mDirPath; }
        
        virtual time_t  WriteLogHeader(FILE *inFile);
        
    protected:
        char    mDirPath[256];
        char    mLogFileName[256];
        bool    mWantsLogging;
    
};

#endif
