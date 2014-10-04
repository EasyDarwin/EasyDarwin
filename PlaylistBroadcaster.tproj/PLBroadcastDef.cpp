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

    8.2.99 - rt updated ShowSettings() to display user names for fields instead of C++ member names.
*/

#include "PLBroadcastDef.h"
#include "MyAssert.h"
#include "SocketUtils.h"

#include "ConfParser.h"
#include <string.h>

#include <stdio.h>  
#include <stdlib.h>
#include "SafeStdLib.h"
#ifndef __Win32__
#include <netdb.h>
#endif

#include "BroadcasterSession.h"

Bool16 PLBroadcastDef::ConfigSetter( const char* paramName, const char* paramValue[], void* userData )
{
    // return true if set fails
    
    
    PLBroadcastDef* broadcastParms = (PLBroadcastDef*)userData;
    
    if (!::strcmp( "destination_ip_address", paramName) )
    {   
        broadcastParms->SetValue( &broadcastParms->mOrigDestAddress, paramValue[0] );
        if (broadcastParms->mIgnoreFileIP)
            return false;
        else
            return broadcastParms->SetValue( &broadcastParms->mDestAddress, paramValue[0] );
    }
    else if (!::strcmp( "destination_base_port", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mBasePort, paramValue[0] );

    }
    else if (!::strcmp( "max_upcoming_list_size", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mMaxUpcomingMovieListSize, paramValue[0] );

    }
    else if (!::strcmp( "play_mode", paramName) )
    {
            
        if ( ::strcmp( "sequential", paramValue[0]) 
            && ::strcmp( "sequential_looped", paramValue[0])
            && ::strcmp( "weighted_random", paramValue[0])
            )
            return true;

        return broadcastParms->SetValue( &broadcastParms->mPlayMode, paramValue[0] );

    }   
    /*
    rt- rremoved for buld 12
    else if (!::strcmp( "limit_play", paramName) )
    {
        if ( ::strcmp( "enabled", paramValue[0]) && ::strcmp( "disabled", paramValue[0]) )
            return true;

        return broadcastParms->SetValue( &broadcastParms->mLimitPlay, paramValue[0] );

    }
    */
    // changed at bulid 12 else if (!::strcmp( "repeats_queue_size", paramName) )
    else if (!::strcmp( "recent_movies_list_size", paramName) )
    {
        if ( ::atoi( paramValue[0] )  < 0 )
            return true;
            
        broadcastParms->mLimitPlayQueueLength = atoi(paramValue[0]);

        return false;
        
    }
    else if (!::strcmp( "playlist_file", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mPlayListFile, paramValue[0] );

    }
    else if (!::strcmp( "sdp_file", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mSDPFile, paramValue[0] );

    }
    else if (!::strcmp( "destination_sdp_file", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mDestSDPFile, paramValue[0] );

    }
    else if (!::strcmp( "logging", paramName) )
    {
        if ( ::strcmp( "enabled", paramValue[0]) && ::strcmp( "disabled", paramValue[0]) )
            return true;

        return broadcastParms->SetValue( &broadcastParms->mLogging, paramValue[0] );

    }
    else if (!::strcmp( "log_file", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mLogFile, paramValue[0] );

    }
    else if (!::strcmp( "sdp_reference_movie", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mSDPReferenceMovie, paramValue[0] );

    }
    else if (!::strcmp( "show_current", paramName) )
    {
        if ( ::strcmp( "enabled", paramValue[0]) && ::strcmp( "disabled", paramValue[0]) )
            return true;
            
        return broadcastParms->SetValue( &broadcastParms->mShowCurrent, paramValue[0] );

    }
    else if (!::strcmp( "show_upcoming", paramName) )
    {
        if ( ::strcmp( "enabled", paramValue[0]) && ::strcmp( "disabled", paramValue[0]) )
            return true;
            
        return broadcastParms->SetValue( &broadcastParms->mShowUpcoming, paramValue[0] );

    }
	else if (!::strcmp( "broadcast_start_time", paramName) )
	{
	    
        const char* theValue = paramValue[0];
        if ('*' == theValue[0])
	    {    
	        UInt32 startTime =  time(NULL) + 2208988800LU + (time_t) ::strtol(&theValue[1], NULL, 10);
	        char startTimeStr[20] = "";
	        qtss_sprintf(startTimeStr,"%"_U32BITARG_"", startTime); // current time
		    return broadcastParms->SetValue( &broadcastParms->mStartTime, startTimeStr );            
        }
          
		return broadcastParms->SetValue( &broadcastParms->mStartTime, paramValue[0] );
	}
    else if (!::strcmp( "broadcast_end_time", paramName) )
    {
        UInt32 endTime = 0;
        const char* theValue = paramValue[0];
        if ('*' == theValue[0])
            endTime =  time(NULL) + 2208988800LU + (SInt32) ::strtol(&theValue[1], NULL, 10);
        else 
            endTime = ::strtoul(theValue, NULL, 10);

        if ( (endTime > 0) && endTime <  2208988800LU) // not a valid time
            return true;
                
        char endTimeStr[20] = "";
        qtss_sprintf(endTimeStr,"%"_U32BITARG_"", endTime); // current time + offset time
        return broadcastParms->SetValue( &broadcastParms->mEndTime, endTimeStr ); 
             
    }
    else if (!::strcmp( "broadcast_SDP_is_dynamic", paramName) )
    {
        if ( ::strcmp( "enabled", paramValue[0]) && ::strcmp( "disabled", paramValue[0]) )
            return true;
            
        return broadcastParms->SetValue( &broadcastParms->mIsDynamic, paramValue[0] );
    }
    else if (!::strcmp( "broadcaster_name", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mName, paramValue[0] );

    }
    else if (!::strcmp( "broadcaster_password", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mPassword, paramValue[0] );

    }
    else if (!::strcmp( "multicast_ttl", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mTTL, paramValue[0] );

    }
    else if (!::strcmp( "rtsp_port", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mRTSPPort, paramValue[0] );

    }
    else if (!::strcmp( "pid_file", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mPIDFile, paramValue[0] );

    }
    else if (!::strcmp( "client_buffer_delay", paramName) )
    {
        return broadcastParms->SetValue( &broadcastParms->mClientBufferDelay, paramValue[0] );

    }
    else if (!::strcmp( "max_err_file_k_size", paramName) )
    {
		if ( !paramValue[0] || !strlen(paramValue[0]) )
			return true;

  		UInt32 setvalue = kSInt32_Max;
		int maxValue = ::atoi( paramValue[0] );
  		if (maxValue >= 0)
			setvalue = (UInt32) maxValue;
		
		qtss_setmaxprintfcharsinK( (UInt32) setvalue);
        return false;
    }

    
    return true;
}

