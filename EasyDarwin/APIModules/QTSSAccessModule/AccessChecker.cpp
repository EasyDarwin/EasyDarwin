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
	File:       AccessChecker.cpp

	Contains:

*/


#ifndef __Win32__
#ifndef __USE_XOPEN
#define __USE_XOPEN 1
#endif
#include <unistd.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include "StringParser.h"
#include "OSFileSource.h"
#include "OSMemory.h"
#include "OSHeaders.h"
#include "AccessChecker.h"
#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"

static StrPtrLen sAuthWord("realm", 5);

// Constructor
// Allocates no memory
AccessChecker::AccessChecker() :
	fGroupsFilePath(NULL),
	fUsersFilePath(NULL),
	fUsersFileModDate(-1),
	fGroupsFileModDate(-1),
	fProfiles(NULL),
	fNumUsers(0),
	fCurrentSize(0)
{
}

// Destructor
// Deletes the fUsersFilePath, fGroupsFilePath, 
// and calls the function to delete the authRealm and all the profiles
AccessChecker::~AccessChecker()
{
	delete[] fGroupsFilePath;
	delete[] fUsersFilePath;

	deleteProfilesAndRealm();
}


// Allocates memory for the fUsersFilePath and fGroupsFilePath
// Before this call is made, make sure that the previous memory allocated is deleted
// or that memory will be orphaned!
void AccessChecker::UpdateFilePaths(const char* inUsersFilePath, const char* inGroupsFilePath) {
	// Assert input arguments are not null
	Assert(inUsersFilePath != NULL);
	Assert(inGroupsFilePath != NULL);

	// Before reassigning, delete old paths
	delete[] fGroupsFilePath;
	delete[] fUsersFilePath;

	fUsersFilePath = NEW char[strlen(inUsersFilePath) + 1];
	::strcpy(fUsersFilePath, inUsersFilePath);

	fGroupsFilePath = NEW char[strlen(inGroupsFilePath) + 1];
	::strcpy(fGroupsFilePath, inGroupsFilePath);
}

// Function to delete memory allocated for all the profiles, and the authRealm
// For each profile, memory is allocated for
//                                  username
//                                  cryptPassword
//                                  digestPassword
//                                  each group that the user belongs to (array of group names)
// All of the above are deleted
void AccessChecker::deleteProfilesAndRealm()
{
	UInt32 i, j;

	// delete the profiles
	if (fProfiles != NULL)
	{
		// For each profile
		for (i = 0; i < fNumUsers; i++)
		{
			UserProfile* profile = fProfiles[i];

			// delete the username
			if ((profile->username).Len != 0)
			{
				delete (profile->username).Ptr;
				(profile->username).Len = 0;
			}

			// delete cryptPassword
			if ((profile->cryptPassword).Len != 0)
			{
				delete (profile->cryptPassword).Ptr;
				(profile->cryptPassword).Len = 0;
			}

			// delete digestPassword
			if ((profile->digestPassword).Len != 0)
			{
				delete (profile->digestPassword).Ptr;
				(profile->digestPassword).Len = 0;
			}

			// delete each group name
			for (j = 0; j < profile->numGroups; j++)
			{
				delete[] profile->groups[j];
			}
			// delete the array of pointers to the group names
			delete[] profile->groups;

			profile->groups = NULL;
			profile->maxGroupNameLen = 0;
			profile->numGroups = 0;
			profile->groupsSize = 0;
			delete profile;
		}

		// delete the array of profile pointers
		delete[] fProfiles;
		fProfiles = NULL;
	}

	// delete the fAuthRealm field
	if (fAuthRealm.Len != 0) {
		delete fAuthRealm.Ptr;
		fAuthRealm.Len = 0;
	}

	fNumUsers = 0;
	fCurrentSize = 0;
}

