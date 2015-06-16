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
// A test program that reads a MPEG-1 or 2 Program Stream file,
// splits it into Audio and Video Elementary Streams,
// and streams both using RTP
// main program

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"

UsageEnvironment* env;
char const* inputFileName = "test.mpg";
MPEG1or2Demux* mpegDemux;
FramedSource* audioSource;
FramedSource* videoSource;
RTPSink* audioSink;
RTPSink* videoSink;

void play(); // forward

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
  const unsigned short rtpPortNumAudio = 6666;
  const unsigned short rtcpPortNumAudio = rtpPortNumAudio+1;
  const unsigned short rtpPortNumVideo = 8888;
  const unsigned short rtcpPortNumVideo = rtpPortNumVideo+1;
  const unsigned char ttl = 7; // low, in case routers don't admin scope

  struct in_addr destinationAddress;
  destinationAddress.s_addr = our_inet_addr(destinationAddressStr);
  const Port rtpPortAudio(rtpPortNumAudio);
  const Port rtcpPortAudio(rtcpPortNumAudio);
  const Port rtpPortVideo(rtpPortNumVideo);
  const Port rtcpPortVideo(rtcpPortNumVideo);

  Groupsock rtpGroupsockAudio(*env, destinationAddress, rtpPortAudio, ttl);
  Groupsock rtcpGroupsockAudio(*env, destinationAddress, rtcpPortAudio, ttl);
  Groupsock rtpGroupsockVideo(*env, destinationAddress, rtpPortVideo, ttl);
  Groupsock rtcpGroupsockVideo(*env, destinationAddress, rtcpPortVideo, ttl);
#ifdef USE_SSM
  rtpGroupsockAudio.multicastSendOnly();
  rtcpGroupsockAudio.multicastSendOnly();
  rtpGroupsockVideo.multicastSendOnly();
  rtcpGroupsockVideo.multicastSendOnly();
#endif

  // Create a 'MPEG Audio RTP' sink from the RTP 'groupsock':
  audioSink = MPEG1or2AudioRTPSink::createNew(*env, &rtpGroupsockAudio);

  // Create (and start) a 'RTCP instance' for this RTP sink:
  const unsigned estimatedSessionBandwidthAudio = 160; // in kbps; for RTCP b/w share
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case
#ifdef IMPLEMENT_RTSP_SERVER
  RTCPInstance* audioRTCP =
#endif
    RTCPInstance::createNew(*env, &rtcpGroupsockAudio,
			    estimatedSessionBandwidthAudio, CNAME,
			    audioSink, NULL /* we're a server */, isSSM);
  // Note: This starts RTCP running automatically

  // Create a 'MPEG Video RTP' sink from the RTP 'groupsock':
  videoSink = MPEG1or2VideoRTPSink::createNew(*env, &rtpGroupsockVideo);

  // Create (and start) a 'RTCP instance' for this RTP sink:
  const unsigned estimatedSessionBandwidthVideo = 4500; // in kbps; for RTCP b/w share
#ifdef IMPLEMENT_RTSP_SERVER
  RTCPInstance* videoRTCP =
#endif
    RTCPInstance::createNew(*env, &rtcpGroupsockVideo,
			      estimatedSessionBandwidthVideo, CNAME,
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
		   "Session streamed by \"testMPEG1or2AudioVideoStreamer\"",
					   isSSM);
  sms->addSubsession(PassiveServerMediaSubsession::createNew(*audioSink, audioRTCP));
  sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, videoRTCP));
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

void afterPlaying(void* clientData) {
  // One of the sinks has ended playing.
  // Check whether any of the sources have a pending read.  If so,
  // wait until its sink ends playing also:
  if (audioSource->isCurrentlyAwaitingData()
      || videoSource->isCurrentlyAwaitingData()) return;

  // Now that both sinks have ended, close both input sources,
  // and start playing again:
  *env << "...done reading from file\n";

  audioSink->stopPlaying();
  videoSink->stopPlaying();
      // ensures that both are shut down
  Medium::close(audioSource);
  Medium::close(videoSource);
  Medium::close(mpegDemux);
  // Note: This also closes the input file that this source read from.

  // Start playing once again:
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

  // We must demultiplex Audio and Video Elementary Streams
  // from the input source:
  mpegDemux = MPEG1or2Demux::createNew(*env, fileSource);
  FramedSource* audioES = mpegDemux->newAudioStream();
  FramedSource* videoES = mpegDemux->newVideoStream();

  // Create a framer for each Elementary Stream:
  audioSource
    = MPEG1or2AudioStreamFramer::createNew(*env, audioES);
  videoSource
    = MPEG1or2VideoStreamFramer::createNew(*env, videoES, iFramesOnly);

  // Finally, start playing each sink.
  *env << "Beginning to read from file...\n";
  videoSink->startPlaying(*videoSource, afterPlaying, videoSink);
  audioSink->startPlaying(*audioSource, afterPlaying, audioSink);
}