Bool16 PLBroadcastDef::SetValue( char** dest, const char* value)
{
    Bool16  didFail = false;
        
    // if same param occurs more than once in file, delete 
    // initial occurance and override with second.
    if ( *dest )
        delete [] *dest;
        
    *dest = new char[ strlen(value) + 1 ];
    Assert( *dest );
    
    if ( *dest )
        ::strcpy( *dest, value );
    else 
        didFail = true;

    return didFail;
}

Bool16 PLBroadcastDef::SetDefaults( const char* setupFileName )
{
    Bool16  didFail = false;
    
    if (mDestAddress != NULL)
        mIgnoreFileIP = true;
        
    if ( !didFail && !mIgnoreFileIP)    
        didFail = this->SetValue( &mDestAddress, SocketUtils::GetIPAddrStr(0)->Ptr );

    if ( !didFail ) 
        didFail = this->SetValue( &mBasePort, "5004" );

    if ( !didFail ) 
        didFail = this->SetValue( &mPlayMode, "sequential_looped" );
    
    if ( !didFail ) 
        didFail = this->SetValue( &mMaxUpcomingMovieListSize, "7" );
    
    if ( !didFail ) 
        didFail = this->SetValue( &mLogging, "enabled" );

    if ( !didFail ) 
        didFail = this->SetValue( &mRTSPPort, "554" );

    char    nameBuff[kBufferLen];
    int     maxNameLen = kMaxBufferStringLen;  //maxNameLen = 492
    nameBuff[ sizeof(nameBuff) -1] = 0; //term buffer
    ::strncpy( nameBuff, "broadcast" , maxNameLen);
    
    if (setupFileName)
        ::strncpy( nameBuff, setupFileName , maxNameLen);

    nameBuff[maxNameLen] = '\0'; //zero term the name
        
    int baseLen = ::strlen(nameBuff); //maxNameLen max

//add .log to the base name of the description file with the .ext stripped

    char    *ext = NULL;
    ext = ::strrchr( nameBuff, '.' );
    if ( ext )
    {   
        *ext  = 0;
        baseLen = ::strlen(nameBuff);        
    }
    nameBuff[baseLen] = 0;

        
    ::strncat( nameBuff, ".log",sizeof(nameBuff) - strlen(nameBuff) - 1 );   
    if ( !didFail )
        didFail = this->SetValue( &mLogFile, nameBuff );

    nameBuff[baseLen] = 0;  
    ::strncat( nameBuff, ".ply" ,sizeof(nameBuff) - strlen(nameBuff) - 1);   
    if ( !didFail ) 
        didFail = this->SetValue( &mPlayListFile, nameBuff );
    
    
    nameBuff[baseLen] = 0;
    ::strncat( nameBuff, ".sdp" ,sizeof(nameBuff) - strlen(nameBuff) - 1 );
    if ( !didFail ) 
        didFail = this->SetValue( &mSDPFile, nameBuff );

    if ( !didFail ) 
        didFail = this->SetValue( &mDestSDPFile, "no_name" );
    

/* current, upcoming, and replacelist created by emil@popwire.com */
    nameBuff[baseLen] = 0;
    ::strncat( nameBuff, ".current" ,sizeof(nameBuff) - strlen(nameBuff) - 1 );   
    if ( !didFail )
        didFail = this->SetValue( &mCurrentFile, nameBuff );

    nameBuff[baseLen] = 0;
    ::strncat( nameBuff, ".upcoming"  ,sizeof(nameBuff) - strlen(nameBuff) - 1);  
    if ( !didFail )
        didFail = this->SetValue( &mUpcomingFile, nameBuff );

    nameBuff[baseLen] = 0;
    ::strncat( nameBuff, ".replacelist" ,sizeof(nameBuff) - strlen(nameBuff) - 1 );   
    if ( !didFail )
        didFail = this->SetValue( &mReplaceFile, nameBuff );

    nameBuff[baseLen] = 0;
    ::strncat( nameBuff, ".stoplist"  ,sizeof(nameBuff) - strlen(nameBuff) - 1);  
    if ( !didFail )
        didFail = this->SetValue( &mStopFile, nameBuff );
        
    nameBuff[baseLen] = 0;
    ::strncat( nameBuff, ".insertlist" ,sizeof(nameBuff) - strlen(nameBuff) - 1 );    
    if ( !didFail )
        didFail = this->SetValue( &mInsertFile, nameBuff );
    
    if ( !didFail ) 
        didFail = this->SetValue( &mShowCurrent, "enabled" );
        
    if ( !didFail ) 
        didFail = this->SetValue( &mShowUpcoming, "enabled" );
        
    if ( !didFail ) 
        didFail = this->SetValue( &mStartTime, "0" );

    if ( !didFail ) 
        didFail = this->SetValue( &mEndTime, "0" );

    if ( !didFail ) 
        didFail = this->SetValue( &mIsDynamic, "disabled" );
        
    if ( !didFail ) 
        didFail = this->SetValue( &mName, "" );

    if ( !didFail ) 
        didFail = this->SetValue( &mPassword, "" );

    if ( !didFail ) 
        didFail = this->SetValue( &mTTL, "1" );

    if ( !didFail ) 
        didFail = this->SetValue( &mClientBufferDelay, "0" );
   
    //see if there is a defaults File.
    //if there is one load it and over-ride the other defaults
    if (NULL != setupFileName)
    {
       int len = ::strlen(setupFileName) + 10;
       char *defaultFileName = new char[len];
       qtss_snprintf(defaultFileName, len, "%s%s", setupFileName, ".def");
       (void) ::ParseConfigFile( false, defaultFileName, ConfigSetter, this ); //ignore if no defaults file
       delete [] defaultFileName;
    }
 
/* ***************************************************** */
    return didFail;
}


