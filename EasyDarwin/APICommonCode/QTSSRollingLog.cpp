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
    File:       QTSSRollingLog.cpp

    Contains:   Implements object defined in .h file


*/

#include <time.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h> 
#ifndef __Win32__
#include <sys/time.h>
#endif
#include "QTSSRollingLog.h"
#include "OS.h"
#include "OSArrayObjectDeleter.h"
#include "ResizeableStringFormatter.h"

static bool sCloseOnWrite = true;

 QTSSRollingLog::QTSSRollingLog() :     
    fLog(nullptr), 
    fLogCreateTime(-1),
    fLogFullPath(nullptr),
    fAppendDotLog(true),
    fLogging(true)
{
    this->SetTaskName("QTSSRollingLog");
}

QTSSRollingLog::~QTSSRollingLog()
{
    //
    // Log should already be closed, but just in case...
    this->CloseLog();
    delete [] fLogFullPath;
}

// Set this to true to get the log to close the file between writes.
void QTSSRollingLog::SetCloseOnWrite(bool closeOnWrite) 
{ 
    sCloseOnWrite = closeOnWrite; 
}

bool  QTSSRollingLog::IsLogEnabled() 
{ 
    return sCloseOnWrite || (fLog != nullptr); 
}

void QTSSRollingLog::WriteToLog(char* inLogData, bool allowLogToRoll)
{
    OSMutexLocker locker(&fMutex);
    
    if (fLogging == false)
        return;
        
    if (sCloseOnWrite && fLog == nullptr)
        this->EnableLog(fAppendDotLog ); //re-open log file before we write
    
    if (allowLogToRoll)
        (void)this->CheckRollLog();
        
    if (fLog != nullptr)
    {
        qtss_fprintf(fLog, "%s", inLogData);
        ::fflush(fLog);
    }
    
    if (sCloseOnWrite)
        this->CloseLog( false );
}

bool QTSSRollingLog::RollLog()
{
    OSMutexLocker locker(&fMutex);
    
    //returns false if an error occurred, true otherwise

    //close the old file.
    if (fLog != nullptr)
        this->CloseLog();
        
    if (fLogging == false)
        return false;
 
    //rename the old file
    bool result = this->RenameLogFile(fLogFullPath);
    if (result)
        this->EnableLog(fAppendDotLog);//re-opens log file

    return result;
}

char* QTSSRollingLog::GetLogPath(char *extension)
{
    char *thePath = nullptr;
    
    OSCharArrayDeleter logDir(this->GetLogDir()); //The string passed into this function is a copy
    OSCharArrayDeleter logName(this->GetLogName());  //The string passed into this function is a copy
    
    ResizeableStringFormatter formatPath(nullptr,0); //allocate the buffer
    formatPath.PutFilePath(logDir, logName);
    
    if ( extension != nullptr)
        formatPath.Put(extension);
        
    formatPath.PutTerminator();
    thePath = formatPath.GetBufPtr();
    
    formatPath.Set(nullptr,0); //don't delete buffer we are returning the path as  a result
    
    return thePath;
}

void QTSSRollingLog::EnableLog( bool appendDotLog )
{
   //
    // Start this object running!
    this->Signal(Task::kStartEvent);

    OSMutexLocker locker(&fMutex);

    fAppendDotLog = appendDotLog;
    
    if (fLogging == false)
        return;

    char *extension = ".log";
    if (!appendDotLog)
        extension = nullptr;
        
    delete[] fLogFullPath;
    fLogFullPath = this->GetLogPath(extension);

    //we need to make sure that when we create a new log file, we write the
    //log header at the top
    bool logExists = this->DoesFileExist(fLogFullPath);
    
    //create the log directory if it doesn't already exist
    if (!logExists)
    {
       OSCharArrayDeleter tempDir(this->GetLogDir());
       OS::RecursiveMakeDir(tempDir.GetObject());
    }
 
    fLog = ::fopen(fLogFullPath, "a+");//open for "append"
    if (nullptr != fLog)
    { 
        if (!logExists) //the file is new, write a log header with the create time of the file.
        {    fLogCreateTime = this->WriteLogHeader(fLog);
#if __MacOSX__
             (void) ::chown(fLogFullPath, 76, (gid_t)-1);//set owner to user qtss.
#endif
        }
        else            //the file is old, read the log header to find the create time of the file.
            fLogCreateTime = this->ReadLogHeader(fLog);
    }
}

