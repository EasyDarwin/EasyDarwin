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
	Website: http://www.EasyDarwin.org
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
/* 1 */ "qtssMsgNoURLInRequest",
/* 2 */ "qtssMsgBadRTSPMethod",
/* 3 */ "qtssMsgNoRTSPVersion",
/* 4 */ "qtssMsgNoRTSPInURL",
/* 5 */ "qtssMsgURLTooLong",
/* 6 */ "qtssMsgURLInBadFormat",
/* 7 */ "qtssMsgNoColonAfterHeader",
/* 8 */ "qtssMsgNoEOLAfterHeader",
/* 9 */ "qtssMsgRequestTooLong",
/* 10*/ "qtssMsgNoModuleFolder",
/* 11*/ "qtssMsgCouldntListen",
/* 12*/ "qtssMsgInitFailed",
/* 13*/ "qtssMsgNotConfiguredForIP",
/* 14*/ "qtssMsgDefaultRTSPAddrUnavail",
/* 15*/ "qtssMsgBadModule",
/* 16*/ "qtssMsgRegFailed",
/* 17*/ "qtssMsgRefusingConnections",
/* 18*/ "qtssMsgTooManyClients",
/* 19*/ "qtssMsgTooMuchThruput",
/* 20*/ "qtssMsgNoSessionID",
/* 21*/ "qtssMsgFileNameTooLong",
/* 22*/ "qtssMsgNoClientPortInTransport",


/* 23*/ "qtssMsgOutOfPorts",
/* 24*/ "qtssMsgNoModuleForRequest",
/* 25*/ "qtssMsgAltDestNotAllowed",
/* 26*/ "qtssMsgCantSetupMulticast",
/* 27*/ "qtssListenPortInUse",
/* 28*/ "qtssListenPortAccessDenied",
/* 29*/ "qtssListenPortError",
/* 30*/ "qtssMsgBadBase64",
/* 31*/ "qtssMsgSomePortsFailed",
/* 32*/ "qtssMsgNoPortsSucceeded",
/* 33*/ "qtssMsgCannotCreatePidFile",
/* 34*/ "qtssMsgCannotSetRunUser",
/* 35*/ "qtssMsgCannotSetRunGroup",
/* 36*/ "qtssMsgNoSesIDOnDescribe",
/* 37*/ "qtssServerPrefMissing",
/* 38*/ "qtssServerPrefWrongType",
/* 39*/ "qtssMsgCantWriteFile",
/* 40*/ "qtssMsgSockBufSizesTooLarge",
/* 41*/ "qtssMsgBadFormat",
};

// see QTSS.h (QTSS_TextMessagesObject) for list of enums to map these strings

char*       QTSSMessages::sMessages[] =
{
/* 0 */ "%s%s",
/* 1 */ "There was no URL contained in the following request: %s",
/* 2 */ "The following RTSP method: %s, was not understood by the server",
/* 3 */ "There is no RTSP version in the following request: %s",
/* 4 */ "Server expected 'rtsp://' and instead received: %s",
/* 5 */ "The following URL is too long to be processed by the server: %s",
/* 6 */ "The following URL is not in proper URL format: %s",
/* 7 */ "There was no colon after a header in the following request: %s",
/* 8 */ "There was no EOL after a header in the following request: %s",
/* 9 */ "That request is too long to be processed by the server.",
/* 10*/ "No module folder exists.",
/* 11*/ "Streaming Server couldn't listen on a specified RTSP port. Quitting.",
/* 12*/ "The module %s failed to Initialize.",
/* 13*/ "This machine is currently not configured for IP.",
/* 14*/ "The specified RTSP listening IP address doesn't exist.",
/* 15*/ "The module %s is not a compatible QTSS API module.",
/* 16*/ "The module %s failed to Register.",
/* 17*/ "Streaming Server is currently refusing new connections",
/* 18*/ "Too many clients connected",
/* 19*/ "Too much bandwidth being served",
/* 20*/ "No active RTP session for that RTSP session ID",
/* 21*/ "Specified file name is too long to be handled by the server.",
/* 22*/ "No client port pair found in transport header",


/* 23*/ "Streaming Server couldn't find any available UDP ports",
/* 24*/ "There is no QTSS API module installed to process this request.",
/* 25*/ "Not allowed to specify an alternate destination address",
/* 26*/ "Can't setup multicast.",
/* 27*/ "Another process is already using the following RTSP port: %s",
/* 28*/ "You must be root to use the following RTSP port: %s",
/* 29*/ "An error occurred when attempting to listen on the following RTSP port: %s",
/* 30*/ "The base64 you just sent to the server is corrupt!",
/* 31*/ "Streaming Server failed to listen on all requested RTSP port(s).",
/* 32*/ "Streaming Server is not listening for RTSP on any ports.",
/* 33*/ "Error creating pid file %s: %s",
/* 34*/ "Error switching to user %s: %s",
/* 35*/ "Error switching to group %s: %s",
/* 36*/ "A DESCRIBE request cannot contain the Session header",
/* 37*/ "The following pref, %s, wasn't found. Using a default value of: %s",
/* 38*/ "The following pref, %s, has the wrong type. Using a default value of: %s",
/* 39*/ "Couldn't re-write server prefs file",
/* 40*/ "Couldn't set desired UDP receive socket buffer size. Using size of %s K",
/* 41*/ "The request is incorrectly formatted."
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
