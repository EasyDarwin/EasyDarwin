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
	File:       AccessChecker.h

	Contains:

*/

#ifndef _QTSSACCESSCHECKER_H_
#define _QTSSACCESSCHECKER_H_

#include "QTSS.h"
#include "StrPtrLen.h"
#include "OSHeaders.h"

class AccessChecker
{
	/*
		Access check logic:

		If "modAccess_enabled" == "enabled,
		Starting at URL dir, walk up directories to Movie Folder until a "qtaccess" file is found
			If not found,
				allow access
			If found,
				send a challenge to the client
				verify user against QTSSPasswd
				verify that user or member group is in the lowest ".qtacess"
				walk up directories until a ".qtaccess" is found
				If found,
					allow access
				If not found,
					deny access

		ToDo:
			would probably be a good idea to do some caching of ".qtaccess" data to avoid
			multiple directory walks
	*/

public:
	struct UserProfile
	{
		StrPtrLen   username;
		StrPtrLen   cryptPassword;
		StrPtrLen   digestPassword;
		char**      groups;
		UInt32      maxGroupNameLen;
		UInt32      numGroups;
		UInt32      groupsSize;
	};

	AccessChecker();
	virtual ~AccessChecker();

	void UpdateFilePaths(const char* inUsersFilePath, const char* inGroupsFilePath);
	UInt32 UpdateUserProfiles();

	bool  HaveFilePathsChanged(const char* inUsersFilePath, const char* inGroupsFilePath);
	UserProfile* RetrieveUserProfile(const StrPtrLen* inUserName);
	inline StrPtrLen* GetAuthRealm() { return &fAuthRealm; }
	inline char* GetUsersFilePathPtr() { return fUsersFilePath; }
	inline char* GetGroupsFilePathPtr() { return fGroupsFilePath; }

	enum { kDefaultNumProfiles = 10, kDefaultNumGroups = 2 };
	enum {
		kNoErr = 0x00000000,
		kUsersFileNotFoundErr = 0x00000001,
		kGroupsFileNotFoundErr = 0x00000002,
		kBadUsersFileErr = 0x00000004,
		kBadGroupsFileErr = 0x00000008,
		kUsersFileUnknownErr = 0x00000010,
		kGroupsFileUnknownErr = 0x00000020
	};

protected:
	char*               fGroupsFilePath;
	char*               fUsersFilePath;
	QTSS_TimeVal        fUsersFileModDate;
	QTSS_TimeVal        fGroupsFileModDate;
	StrPtrLen           fAuthRealm;

	UserProfile**       fProfiles;
	UInt32              fNumUsers;
	UInt32              fCurrentSize;

	static const char*  kDefaultUsersFilePath;
	static const char*  kDefaultGroupsFilePath;

private:
	void deleteProfilesAndRealm();
};

#endif //_QTSSACCESSCHECKER_H_
