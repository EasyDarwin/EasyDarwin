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
    Modified htpasswd.c
    format of the file:
    username:crypt(password):MD5(username:realm:password)
*/

/******************************************************************************
 ******************************************************************************
 * NOTE! This program is not safe as a setuid executable!  Do not make it
 * setuid!
 ******************************************************************************
 *****************************************************************************/

#ifndef __Win32__
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#ifndef __MacOSX__
#include "getopt.h"
#endif
#include <unistd.h>
#else
#include "getopt.h"
#include "OSHeaders.h"
#include <conio.h>
#include <time.h>
#endif

#ifndef __Win32__
   #include <grp.h>
   #include <pwd.h>
#endif

#ifdef __solaris__
	#include <crypt.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <errno.h>
#include "StrPtrLen.h"
#include "md5digest.h"
#include "md5.h"
#include "defaultPaths.h"
#include "revision.h"

#ifdef __linux__
    #include <time.h>
    #include <crypt.h>
#endif

#ifndef CHARSET_EBCDIC
#define LF 10
#define CR 13
#else /*CHARSET_EBCDIC*/
#define LF '\n'
#define CR '\r'
#endif /*CHARSET_EBCDIC*/

#define MAX_STRING_LEN 255
#define MAX_LINE_LEN 5120
#define MAX_PASSWORD_LEN 80

const char* kDefaultQTPasswdFilePath = DEFAULTPATHS_ETC_DIR "qtusers";
const char*	kDefaultQTGroupsFilePath = DEFAULTPATHS_ETC_DIR "qtgroups";
const char* kDefaultRealmString = "Streaming Server";
const char* kDefaultFileOwner = "qtss";

char *tempUsersFile = NULL;
char *tempGroupsFile = NULL;

char *fileOwner = (char*) kDefaultFileOwner;

FILE *usersFilePtr = NULL, *tempUsersFilePtr = NULL, *passFilePtr = NULL, *groupsFilePtr = NULL;

void SetPrivileges(char *filePath);

static void closeFile(FILE **filePtr)
{
    if (filePtr && *filePtr)
    {    fclose(*filePtr);
         *filePtr = NULL;
    }

}

static void CleanUp(void)
{
    closeFile(&usersFilePtr);
    closeFile(&tempUsersFilePtr);
    closeFile(&passFilePtr);
    closeFile(&groupsFilePtr);
        
        
	if(tempUsersFile)
	{
		unlink(tempUsersFile);
		delete [] tempUsersFile;
	}
	
	if(tempGroupsFile)
	{
		unlink(tempGroupsFile);
		delete [] tempGroupsFile;
	}
}

/*
 * CleanupAndExit:  Deletes the temp file
 *                  and exits with code 1
 *
 */
static void CleanupAndExit(void)
{	
 #ifndef __Win32__
    if (EACCES == errno || EPERM == errno)
    {    
        qtss_fprintf(stderr, "You must use the sudo command to edit the users and groups files.\n");
        qtss_fprintf(stderr, "Example: sudo qtpasswd user\n");
    }
#endif

    CleanUp();
	exit(1);
}

/*
 * CopyString:  Returns a malloc'd copy
 *              of inString
 */
static char *CopyString(char *inString)
{
    char *outString = (char *) malloc(strlen(inString) + 1);
    strcpy(outString, inString);
    return outString;
}

/*
 * EatWhitespace: Goes past any leading whitespace
 *
 */
static void EatWhitespace(char *line)
{
    int x = 0, y = 0;
	
	while (line[x] == ' ')
		x++;

    while ((line[y++] = line[x++]));
}
 
/*
 * GetWord:     Reads all characters from the
 *              line until the stop character
 *              and returns it in word
 */
static void GetWord(char *word, char *line, char stop)
{
    int x = 0, y;

    for (x = 0; ((line[x]) && (line[x] != stop)); x++)
    word[x] = line[x];

    word[x] = '\0';
    if (line[x])
    ++x;
    y = 0;

    while ((line[y++] = line[x++]));
}

/*
 * GetLine:     Reads from file f, n characters
 *              or until newline and returns
 *              puts the line in s.
 */
static int GetLine(char *s, int n, FILE *f)
{
    register int i = 0;

    while (1) {
    s[i] = (char) fgetc(f);

    if (s[i] == CR)
        s[i] = fgetc(f);

    if ((s[i] == 0x4) || (s[i] == LF) || (i == (n - 1))) {
        s[i] = '\0';
        return (feof(f) ? 1 : 0);
    }
    ++i;
    }
}

/*
 * PutLine:     Writes string l to f and 
 *              puts a '\n' at the end
 */
static void PutLine(FILE *f, char *l)
{
    int x;

    for (x = 0; l[x]; x++)
    fputc(l[x], f);
    fputc('\n', f);
}

/*
 * PutWord:     Writes string l to f
 *				and puts the endChar
 *				at the end             
 */
static void PutWord(FILE *f, char *l, char endChar)
{
    int x;

    for (x = 0; l[x]; x++)
    fputc(l[x], f);
	fputc(endChar, f);
}

#if __Win32__
/*
 * Windows lacks getpass().  So we'll re-implement it here.
 */

