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
	Website: http://www.easydarwin.org
*/
/*
    File:       QTAccessFile.cpp
    Contains:   This file contains the implementation for finding and parsing qtaccess files.         
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include "QTSS.h"
#include "OSHeaders.h"
#include "StrPtrLen.h"
#include "StringParser.h"
#include "QTSSModuleUtils.h"
#include "OSFileSource.h"
#include "OSMemory.h"
#include "OSHeaders.h"
#include "QTAccessFile.h"
#include "OSArrayObjectDeleter.h"

#ifdef __MacOSX__
#include <membership.h>
#endif
#ifndef __Win32__
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <unistd.h>
#endif
     
#define DEBUG_QTACCESS 0
#define debug_printf if (DEBUG_QTACCESS) qtss_printf

char* QTAccessFile::sAccessValidUser = "require valid-user\n";
char* QTAccessFile::sAccessAnyUser = "require any-user\n";

UInt8 QTAccessFile::sWhitespaceAndGreaterThanMask[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //0-9     // \t is a stop
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, //10-19    //'\r' & '\n' are stop conditions
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //30-39   ' '  is a stop
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40-49
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //60-69  // '>' is a stop
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79  
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
    0, 0, 0, 0, 0, 0             //250-255
};

char*       QTAccessFile::sQTAccessFileName = "qtaccess";
Bool16      QTAccessFile::sAllocatedName = false;
OSMutex*    QTAccessFile::sAccessFileMutex = NULL;//QTAccessFile isn't reentrant
const int kBuffLen = 512;

void QTAccessFile::Initialize() // called by server at initialize never call again
{
    if (NULL == sAccessFileMutex)
    {   sAccessFileMutex = NEW OSMutex();
    }
}

void QTAccessFile::SetAccessFileName(const char *inQTAccessFileName)
{
    OSMutexLocker locker(sAccessFileMutex);
    if (NULL == inQTAccessFileName)
    {   Assert(NULL != inQTAccessFileName);
        return;
    }
    
    if (sAllocatedName)
    {   delete [] sQTAccessFileName;
    }
    
    sAllocatedName = true;
    sQTAccessFileName = NEW char[strlen(inQTAccessFileName)+1];
    ::strcpy(sQTAccessFileName, inQTAccessFileName);
    
}


Bool16 QTAccessFile::HaveUser(char *userName, void* extraDataPtr)
{
    Bool16 result = false;

    if (NULL != userName && 0 != userName[0])
        result = true;

   return result;        
}

Bool16 QTAccessFile::HaveGroups( char** groupArray, UInt32 numGroups, void* extraDataPtr)
{
   Bool16 result = false;

   if (numGroups > 0 && groupArray != NULL)
        result = true;
        
   return result;   
}

Bool16 QTAccessFile::HaveRealm(   char *userName, StrPtrLen* ioRealmNameStr, void *extraData )
{
    Bool16 result = false;

    if (ioRealmNameStr != NULL && ioRealmNameStr->Ptr != NULL && ioRealmNameStr->Len > 0)
        result = true;
       
    return result;   
}

void QTAccessFile::GetRealm(StrPtrLen* accessRealm, StrPtrLen* ioRealmNameStr, char *userName,void *extraDataPtr )
{
    
    if (ioRealmNameStr->Len <= accessRealm->Len) 
        accessRealm->Len = ioRealmNameStr->Len -1; // just copy what we can
    ::memcpy(ioRealmNameStr->Ptr, accessRealm->Ptr,accessRealm->Len);
    ioRealmNameStr->Ptr[accessRealm->Len] = 0; 

}

Bool16 QTAccessFile::TestUser(StrPtrLen* accessUser, char *userName,void *extraDataPtr )
{
   Bool16 result = false;
    
    if ( accessUser->Equal(userName) ) 
        result = true;

  return result;   
}

Bool16 QTAccessFile::TestGroup( StrPtrLen* accessGroup, char *userName, char**groupArray, UInt32 numGroups, void *extraDataPtr)
{
   
    for (UInt32 index = 0; index < numGroups; index ++)
    {   if(accessGroup->Equal(groupArray[index])) 
            return true;
    }

  return false;   

}

Bool16 QTAccessFile::TestExtraData( StrPtrLen* wordPtr, StringParser* lineParserPtr, void* extraDataPtr)
{
    return false;
}


Bool16 QTAccessFile::AccessAllowed  (   char *userName, char**groupArray, UInt32 numGroups, StrPtrLen *accessFileBufPtr,
                                        QTSS_ActionFlags inFlags,StrPtrLen* ioRealmNameStr, Bool16 *outAllowAnyUserPtr, void *extraDataPtr
                                    )
{       
    if (NULL == accessFileBufPtr || NULL == accessFileBufPtr->Ptr || 0 == accessFileBufPtr->Len)
        return true; // nothing to check
        
    if (ioRealmNameStr != NULL && ioRealmNameStr->Ptr != NULL && ioRealmNameStr->Len > 0)
        ioRealmNameStr->Ptr[0] = 0;
        
    StringParser            accessFileParser(accessFileBufPtr);
    QTSS_ActionFlags        currentFlags = qtssActionFlagsRead; 
    StrPtrLen               line;
    StrPtrLen               word;
    Bool16                  haveUserName = false;
    Bool16                  haveRealmResultBuffer = false;
    Bool16                  haveGroups = false;
    
    *outAllowAnyUserPtr = false;
        
    haveUserName = HaveUser(userName, extraDataPtr);
  
    haveGroups = HaveGroups(groupArray, numGroups, extraDataPtr);

    haveRealmResultBuffer =  HaveRealm(userName, ioRealmNameStr, extraDataPtr );
  
    while( accessFileParser.GetDataRemaining() != 0 ) 
    {
        accessFileParser.GetThruEOL(&line);  // Read each line  
        StringParser lineParser(&line);
        lineParser.ConsumeWhitespace();//skip over leading whitespace
        if (lineParser.GetDataRemaining() == 0) // must be an empty line
            continue;

        char firstChar = lineParser.PeekFast();
        if ( (firstChar == '#') || (firstChar == '\0') )
            continue; //skip over comments and blank lines...
        
        lineParser.ConsumeUntilWhitespace(&word);
        if ( word.Equal("<Limit") ) // a limit line
        {
            currentFlags = qtssActionFlagsNoFlags; // clear to no access
            lineParser.ConsumeWhitespace();
            lineParser.ConsumeUntil( &word, sWhitespaceAndGreaterThanMask); // the flag <limit Read> or <limit Read >
            while (word.Len != 0) // compare each word in the line
            {   
                if (word.Equal("WRITE")  ) 
                {   currentFlags |= inFlags & qtssActionFlagsWrite; // accept following lines if inFlags has write
                }
                
                if (word.Equal("READ") ) 
                {   currentFlags |= inFlags & qtssActionFlagsRead; // accept following lines if inFlags has read
                }
                lineParser.ConsumeWhitespace();
                lineParser.ConsumeUntil(&word, sWhitespaceAndGreaterThanMask);
            }
            continue; //done with limit line
        }
        if ( word.Equal("</Limit>") )
            currentFlags = qtssActionFlagsRead; // set the current access state to the default of read access
        
        if (0 == (currentFlags & inFlags))
            continue; // ignore lines because inFlags doesn't match the current access state
            
        if (haveRealmResultBuffer && word.Equal("AuthName") ) //realm name
        {   
            lineParser.ConsumeWhitespace();
            lineParser.GetThruEOL(&word);
            StringParser::UnQuote(&word);// if the parsed string is surrounded by quotes then remove them.
            
            GetRealm(&word, ioRealmNameStr, userName, extraDataPtr );

            // we don't change the buffer len ioRealmNameStr->Len because we might have another AuthName tag to copy
            continue; // done with AuthName (realm)
        }
        
        
        if (word.Equal("require") )
        {
            lineParser.ConsumeWhitespace();
            lineParser.ConsumeUntilWhitespace(&word);       

            if (haveUserName && word.Equal("valid-user") ) 
            {   return true; 
            }
            
            if ( word.Equal("any-user")  ) 
            {   
                *outAllowAnyUserPtr = true;
                return true; 
            }
    
            if (!haveUserName)
                continue;
                
            if (word.Equal("user") )
            {   
                lineParser.ConsumeWhitespace();
                lineParser.ConsumeUntilWhitespace(&word);   
                    
                while (word.Len != 0) // compare each word in the line
                {   
                    if (TestUser(&word, userName, extraDataPtr ))
                        return true;

                    lineParser.ConsumeWhitespace();
                    lineParser.ConsumeUntilWhitespace(&word);       
                }
                continue; // done with "require user" line
            }
            
            if (haveGroups && word.Equal("group")) // check if we have groups for the user
            {
                lineParser.ConsumeWhitespace();
                lineParser.ConsumeUntilWhitespace(&word);       
                
                while (word.Len != 0) // compare each word in the line
                {   
                    if (TestGroup(&word, userName, groupArray, numGroups, extraDataPtr) )
                        return true;

                    lineParser.ConsumeWhitespace();
                    lineParser.ConsumeUntilWhitespace(&word);
                }
                continue; // done with "require group" line
            }
            
            if (TestExtraData(&word, &lineParser, extraDataPtr))
                return true;
                
                    
            continue; // done with unparsed "require" line
        }
        
    }
    
    return false; // user or group not found
}

char*  QTAccessFile::GetAccessFile_Copy( const char* movieRootDir, const char* dirPath)
{   
    OSMutexLocker locker(sAccessFileMutex);

    char* currentDir= NULL;
    char* lastSlash = NULL;
    int movieRootDirLen = ::strlen(movieRootDir);
    int maxLen = strlen(dirPath)+strlen(sQTAccessFileName) + strlen(kPathDelimiterString) + 1;
    currentDir = NEW char[maxLen];

    ::strcpy(currentDir, dirPath);

    //strip off filename
    lastSlash = ::strrchr(currentDir, kPathDelimiterChar);
    if (lastSlash != NULL)
        lastSlash[0] = '\0';
    
    //check qtaccess files
    
    while ( true )  //walk backward up the dir tree.
    {
        int curLen = strlen(currentDir) + strlen(sQTAccessFileName) + strlen(kPathDelimiterString);

        if ( curLen >= maxLen )
            break;
    
        ::strcat(currentDir, kPathDelimiterString);
        ::strcat(currentDir, sQTAccessFileName);
    
        QTSS_Object fileObject = NULL;
        if( QTSS_OpenFileObject(currentDir, qtssOpenFileNoFlags, &fileObject) == QTSS_NoErr) 
        {
            (void)QTSS_CloseFileObject(fileObject);
            return currentDir;
        }
                
        //strip off the "/qtaccess"
        lastSlash = ::strrchr(currentDir, kPathDelimiterChar);
        lastSlash[0] = '\0';
            
        //strip of the tailing directory
        lastSlash = ::strrchr(currentDir, kPathDelimiterChar);
        if (lastSlash == NULL)
            break;
        else
            lastSlash[0] = '\0';
        
        if ( (lastSlash-currentDir) < movieRootDirLen ) //bail if we start eating our way out of fMovieRootDir
            break;
    }
    
    delete[] currentDir;
    return NULL;
}

bool DSAccessFile::CheckGroupMembership(const char* inUsername, const char* inGroupName)
{   
#ifdef __MacOSX__
	// In Tiger, group membership is painfully simple: we ask memberd for it!
	struct passwd	*user		= NULL;
	struct group	*group		= NULL;
	uuid_t			userID;
	uuid_t			groupID;
	int				isMember	= 0;

	// Look up the user using the POSIX APIs: only care about the UID.
	user = getpwnam(inUsername);
	if ( user == NULL )
		return false;
	uuid_clear(userID);
	if ( mbr_uid_to_uuid(user->pw_uid, userID) )
		return false;

	// Look up the group using the POSIX APIs: only care about the GID.
	group = getgrnam(inGroupName);
	endgrent();
	if ( group == NULL )
		return false;
	uuid_clear(groupID);
	if ( mbr_gid_to_uuid(group->gr_gid, groupID) )
		return false;

	// mbr_check_membership() returns 0 on success and error code on failure.
	if ( mbr_check_membership(userID, groupID, &isMember) )
		return false;
	return (bool)isMember;
#elif __linux__
	struct group *group = getgrnam(inGroupName);
	if(!group)
		return false;
	int i=0;
	while(group->gr_mem[i])
		if(!strcasecmp(inUsername, group->gr_mem[i]))
			return true;
	return false;
#else
	return false;
#endif
}

Bool16 DSAccessFile::ValidUser( char*userName, void* extraDataPtr)
{
#ifndef __Win32__
    struct passwd	*user = getpwnam(userName);
    Bool16 result =true;
    if ( user == NULL )
    {    
         return result;
    }
#endif    
    return true;
}



