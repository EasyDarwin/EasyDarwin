
#ifndef __PLBroadcastDef__
#define __PLBroadcastDef__


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

#example (1) A FULL 1.0 DECLARED FILE
#Lines beginning with "#" characters are comments
#The order of the following entries is unimportant
#Quotes are optional for values

destination_ip_address  225.225.225.225 
destination_base_port  5004
play_mode  [sequential, sequential_looped, weighted]
limit_play  enabled
limit_seq_length  10
play_list_file /path/file
sdp_file /path/file
log_file /path/file

*/

#include "OSHeaders.h"
#include "BroadcasterSession.h"

class PLBroadcastDef {

    public:
        PLBroadcastDef( const char* setupFileName,  char *destinationIP, Bool16 debug );
        virtual ~PLBroadcastDef();
        
        Bool16  ParamsAreValid() { return mParamsAreValid; }
        
        void    ValidateSettings();
        void    ShowErrorParams();
        
        void    ShowSettings();
        
        static Bool16 ConfigSetter( const char* paramName, const char* paramValue[], void * userData );

                                        // * == default value, <r> required input
        char*   mDestAddress;           // set by PLB to be resolved address
        char*   mOrigDestAddress;       // [0.0.0.0 | domain name?] *127.0.0.1 ( self )
        char*   mBasePort;              // [ 0...32k?] *5004
        
        
        char*   mPlayMode;              // [sequential | *sequential_looped | weighted]
        // removed at build 12 char*    mLimitPlay;             // [*enabled | disabled]
        int     mLimitPlayQueueLength; // [ 0...32k?] *20
        //char* mLimitPlayQueueLength; // [ 0...32k?] *20
        char*   mPlayListFile;          // [os file path] *<PLBroadcastDef-name>.ply
        char*   mSDPFile;               // [os file path] <r>
        char*   mLogging;               // [*enabled | disabled]
        char*   mLogFile;               // [os file path] *<PLBroadcastDef-name>.log
        char*   mSDPReferenceMovie;     // [os file path]
        char*   mCurrentFile;           // [os file path] *<PLBroadcastDef-name>.current
        char*   mUpcomingFile;          // [os file path] *<PLBroadcastDef-name>.upcoming
        char*   mReplaceFile;           // [os file path] *<PLBroadcastDef-name>.replacelist
        char*   mStopFile;              // [os file path] *<PLBroadcastDef-name>.stoplist
        char*   mInsertFile;            // [os file path] *<PLBroadcastDef-name>.insertlist
        char*   mShowCurrent;           // [*enabled | disabled]
        char*   mShowUpcoming;          // [*enabled | disabled]
        
        BroadcasterSession *mTheSession;// a broadcaster RTSP/RTP session with the server.
        
        bool    mIgnoreFileIP;
        char*   mMaxUpcomingMovieListSize; // [ 2^31] *7
        char*   mDestSDPFile;           // [movies folder relative file path] 
        
        char*   mStartTime;         // NTP start time
        char*   mEndTime;           // NTP end time
        char*   mIsDynamic;         // true
        
        char*   mName;              // Authentication name
        char*   mPassword;          // Authentication password
        
        char *  mTTL;               // TTL for multicast [1..15] *1
        
        char *  mRTSPPort;

        char *  mPIDFile;
        
        char *  mClientBufferDelay;     // sdp option a=x-bufferdelay: float
        
        enum {
            kInvalidNone = 0x00000000,
            kInvalidDestAddress = 0x00000001,
            kBufferLen = 512,
            kExtensionLen = 20,
            kMaxBufferStringLen = kBufferLen - kExtensionLen
        };
        
    protected:
        Bool16  mParamsAreValid;
        UInt32  mInvalidParamFlags;
        Bool16  SetValue( char** dest, const char* value);
        Bool16  SetDefaults( const char* setupFileName );
        Bool16  fDebug;
};

#endif
