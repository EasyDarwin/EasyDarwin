/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2015, Live Networks, Inc.  All rights reserved
// A SIP client test program that opens a SIP URL argument,
// and extracts the data from each incoming RTP stream.

#include "playCommon.hh"
#include "SIPClient.hh"

static char* getLine(char* startOfLine) {
  // returns the start of the next line, or NULL if none
  for (char* ptr = startOfLine; *ptr != '\0'; ++ptr) {
    if (*ptr == '\r' || *ptr == '\n') {
      // We found the end of the line
      *ptr++ = '\0';
      if (*ptr == '\n') ++ptr;
      return ptr;
    }
  }
  
  return NULL;
}

SIPClient* ourSIPClient = NULL;
Medium* createClient(UsageEnvironment& env, char const* /*url*/, int verbosityLevel, char const* applicationName) {
  // First, trim any directory prefixes from "applicationName":
  char const* suffix = &applicationName[strlen(applicationName)];
  while (suffix != applicationName) {
    if (*suffix == '/' || *suffix == '\\') {
      applicationName = ++suffix;
      break;
    }
    --suffix;
  }

  extern unsigned char desiredAudioRTPPayloadFormat;
  extern char* mimeSubtype;
  return ourSIPClient = SIPClient::createNew(env, desiredAudioRTPPayloadFormat, mimeSubtype, verbosityLevel, applicationName);
}

// The followign function is implemented, but is not used for "playSIP":
void assignClient(Medium* /*client*/) {
}

void getOptions(RTSPClient::responseHandler* afterFunc) { 
  ourSIPClient->envir().setResultMsg("NOT SUPPORTED IN CLIENT");
  afterFunc(NULL, -1, strDup(ourSIPClient->envir().getResultMsg()));
}

void getSDPDescription(RTSPClient::responseHandler* afterFunc) {
  extern char* proxyServerName;
  if (proxyServerName != NULL) {
    // Tell the SIP client about the proxy:
    NetAddressList addresses(proxyServerName);
    if (addresses.numAddresses() == 0) {
      ourSIPClient->envir() << "Failed to find network address for \"" << proxyServerName << "\"\n";
    } else {
      NetAddress address = *(addresses.firstAddress());
      unsigned proxyServerAddress // later, allow for IPv6 #####
	= *(unsigned*)(address.data());
      extern unsigned short proxyServerPortNum;
      if (proxyServerPortNum == 0) proxyServerPortNum = 5060; // default

      ourSIPClient->setProxyServer(proxyServerAddress, proxyServerPortNum);
    }
  }

  extern unsigned short desiredPortNum;
  unsigned short clientStartPortNum = desiredPortNum;
  if (clientStartPortNum == 0) clientStartPortNum = 8000; // default
  ourSIPClient->setClientStartPortNum(clientStartPortNum);

  extern char const* streamURL;
  char const* username = ourAuthenticator == NULL ? NULL : ourAuthenticator->username();
  char const* password = ourAuthenticator == NULL ? NULL : ourAuthenticator->password();
  char* result;
  if (username != NULL && password != NULL) {
    result = ourSIPClient->inviteWithPassword(streamURL, username, password);
  } else {
    result = ourSIPClient->invite(streamURL);
  }

  int resultCode = result == NULL ? -1 : 0;
  afterFunc(NULL, resultCode, strDup(result));
}

void setupSubsession(MediaSubsession* subsession, Boolean /*streamUsingTCP*/, Boolean /*forceMulticastOnUnspecified*/,RTSPClient::responseHandler* afterFunc) {
  subsession->setSessionId("mumble"); // anything that's non-NULL will work

  ////////// BEGIN hack code that should really be implemented in SIPClient //////////
  // Parse the "Transport:" header parameters:
  // We do not send audio, but we need port for RTCP
  char* serverAddressStr;
  portNumBits serverPortNum;
  unsigned char rtpChannelId, rtcpChannelId;

  rtpChannelId = rtcpChannelId = 0xff;
  serverPortNum = 0;
  serverAddressStr = NULL;

  char* sdp = strDup(ourSIPClient->getInviteSdpReply());

  char* lineStart;
  char* nextLineStart = sdp;
  while (1) {
    lineStart = nextLineStart;
    if (lineStart == NULL) {
      break;
    }
    nextLineStart = getLine(lineStart);

    char* toTagStr = strDupSize(lineStart);

    if (sscanf(lineStart, "m=audio %[^/\r\n]", toTagStr) == 1) {
      sscanf(toTagStr, "%hu", &serverPortNum);
    } else if (sscanf(lineStart, "c=IN IP4 %[^/\r\n]", toTagStr) == 1) {
      serverAddressStr = strDup(toTagStr);
    }
    delete[] toTagStr;
  }

  if(sdp != NULL) {
    delete[] sdp;
  }

  delete[] subsession->connectionEndpointName();
  subsession->connectionEndpointName() = serverAddressStr;
  subsession->serverPortNum = serverPortNum;
  subsession->rtpChannelId = rtpChannelId;
  subsession->rtcpChannelId = rtcpChannelId;

  // Set the RTP and RTCP sockets' destination address and port from the information in the SETUP response (if present):
  netAddressBits destAddress = subsession->connectionEndpointAddress();
  if (destAddress != 0) {
    subsession->setDestinations(destAddress);
  }
  ////////// END hack code that should really be implemented in SIPClient //////////

  afterFunc(NULL, 0, NULL);
}

void startPlayingSession(MediaSession* /*session*/, double /*start*/, double /*end*/, float /*scale*/, RTSPClient::responseHandler* afterFunc) {
  if (ourSIPClient->sendACK()) {
    //##### This isn't quite right, because we should really be allowing
    //##### for the possibility of this ACK getting lost, by retransmitting
    //##### it *each time* we get a 2xx response from the server.
    afterFunc(NULL, 0, NULL);
  } else {
    afterFunc(NULL, -1, strDup(ourSIPClient->envir().getResultMsg()));
  }
}
void startPlayingSession(MediaSession* /*session*/, const char* /*start*/, const char* /*end*/, float /*scale*/, RTSPClient::responseHandler* afterFunc) {
	startPlayingSession(NULL,(double)0,(double)0,0,afterFunc);
}

void tearDownSession(MediaSession* /*session*/, RTSPClient::responseHandler* afterFunc) {
  if (ourSIPClient == NULL || ourSIPClient->sendBYE()) {
    afterFunc(NULL, 0, NULL);
  } else {
    afterFunc(NULL, -1, strDup(ourSIPClient->envir().getResultMsg()));
  }
}

void setUserAgentString(char const* userAgentString) {
  ourSIPClient->setUserAgentString(userAgentString);
}

Boolean allowProxyServers = True;
Boolean controlConnectionUsesTCP = False;
Boolean supportCodecSelection = True;
char const* clientProtocolName = "SIP";