static char *getpass(const char *prompt)
{
  static char password[MAX_PASSWORD_LEN + 1];
  int n = 0;
  
  fputs(prompt, stderr);
  
  while ((password[n] = _getch()) != '\r') {
    
    if(n == MAX_PASSWORD_LEN) {
      fputs("password can't be longer than MAX_PASSWORD_LEN chars.\n", stderr);
      fputs(prompt, stderr);
      for(n = 0; n < (MAX_PASSWORD_LEN + 1); n++)
    password[n] = '\0';
      n = 0;
      continue;
    }
      
    if (password[n] >= ' ' && password[n] <= '~') {
      n++;
      qtss_printf("*");
    }
    else {
      qtss_printf("\n");
      fputs(prompt, stderr);
      n = 0;
    }
  }

  password[n] = '\0';
  qtss_printf("\n");

  return (char *) &password;
}

#endif

/*
 * Digest:  Returns the MD5 hash of user:realm:password 
 */
static char* Digest(char *user, char *passwd, char *realm) 
{
    StrPtrLen userSPL(user), passwdSPL(passwd), realmSPL(realm), hashHex16Bit, hashSPL;
    CalcMD5HA1(&userSPL, &realmSPL, &passwdSPL, &hashHex16Bit); // memory allocated for hashHex16Bit.Ptr
    HashToString((unsigned char *)hashHex16Bit.Ptr, &hashSPL);  // memory allocated for hashSPL.Ptr
    char* digestStr = hashSPL.GetAsCString();
    delete [] hashSPL.Ptr;              // freeing memory allocated in above calls      
    delete [] hashHex16Bit.Ptr;
    return digestStr;
}

/*
 * AddPasswordWithoutPrompt:    Adds the entry in the file
 *                              user:crpytofpassword:md5hash(user:realm:password) 
 */
static void AddPasswordWithoutPrompt(char *user, char* password, char* realm, FILE *f)
{
    char salt[9];

    (void) srand((int) time((time_t *) NULL));
    to64(&salt[0], rand(), 8);
    salt[8] = '\0';

    char cpw[120], *dpw;
    int crpwLen = 0;

#ifdef __Win32__
    MD5Encode((char *)password, (char *)salt, cpw, sizeof(cpw));
#else
    char *crpw = (char *)crypt(password, salt); // cpw is crypt of password
    crpwLen = ::strlen(crpw);
    strncpy(cpw, crpw, crpwLen);
    cpw[crpwLen] = '\0';
#endif
    
    dpw = (char *)Digest(user, password, realm); // dpw is digest of password
    
    qtss_fprintf(f, "%s:%s:%s\n", user, cpw, dpw);
}

/*
 * AddPassword: Prompts the user for a password
 *              and adds the entry in the file
 *              user:crpytofpassword:md5hash(user:realm:password) 
 */
static void AddPassword(char *user, char* realm, FILE *f)
{
    char *pw, *crpw, cpw[120], salt[9], *dpw;
    int len = 0, i = 0, crpwLen = 0;
    char *checkw;

    pw = CopyString((char *) getpass("New password:"));
    /* check for a blank password */    
    len = strlen(pw);
    checkw = new char[len+1];
    for(i = 0; i < len; i++)
      checkw[i] = ' ';
    checkw[len] = '\0';
	
    if(strcmp(pw, checkw) == 0)
	{
        qtss_fprintf(stderr, "Password cannot be blank, sorry.\n");
        delete(checkw);
        CleanupAndExit();
    }
    delete(checkw);
    
    if (strcmp(pw, (char *) getpass("Re-type new password:")))
	{
		qtss_fprintf(stderr, "They don't match, sorry.\n");
		CleanupAndExit();
    }
	
    (void) srand((int) time((time_t *) NULL));
    to64(&salt[0], rand(), 8);
    salt[8] = '\0';

#ifdef __Win32__
    MD5Encode((char *)pw, (char *)salt, cpw, sizeof(cpw));
#else
    crpw = (char *)crypt(pw, salt); // cpw is crypt of password
    crpwLen = ::strlen(crpw);
    strncpy(cpw, crpw, crpwLen);
    cpw[crpwLen] = '\0';
#endif
    
    dpw = (char *)Digest(user, pw, realm); // dpw is digest of password
    
    qtss_fprintf(f, "%s:%s:%s\n", user, cpw, dpw);
    free(pw); // Do after cpw and dpw are used. 
}

/*
 * Usage:   Prints the usage and calls exit
 */
