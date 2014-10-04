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
/* 23*/ "qtssMsgRTPPortMustBeEven",
/* 24*/ "qtssMsgRTCPPortMustBeOneBigger",
/* 25*/ "qtssMsgOutOfPorts",
/* 26*/ "qtssMsgNoModuleForRequest",
/* 27*/ "qtssMsgAltDestNotAllowed",
/* 28*/ "qtssMsgCantSetupMulticast",
/* 29*/ "qtssListenPortInUse",
/* 30*/ "qtssListenPortAccessDenied",
/* 31*/ "qtssListenPortError",
/* 32*/ "qtssMsgBadBase64",
/* 33*/ "qtssMsgSomePortsFailed",
/* 34*/ "qtssMsgNoPortsSucceeded",
/* 35*/ "qtssMsgCannotCreatePidFile",
/* 36*/ "qtssMsgCannotSetRunUser",
/* 37*/ "qtssMsgCannotSetRunGroup",
/* 38*/ "qtssMsgNoSesIDOnDescribe",
/* 39*/ "qtssServerPrefMissing",
/* 40*/ "qtssServerPrefWrongType",
/* 41*/ "qtssMsgCantWriteFile",
/* 42*/ "qtssMsgSockBufSizesTooLarge",
/* 43*/ "qtssMsgBadFormat",

 // module specific messages follow (these are dynamically numbered)
 
/* 44*/ "QTSSvrControlModuleCantRegisterMachPort",
/* 45*/ "QTSSvrControlModuleServerControlFatalErr",
/* 46*/ "QTSSReflectorModuleCantBindReflectorSocket",
/* 47*/ "QTSSReflectorModuleCantJoinMulticastGroup",
/* 48*/ "QTSSFileModuleSeekToNonExistentTime",
/* 49*/ "QTSSFileModuleNoSDPFileFound",
/* 50*/ "QTSSFileModuleBadQTFile",
/* 51*/ "QTSSFileModuleFileIsNotHinted",
/* 52*/ "QTSSFileModuleExpectedDigitFilename",
/* 53*/ "QTSSFileModuleTrackDoesntExist",
/* 54*/ "QTSSReflectorModuleExpectedDigitFilename",
/* 55*/ "QTSSSpamDefenseModuleTooManyConnections",
/* 56*/ "QTSSReflectorModuleBadTrackID",
/* 57*/ "QTSSAccessModuleBadAccessFileName",
/* 58*/ "QTSSReflectorModuleNoRelaySources",
/* 59*/ "QTSSReflectorModuleNoRelayDests",
/* 60*/ "QTSSReflectorModuleNoRelayStreams",
/* 61*/ "QTSSReflectorModuleNoRelayConfig",
/* 62*/ "QTSSReflectorModuleDuplicateBroadcastStream",
/* 63*/ "QTSSAccessModuleUsersFileNotFound",
/* 64*/ "QTSSAccessModuleGroupsFileNotFound",
/* 65*/ "QTSSAccessModuleBadUsersFile",
/* 66*/ "QTSSAccessModuleBadGroupsFile",
/* 67*/ "QTSSReflectorModuleAnnounceRequiresSDPSuffix",
/* 68*/ "QTSSReflectorModuleAnnounceDisabled",
/* 69*/ "QTSSReflectorModuleSDPPortMinimumPort",
/* 70*/ "QTSSReflectorModuleSDPPortMaximumPort",
/* 71*/ "QTSSReflectorModuleStaticPortsConflict",
/* 72*/ "QTSSReflectorModuleStaticPortPrefsBadRange",
/* 73*/ "QTSSRelayModulePrefParseError"
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
/* 23*/ "Reported client RTP port is not an even number",
/* 24*/ "Reported client RTCP port is not one greater than RTP port",
/* 25*/ "Streaming Server couldn't find any available UDP ports",
/* 26*/ "There is no QTSS API module installed to process this request.",
/* 27*/ "Not allowed to specify an alternate destination address",
/* 28*/ "Can't setup multicast.",
/* 29*/ "Another process is already using the following RTSP port: %s",
/* 30*/ "You must be root to use the following RTSP port: %s",
/* 31*/ "An error occurred when attempting to listen on the following RTSP port: %s",
/* 32*/ "The base64 you just sent to the server is corrupt!",
/* 33*/ "Streaming Server failed to listen on all requested RTSP port(s).",
/* 34*/ "Streaming Server is not listening for RTSP on any ports.",
/* 35*/ "Error creating pid file %s: %s",
/* 36*/ "Error switching to user %s: %s",
/* 37*/ "Error switching to group %s: %s",
/* 38*/ "A DESCRIBE request cannot contain the Session header",
/* 39*/ "The following pref, %s, wasn't found. Using a default value of: %s",
/* 40*/ "The following pref, %s, has the wrong type. Using a default value of: %s",
/* 41*/ "Couldn't re-write server prefs file",
/* 42*/ "Couldn't set desired UDP receive socket buffer size. Using size of %s K",
/* 43*/ "The request is incorrectly formatted.",

// module specific messages follow (these are dynamically numbered)

/* 44*/ "Fatal error: Can't register Mach Ports.",
/* 45*/ "A fatal error occcured while starting up server control module",
/* 46*/ "Can't bind reflector sockets",
/* 47*/ "Reflector sockets couldn't join multicast",
/* 48*/ "Couldn't seek to specified time.",
/* 49*/ "No SDP file found for the following URL: %s",
/* 50*/ "The requested file is not a movie file.",
/* 51*/ "Requested movie hasn't been hinted.",
/* 52*/ "Expected a digit at the end of the following URL: %s",
/* 53*/ "Specified trackID doesn't exist in the movie",
/* 54*/ "Expected a digit at the end of the following URL: %s",
/* 55*/ "Too many connections from your IP address!",
/* 56*/ "TrackID doesn't match any trackID for this ReflectorSession",
/* 57*/ "Invalid config value for qtaccessfilename: name contains %s. Now using default name:%s",
/* 58*/ "The relay configuration file at: %s has no relay_source lines",
/* 59*/ "Could not find any relay_destination lines for one of the relay_source lines of the relay configuration file",
/* 60*/ "Could not find any input stream information for one of the relay_source lines of the relay configuration file",
/* 61*/ "Found an empty relay configuration file at %s",
/* 62*/ "A broadcast stream is already setup for this URL",
/* 63*/ "No users file found at %s.",
/* 64*/ "No groups file found at %s.",
/* 65*/ "Unable to read the users file at %s. It may be corrupted.",
/* 66*/ "Unable to read the groups file at %s. It may be corrupted.",
/* 67*/ "The Announced file does not end with .sdp",
/* 68*/ "The Announce feature is disabled. Your request is denied.",
/* 69*/ "The SDP file's static port %s is less than the QTSSReflectorModule's minimum_static_sdp_port preference.",
/* 70*/ "The SDP file's static port %s is greater than the QTSSReflectorModule's maximum_static_sdp_port preference.",
/* 71*/ "The QTSSReflectorModule's minimum_static_sdp_port and maximum_static_sdp_port preferences conflict with the client and dynamic broadcast port range= %s to %s.",
/* 72*/ "The QTSSReflectorModule's minimum_static_sdp_port and maximum_static_sdp_port preferences define an invalid range (min=%s > max=%s).",
/* 73*/ "The QTSSRelayModule encountered an error while parsing the relay config file. No relays setup in relayconfig.xml."
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
