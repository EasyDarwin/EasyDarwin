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
    File:       PathEditor.cpp
    
    Contains:   This file simply edits the Win32 registry to add
                the path given as input to the system path. 
*/

#include <stdio.h>

#include "OS.h"
#include "OSHeaders.h"
#include "OSMemory.h"
#include "StrPtrLen.h"

int main(int argc, char *argv[])
{
    HKEY    hSystemkey;
    HKEY    hCurrentControlSetkey;
    HKEY    hControlkey;
    HKEY    hSessionManagerkey;
    HKEY    hEnvironmentkey;
    
    enum { kSuccess, kOpenKeyFailed, kQueryCurrentValueFailed, kSetValueFailed, kQueryNewValueFailed };

    UInt32 status = kSuccess;

    // current system path
    char* systemPath = NEW char [2049];
    UInt32 systemPathLen = 2048;
    
    // default path to append to system path if path not given
    char* defaultPath = "C:\\Program Files\\Darwin Streaming Server";
    
    // path to append to system path
    char* path = NULL;
    UInt32 pathLen = 0;
    
    // new system path
    char* newSystemPath = NULL;
    UInt32 newSystemPathLen = 0;
    

    if(argc == 1)
    {
        printf("No input path given.\n");
        printf("Default path of \"%s\" will be used\n\n", defaultPath);
        pathLen = ::strlen(defaultPath);
        path = NEW char [pathLen + 1];
        ::strcpy(path, defaultPath);
    }
    else
    {   
        pathLen = ::strlen(argv[1]);
        path = NEW char [pathLen + 1];
        ::strcpy(path, argv[1]);
        printf("path \"%s\" will be added to the system path\n", path);
    }
    
    {

        if ( (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "System", 0, KEY_ALL_ACCESS, &hSystemkey) != ERROR_SUCCESS)
            ||(RegOpenKeyEx(hSystemkey, "CurrentControlSet", 0, KEY_ALL_ACCESS, &hCurrentControlSetkey) != ERROR_SUCCESS)
            ||(RegOpenKeyEx(hCurrentControlSetkey, "Control", 0, KEY_ALL_ACCESS, &hControlkey) != ERROR_SUCCESS)
            ||(RegOpenKeyEx(hControlkey, "Session Manager", 0, KEY_ALL_ACCESS, &hSessionManagerkey) != ERROR_SUCCESS)
            ||(RegOpenKeyEx(hSessionManagerkey, "Environment", 0, KEY_ALL_ACCESS, &hEnvironmentkey) != ERROR_SUCCESS) )
        {
            status = kOpenKeyFailed;
            goto bail;
        }

        if (RegQueryValueEx(hEnvironmentkey, "Path", NULL, NULL, (LPBYTE)systemPath, &systemPathLen) != ERROR_SUCCESS)
        {
            status = kQueryCurrentValueFailed;
            goto bail;
        }

        // need to do this as the returned length includes a bunch of null characters at the end!
        systemPathLen = ::strlen(systemPath);

        // print the current system path
        printf ("Current system path is %s\n\n", systemPath);
    
        StrPtrLen systemPathPtrLen(systemPath, systemPathLen);
        if (systemPathPtrLen.FindString(path) != NULL)
            // if string already exists in the path,do nothing
            printf ("Path \"%s\" already exists in system path\n\n", path);
        else
        {
            // copy current system path
            newSystemPath = NEW char[systemPathLen + 1 + pathLen + 1];
            ::memcpy (newSystemPath, systemPath, systemPathLen);
            newSystemPathLen += systemPathLen;
    
            // if system path doesn't end with a ';' (semicolon) then append one
            if(systemPath[systemPathLen - 1] != ';')
                newSystemPath[newSystemPathLen++] = ';'; 
        
            // append our path
            ::memcpy (newSystemPath + newSystemPathLen, path, pathLen);
            newSystemPathLen += pathLen;

            newSystemPath[newSystemPathLen++] = '\0';
        }

        // set the registry to the new path
        if (newSystemPath != NULL)
            if (RegSetValueEx(hEnvironmentkey, "Path", 0, REG_EXPAND_SZ, (CONST BYTE *)newSystemPath, newSystemPathLen) != ERROR_SUCCESS)
            {
                status = kSetValueFailed;
                goto bail;
            }

        // get the value again to make sure
        char* verifiedSystemPath = NEW char [3073];
        UInt32 verifiedSystemPathLen = 3072;

        if (RegQueryValueEx(hEnvironmentkey, "Path", NULL, NULL, (LPBYTE)verifiedSystemPath, &verifiedSystemPathLen) != ERROR_SUCCESS)
        {
            status = kQueryNewValueFailed;
            goto bail;
        }
    
        // print the new system path (with our path appended)
        printf ("The new system path is %s\n\n", verifiedSystemPath);
    }

bail:

    // close all the open keys
    RegCloseKey(hEnvironmentkey);
    RegCloseKey(hSessionManagerkey);
    RegCloseKey(hControlkey);
    RegCloseKey(hCurrentControlSetkey);
    RegCloseKey(hSystemkey);

    switch (status) {
        case kSuccess: printf("Path successfully added to the system path\n"); break;
        case kOpenKeyFailed: printf("Error: Couldn't open the system path key in the registry\n"); break;
        case kQueryCurrentValueFailed: printf("Error: Couldn't read the current value of the system path from the registry\n"); break;
        case kSetValueFailed: printf("Error: Couldn't set the new value of the system path in the registry\n"); break;
        case kQueryNewValueFailed: printf("Warning: Couldn't read the new value of the system path from the registry\n"); break;
        default: printf("Error: Unknown\n");
    }

	return 0;
}