static void usage(void)
{
    qtss_fprintf(stderr, "  qtpasswd %s built on: %s\n", kVersionString, __DATE__ ", "__TIME__);
    qtss_fprintf(stderr, "  Usage: qtpasswd [-F] [-f filename] [-c] [-g groupsfilename] [-r realm] [-p password] [-P passwordfile] [-A group] [-D group] [-d] [username]\n");
    qtss_fprintf(stderr, "  -F   Don't ask for confirmation when deleting users or overwriting existing files.\n");
    qtss_fprintf(stderr, "  -f   Password file to manipulate (Default is \"%s\").\n", kDefaultQTPasswdFilePath);
    qtss_fprintf(stderr, "  -c   Create new file.\n");
    qtss_fprintf(stderr, "  -g   Groups file to manipulate (Default is \"%s\"). If not found, will create one when necessary.\n", kDefaultQTGroupsFilePath);
    qtss_fprintf(stderr, "  -r   The realm name to use when creating a new file via \"-c\" (Default is \"%s\").\n", kDefaultRealmString);
    qtss_fprintf(stderr, "  -p   Allows entry of password at command line rather than prompting for it.\n");
    qtss_fprintf(stderr, "  -P   File to read the password from rather than prompting for it.\n");
    qtss_fprintf(stderr, "  -d   Delete the user. (Deletes the user from all groups)\n");
    qtss_fprintf(stderr, "  -A   Add user to group. Will create group automatically if group is not already present.\n");
    qtss_fprintf(stderr, "  -D   Delete the user from the group.\n");
    qtss_fprintf(stderr, "  -C   Create new group. Do not specify username with this option.\n");
    qtss_fprintf(stderr, "  -R   Delete the group. Do not specify username with this option.\n");
    qtss_fprintf(stderr, "  -O   Set the owner of the file (Default is \"%s\").\n", kDefaultFileOwner);
    qtss_fprintf(stderr, "  -h   Displays usage.\n");
    qtss_fprintf(stderr, "  -v   Displays usage.\n");
    qtss_fprintf(stderr, "  -?   Displays usage.\n");
    qtss_fprintf(stderr, "  Note:\n");
    qtss_fprintf(stderr, "  The username must always be specified except when -C and -R options are used to create/delete group.\n");
    qtss_fprintf(stderr, "  Usernames cannot be more than %d characters long and must not include a colon [:].\n", MAX_STRING_LEN);
    qtss_fprintf(stderr, "  Passwords cannot be more than %d characters long.\n", MAX_PASSWORD_LEN);
    qtss_fprintf(stderr, "  Groups cannot be more than %d characters long and must not include a colon [:].\n", MAX_STRING_LEN);
    qtss_fprintf(stderr, "  If the username/password contains whitespace or characters that may be\n");
    qtss_fprintf(stderr, "  interpreted by the shell please enclose it in single quotes,\n");
    qtss_fprintf(stderr, "  to prevent it from being interpolated.\n");
    qtss_fprintf(stderr, "\n");
    exit(1);
}

// unused routine
static char* SetTempPath(char* bufferToSet, int bufferLen, char* base, int baseLen, char id)
{
    if (bufferLen > 0 && bufferToSet != NULL)
        memset(bufferToSet, 0, bufferLen);
        
    if (baseLen + 2 > bufferLen)
        return bufferToSet;
        
    strcpy(bufferToSet, base);
	bufferToSet[baseLen] = id;
	bufferToSet[baseLen + 1] = '\0';
  
    return bufferToSet;
}



static void AddOrDeleteUserFromGroup(int add, char *userName, char *groupName, char *inGroupsFilePath, char *inTempGroupsFilePath)
{
    char line[MAX_LINE_LEN];
    char lineFromFile[MAX_LINE_LEN];
    char groupNameFromFile[MAX_STRING_LEN + 1];
    bool foundGroup = false;
	char userInGroup[MAX_STRING_LEN + 1];
	bool addedUserToGroup = false;
	FILE *groupsFilePtr, *tempGroupsFilePtr;

#if __Win32__
	char groupBackupPath[1024] = "";
	char groupTempPath[1024] = "";

    int pathBaseLen = strlen(inTempGroupsFilePath);

   (void) SetTempPath(groupBackupPath, sizeof(groupBackupPath), inTempGroupsFilePath, pathBaseLen, 'b');
    inTempGroupsFilePath = SetTempPath(groupTempPath, sizeof(groupTempPath), inTempGroupsFilePath, pathBaseLen, 'x');
#endif

    if (!(groupsFilePtr = fopen(inGroupsFilePath, "r")))
    {   
		if (add)
		{
			char buffer[kErrorStrSize];
			qtss_fprintf(stderr, "Could not open groups file %s for reading. (err=%d:%s)\n", inGroupsFilePath, errno,  qtss_strerror(errno, buffer, sizeof(buffer)));
			CleanupAndExit();
		}
		else
			return;
    }
		
	if (!(tempGroupsFilePtr = fopen(inTempGroupsFilePath, "w"))) 
    {   
        char buffer[kErrorStrSize];
        qtss_fprintf(stderr, "Could not open temp groups file. (err=%d %s)\n", errno,  qtss_strerror(errno, buffer, sizeof(buffer)));
		CleanupAndExit();
    }
	
	while (!(GetLine(line, MAX_LINE_LEN, groupsFilePtr))) 
    {
        //write comments and blank lines out to temp file
        if (foundGroup || (line[0] == '#') || (line[0] == 0)) 
        {
            PutLine(tempGroupsFilePtr, line);
            continue;
        }
		
        strcpy(lineFromFile, line);
		EatWhitespace(lineFromFile);
        GetWord(groupNameFromFile, lineFromFile, ':');

        //if it's not the group we're looking for, write the line out to the temp file
        if ((groupName != NULL) && strcmp(groupName, groupNameFromFile) != 0) 
        {
            PutLine(tempGroupsFilePtr, line);
            continue;
        }
        else 
        {
			PutWord(tempGroupsFilePtr, groupNameFromFile, ':');
			
			while (true)
			{
				userInGroup[0] = '\0';
				EatWhitespace(lineFromFile);
				GetWord(userInGroup, lineFromFile, ' ');
				
				if (userInGroup[0] == '\0')
					break;
					
				if (strcmp(userName, userInGroup) != 0)
					PutWord(tempGroupsFilePtr, userInGroup, ' ');
				else if(add)
				{
					PutWord(tempGroupsFilePtr, userInGroup, ' ');
					addedUserToGroup = true;
				}
			}
			
			if (add && !addedUserToGroup)
			{
				PutWord(tempGroupsFilePtr, userName, ' ');
				addedUserToGroup = true;
			}
			
			fputc('\n', tempGroupsFilePtr);
				
            if (groupName != NULL)
				foundGroup = true;
        }
    }
	
	if (add && !addedUserToGroup)
	{
		PutWord(tempGroupsFilePtr, groupName, ':');
		PutLine(tempGroupsFilePtr, userName);
	}
	closeFile(&groupsFilePtr);
    closeFile(&tempGroupsFilePtr);
	
    // Rename the temp groups file to the groups file

#if __Win32__
	unlink(groupBackupPath);
	rename(inGroupsFilePath, groupBackupPath);
	unlink(inGroupsFilePath);
    if (rename(inTempGroupsFilePath, inGroupsFilePath) != 0)
	{
	  rename(groupBackupPath, inGroupsFilePath);
	  unlink(groupBackupPath);
      perror("rename failed with error");
	  CleanupAndExit();
	}
	unlink(groupBackupPath);
	unlink(inTempGroupsFilePath);
	
#else
    if (rename(inTempGroupsFilePath, inGroupsFilePath) != 0)
	{
        perror("rename failed with error");
		CleanupAndExit();
	}  
#endif
 
	
	SetPrivileges(inGroupsFilePath);

}