void QTSSRollingLog::CloseLog( bool leaveEnabled )
{
    OSMutexLocker locker(&fMutex);
    
    if (leaveEnabled)
        sCloseOnWrite = true;

    if (fLog != nullptr)
    {
        ::fclose(fLog);
        fLog = nullptr;
    }
}

//returns false if some error has occurred
bool QTSSRollingLog::FormatDate(char *ioDateBuffer, bool logTimeInGMT)
{
    Assert(nullptr != ioDateBuffer);
    
    //use ansi routines for getting the date.
    time_t calendarTime = ::time(nullptr);
    Assert(-1 != calendarTime);
    if (-1 == calendarTime)
        return false;
        
    struct tm* theTime = nullptr;
    struct tm  timeResult;
    
    if (logTimeInGMT)
        theTime = ::qtss_gmtime(&calendarTime, &timeResult);
    else
        theTime = qtss_localtime(&calendarTime, &timeResult);
    
    Assert(nullptr != theTime);
    
    if (nullptr == theTime)
        return false;
        
    // date time needs to look like this for extended log file format: 2001-03-16 23:34:54
    // this wonderful ANSI routine just does it for you.
    // the format is YYYY-MM-DD HH:MM:SS
    // the date time is in GMT, unless logTimeInGMT is false, in which case
    // the time logged is local time
    //qtss_strftime(ioDateBuffer, kMaxDateBufferSize, "%d/%b/%Y:%H:%M:%S", theLocalTime);
    qtss_strftime(ioDateBuffer, kMaxDateBufferSizeInBytes, "%Y-%m-%d %H:%M:%S", theTime);  
    return true;
}

bool QTSSRollingLog::CheckRollLog()
{
    //returns false if an error occurred, true otherwise
    if (fLog == nullptr)
        return true;
    
    //first check to see if log rolling should happen because of a date interval.
    //This takes precedence over size based log rolling
    // this is only if a client connects just between 00:00:00 and 00:01:00
    // since the task object runs every minute
    
    // when an entry is written to the log file, only the file size must be checked
    // to see if it exceeded the limits
    
    // the roll interval should be monitored in a task object 
    // and rolled at midnight if the creation time has exceeded.
    if ((-1 != fLogCreateTime) && (0 != this->GetRollIntervalInDays()))
    {   
        time_t logCreateTimeMidnight = -1;
        QTSSRollingLog::ResetToMidnight(&fLogCreateTime, &logCreateTimeMidnight);
        Assert(logCreateTimeMidnight != -1);
        
        time_t calendarTime = ::time(nullptr);

        Assert(-1 != calendarTime);
        if (-1 != calendarTime)
        {
            double theExactInterval = ::difftime(calendarTime, logCreateTimeMidnight);
            SInt32 theCurInterval = (SInt32)::floor(theExactInterval);
            
            //transfer_roll_interval is in days, theCurInterval is in seconds
            SInt32 theRollInterval = this->GetRollIntervalInDays() * 60 * 60 * 24;
            if (theCurInterval > theRollInterval)
                return this->RollLog();
        }
    }
    
    
    //now check size based log rolling
    UInt32 theCurrentPos = ::ftell(fLog);
    //max_transfer_log_size being 0 is a signal to ignore the setting.
    if ((this->GetMaxLogBytes() != 0) &&
        (theCurrentPos > this->GetMaxLogBytes()))
        return this->RollLog();
    return true;
}