PLBroadcastDef::PLBroadcastDef( const char* setupFileName,  char *destinationIP, Bool16 debug )
    : mDestAddress(destinationIP)
    , mOrigDestAddress(NULL)
    , mBasePort(NULL)
    , mPlayMode(NULL)
    // removed at build 12 , mLimitPlay(NULL)
    , mLimitPlayQueueLength(0)
    , mPlayListFile(NULL)
    , mSDPFile(NULL)
    , mLogging(NULL)
    , mLogFile(NULL)
    , mSDPReferenceMovie( NULL )
    , mCurrentFile( NULL )
    , mUpcomingFile( NULL )
    , mReplaceFile( NULL )
    , mStopFile( NULL )
    , mInsertFile( NULL )
    , mShowCurrent( NULL )
    , mShowUpcoming( NULL )
    , mTheSession( NULL )
    , mIgnoreFileIP(false)
    , mMaxUpcomingMovieListSize(NULL)
    , mDestSDPFile(NULL)
    , mStartTime(NULL)
    , mEndTime(NULL)
    , mIsDynamic(NULL)
    , mName( NULL)
    , mPassword( NULL)
    , mTTL(NULL)
    , mRTSPPort(NULL)
    , mPIDFile(NULL)
    , mClientBufferDelay(NULL)
    , mParamsAreValid(false)
    , mInvalidParamFlags(kInvalidNone)
    