static void AddOrDeleteGroup(int add, char *groupName, char *inGroupsFilePath, char *inTempGroupsFilePath)
{
    char line[MAX_LINE_LEN];
    char lineFromFile[MAX_LINE_LEN];
    char groupNameFromFile[MAX_STRING_LEN + 1];
    bool foundGroup = false;
	FILE *groupsFilePtr, *tempGroupsFilePtr;
	bool addedGroup = false;
	
#if __Win32__
	char groupBackupPath[1024] = "";
	char groupTempPath[1024] = "";

    int pathBaseLen = strlen(inTempGroupsFilePath);

   (void) SetTempPath(groupBackupPath, sizeof(groupBackupPath), inTempGroupsFilePath, pathBaseLen, 'b');
    inTempGroupsFilePath = SetTempPath(groupTempPath, sizeof(groupTempPath), inTempGroupsFilePath, pathBaseLen, 'x');
#endif
	
    if (!(groupsFilePtr = fopen(inGroupsFilePath, "r")))
    {   
		if (add)
		{
			char buffer[kErrorStrSize];
			qtss_fprintf(stderr, "Could not open groups file %s for reading. (err=%d:%s)\n", inGroupsFilePath, errno,  qtss_strerror(errno, buffer, sizeof(buffer)));
			CleanupAndExit();
		}
		else
			return;
    }
		
	if (!(tempGroupsFilePtr = fopen(inTempGroupsFilePath, "w"))) 
    {   
        char buffer[kErrorStrSize];
        qtss_fprintf(stderr, "Could not open temp groups file. (err=%d %s)\n", errno,  qtss_strerror(errno, buffer, sizeof(buffer)));
		CleanupAndExit();
    }
	
	while (!(GetLine(line, MAX_LINE_LEN, groupsFilePtr))) 
    {
        //write comments and blank lines out to temp file
        if (foundGroup || (line[0] == '#') || (line[0] == 0)) 
        {
            PutLine(tempGroupsFilePtr, line);
            continue;
        }
		
        strcpy(lineFromFile, line);
		EatWhitespace(lineFromFile);
        GetWord(groupNameFromFile, lineFromFile, ':');

        //if it's not the group we're looking for, write the line out to the temp file
        if ((groupName != NULL) && strcmp(groupName, groupNameFromFile) != 0) 
        {
            PutLine(tempGroupsFilePtr, line);
            continue;
        }
        else if (add) // if we are trying to add the group and it already exists, leave it in
		{
			PutLine(tempGroupsFilePtr, line);
			addedGroup = true;
		}
		
		foundGroup = true;
	}		
	
	if (add && !addedGroup)
	{
		PutWord(tempGroupsFilePtr, groupName, ':');
		fputc('\n', tempGroupsFilePtr);
	}
							
	closeFile(&groupsFilePtr);
    closeFile(&tempGroupsFilePtr);

	
    // Rename the temp groups file to the groups file
#if __Win32__
	_unlink(groupBackupPath); //make sure it is clean
    rename(inGroupsFilePath, groupBackupPath); //move the groups file to the new name
	_unlink(inGroupsFilePath);// make sure it is clean
	
    if (rename(inTempGroupsFilePath, inGroupsFilePath) != 0) // move the temp to the real name
	{
      perror("rename failed with error");
	  rename(groupBackupPath, inGroupsFilePath);
	  unlink(groupBackupPath);
	  unlink(inTempGroupsFilePath);
	  CleanupAndExit();
	}
	unlink(groupBackupPath); //clean up
	unlink(inTempGroupsFilePath);
#else
    if (rename(inTempGroupsFilePath, inGroupsFilePath) != 0)
	{
        perror("rename failed with error");
		CleanupAndExit();
	}  
#endif
     
 }