// Memory is allocated for each username record found in the users file
// Memory is also allocated for each group name found in the groups file per user
// All this memory must be deleted if the profiles are deleted, before parsing
// the file again
UInt32 AccessChecker::UpdateUserProfiles() {

	UInt32 index = 0;
	UInt32 i = 0, j = 0;
	UInt32 resultErr = kNoErr;
	Bool16 groupFileErrors = true;
	Bool16 userFileErrors = true;

	StrPtrLen line;

	QTSS_TimeVal oldUsersFileModDate = fUsersFileModDate;
	QTSS_TimeVal oldGroupsFileModDate = fGroupsFileModDate;

	// Read the users file into a buffer
	StrPtrLen userData;
	QTSS_TimeVal newModDate = -1;
	// QTSSModuleUtils::ReadEntireFile allocates memory for userData
	QTSS_Error err = QTSSModuleUtils::ReadEntireFile(fUsersFilePath, &userData, fUsersFileModDate, &newModDate);
	if (err == QTSS_FileNotFound)
		resultErr |= kUsersFileNotFoundErr;
	else if (err != QTSS_NoErr)
		resultErr |= kUsersFileUnknownErr;
	else
		userFileErrors = false;

	if (userFileErrors)
		fUsersFileModDate = -1;
	else if (userData.Len != 0)
		fUsersFileModDate = newModDate;

	newModDate = -1;

	// Read the groups file into a buffer
	StrPtrLen groupData;
	// QTSSModuleUtils::ReadEntireFile allocates memory for groupData
	err = QTSSModuleUtils::ReadEntireFile(fGroupsFilePath, &groupData, fGroupsFileModDate, &newModDate);
	if (err == QTSS_FileNotFound)
		resultErr |= kGroupsFileNotFoundErr;
	else if (err != QTSS_NoErr)
		resultErr |= kGroupsFileUnknownErr;
	else
		groupFileErrors = false;

	if (groupFileErrors)
		fGroupsFileModDate = -1;
	else if (groupData.Len != 0)
		fGroupsFileModDate = newModDate;

	if (userFileErrors)
	{
		// delete user profiles and exit
		deleteProfilesAndRealm();
		return resultErr;
	}

	if ((fUsersFileModDate == oldUsersFileModDate) && (fGroupsFileModDate == oldGroupsFileModDate))
		return resultErr;

	// If either the users or groups file has changed
	// the old profiles and the old realm should be deleted
	// before a new array of profiles is created and a new realm from the users file is read    
	deleteProfilesAndRealm();

	// Since one or both of the files has changed, reread the files and create user profiles    
	if (userData.Len == 0)
		(void)QTSSModuleUtils::ReadEntireFile(fUsersFilePath, &userData, -1, NULL);
	if (groupData.Len == 0 && !groupFileErrors)
		(void)QTSSModuleUtils::ReadEntireFile(fGroupsFilePath, &groupData, -1, NULL);


	// This will delete the memory allocated for userData when we return from this function
	OSCharArrayDeleter userDataPtrDeleter(userData.Ptr);
	// This will delete the memory allocated for groupData when we return from this function
	OSCharArrayDeleter groupDataPtrDeleter(groupData.Ptr);

	// Create the fProfiles array of size kDefaultNumProfiles
	fProfiles = NEW UserProfile*[kDefaultNumProfiles];
	fCurrentSize = kDefaultNumProfiles;

	StringParser userDataParser(&userData);
	// check if the first line is "realm"
	while (true) {
		StrPtrLen word;
		userDataParser.GetThruEOL(&line);
		StringParser authLineParser(&line);
		// Skip over leading whitespace
		authLineParser.ConsumeUntil(NULL, StringParser::sWhitespaceMask);
		// Skip over comments and blank lines
		if ((authLineParser.GetDataRemaining() == 0) || (authLineParser[0] == '#') || (authLineParser[0] == '\0'))
			continue;
		authLineParser.ConsumeWord(&word);
		if (sAuthWord.Equal(word)) {
			authLineParser.ConsumeWhitespace();
			authLineParser.ConsumeUntil(&word, StringParser::sEOLMask);
			fAuthRealm.Set(word.GetAsCString(), word.Len);
		}
		else {
			// This shouldn't happen because it means that the realm line
			// is not the first non-commented out line in the file
			// Implies the users file is corrupted!
			resultErr |= kBadUsersFileErr;

			// Create a new user profile for the first username
			UserProfile* profile = NEW UserProfile;
			profile->groups = NEW char*[kDefaultNumGroups];
			profile->maxGroupNameLen = 0;
			profile->numGroups = 0;
			profile->groupsSize = kDefaultNumGroups;
			(profile->username).Set(word.GetAsCString(), word.Len);
			// Get the crypted password
			if (authLineParser.Expect(':'))
			{
				authLineParser.ConsumeUntil(&word, ':');
				(profile->cryptPassword).Set(word.GetAsCString(), word.Len);
				// Get the digest password
				authLineParser.GetThruEOL(&word);
				(profile->digestPassword).Set(word.GetAsCString(), word.Len);
			}

			fProfiles[index] = profile;
			index++;
		}
		break;
	}

	while (userDataParser.GetDataRemaining() != 0) {
		// Read each line
		userDataParser.GetThruEOL(&line);
		StringParser userLineParser(&line);
		//parse the line
		//skip over leading whitespace
		userLineParser.ConsumeUntil(NULL, StringParser::sWhitespaceMask);

		//skip over comments and blank lines
		if ((userLineParser.GetDataRemaining() == 0) || (userLineParser[0] == '#') || (userLineParser[0] == '\0'))
			continue;

		// Create a new user profile for each username found
		UserProfile* profile = NEW UserProfile;
		profile->groups = NEW char*[kDefaultNumGroups];
		profile->maxGroupNameLen = 0;
		profile->numGroups = 0;
		profile->groupsSize = kDefaultNumGroups;
		StrPtrLen word;
		userLineParser.ConsumeUntil(&word, ':');
		(profile->username).Set(word.GetAsCString(), word.Len);
		// Get the crypted password
		if (userLineParser.Expect(':'))
		{
			userLineParser.ConsumeUntil(&word, ':');
			(profile->cryptPassword).Set(word.GetAsCString(), word.Len);
			if (userLineParser.Expect(':')) {
				// Get the digest password
				userLineParser.GetThruEOL(&word);
				(profile->digestPassword).Set(word.GetAsCString(), word.Len);
			}
		}

		if (index >= fCurrentSize) {
			UserProfile** oldProfiles = fProfiles;
			fProfiles = NEW UserProfile*[fCurrentSize * 2];
			for (i = 0; i < fCurrentSize; i++)
			{
				fProfiles[i] = oldProfiles[i];
			}
			fCurrentSize *= 2;
			delete[] oldProfiles;
		}

		fProfiles[index] = profile;
		index++;
	}
	fNumUsers = index;

	if (!groupFileErrors)
	{
		StringParser groupDataParser(&groupData);
		while (groupDataParser.GetDataRemaining() != 0) {
			// Read each line
			groupDataParser.GetThruEOL(&line);
			StringParser groupLineParser(&line);
			//parse the line
			//skip over leading whitespace
			groupLineParser.ConsumeUntil(NULL, StringParser::sWhitespaceMask);

			//skip over comments and blank lines
			if ((groupLineParser.GetDataRemaining() == 0) || (groupLineParser[0] == '#') || (groupLineParser[0] == '\0'))
				continue;

			//parse the groupname
			StrPtrLen groupName;
			groupLineParser.ConsumeUntil(&groupName, ':');

			if (groupLineParser.Expect(':')) {
				StrPtrLen groupUser;
				UInt32 nameLen = groupName.Len + 1;

				while (groupLineParser.GetDataRemaining() != 0)
				{
					groupLineParser.ConsumeWhitespace();
					groupLineParser.ConsumeUntilWhitespace(&groupUser);
					for (i = 0; i < fNumUsers; i++) {
						if (fProfiles[i]->username.Equal(groupUser))
						{
							UInt32 grpSize = fProfiles[i]->groupsSize;
							if (fProfiles[i]->numGroups >= grpSize) {
								char** oldGroups = fProfiles[i]->groups;
								fProfiles[i]->groups = NEW char*[grpSize * 2];
								for (j = 0; j < grpSize; j++) {
									fProfiles[i]->groups[j] = oldGroups[j];
								}
								fProfiles[i]->groupsSize *= 2;
								delete[] oldGroups;
							}

							fProfiles[i]->groups[fProfiles[i]->numGroups] = groupName.GetAsCString();
							if (nameLen > fProfiles[i]->maxGroupNameLen)
								fProfiles[i]->maxGroupNameLen = nameLen;
							fProfiles[i]->numGroups++;
							break;
						}
					}
				}
			}
		}
	}

	return resultErr;
}

// No memory is allocated
Bool16 AccessChecker::HaveFilePathsChanged(const char* inUsersFilePath, const char* inGroupsFilePath)
{
	Bool16 changed = true;
	if ((inUsersFilePath != NULL) && (inGroupsFilePath != NULL) && (fUsersFilePath != NULL) && (fGroupsFilePath != NULL)) {
		if ((strcmp(inUsersFilePath, fUsersFilePath) == 0) && (strcmp(inGroupsFilePath, fGroupsFilePath) == 0))
			changed = false;
	}
	return changed;
}

// No memory is allocated
AccessChecker::UserProfile* AccessChecker::RetrieveUserProfile(const StrPtrLen* inUserName)
{
	UInt32 index = 0;
	for (index = 0; index < fNumUsers; index++) {
		if (fProfiles[index]->username.Equal(*inUserName))
			return fProfiles[index];
	}
	return NULL;
}

