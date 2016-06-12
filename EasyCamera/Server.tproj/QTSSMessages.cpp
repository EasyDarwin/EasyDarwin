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
    File:       QTSSMessages.cpp

    Contains:   Implementation of object defined in .h
    
    
    
*/

#include "QTSSMessages.h"
#include "OSMemory.h"

// see QTSS.h (QTSS_TextMessagesObject) for list of enums to map these strings

char*       QTSSMessages::sMessagesKeyStrings[] =
{ /* index */
/* 0 */ "qtssMsgNoMessage",
/* 1*/ "qtssMsgInitFailed",
/* 2*/ "qtssServerPrefMissing",
/* 3*/ "qtssServerPrefWrongType",
/* 4*/ "qtssMsgCantWriteFile"
};

// see QTSS.h (QTSS_TextMessagesObject) for list of enums to map these strings

char*       QTSSMessages::sMessages[] =
{
/* 0 */ "%s%s",
/* 1 */ "The module %s failed to Initialize.",
/* 2 */ "The following pref, %s, wasn't found. Using a default value of: %s",
/* 3 */ "The following pref, %s, has the wrong type. Using a default value of: %s",
/* 4 */ "Couldn't re-write server prefs file"
};

// need to maintain numbers to update kNumMessages in QTSSMessages.h.

void QTSSMessages::Initialize()
{
    QTSSDictionaryMap* theMap = QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kTextMessagesDictIndex);
    Assert(theMap != NULL);
    
    for (UInt32 x = 0; x < qtssMsgNumParams; x++)
        theMap->SetAttribute(x, sMessagesKeyStrings[x], NULL, qtssAttrDataTypeCharArray, qtssAttrModeRead | qtssAttrModePreempSafe);
}

QTSSMessages::QTSSMessages(PrefsSource* inMessages)
: QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kTextMessagesDictIndex)),
  numAttrs(GetDictionaryMap()->GetNumAttrs())
{
    static const UInt32 kMaxMessageSize = 2048;
    char theMessage[kMaxMessageSize];
    
    // Use the names of the attributes in the attribute map as the key values for
    // finding preferences in the config file.
    attrBuffer = NEW char* [numAttrs];
    ::memset(attrBuffer, 0, sizeof(char*) * numAttrs);
    for (UInt32 x = 0; x < numAttrs; x++)
    {
        theMessage[0] = '\0';
        (void)inMessages->GetValue(this->GetDictionaryMap()->GetAttrName(x), &theMessage[0]);
        
        if (theMessage[0] == '\0')
        {
            // If a message doesn't exist in the file, check to see if this attribute
            // name matches one of the compiled-in strings. If so, use that instead
            for (UInt32 y = 0; y < kNumMessages; y++)
            {
                if (::strcmp(this->GetDictionaryMap()->GetAttrName(x), sMessagesKeyStrings[y]) == 0)
                {
                    ::strcpy(theMessage, sMessages[y]);
                    break;
                }
            }
            // If we didn't find a match, just copy in this last-resort message
            if (theMessage[0] == '\0')
                ::strcpy(theMessage, "No Message");
        }
        
        // Add this preference into the dictionary.
        
        // If there is a message, allocate some new memory for 
        // the new attribute, and copy the data into the newly allocated buffer
        if (theMessage[0] != '\0')
        {
            attrBuffer[x] = NEW char[::strlen(theMessage) + 2];
            ::strcpy(attrBuffer[x], theMessage);
            this->SetVal(this->GetDictionaryMap()->GetAttrID(x),
                         attrBuffer[x], ::strlen(attrBuffer[x]));
        }
    }
}