bool QTSSRollingLog::RenameLogFile(const char* inFileName)
{
    //returns false if an error occurred, true otherwise

    //this function takes care of renaming a log file from "myLogFile.log" to
    //"myLogFile.981217000.log" or if that is already taken, myLogFile.981217001.log", etc 
    
    //fix 2287086. Rolled log name can be different than original log name
    //GetLogDir returns a copy of the log dir
    OSCharArrayDeleter logDirectory(this->GetLogDir());

    //create the log directory if it doesn't already exist
    OS::RecursiveMakeDir(logDirectory.GetObject());
    
    //GetLogName returns a copy of the log name
    OSCharArrayDeleter logBaseName(this->GetLogName());
        
    //QTStreamingServer.981217003.log
    //format the new file name
    OSCharArrayDeleter theNewNameBuffer(new char[::strlen(logDirectory) + kMaxFilenameLengthInBytes + 3]);
    
    //copy over the directory - append a '/' if it's missing
    ::strcpy(theNewNameBuffer, logDirectory);
    if (theNewNameBuffer[::strlen(theNewNameBuffer)-1] != kPathDelimiterChar)
    {
        ::strcat(theNewNameBuffer, kPathDelimiterString);
    }
    
    //copy over the base filename
    ::strcat(theNewNameBuffer, logBaseName.GetObject());

    //append the date the file was created
    struct tm  timeResult;
    struct tm* theLocalTime = qtss_localtime(&fLogCreateTime, &timeResult);
    char timeString[10];
    qtss_strftime(timeString,  10, ".%y%m%d", theLocalTime);
    ::strcat(theNewNameBuffer, timeString);
    
    SInt32 theBaseNameLength = ::strlen(theNewNameBuffer);


    //loop until we find a unique name to rename this file
    //and append the log number and suffix
    SInt32 theErr = 0;
    for (SInt32 x = 0; (theErr == 0) && (x<=1000); x++)
    {
        if (x  == 1000) //we don't have any digits left, so just reuse the "---" until tomorrow...
        {
            //add a bogus log number and exit the loop
            qtss_sprintf(theNewNameBuffer + theBaseNameLength, "---.log");
            break;
        }

        //add the log number & suffix
        qtss_sprintf(theNewNameBuffer + theBaseNameLength, "%03" _S32BITARG_ ".log", x);

        //assume that when ::stat returns an error, it is becase
        //the file doesnt exist. Once that happens, we have a unique name
        // csl - shouldn't you watch for a ENOENT result?
        struct stat theIdontCare;
        theErr = ::stat(theNewNameBuffer, &theIdontCare);
        WarnV((theErr == 0 || OSThread::GetErrno() == ENOENT), "unexpected stat error in RenameLogFile");
        
    }
    
    //rename the file. Use posix rename function
    int result = ::rename(inFileName, theNewNameBuffer);
    if (result == -1)
        theErr = (SInt32)OSThread::GetErrno();
    else
        theErr = 0;
        
    WarnV(theErr == 0 , "unexpected rename error in RenameLogFile");

    
    if (theErr != 0)
        return false;
    else
        return true;    
}

bool QTSSRollingLog::DoesFileExist(const char *inPath)
{
    struct stat theStat;
    int theErr = ::stat(inPath, &theStat);
    if (theErr != 0)
        return false;
    else
        return true;
}

time_t QTSSRollingLog::WriteLogHeader(FILE* inFile)
{
    OSMutexLocker locker(&fMutex);

    //The point of this header is to record the exact time the log file was created,
    //in a format that is easy to parse through whenever we open the file again.
    //This is necessary to support log rolling based on a time interval, and POSIX doesn't
    //support a create date in files.
    time_t calendarTime = ::time(nullptr);
    Assert(-1 != calendarTime);
    if (-1 == calendarTime)
        return -1;

    struct tm  timeResult;
    struct tm* theLocalTime = qtss_localtime(&calendarTime, &timeResult);
    Assert(nullptr != theLocalTime);
    if (nullptr == theLocalTime)
        return -1;
    
    //
    // Files are always created at hour 0 (we don't care about the time, we always
    // want them to roll at midnight.
    //theLocalTime->tm_hour = 0;
    //theLocalTime->tm_min = 0;
    //theLocalTime->tm_sec = 0;

    char tempbuf[1024];
    qtss_strftime(tempbuf, sizeof(tempbuf), "#Log File Created On: %m/%d/%Y %H:%M:%S\n", theLocalTime);
    //qtss_sprintf(tempbuf, "#Log File Created On: %d/%d/%d %d:%d:%d %d:%d:%d GMT\n",
    //          theLocalTime->tm_mon, theLocalTime->tm_mday, theLocalTime->tm_year,
    //          theLocalTime->tm_hour, theLocalTime->tm_min, theLocalTime->tm_sec,
    //          theLocalTime->tm_yday, theLocalTime->tm_wday, theLocalTime->tm_isdst);
    this->WriteToLog(tempbuf, !kAllowLogToRoll);
    
    return this->ReadLogHeader(inFile);
}