/* Allocates memory; remember to delete it afterwards */
char* GetTempFileAtPath(char* templatePath, int templatePathLength)
{
	char* tempFile = new char[templatePathLength];
	memcpy(tempFile, templatePath, templatePathLength);
	char* theResultTempFile = NULL;
	
#ifdef __Win32__	
	theResultTempFile = mktemp(tempFile);
#else	
	int theErr = mkstemp(tempFile);
	if (theErr != -1)
		theResultTempFile = tempFile;
#endif

	if (theResultTempFile == NULL)
	{
		char buffer[kErrorStrSize];
		qtss_fprintf(stderr, "Could not create a temp file at %s. (err=%d:%s)\n", tempFile, errno,  qtss_strerror(errno, buffer, sizeof(buffer)));
		CleanupAndExit();
	}
	
	return theResultTempFile;
}

int main(int argc, char *argv[])
{
    //char line[MAX_STRING_LEN + 1];
    char line[MAX_LINE_LEN];
    char lineFromFile[MAX_LINE_LEN];
    char usernameFromFile[MAX_STRING_LEN + 1];
    char realmFromFile[MAX_STRING_LEN + 1];
    int found;
    int result;
    static char choice[81];

    int doCreateNewFile = 0;
    int doDeleteUser = 0;
	int addUserToGroup = 0;
	int deleteUserFromGroup = 0;
	int createGroup = 0;
	int deleteGroup = 0;
    int confirmPotentialDamage = 1;
    char* qtusersFilePath = NULL;
	char* qtgroupsFilePath = NULL;
    char* userName = NULL;
	char* groupName = NULL;
    char* realmString = NULL;
    char* password = NULL;
    char* passwordFilePath = NULL;
    int ch;
    extern char* optarg;
    extern int optind;

	/* Read command line arguments */
    while ((ch = getopt(argc, argv, "O:f:cg:r:p:P:A:D:C:R:dFhv?")) != EOF)
    {
        switch(ch) 
        {

            case 'f':
                    qtusersFilePath = CopyString(optarg);
            break;
			
            case 'c':
                    doCreateNewFile = 1;
            break;
			
            case 'g':
                    qtgroupsFilePath = CopyString(optarg);
            break;
			
            case 'r':
                    realmString = CopyString(optarg);
                    if (::strlen(realmString) > MAX_STRING_LEN)
                    {
                        qtss_fprintf(stderr, "Realm cannot have more than %d characters.\n", MAX_STRING_LEN);
                        qtss_printf("Exiting! \n");
                        exit(1);
                    }
            break;

            case 'p':
                    password = CopyString(optarg);
					::memset(optarg, 0, ::strlen(optarg));
					
                    if (::strlen(password) > MAX_PASSWORD_LEN)
                    {
                        qtss_fprintf(stderr, "Password cannot have more than %d characters.\n", MAX_PASSWORD_LEN);
                        qtss_printf("Exiting! \n");
                        exit(1);
                    }
            break;

            case 'O':
                    fileOwner = CopyString(optarg);
            break;

            case 'P':
                    passwordFilePath = CopyString(optarg);
            break;

            case 'A':
                    groupName = CopyString(optarg);
					addUserToGroup = 1;
            break;

            case 'D':
                    groupName = CopyString(optarg);
					deleteUserFromGroup = 1;
            break;

            case 'C':
                    groupName = CopyString(optarg);
					createGroup = 1;
            break;

            case 'R':
                    groupName = CopyString(optarg);
					deleteGroup = 1;
            break;			

            case 'd':
                    doDeleteUser = 1;
            break;
			
            case 'F':
                    confirmPotentialDamage = 0;
            break;

        case 'h':
        case 'v':
            case '?':
            default:
                usage(); 
            break;
        }
    }

	/* If password is to be read from a file, check validity of the password */
	if ((password == NULL) && (passwordFilePath != NULL))
    {
		if ((passFilePtr = fopen(passwordFilePath, "r")) != NULL ) 
		{
			char passline[MAX_STRING_LEN];
			char passFromFile[MAX_STRING_LEN];
			int passLen = 0;
			
			::memset(passline, 0, MAX_STRING_LEN);
			::memset(passFromFile, 0, MAX_STRING_LEN);
			
			GetLine(passline, MAX_STRING_LEN, passFilePtr);
	
			if (passline[0] == '\'')        // if it is single quoted, read until the end single quote
				GetWord(passFromFile, (passline + 1), '\'');
			else if (passline[0] == '"')        // if it is double quoted, read until the end double quote
				GetWord(passFromFile, (passline + 1), '"');
			else                    // if it is not quoted, read until the first whitespace
				GetWord(passFromFile, passline, ' ');
				
			passLen = ::strlen(passFromFile);
				
			if (passLen == 0)
			{
				qtss_fprintf(stderr, "Password in file %s is blank.\n", passwordFilePath);
				qtss_printf("Exiting! \n");
				exit(1);
			}
			else if (passLen > MAX_PASSWORD_LEN)
			{
				qtss_fprintf(stderr, "Password in file %s has more than %d characters. Cannot accept password.\n", passwordFilePath, MAX_PASSWORD_LEN);
				qtss_printf("Exiting! \n");
				exit(1);
			}
			else
				password = CopyString(passFromFile);

	        closeFile(&passFilePtr);
        }
    }

    /* deleting a user and (creating a file or setting a password) don't make sense together */
    if ( doDeleteUser && (doCreateNewFile || password != NULL) )
    {
		qtss_fprintf(stderr, "Cannot use the -c option (to create the file) with the -d option (to delete the user).\n");
		qtss_printf("Exiting! \n");
        usage();
    }

    /* realm name only makes sense when creating a new password file */
    if ( !doCreateNewFile && (realmString != NULL) )
    {
		qtss_fprintf(stderr, "Can use the -r option only with the -c option (when creating the file).\n");
		qtss_printf("Exiting! \n");
		usage();
    }

	/* group name checks */
	if (groupName != NULL) 
	{
		/* check length < MAX_STRING_LEN */
		if (::strlen(groupName) > MAX_STRING_LEN)
		{
			qtss_fprintf(stderr, "Group name cannot have more than %d characters.\n", MAX_STRING_LEN);
			qtss_printf("Exiting! \n");
			exit(1);
		}	
		
		/* check for : */
        if (strchr(groupName, ':') != NULL) 
        {
            qtss_printf("Group name cannot contain a ':' character.\n");
			qtss_printf("Exiting! \n");
			exit(1);
        }
		
		/* can't add user to group and delete user from a group at the same time */
		if (addUserToGroup && deleteUserFromGroup)
        {
            qtss_printf("Cannot add to or delete from a group at the same time (use either -A or -D option, not both!).\n");
			qtss_printf("Exiting! \n");
			exit(1);
        }
		
		/* can't create and delete group at the same time */
		if (createGroup && deleteGroup)
        {
            qtss_printf("Cannot create new group and delete group at the same time (use either -C or -R option, not both!).\n");
			qtss_printf("Exiting! \n");
			exit(1);
        }
	}

	/* Read in the username */
    if (argv[optind] != NULL)
    {
		/* If group needs to be created or deleted, username will be ignored. */
		if (createGroup || deleteGroup)
		{
			qtss_fprintf(stderr, "Warning: username cannot be specified with -C or -R option and will be ignored!\n");
		}
		else
		{
			userName = CopyString(argv[optind]);
			
			/* check length < MAX_STRING_LEN */		
			if (::strlen(userName) > MAX_STRING_LEN)
			{
			qtss_fprintf(stderr, "Username cannot have more than %d characters.\n", MAX_STRING_LEN);
			qtss_printf("Exiting! \n");
			exit(1);
			}
		}
    }
    else
	{
		/* Exit if username is not given, unless a group has to be created or deleted. */
		if (!createGroup && !deleteGroup)
		{
			qtss_fprintf(stderr, "Username not given!\n");
			qtss_printf("Exiting! \n");
			usage();
		}
    }

    if (confirmPotentialDamage && doDeleteUser)
    {
        qtss_printf("Delete user %s (will also delete user from groups in the groups file)? y or n [y] ", userName);
        fgets( (char*)&choice, 80, stdin);
        if( choice[0] == 'n' || choice[0] == 'N' ) 
            exit(0);
    }

    if (qtusersFilePath == NULL)
    {
        qtusersFilePath = new char[strlen(kDefaultQTPasswdFilePath)+1];
        strcpy(qtusersFilePath, kDefaultQTPasswdFilePath);
        
    }

    if (qtgroupsFilePath == NULL)
    {
        qtgroupsFilePath = new char[strlen(kDefaultQTGroupsFilePath)+1];
        strcpy(qtgroupsFilePath, kDefaultQTGroupsFilePath);
    }
		
    if (realmString  == NULL)
    {
        char* kDefaultRealmString = "Streaming Server";

        realmString = new char[strlen(kDefaultRealmString)+1];
        strcpy(realmString, kDefaultRealmString);
    }

    
    tempUsersFile = NULL;
	tempGroupsFile = NULL;
	
#ifndef __Win32__
    signal(SIGINT, (void(*)(int))CleanupAndExit);
    //create file with owner RW permissions only
    umask(S_IRWXO|S_IRWXG);
#endif

    if (doCreateNewFile)
    {
        if (confirmPotentialDamage)
            if( (usersFilePtr = fopen(qtusersFilePath, "r")) != NULL ) 
            {
	            closeFile(&usersFilePtr);
    
                qtss_printf("File already exists. Do you wish to overwrite it? y or n [y] ");
                fgets( (char*)&choice, 80, stdin);
                if( choice[0] == 'n' || choice[0] == 'N' ) 
                    CleanupAndExit();
                    
            }

        //create new file or truncate existing one to 0
        if ( (usersFilePtr = fopen(qtusersFilePath, "w")) == NULL)
        {   
			char buffer[kErrorStrSize];
            qtss_fprintf(stderr, "Could not open password file %s for writing. (err=%d %s)\n", qtusersFilePath, errno, qtss_strerror(errno, buffer, sizeof(buffer)));
            perror("fopen");
            CleanupAndExit();
        }

        qtss_printf("Creating password file for realm %s.\n", realmString);

        //write the realm into the file
        qtss_fprintf(usersFilePtr, "realm %s\n", realmString);

	    closeFile(&usersFilePtr);
	    SetPrivileges(qtusersFilePath);


    }

#ifdef __Win32__
    char separator = '\\';
#else
    char separator = '/';
#endif
	char* tmpFile = "tmp.XXXXXX";
	char* alternateTempPath = "./tmp.XXXXXX";
    char* tempFilePath;
	int	tempFilePathLength = 0;
    char* lastOccurOfSeparator = strrchr(qtusersFilePath, separator);
    int pathLength = strlen(qtusersFilePath);
	
   if(lastOccurOfSeparator != NULL) 
    {
		int filenameLength = ::strlen(lastOccurOfSeparator) + sizeof(char);
		tempFilePathLength = pathLength - filenameLength + sizeof(char) + ::strlen(tmpFile) + 2;
		
		tempFilePath = new char[tempFilePathLength + 2];

        memcpy(tempFilePath, qtusersFilePath, (pathLength - filenameLength + 2));
	    memcpy(tempFilePath + (pathLength - filenameLength) + 2, tmpFile, ::strlen(tmpFile));
	    tempFilePath[pathLength - filenameLength + ::strlen(tmpFile) + 2] = '\0';

		/* Get temp users file path name */
		if (!createGroup && !deleteGroup)
			tempUsersFile = GetTempFileAtPath(tempFilePath, tempFilePathLength);

		/* Get temp groups file path name */		
		if ((groupName != NULL) || doDeleteUser)
			tempGroupsFile = GetTempFileAtPath(tempFilePath, tempFilePathLength);
		
		delete [] tempFilePath;
    }
    else 
    {
		if (!createGroup && !deleteGroup)
			tempUsersFile = GetTempFileAtPath(alternateTempPath, ::strlen(alternateTempPath));
		if ((groupName != NULL) || doDeleteUser)
			tempGroupsFile = GetTempFileAtPath(alternateTempPath, ::strlen(alternateTempPath));
	}
		
	if ((groupName != NULL) && !(groupsFilePtr = fopen(qtgroupsFilePath, "r")))
	{
		char buffer[kErrorStrSize];
		qtss_fprintf(stderr, "Could not open groups file %s to manipulate groups file. (err=%d:%s)\n", qtgroupsFilePath, errno,  qtss_strerror(errno, buffer, sizeof(buffer)));
				
		//create new file
        if ( (groupsFilePtr = fopen(qtgroupsFilePath, "w")) == NULL)
        {   
			char buffer[kErrorStrSize];
            qtss_fprintf(stderr, "Could not create a new groups file %s either. (err=%d %s)\n", qtgroupsFilePath, errno, qtss_strerror(errno, buffer, sizeof(buffer)));
			CleanupAndExit();
        }
		else
			qtss_printf("Created new groups file %s.\n", qtgroupsFilePath);

	    closeFile(&groupsFilePtr);
	    SetPrivileges(qtgroupsFilePath);
	}
	else
       closeFile(&groupsFilePtr);
 
	if (createGroup)
	{
		AddOrDeleteGroup(1, groupName, qtgroupsFilePath, tempGroupsFile);
		qtss_printf("Created new group %s\n", groupName);
		return 0;
	}
	
	if (deleteGroup)
	{
		AddOrDeleteGroup(0, groupName, qtgroupsFilePath, tempGroupsFile);
		qtss_printf("Deleted group %s\n", groupName);
		return 0;
	}
	
    if (!(tempUsersFilePtr = fopen(tempUsersFile, "w"))) 
    {   
        char buffer[kErrorStrSize];
        qtss_printf("failed\n");
        qtss_fprintf(stderr, "Could not open temp users file. (err=%d %s)\n", errno,  qtss_strerror(errno, buffer, sizeof(buffer)));
		CleanupAndExit();
    }
	
    if (!(usersFilePtr = fopen(qtusersFilePath, "r")))
    {   
        char buffer[kErrorStrSize];
        qtss_fprintf(stderr, "Could not open passwd file %s for reading. (err=%d:%s)\n", qtusersFilePath, errno,  qtss_strerror(errno, buffer, sizeof(buffer)));
        qtss_fprintf(stderr, "Use -c option to create new one.\n");
		CleanupAndExit();
    }

    // Get the realm from the first line
    while (!(GetLine(line, MAX_LINE_LEN, usersFilePtr))) 
    {
        if ((line[0] == '#') || (!line[0]))
        {
            PutLine(tempUsersFilePtr, line);
            continue;
        }
        else
        {
            // line is "realm somename"
            if( strncmp(line, "realm", strlen("realm")) != 0 )
            {
                    qtss_fprintf(stderr, "Invalid users file.\n");
                    qtss_fprintf(stderr, "The first non-comment non-blank line must be the realm line\n");
                    qtss_fprintf(stderr, "The file may have been tampered manually!\n");				
					CleanupAndExit();
            }
            strcpy(realmFromFile ,line + strlen("realm")+1); 
            PutLine(tempUsersFilePtr, line);
            break;  
        }
    }
    // Look for an existing entry with the username
    found = 0;
    while (!(GetLine(line, MAX_LINE_LEN, usersFilePtr))) 
    {
        //write comments and blank lines out to temp file
        if (found || (line[0] == '#') || (line[0] == 0)) 
        {
            PutLine(tempUsersFilePtr, line);
            continue;
        }
        strcpy(lineFromFile, line);
        GetWord(usernameFromFile, lineFromFile, ':');

        //if not the user we're looking for, write the line out to the temp file
        if (strcmp(userName, usernameFromFile) != 0) 
        {
            PutLine(tempUsersFilePtr, line);
            continue;
        }
        else 
        {
            if (doDeleteUser)
            {   //to delete a user - just don't write it out to the temp file
                qtss_printf("Deleting user %s\n", userName);
				//delete user from all groups in the group file
				qtss_printf("Deleting user %s from all groups\n", userName);
				AddOrDeleteUserFromGroup(0, userName, NULL, qtgroupsFilePath, tempGroupsFile);
				
            }
            else
            {
				if (addUserToGroup)
				{
					PutLine(tempUsersFilePtr, line);
					
					qtss_printf("Adding user %s to group %s\n", userName, groupName);
					AddOrDeleteUserFromGroup(1, userName, groupName, qtgroupsFilePath, tempGroupsFile);
				}
				else if (deleteUserFromGroup)
				{
					PutLine(tempUsersFilePtr, line);
					
					qtss_printf("Deleting user %s from group %s\n", userName, groupName);
					AddOrDeleteUserFromGroup(0, userName, groupName, qtgroupsFilePath, tempGroupsFile);
				}
				else
				{
					qtss_printf("Changing password for user %s\n", userName);
					if(password != NULL)
						AddPasswordWithoutPrompt(userName, password, realmFromFile, tempUsersFilePtr);
					else 
						AddPassword(userName, realmFromFile, tempUsersFilePtr);
				}
            }
            found = 1;
        }
    }
    
    if (!found) 
    {
        if (doDeleteUser)
        {
            qtss_printf("Username %s not found in users file.\n", userName);
			
			//delete user from all groups in the group file
			qtss_printf("Deleting user %s from all groups if found in groups file\n", userName);
			AddOrDeleteUserFromGroup(0, userName, NULL, qtgroupsFilePath, tempGroupsFile);
			CleanupAndExit();
        }

        /* check for : in name before adding user */
        if(strchr(userName, ':') != NULL) 
        {
            qtss_printf("Username cannot contain a ':' character.");
			CleanupAndExit();
        }
    
        qtss_printf("Adding userName %s\n", userName);
        if(password != NULL)
            AddPasswordWithoutPrompt(userName, password, realmFromFile, tempUsersFilePtr);
        else 
            AddPassword(userName, realmFromFile, tempUsersFilePtr);
			
		if (addUserToGroup)
		{
			qtss_printf("Adding user %s to group %s\n", userName, groupName);
			AddOrDeleteUserFromGroup(1, userName, groupName, qtgroupsFilePath, tempGroupsFile);
		}
		else if (deleteUserFromGroup)
		{
			qtss_printf("Deleting user %s from group %s\n", userName, groupName);
			AddOrDeleteUserFromGroup(0, userName, groupName, qtgroupsFilePath, tempGroupsFile);
		}
    }
    	
    closeFile(&usersFilePtr); 
    closeFile(&tempUsersFilePtr); 
    
    // Remove old file and change name of temp file to match new file
    remove(qtusersFilePath);

    result = rename(tempUsersFile, qtusersFilePath);
    if(result != 0)
	{
        perror("rename failed with error");
		CleanupAndExit();
	}

	SetPrivileges(qtusersFilePath);
	CleanUp();
    return 0;
}


void SetPrivileges(char *filePath)
{
#ifndef __Win32__
  
    int result =0;
    uid_t owner = (uid_t) -1; // if fileOwner not found set to ignore owner setting

    struct passwd* pw = ::getpwnam(fileOwner); // set by -O at the command line
    if (pw) 
        owner = pw->pw_uid;

    
#if __MacOSX__
    if (owner ==  (uid_t) -1) // force user 76 on OS X.
        owner = 76;
        
    result = ::chown(filePath,owner,80);//default is owner qtss, group admin
    //printf("chown %s result =%d errno=%d\n",filePath, result, errno);
      
   if (result != 0)
   {  
        qtss_fprintf(stderr, "permission failure accessing file %s\n", filePath);
        CleanupAndExit();
   }
#endif

    result = ::chown(filePath, (uid_t) owner, (gid_t) -1);//default is owner qtss, group root
   
    if (result != 0)
        result = ::chmod(filePath, S_IRUSR | S_IWUSR | S_IRGRP );
        
    //printf("chmod %s result =%d errno=%d\n",filePath, result,errno);
   
   if (result != 0)
   {    qtss_fprintf(stderr, "permission failure accessing file %s\n", filePath);
        CleanupAndExit();
   }
#endif	


}
