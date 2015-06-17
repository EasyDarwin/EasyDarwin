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
    File:       NetInfoPrefsSource.cpp

    Contains:   Implements object defined in NetInfoPrefsSource.h.

    Written by: Denis Serenyi

    Change History (most recent first):

    

*/

#include "nilib2.h"

#include "NetInfoPrefsSource.h"

static char* gQTSSPropertiesPath = "/services/QuickTimeStreamingServer";

NetInfoPrefsSource::NetInfoPrefsSource()
{}

int NetInfoPrefsSource::GetValue(const char* inKey, char* ioValue)
{
    return this->GetValueByIndex(inKey, 0, ioValue);
}

int NetInfoPrefsSource::GetValueByIndex(const char* inKey, UInt32 inIndex, char* ioValue)
{
    ni_status status = NI_OK;
    ni_namelist nameList = {};
    void* localDomain = NULL;
    ni_id qtssDir = {}; 
    
    ioValue[0] = '\0';

    status = ni_open(NULL, ".", &localDomain);
    if (status != NI_OK)
        return false;
        
    if (status == NI_OK)
        status = ni_pathsearch(localDomain, &qtssDir, gQTSSPropertiesPath);
        
    if (status == NI_OK)
        status = ni_lookupprop(localDomain, &qtssDir, inKey, &nameList);

    if (status == NI_OK)
    {
        if (nameList.ni_namelist_len > inIndex)
            strcpy(ioValue, nameList.ni_namelist_val[inIndex]);
        else
            status = NI_BADID;
        ni_namelist_free(&nameList);
    }
    
    ni_free(localDomain);
    
    return (status == NI_OK);
}

void NetInfoPrefsSource::SetValue(char* inKey, char* inValue)
{
    this->SetValueByIndex(inKey, inValue, NI_INDEX_NULL);
}

void NetInfoPrefsSource::SetValueByIndex(char* inKey, char* inValue, UInt32 inIndex)
{
    void* localDomain = NULL;
    ni_status status = NI_OK;
    ni_namelist nameList = {};

    //ni_namelist_insert(&nameList, (char*)inValue, NI_INDEX_NULL);
    ni_namelist_insert(&nameList, inValue, inIndex);

    status = ni_open(NULL, ".", &localDomain);

    if (status == NI_OK)
    {
        //create the path if it doesn't already exist
        status = ni2_create(localDomain, gQTSSPropertiesPath);

        if (status == NI_OK)
            status = ni2_appendprop(localDomain, gQTSSPropertiesPath, inKey, nameList);
    }
    ni_namelist_free(&nameList);
    ni_free(localDomain);
}
