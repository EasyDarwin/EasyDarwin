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
    File:       FilePrefsSource.cpp

    Contains:   Implements object defined in FilePrefsSource.h.

    Written by: Chris LeCroy
*/

#include "FilePrefsSource.h"
#include <string.h>
#include <stdio.h>

#include <errno.h>

#include "MyAssert.h"
#include "OSMemory.h"
#include "ConfParser.h"

const int kMaxLineLen = 2048;
const int kMaxValLen = 1024;

class KeyValuePair
{
 public:
 
    char*   GetValue() { return fValue; }
    
 private:
    friend class FilePrefsSource;

    KeyValuePair(const char* inKey, const char* inValue, KeyValuePair* inNext);
    ~KeyValuePair();

    char* fKey;
    char* fValue;
    KeyValuePair* fNext;

     void ResetValue(const char* inValue);
};


KeyValuePair::KeyValuePair(const char* inKey, const char* inValue, KeyValuePair* inNext) :
    fKey(NULL),
    fValue(NULL),
    fNext(NULL)
{
    fKey = NEW char[::strlen(inKey)+1];
    ::strcpy(fKey, inKey);
    fValue = NEW char[::strlen(inValue)+1];
    ::strcpy(fValue, inValue);
    fNext = inNext;
}


KeyValuePair::~KeyValuePair()
{
    delete [] fKey;
    delete [] fValue;
}


void KeyValuePair::ResetValue(const char* inValue)
{
    delete [] fValue;
    fValue = NEW char[::strlen(inValue)+1];
    ::strcpy(fValue, inValue);
}


FilePrefsSource::FilePrefsSource( Bool16 allowDuplicates)
:   fKeyValueList(NULL),
    fNumKeys(0),
    fAllowDuplicates(allowDuplicates)
{
    
}

FilePrefsSource::~FilePrefsSource()
{
    while (fKeyValueList != NULL)
    {
        KeyValuePair* keyValue = fKeyValueList;
        fKeyValueList = fKeyValueList->fNext;
        delete keyValue;
    }

}

int FilePrefsSource::GetValue(const char* inKey, char* ioValue)
{
    return (this->FindValue(inKey, ioValue) != NULL);
}


int FilePrefsSource::GetValueByIndex(const char* inKey, UInt32 inIndex, char* ioValue)
{
    KeyValuePair* thePair = this->FindValue(inKey, ioValue, inIndex);
    
    if (thePair == NULL)
        return false;
    
    return true;
    
    /*
    char* valuePtr = thePair->fValue;

    //this function makes the assumption that fValue doesn't start with whitespace
    Assert(*valuePtr != '\t');
    Assert(*valuePtr != ' ');
    
    for (UInt32 count = 0; ((count < inIndex) && (valuePtr != '\0')); count++)
    {
        //go through all the "words" on this line (delimited by whitespace)
        //until we hit the one specified by inIndex

        //we aren't at the proper word yet, so skip...
        while ((*valuePtr != ' ') && (*valuePtr != '\t') && (*valuePtr != '\0'))
            valuePtr++;

        //skip over all the whitespace between words
        while ((*valuePtr == ' ') || (*valuePtr == '\t'))
            valuePtr++;
        
    }
    
    //We've exhausted the data on this line before getting to our pref,
    //so return an error.
    if (*valuePtr == '\0')
        return false;

    //if we are here, then valuePtr is pointing to the beginning of the right word
    while ((*valuePtr != ' ') && (*valuePtr != '\t') && (*valuePtr != '\0'))
        *ioValue++ = *valuePtr++;
    *ioValue = '\0';
    
    return true;
    */
}

char* FilePrefsSource::GetValueAtIndex(UInt32 inIndex)
{
    // Iterate through the queue until we have the right entry
    KeyValuePair* thePair = fKeyValueList;
    while ((thePair != NULL) && (inIndex-- > 0))
        thePair = thePair->fNext;
        
    if (thePair != NULL)
        return thePair->fValue;
    return NULL;
}

char* FilePrefsSource::GetKeyAtIndex(UInt32 inIndex)
{
    // Iterate through the queue until we have the right entry
    KeyValuePair* thePair = fKeyValueList;
    while ((thePair != NULL) && (inIndex-- > 0))
        thePair = thePair->fNext;
        
    if (thePair != NULL)
        return thePair->fKey;
    return NULL;
}

void FilePrefsSource::SetValue(const char* inKey, const char* inValue)
{
    KeyValuePair* keyValue = NULL;
    
    // If the key/value already exists update the value.
    // If duplicate keys are allowed, however, add a new entry regardless
    if ((!fAllowDuplicates) && ((keyValue = this->FindValue(inKey, NULL)) != NULL))
    {
        keyValue->ResetValue(inValue);
    }
    else
    {
        fKeyValueList  = NEW KeyValuePair(inKey, inValue, fKeyValueList);
        fNumKeys++;
    }
}



