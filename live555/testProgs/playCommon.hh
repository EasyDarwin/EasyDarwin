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
// A common framework, used for the "openRTSP" and "playSIP" applications
// Interfaces

#include "liveMedia.hh"

extern Medium* createClient(UsageEnvironment& env, char const* URL, int verbosityLevel, char const* applicationName);
extern void assignClient(Medium* client);
extern RTSPClient* ourRTSPClient;
extern SIPClient* ourSIPClient;

extern void getOptions(RTSPClient::responseHandler* afterFunc);

extern void getSDPDescription(RTSPClient::responseHandler* afterFunc);

extern void setupSubsession(MediaSubsession* subsession, Boolean streamUsingTCP, Boolean forceMulticastOnUnspecified, RTSPClient::responseHandler* afterFunc);

extern void startPlayingSession(MediaSession* session, double start, double end, float scale, RTSPClient::responseHandler* afterFunc);

extern void startPlayingSession(MediaSession* session, char const* absStartTime, char const* absEndTime, float scale, RTSPClient::responseHandler* afterFunc);
  // For playing by 'absolute' time (using strings of the form "YYYYMMDDTHHMMSSZ" or "YYYYMMDDTHHMMSS.<frac>Z"

extern void tearDownSession(MediaSession* session, RTSPClient::responseHandler* afterFunc);

extern void setUserAgentString(char const* userAgentString);

extern Authenticator* ourAuthenticator;
extern Boolean allowProxyServers;
extern Boolean controlConnectionUsesTCP;
extern Boolean supportCodecSelection;
extern char const* clientProtocolName;
extern unsigned statusCode;