{

    if (!setupFileName && !destinationIP)
    {   this->SetDefaults( NULL );
        qtss_printf( "default settings\n" ); 
        this->ShowSettings();
        return;
    }

    fDebug = debug;
    if (destinationIP != NULL) //we were given an IP to use
        mIgnoreFileIP = true;
            
    Assert( setupFileName );
    
    if (setupFileName )
    {
        int err = -1;
    
        if ( !this->SetDefaults( setupFileName ) )
        {   err = ::ParseConfigFile( false, setupFileName, ConfigSetter, this );
        }


        if ( !err )
        {   mParamsAreValid = true;     
        }
        
        ValidateSettings();
    }
}

void PLBroadcastDef::ValidateSettings()
{

    // For now it just validates the destination ip address
    UInt32 inAddr = 0;
    inAddr = SocketUtils::ConvertStringToAddr(mDestAddress);
    if(inAddr == INADDR_NONE)
    {
        struct hostent* theHostent = ::gethostbyname(mDestAddress);     
        if (theHostent != NULL)
        {
            inAddr = ntohl(*(UInt32*)(theHostent->h_addr_list[0]));
            
            struct in_addr inAddrStruct;
            char buffer[50];
            StrPtrLen temp(buffer);
            inAddrStruct.s_addr = *(UInt32*)(theHostent->h_addr_list[0]);
            SocketUtils::ConvertAddrToString(inAddrStruct, &temp);
            SetValue( &mDestAddress, buffer );
        }
    }
    if(inAddr == INADDR_NONE)
        mInvalidParamFlags |= kInvalidDestAddress;

    // If mInvalidParamFlags is set, set mParamsAreValid to false
    if ( mInvalidParamFlags | kInvalidNone )
        mParamsAreValid = false;
}

void PLBroadcastDef::ShowErrorParams()
{
    if( mInvalidParamFlags & kInvalidDestAddress )
        qtss_printf( "destination_ip_address \"%s\" is Invalid\n", mOrigDestAddress );
}