time_t QTSSRollingLog::ReadLogHeader(FILE* inFile)
{
    OSMutexLocker locker(&fMutex);

    //This function reads the header in a log file, returning the time stored
    //at the beginning of this file. This value is used to determine when to
    //roll the log.
    //Returns -1 if the header is bogus. In that case, just ignore time based log rolling

    //first seek to the beginning of the file
    SInt32 theCurrentPos = ::ftell(inFile);
    if (theCurrentPos == -1)
        return -1;
    (void)::rewind(inFile);

    const UInt32 kMaxHeaderLength = 500;
    char theFirstLine[kMaxHeaderLength];
    
    if (nullptr == ::fgets(theFirstLine, kMaxHeaderLength, inFile))
    {
        ::fseek(inFile, 0, SEEK_END);
        return -1;
    }
    ::fseek(inFile, 0, SEEK_END);
    
    struct tm theFileCreateTime;
    
    // Zero out fields we will not be using
    theFileCreateTime.tm_isdst = -1;
    theFileCreateTime.tm_wday = 0;
    theFileCreateTime.tm_yday = 0;
    
    //if (EOF == ::sscanf(theFirstLine, "#Log File Created On: %d/%d/%d %d:%d:%d\n",
    //          &theFileCreateTime.tm_mon, &theFileCreateTime.tm_mday, &theFileCreateTime.tm_year,
    //          &theFileCreateTime.tm_hour, &theFileCreateTime.tm_min, &theFileCreateTime.tm_sec))
    //  return -1;
    
    //
    // We always want to roll at hour 0, so ignore the time of creation
    
    if (EOF == ::sscanf(theFirstLine, "#Log File Created On: %d/%d/%d %d:%d:%d\n",
                &theFileCreateTime.tm_mon, &theFileCreateTime.tm_mday, &theFileCreateTime.tm_year,
                &theFileCreateTime.tm_hour, &theFileCreateTime.tm_min, &theFileCreateTime.tm_sec))
        return -1;

    //
    // It should be like this anyway, but if the log file is legacy, then...
    // No! The log file will have the actual time in it but we shall return the exact time
    //theFileCreateTime.tm_hour = 0;
    //theFileCreateTime.tm_min = 0;
    //theFileCreateTime.tm_sec = 0;
    
    // Actually, it seems like all platforms need this.
//#ifdef __Win32__
    // Win32 has slightly different atime basis than UNIX.
    theFileCreateTime.tm_yday--;
    theFileCreateTime.tm_mon--;
    theFileCreateTime.tm_year -= 1900;
//#endif

#if 0
    //use ansi routines for getting the date.
    time_t calendarTime = ::time(nullptr);
    Assert(-1 != calendarTime);
    if (-1 == calendarTime)
        return false;
        
    struct tm  timeResult;
    struct tm* theLocalTime = qtss_localtime(&calendarTime, &timeResult);
    Assert(nullptr != theLocalTime);
    if (nullptr == theLocalTime)
        return false;
#endif

    //ok, we should have a filled in tm struct. Convert it to a time_t.
    //time_t thePoopTime = ::mktime(theLocalTime);
    time_t theTime = ::mktime(&theFileCreateTime);
    return theTime;
}

SInt64 QTSSRollingLog::Run()
{
    //
    // If we are going away, just return
    EventFlags events = this->GetEvents();
    if (events & Task::kKillEvent)
        return -1;
    
    OSMutexLocker locker(&fMutex);
    
    UInt32 theRollInterval = (this->GetRollIntervalInDays())  * 60 * 60 * 24;
    
    if((fLogCreateTime != -1) && (fLog != nullptr))
    {
        time_t logRollTimeMidnight = -1;
        this->ResetToMidnight(&fLogCreateTime, &logRollTimeMidnight);
        Assert(logRollTimeMidnight != -1);
        
        if(theRollInterval != 0)
        {
            time_t calendarTime = ::time(nullptr);
            Assert(-1 != calendarTime);
            double theExactInterval = ::difftime(calendarTime, logRollTimeMidnight);
            if(theExactInterval > 0) {
                UInt32 theCurInterval = (UInt32)::floor(theExactInterval);
                if (theCurInterval >= theRollInterval)
                    this->RollLog();
            }
        }
    }
    return 60 * 1000;
}

void QTSSRollingLog::ResetToMidnight(time_t* inTimePtr, time_t* outTimePtr) 
{
    if(*inTimePtr == -1)
    {
        *outTimePtr = -1;
        return;
    }
    
    struct tm  timeResult;
    struct tm* theLocalTime = qtss_localtime(inTimePtr, &timeResult);
    Assert(theLocalTime != nullptr);

    theLocalTime->tm_hour = 0;
    theLocalTime->tm_min = 0;
    theLocalTime->tm_sec = 0;
    
    // some weird stuff
    //theLocalTime->tm_yday--;
    //theLocalTime->tm_mon--;
    //theLocalTime->tm_year -= 1900;

    *outTimePtr = ::mktime(theLocalTime);

}
