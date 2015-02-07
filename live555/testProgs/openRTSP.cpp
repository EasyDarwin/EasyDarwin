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
// A RTSP client application that opens a RTSP URL argument,
// and extracts and records the data from each incoming RTP stream.
//
// NOTE: If you want to develop your own RTSP client application (or embed RTSP client functionality into your own application),
// then we don't recommend using this code as a model, because it is too complex (with many options).
// Instead, we recommend using the "testRTSPClient" application code as a model.

#include "playCommon.hh"

RTSPClient* ourRTSPClient = NULL;
Medium* createClient(UsageEnvironment& env, char const* url, int verbosityLevel, char const* applicationName) {
  extern portNumBits tunnelOverHTTPPortNum;
  return ourRTSPClient = RTSPClient::createNew(env, url, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

void assignClient(Medium* client) {
  ourRTSPClient = (RTSPClient*)client;
}

void getOptions(RTSPClient::responseHandler* afterFunc) { 
  ourRTSPClient->sendOptionsCommand(afterFunc, ourAuthenticator);
}

void getSDPDescription(RTSPClient::responseHandler* afterFunc) {
  ourRTSPClient->sendDescribeCommand(afterFunc, ourAuthenticator);
}

void setupSubsession(MediaSubsession* subsession, Boolean streamUsingTCP, Boolean forceMulticastOnUnspecified, RTSPClient::responseHandler* afterFunc) {
  
  ourRTSPClient->sendSetupCommand(*subsession, afterFunc, False, streamUsingTCP, forceMulticastOnUnspecified, ourAuthenticator);
}

void startPlayingSession(MediaSession* session, double start, double end, float scale, RTSPClient::responseHandler* afterFunc) {
  ourRTSPClient->sendPlayCommand(*session, afterFunc, start, end, scale, ourAuthenticator);
}

void startPlayingSession(MediaSession* session, char const* absStartTime, char const* absEndTime, float scale, RTSPClient::responseHandler* afterFunc) {
  ourRTSPClient->sendPlayCommand(*session, afterFunc, absStartTime, absEndTime, scale, ourAuthenticator);
}

void tearDownSession(MediaSession* session, RTSPClient::responseHandler* afterFunc) {
  ourRTSPClient->sendTeardownCommand(*session, afterFunc, ourAuthenticator);
}

void setUserAgentString(char const* userAgentString) {
  ourRTSPClient->setUserAgentString(userAgentString);
}

Boolean allowProxyServers = False;
Boolean controlConnectionUsesTCP = True;
Boolean supportCodecSelection = False;
char const* clientProtocolName = "RTSP";
