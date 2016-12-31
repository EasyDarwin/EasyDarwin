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
#include "QTAccessFile.h"

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
bool      QTAccessFile::sAllocatedName = false;
OSMutex*    QTAccessFile::sAccessFileMutex = nullptr;//QTAccessFile isn't reentrant
const int kBuffLen = 512;

void QTAccessFile::Initialize() // called by server at initialize never call again
{
	if (nullptr == sAccessFileMutex)
	{
		sAccessFileMutex = new OSMutex();
	}
}

void QTAccessFile::SetAccessFileName(const char* inQTAccessFileName)
{
	OSMutexLocker locker(sAccessFileMutex);
	if (nullptr == inQTAccessFileName)
	{
		Assert(nullptr != inQTAccessFileName);
		return;
	}

	if (sAllocatedName)
	{
		delete[] sQTAccessFileName;
	}

	sAllocatedName = true;
	sQTAccessFileName = new char[strlen(inQTAccessFileName) + 1];
	::strcpy(sQTAccessFileName, inQTAccessFileName);

}


bool QTAccessFile::HaveUser(char* userName, void* extraDataPtr)
{
	bool result = false;

	if (nullptr != userName && 0 != userName[0])
		result = true;

	return result;
}

bool QTAccessFile::HaveGroups(char** groupArray, UInt32 numGroups, void* extraDataPtr)
{
	bool result = false;

	if (numGroups > 0 && groupArray != nullptr)
		result = true;

	return result;
}

bool QTAccessFile::HaveRealm(char* userName, StrPtrLen* ioRealmNameStr, void* extraData)
{
	bool result = false;

	if (ioRealmNameStr != nullptr && ioRealmNameStr->Ptr != nullptr && ioRealmNameStr->Len > 0)
		result = true;

	return result;
}

void QTAccessFile::GetRealm(StrPtrLen* accessRealm, StrPtrLen* ioRealmNameStr, char* userName, void* extraDataPtr)
{

	if (ioRealmNameStr->Len <= accessRealm->Len)
		accessRealm->Len = ioRealmNameStr->Len - 1; // just copy what we can
	::memcpy(ioRealmNameStr->Ptr, accessRealm->Ptr, accessRealm->Len);
	ioRealmNameStr->Ptr[accessRealm->Len] = 0;

}

bool QTAccessFile::TestUser(StrPtrLen* accessUser, char* userName, void* extraDataPtr)
{
	bool result = false;

	if (accessUser->Equal(userName))
		result = true;

	return result;
}

bool QTAccessFile::TestGroup(StrPtrLen* accessGroup, char* userName, char** groupArray, UInt32 numGroups, void* extraDataPtr)
{

	for (UInt32 index = 0; index < numGroups; index++)
	{
		if (accessGroup->Equal(groupArray[index]))
			return true;
	}

	return false;

}

bool QTAccessFile::TestExtraData(StrPtrLen* wordPtr, StringParser* lineParserPtr, void* extraDataPtr)
{
	return false;
}

char* QTAccessFile::GetAccessFile_Copy(const char* movieRootDir, const char* dirPath)
{
	OSMutexLocker locker(sAccessFileMutex);

	char* currentDir = nullptr;
	char* lastSlash = nullptr;
	int movieRootDirLen = ::strlen(movieRootDir);
	int maxLen = strlen(dirPath) + strlen(sQTAccessFileName) + strlen(kPathDelimiterString) + 1;
	currentDir = new char[maxLen];

	::strcpy(currentDir, dirPath);

	//strip off filename
	lastSlash = ::strrchr(currentDir, kPathDelimiterChar);
	if (lastSlash != nullptr)
		lastSlash[0] = '\0';

	//check qtaccess files

	while (true)  //walk backward up the dir tree.
	{
		int curLen = strlen(currentDir) + strlen(sQTAccessFileName) + strlen(kPathDelimiterString);

		if (curLen >= maxLen)
			break;

		::strcat(currentDir, kPathDelimiterString);
		::strcat(currentDir, sQTAccessFileName);

		QTSS_Object fileObject = nullptr;
		if (QTSS_OpenFileObject(currentDir, qtssOpenFileNoFlags, &fileObject) == QTSS_NoErr)
		{
			(void)QTSS_CloseFileObject(fileObject);
			return currentDir;
		}

		//strip off the "/qtaccess"
		lastSlash = ::strrchr(currentDir, kPathDelimiterChar);
		lastSlash[0] = '\0';

		//strip of the tailing directory
		lastSlash = ::strrchr(currentDir, kPathDelimiterChar);
		if (lastSlash == nullptr)
			break;
		else
			lastSlash[0] = '\0';

		if ((lastSlash - currentDir) < movieRootDirLen) //bail if we start eating our way out of fMovieRootDir
			break;
	}

	delete[] currentDir;
	return nullptr;
}

bool DSAccessFile::CheckGroupMembership(const char* inUsername, const char* inGroupName)
{
#ifdef __MacOSX__
	// In Tiger, group membership is painfully simple: we ask memberd for it!
	struct passwd	*user = nullptr;
	struct group	*group = nullptr;
	uuid_t			userID;
	uuid_t			groupID;
	int				isMember = 0;

	// Look up the user using the POSIX APIs: only care about the UID.
	user = getpwnam(inUsername);
	if (user == nullptr)
		return false;
	uuid_clear(userID);
	if (mbr_uid_to_uuid(user->pw_uid, userID))
		return false;

	// Look up the group using the POSIX APIs: only care about the GID.
	group = getgrnam(inGroupName);
	endgrent();
	if (group == nullptr)
		return false;
	uuid_clear(groupID);
	if (mbr_gid_to_uuid(group->gr_gid, groupID))
		return false;

	// mbr_check_membership() returns 0 on success and error code on failure.
	if (mbr_check_membership(userID, groupID, &isMember))
		return false;
	return (bool)isMember;
#elif __linux__
	struct group *group = getgrnam(inGroupName);
	if (!group)
		return false;
	int i = 0;
	while (group->gr_mem[i])
		if (!strcasecmp(inUsername, group->gr_mem[i]))
			return true;
	return false;
#else
	return false;
#endif
}

bool DSAccessFile::ValidUser(char* userName, void* extraDataPtr)
{
#ifndef __Win32__
	struct passwd	*user = getpwnam(userName);
	bool result = true;
	if (user == nullptr)
	{
		return result;
	}
#endif    
	return true;
}
