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
// A test program that streams a MP3 file via RTP/RTCP
// main program

#include "liveMedia.hh"
#include "GroupsockHelper.hh"

#include "BasicUsageEnvironment.hh"

// To stream using 'ADUs' rather than raw MP3 frames, uncomment the following:
//#define STREAM_USING_ADUS 1
// To also reorder ADUs before streaming, uncomment the following:
//#define INTERLEAVE_ADUS 1
// (For more information about ADUs and interleaving,
//  see <http://www.live555.com/rtp-mp3/>)

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

// A structure to hold the state of the current session.
// It is used in the "afterPlaying()" function to clean up the session.
struct sessionState_t {
  FramedSource* source;
  RTPSink* sink;
  RTCPInstance* rtcpInstance;
  Groupsock* rtpGroupsock;
  Groupsock* rtcpGroupsock;
} sessionState;

char const* inputFileName = "test.mp3";

void play(); // forward

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Create 'groupsocks' for RTP and RTCP:
  char const* destinationAddressStr
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

  // Create a 'MP3 RTP' sink from the RTP 'groupsock':
#ifdef STREAM_USING_ADUS
  unsigned char rtpPayloadFormat = 96; // A dynamic payload format code
  sessionState.sink
    = MP3ADURTPSink::createNew(*env, sessionState.rtpGroupsock,
			       rtpPayloadFormat);
#else
  sessionState.sink
    = MPEG1or2AudioRTPSink::createNew(*env, sessionState.rtpGroupsock);
#endif

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
  rtspServer = RTSPServer::createNew(*env);
  // Note that this (attempts to) start a server on the default RTSP server
  // port: 554.  To use a different port number, add it as an extra
  // (optional) parameter to the "RTSPServer::createNew()" call above.
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }
  ServerMediaSession* sms
    = ServerMediaSession::createNew(*env, "testStream", inputFileName,
		"Session streamed by \"testMP3Streamer\"", isSSM);
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

void afterPlaying(void* clientData); // forward

void play() {
  // Open the file as a 'MP3 file source':
  sessionState.source = MP3FileSource::createNew(*env, inputFileName);
  if (sessionState.source == NULL) {
    *env << "Unable to open file \"" << inputFileName
	 << "\" as a MP3 file source\n";
    exit(1);
  }

#ifdef STREAM_USING_ADUS
  // Add a filter that converts the source MP3s to ADUs:
  sessionState.source
    = ADUFromMP3Source::createNew(*env, sessionState.source);
  if (sessionState.source == NULL) {
    *env << "Unable to create a MP3->ADU filter for the source\n";
    exit(1);
  }

#ifdef INTERLEAVE_ADUS
  // Add another filter that interleaves the ADUs before packetizing them:
  unsigned char interleaveCycle[] = {0,2,1,3}; // or choose your own order...
  unsigned const interleaveCycleSize
    = (sizeof interleaveCycle)/(sizeof (unsigned char));
  Interleaving interleaving(interleaveCycleSize, interleaveCycle);
  sessionState.source
    = MP3ADUinterleaver::createNew(*env, interleaving, sessionState.source);
  if (sessionState.source == NULL) {
    *env << "Unable to create an ADU interleaving filter for the source\n";
    exit(1);
  }
#endif
#endif

  // Finally, start the streaming:
  *env << "Beginning streaming...\n";
  sessionState.sink->startPlaying(*sessionState.source, afterPlaying, NULL);
}


void afterPlaying(void* /*clientData*/) {
  *env << "...done streaming\n";

  sessionState.sink->stopPlaying();

  // End this loop by closing the current source:
  Medium::close(sessionState.source);

  // And start another loop:
  play();
}
