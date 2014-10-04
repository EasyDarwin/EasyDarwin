
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



#include "MP3BroadcasterLog.h"

#ifndef kVersionString
#include "revision.h"
#endif

static Bool16 sLogTimeInGMT = false;

static char* sLogHeader =   "#Software: %s\n"
                            "#Version: %s\n"    //%s == version
                            "#Date: %s\n"       //%s == date/time
                            "#Remark: All date values are in %s.\n" //%s == "GMT" or "local time"
                            "#Fields: date time filepath title artist album duration result\n";

MP3BroadcasterLog::MP3BroadcasterLog( char* defaultPath, char* logName, Bool16 enabled ) 
            : QTSSRollingLog() 
{
    this->SetTaskName("MP3BroadcasterLog");
    *mDirPath = 0;
    *mLogFileName = 0;
    mWantsLogging = false;
    
    if (enabled)
    {
        mWantsLogging = true;
        
        // check if logName is a full path
        ::strcpy( mDirPath, logName );
        char*   nameBegins = ::strrchr( mDirPath, kPathDelimiterChar );
        if ( nameBegins )
        {
            *nameBegins = 0; // terminate mDirPath at the last PathDelimeter
            nameBegins++;
            ::strcpy( mLogFileName, nameBegins );
        }
        else
        {   // it was just a file name, no dir spec'd
            ::strcpy( mDirPath, defaultPath );
            ::strcpy( mLogFileName, logName );
        }
        
    }
    
   this->SetLoggingEnabled(mWantsLogging);

}

time_t MP3BroadcasterLog::WriteLogHeader(FILE *inFile)
{
    // Write a W3C compatable log header
    time_t calendarTime = ::time(NULL);
    Assert(-1 != calendarTime);
    if (-1 == calendarTime)
        return -1;

    struct tm  timeResult;
    struct tm* theLocalTime = qtss_localtime(&calendarTime, &timeResult);
    Assert(NULL != theLocalTime);
    if (NULL == theLocalTime)
        return -1;
    
    char tempBuffer[1024] = { 0 };
    qtss_strftime(tempBuffer, sizeof(tempBuffer), "#Log File Created On: %m/%d/%Y %H:%M:%S\n", theLocalTime);
    this->WriteToLog(tempBuffer, !kAllowLogToRoll);
    tempBuffer[0] = '\0';
    
    // format a date for the startup time
    
    char theDateBuffer[QTSSRollingLog::kMaxDateBufferSizeInBytes] = { 0 };
    Bool16 result = QTSSRollingLog::FormatDate(theDateBuffer, false);
    
    if (result)
    {
        qtss_sprintf(tempBuffer, sLogHeader, "MP3Broadcaster" , kVersionString, 
                            theDateBuffer, sLogTimeInGMT ? "GMT" : "local time");
        this->WriteToLog(tempBuffer, !kAllowLogToRoll);
    }
        
    return calendarTime;
}


void    MP3BroadcasterLog::LogInfo( const char* infoStr )
{
    // log a generic comment 
    char    strBuff[1024] = "";
    char    dateBuff[80] = "";
    
    if ( this->FormatDate( dateBuff, false ) )
    {   
        if  (   (NULL != infoStr) 
            &&  ( ( strlen(infoStr) + strlen(strBuff) + strlen(dateBuff)  ) < 800)
            )
        {
            qtss_sprintf(strBuff,"#Remark: %s %s\n",dateBuff, infoStr);
            this->WriteToLog( strBuff, kAllowLogToRoll );
        }
        else
        {   
            ::strcat(strBuff,dateBuff);
            ::strcat(strBuff," internal error in LogInfo\n");
            this->WriteToLog( strBuff, kAllowLogToRoll );       
        }

    }
    
}


void MP3BroadcasterLog::LogMediaError( const char* path, const char* errStr , const char* messageStr)
{
    // log movie play info
    char    strBuff[1024] = "";
    char    dateBuff[80] = "";
    
    if ( this->FormatDate( dateBuff, false ) )
    {   
        if  (   (NULL != path) 
                &&  ( (strlen(path) + strlen(dateBuff) ) < 800)
            )
        {

            qtss_sprintf(strBuff,"#Remark: %s %s ",dateBuff, path);
                    
            if ( errStr )
            {   if  ( (strlen(strBuff) + strlen(errStr) ) < 1000 )
                {
                    ::strcat(strBuff,"Error:");
                    ::strcat(strBuff,errStr);
                }
            }
            else
                if  (   (NULL != messageStr) 
                        && 
                        ( (strlen(strBuff) + strlen(messageStr) ) < 1000 )
                    )
                {   ::strcat(strBuff,messageStr);
                }
                else
                    ::strcat(strBuff,"OK");
                
            ::strcat(strBuff,"\n");
            this->WriteToLog(strBuff, kAllowLogToRoll );
        }
        else
        {   
            ::strcat(strBuff,dateBuff);
            ::strcat(strBuff," internal error in LogMediaError\n");
            this->WriteToLog( strBuff, kAllowLogToRoll );       
        }

    }
    
}

void MP3BroadcasterLog::LogMediaData( const char* song, const char* title, const char* artist, const char* album, 
                                UInt32 duration, SInt16 result)
{
    // log movie play info
    char    strBuff[1024] = "";
    char    dateBuff[80] = "";
    
    if ( this->FormatDate( dateBuff, false ) )
    {   
        if  (   (NULL != song) 
                &&  ( (strlen(song) + strlen(dateBuff) ) < 800)
            )
        {

            qtss_sprintf(strBuff,"%s '%s'",dateBuff, song);
                    
            if ( title || title[0] != 0)
            {   if  ( (strlen(strBuff) + strlen(title) ) < 1000 )
                {
                    ::strcat(strBuff," '");
                    ::strcat(strBuff,title);
                    ::strcat(strBuff,"'");
               }
            }
            else
            {
                ::strcat(strBuff," -");
            }
               
            if ( artist || artist[0] != 0)
            {   if  ( (strlen(strBuff) + strlen(artist) ) < 1000 )
                {
                    ::strcat(strBuff," '");
                    ::strcat(strBuff,artist);
                    ::strcat(strBuff,"'");
               }
            }
            else
            {
                ::strcat(strBuff," -");
            }
            
            if ( album || album[0] != 0)
            {   if  ( (strlen(strBuff) + strlen(album) ) < 1000 )
                {
                    ::strcat(strBuff," '");
                    ::strcat(strBuff,album);
                    ::strcat(strBuff,"'");
               }
            }
            else
            {
                ::strcat(strBuff," -");
            }
            
            // add the duration in seconds
            qtss_sprintf(dateBuff, " %"_S32BITARG_" ", duration);
            ::strcat(strBuff,dateBuff);
            
            // add the result code
            qtss_sprintf(dateBuff, " %d\n", result);
            ::strcat(strBuff,dateBuff);
            
            this->WriteToLog(strBuff, kAllowLogToRoll );
        }
        else
        {   
            ::strcat(strBuff,dateBuff);
            ::strcat(strBuff," internal error in LogMediaData\n");
            this->WriteToLog( strBuff, kAllowLogToRoll );       
        }

    }
    
}