void PLBroadcastDef::ShowSettings()
{
    
    
    qtss_printf( "\n" );
    qtss_printf( "Description File Settings\n" );
    qtss_printf( "----------------------------\n" );
        
    qtss_printf( "destination_ip_address  %s\n", mOrigDestAddress );
    qtss_printf( "destination_sdp_file  %s\n", mDestSDPFile );
    qtss_printf( "destination_base_port  %s\n", mBasePort );
    qtss_printf( "play_mode  %s\n", mPlayMode );
    qtss_printf( "recent_movies_list_size  %d\n", mLimitPlayQueueLength );
    qtss_printf( "playlist_file  %s\n", mPlayListFile );
    qtss_printf( "logging  %s\n", mLogging );
    qtss_printf( "log_file  %s\n", mLogFile );
    if (mSDPReferenceMovie != NULL)
        qtss_printf( "sdp_reference_movie  %s\n", mSDPReferenceMovie );
    qtss_printf( "sdp_file  %s\n", mSDPFile );
    qtss_printf( "max_upcoming_list_size  %s\n", mMaxUpcomingMovieListSize );
    qtss_printf( "show_current  %s\n", mShowCurrent );
    qtss_printf( "show_upcoming  %s\n", mShowUpcoming );
    qtss_printf( "broadcaster_name \"%s\"\n", mName);
    qtss_printf( "broadcaster_password \"XXXXX\"\n");
    qtss_printf( "multicast_ttl %s\n",mTTL);
    qtss_printf( "rtsp_port %s\n",mRTSPPort);

    Float32 bufferDelay = 0.0;
    ::sscanf(mClientBufferDelay, "%f", &bufferDelay);
    if (bufferDelay != 0.0) 
        qtss_printf( "client_buffer_delay %.2f\n",bufferDelay);
    else
        qtss_printf( "client_buffer_delay default\n");
    
    if (mPIDFile != NULL)
        qtss_printf( "pid_file %s\n",mPIDFile);
    
    qtss_printf( "broadcast_SDP_is_dynamic  %s\n", mIsDynamic );

    UInt32 startTime = (UInt32) ::strtoul(mStartTime, NULL, 10);
    if ( startTime > 2208988800LU)
    {
        qtss_printf( "broadcast_start_time %s (NTP seconds)\n",mStartTime);

        startTime -= 2208988800LU; //1970 - 1900 secs      
        qtss_printf( "-->broadcast_start_time = %"_U32BITARG_" (unix seconds)\n",startTime);
        
        time_t tmpTime;
        tmpTime = (time_t) startTime;
        struct tm  timeResult;
        struct tm *localTM = qtss_localtime(&tmpTime, &timeResult);
        char timeBuffer[kTimeStrSize];
        char *theTime = qtss_asctime(localTM,timeBuffer, sizeof(timeBuffer));
        if (theTime[0] != 0)
            theTime[::strlen(theTime) -1] = 0;
        qtss_printf( "-->broadcast_start_time = %s (local time)\n",theTime);
        
        tmpTime = (time_t) startTime;
        struct tm *gmTM = qtss_gmtime(&tmpTime, &timeResult);
        theTime = qtss_asctime(gmTM, timeBuffer, sizeof(timeBuffer));
        if (theTime[0] != 0)
            theTime[::strlen(theTime) -1] = 0;
        qtss_printf( "-->broadcast_start_time = %s (UTC/GM time)\n",theTime);
    }
    else if (0 == startTime)
        qtss_printf( "broadcast_start_time   0 (allow all)\n");    
    else
        qtss_printf( "broadcast_start_time   %s (NTPseconds allow all)\n", mStartTime);
    
    UInt32 endTime = strtoul(mEndTime, NULL, 10);
    if (endTime > 2208988800LU)
    {
        qtss_printf( "broadcast_end_time   %s (NTP seconds)\n",mEndTime);
        
        endTime -= 2208988800LU;//convert to 1970 secs
        qtss_printf( "-->broadcast_end_time   = %"_U32BITARG_" (unix seconds)\n",endTime);
        
        time_t tmpTime = (time_t) endTime;
        struct tm  timeResult;
        struct tm *localTM = qtss_localtime(&tmpTime, &timeResult);
        char timeBuffer[kTimeStrSize];
        char *theTime = qtss_asctime(localTM,timeBuffer, sizeof(timeBuffer));
        if (theTime[0] != 0)
            theTime[::strlen(theTime) -1] = 0;
        qtss_printf( "-->broadcast_end_time   = %s (local time)\n",theTime);
        
        tmpTime = (time_t) endTime;
        struct tm *gmTM = qtss_gmtime(&tmpTime, &timeResult);
        theTime = qtss_asctime(gmTM, timeBuffer, sizeof(timeBuffer));
        if (theTime[0] != 0)
            theTime[::strlen(theTime) -1] = 0;
        qtss_printf( "-->broadcast_end_time   = %s (UTC/GM time)\n",theTime);
    }
    else if (0 == endTime)
        qtss_printf( "broadcast_end_time   0 (unbounded)\n");
    else
        qtss_printf( "broadcast_end_time   1900 + %s seconds (looks invalid)\n", mEndTime);
        
	qtss_printf( "max_err_file_k_size %"_U32BITARG_"\n", qtss_getmaxprintfcharsinK());

    
    qtss_printf( "============================\n" );
    
}


PLBroadcastDef::~PLBroadcastDef()
{
    delete [] mDestAddress;
    delete [] mOrigDestAddress;
    delete [] mBasePort;
    delete [] mPlayMode;
    // removed at build 12 delete [] mLimitPlay;
    delete [] mPlayListFile;
    delete [] mSDPFile;
    delete [] mLogging;
    delete [] mLogFile;
    if (mSDPReferenceMovie != NULL)
        delete [] mSDPReferenceMovie;
    delete [] mMaxUpcomingMovieListSize;
    delete [] mTTL;
    delete [] mName;
    delete [] mShowUpcoming;
    delete [] mShowCurrent;
    delete [] mPassword;
    delete [] mIsDynamic;
    delete [] mStartTime;
    delete [] mEndTime;
    
}