Bool16 FilePrefsSource::FilePrefsConfigSetter( const char* paramName, const char* paramValue[], void* userData )
{
/*
    static callback routine for ParseConfigFile
*/
    int     valueIndex = 0;
    
    FilePrefsSource *theFilePrefs = (FilePrefsSource*)userData;
    
    Assert( theFilePrefs );
    Assert(  paramName );
//  Assert(  paramValue[0] );
    
    
    // multiple values are passed in the paramValue array as distinct strs
    while ( paramValue[valueIndex] != NULL )
    {   
        //qtss_printf("Adding config setting  <key=\"%s\", value=\"%s\">\n", paramName,  paramValue[valueIndex] );
        theFilePrefs->SetValue(paramName, paramValue[valueIndex] );
        valueIndex++;
    }
    
    return false; // always succeeds
}


int FilePrefsSource::InitFromConfigFile(const char* configFilePath)
{
    /* 
        load config from specified file.  return non-zero
        in the event of significant error(s).
    
    */
    
    return ::ParseConfigFile( true, configFilePath, FilePrefsConfigSetter, this );
    
    /*
    int err = 0;
    char bufLine[kMaxLineLen];
    char key[kMaxValLen];
    char value[kMaxLineLen];
    
    FILE* fileDesc = ::fopen( configFilePath, "r");

    if (fileDesc == NULL)
    {
        // report some problem here...
        err = OSThread::GetErrno();
        
        Assert( err );
    }
    else
    {
    
        while (fgets(bufLine, sizeof(bufLine) - 1, fileDesc) != NULL)
        {
            if (bufLine[0] != '#' && bufLine[0] != '\0')
            {
                int i = 0;
                int n = 0;

                while ( bufLine[i] == ' ' || bufLine[i] == '\t')
                        { ++i;}

                n = 0;
                while ( bufLine[i] != ' ' &&
                         bufLine[i] != '\t' &&
                         bufLine[i] != '\n' &&
                         bufLine[i] != '\r' &&
                         bufLine[i] != '\0' &&
                         n < (kMaxLineLen - 1) )
                {
                    key[n++] = bufLine[i++];
                }
                key[n] = '\0';

                while (bufLine[i] == ' ' || bufLine[i] == '\t')
                {++i;}

                n = 0;
                while ((bufLine[i] != '\n') && (bufLine[i] != '\0') && 
                        (bufLine[i] != '\r') && (n < kMaxLineLen - 1))
                {
                          value[n++] = bufLine[i++];
                }
                value[n] = '\0';

                if (key[0] != '#' && key[0] != '\0' && value[0] != '\0')
                {
                    qtss_printf("Adding config setting  <key=\"%s\", value=\"%s\">\n", key, value);
                    this->SetValue(key, value);
                }
                else
                {
                        //assert(false);
                }
            }
        }


        int closeErr = ::fclose(fileDesc);
        Assert(closeErr == 0);
    }
    
    return err;
  */
    
    
}

void FilePrefsSource::DeleteValue(const char* inKey)
{
    KeyValuePair* keyValue = fKeyValueList;
    KeyValuePair* prevKeyValue = NULL;
    
    while (keyValue != NULL)
    {
        if (::strcmp(inKey, keyValue->fKey) == 0)
        {
            if (prevKeyValue != NULL)
            {
                prevKeyValue->fNext = keyValue->fNext;
                delete keyValue;
            }
            else
            {
                fKeyValueList = prevKeyValue;
            }
            
            return;
            
        }
        prevKeyValue = keyValue;
        keyValue = keyValue->fNext;
    }
}


void FilePrefsSource::WriteToConfigFile(const char* configFilePath)
{
    int err = 0;
    FILE* fileDesc = ::fopen( configFilePath,   "w");

        if (fileDesc != NULL)
        {
            err = ::fseek(fileDesc, 0, SEEK_END);
            Assert(err == 0);

            KeyValuePair* keyValue = fKeyValueList;

            while (keyValue != NULL)
            {
                    (void)qtss_fprintf(fileDesc, "%s   %s\n\n", keyValue->fKey, keyValue->fValue);

                    keyValue = keyValue->fNext;
            }

            err = ::fclose(fileDesc);
            Assert(err == 0);
        }
}


KeyValuePair* FilePrefsSource::FindValue(const char* inKey, char* ioValue, UInt32 index )
{
    KeyValuePair    *keyValue = fKeyValueList;
    UInt32          foundIndex = 0;

        if ( ioValue != NULL)
            ioValue[0] = '\0';
    
    while (keyValue != NULL)
    {
        if (::strcmp(inKey, keyValue->fKey) == 0)
        {
            if ( foundIndex == index )
            {
                if (ioValue != NULL)
                    ::strcpy(ioValue, keyValue->fValue);
                return keyValue;
            }
            foundIndex++;
        }
        keyValue = keyValue->fNext;
    }
    
    return NULL;
}
