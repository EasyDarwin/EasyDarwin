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
// A test program that streams GSM audio via RTP/RTCP
// main program

// NOTE: This program assumes the existence of a (currently nonexistent)
// function called "createNewGSMAudioSource()".

#include "liveMedia.hh"
#include "GroupsockHelper.hh"

#include "BasicUsageEnvironment.hh"

////////// Main program //////////

// To stream using "source-specific multicast" (SSM), uncomment the following:
//#define USE_SSM 1
#ifdef USE_SSM
Boolean const isSSM = True;
#else
Boolean const isSSM = False;
#endif

// To set up an internal RTSP server, uncomment the following:
//#define IMPLEMENT_RTSP_SERVER 1
// (Note that this RTSP server works for multicast only)

#ifdef IMPLEMENT_RTSP_SERVER
RTSPServer* rtspServer;
#endif

UsageEnvironment* env;

void afterPlaying(void* clientData); // forward

// A structure to hold the state of the current session.
// It is used in the "afterPlaying()" function to clean up the session.
struct sessionState_t {
  FramedSource* source;
  RTPSink* sink;
  RTCPInstance* rtcpInstance;
  Groupsock* rtpGroupsock;
  Groupsock* rtcpGroupsock;
} sessionState;

void play(); // forward

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Create 'groupsocks' for RTP and RTCP:
  char* destinationAddressStr
#ifdef USE_SSM
    = "232.255.42.42";
#else
    = "239.255.42.42";
  // Note: This is a multicast address.  If you wish to stream using
  // unicast instead, then replace this string with the unicast address
  // of the (single) destination.  (You may also need to make a similar
  // change to the receiver program.)
#endif
  const unsigned short rtpPortNum = 6666;
  const unsigned short rtcpPortNum = rtpPortNum+1;
  const unsigned char ttl = 1; // low, in case routers don't admin scope

  struct in_addr destinationAddress;
  destinationAddress.s_addr = our_inet_addr(destinationAddressStr);
  const Port rtpPort(rtpPortNum);
  const Port rtcpPort(rtcpPortNum);

  sessionState.rtpGroupsock
    = new Groupsock(*env, destinationAddress, rtpPort, ttl);
  sessionState.rtcpGroupsock
    = new Groupsock(*env, destinationAddress, rtcpPort, ttl);
#ifdef USE_SSM
  sessionState.rtpGroupsock->multicastSendOnly();
  sessionState.rtcpGroupsock->multicastSendOnly();
#endif

  // Create a 'GSM RTP' sink from the RTP 'groupsock':
  sessionState.sink
    = GSMAudioRTPSink::createNew(*env, sessionState.rtpGroupsock);

  // Create (and start) a 'RTCP instance' for this RTP sink:
  const unsigned estimatedSessionBandwidth = 160; // in kbps; for RTCP b/w share
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case
  sessionState.rtcpInstance
    = RTCPInstance::createNew(*env, sessionState.rtcpGroupsock,
			      estimatedSessionBandwidth, CNAME,
			      sessionState.sink, NULL /* we're a server */,
			      isSSM);
  // Note: This starts RTCP running automatically

#ifdef IMPLEMENT_RTSP_SERVER
  rtspServer = RTSPServer::createNew(*env, 8554);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "%s\n";
    exit(1);
  }
  ServerMediaSession* sms
    = ServerMediaSession::createNew(*env, "testStream", "GSM input",
		"Session streamed by \"testGSMStreamer\"", isSSM);
  sms->addSubsession(PassiveServerMediaSubsession::createNew(*sessionState.sink, sessionState.rtcpInstance));
  rtspServer->addServerMediaSession(sms);

  char* url = rtspServer->rtspURL(sms);
  *env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
#endif

  play();

  env->taskScheduler().doEventLoop(); // does not return
  return 0; // only to prevent compiler warning
}

void play() {
  // Open the input source:
  extern FramedSource* createNewGSMAudioSource(UsageEnvironment&);
  sessionState.source = createNewGSMAudioSource(*env);
  if (sessionState.source == NULL) {
    *env << "Failed to create GSM source\n";
    exit(1);
  }

  // Finally, start the streaming:
  *env << "Beginning streaming...\n";
  sessionState.sink->startPlaying(*sessionState.source, afterPlaying, NULL);
}


void afterPlaying(void* /*clientData*/) {
  *env << "...done streaming\n";

  sessionState.sink->stopPlaying();

  // End this loop by closing the media:
#ifdef IMPLEMENT_RTSP_SERVER
  Medium::close(rtspServer);
#endif
  Medium::close(sessionState.rtcpInstance);
  Medium::close(sessionState.sink);
  delete sessionState.rtpGroupsock;
  Medium::close(sessionState.source);
  delete sessionState.rtcpGroupsock;

  // And start another loop:
  play();
}
