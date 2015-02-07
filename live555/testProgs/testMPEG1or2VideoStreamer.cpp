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
// A test program that reads a MPEG-1 or 2 Video Elementary Stream file,
// and streams it using RTP
// main program

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"

// Uncomment the following if the input file is a MPEG Program Stream
// rather than a MPEG Video Elementary Stream
//#define SOURCE_IS_PROGRAM_STREAM 1

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

// To stream *only* MPEG "I" frames (e.g., to reduce network bandwidth),
// change the following "False" to "True":
Boolean iFramesOnly = False;

UsageEnvironment* env;
char const* inputFileName = "test.mpg";
#ifdef SOURCE_IS_PROGRAM_STREAM
MPEG1or2Demux* mpegDemux;
#endif
MediaSource* videoSource;
RTPSink* videoSink;

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
  const unsigned short rtpPortNum = 8888;
  const unsigned short rtcpPortNum = rtpPortNum+1;
  const unsigned char ttl = 7; // low, in case routers don't admin scope

  struct in_addr destinationAddress;
  destinationAddress.s_addr = our_inet_addr(destinationAddressStr);
  const Port rtpPort(rtpPortNum);
  const Port rtcpPort(rtcpPortNum);

  Groupsock rtpGroupsock(*env, destinationAddress, rtpPort, ttl);
  Groupsock rtcpGroupsock(*env, destinationAddress, rtcpPort, ttl);
#ifdef USE_SSM
  rtpGroupsock.multicastSendOnly();
  rtcpGroupsock.multicastSendOnly();
#endif

  // Create a 'MPEG Video RTP' sink from the RTP 'groupsock':
  videoSink = MPEG1or2VideoRTPSink::createNew(*env, &rtpGroupsock);

  // Create (and start) a 'RTCP instance' for this RTP sink:
  const unsigned estimatedSessionBandwidth = 4500; // in kbps; for RTCP b/w share
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case
#ifdef IMPLEMENT_RTSP_SERVER
  RTCPInstance* rtcp =
#endif
    RTCPInstance::createNew(*env, &rtcpGroupsock,
			      estimatedSessionBandwidth, CNAME,
			      videoSink, NULL /* we're a server */, isSSM);
  // Note: This starts RTCP running automatically

#ifdef IMPLEMENT_RTSP_SERVER
  RTSPServer* rtspServer = RTSPServer::createNew(*env);
  // Note that this (attempts to) start a server on the default RTSP server
  // port: 554.  To use a different port number, add it as an extra
  // (optional) parameter to the "RTSPServer::createNew()" call above.
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }
  ServerMediaSession* sms
    = ServerMediaSession::createNew(*env, "testStream", inputFileName,
		   "Session streamed by \"testMPEG1or2VideoStreamer\"",
					   isSSM);
  sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, rtcp));
  rtspServer->addServerMediaSession(sms);

  char* url = rtspServer->rtspURL(sms);
  *env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
#endif

  // Finally, start the streaming:
  *env << "Beginning streaming...\n";
  play();

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void afterPlaying(void* /*clientData*/) {
  *env << "...done reading from file\n";

  videoSink->stopPlaying();
  Medium::close(videoSource);
#ifdef SOURCE_IS_PROGRAM_STREAM
  Medium::close(mpegDemux);
#endif
  // Note that this also closes the input file that this source read from.

  play();
}

void play() {
  // Open the input file as a 'byte-stream file source':
  ByteStreamFileSource* fileSource
    = ByteStreamFileSource::createNew(*env, inputFileName);
  if (fileSource == NULL) {
    *env << "Unable to open file \"" << inputFileName
	 << "\" as a byte-stream file source\n";
    exit(1);
  }

  FramedSource* videoES;
#ifdef SOURCE_IS_PROGRAM_STREAM
  // We must demultiplex a Video Elementary Stream from the input source:
  mpegDemux = MPEG1or2Demux::createNew(*env, fileSource);
  videoES = mpegDemux->newVideoStream();
#else
  // The input source is assumed to already be a Video Elementary Stream:
  videoES = fileSource;
#endif

  // Create a framer for the Video Elementary Stream:
  videoSource
    = MPEG1or2VideoStreamFramer::createNew(*env, videoES, iFramesOnly);

  // Finally, start playing:
  *env << "Beginning to read from file...\n";
  videoSink->startPlaying(*videoSource, afterPlaying, videoSink);
}
