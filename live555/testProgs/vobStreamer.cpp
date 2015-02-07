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
// A test program that reads a VOB file
// splits it into Audio (AC3) and Video (MPEG) Elementary Streams,
// and streams both using RTP.
// main program

#include "liveMedia.hh"
#include "AC3AudioStreamFramer.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"

char const* programName;
// Whether to stream *only* "I" (key) frames
// (e.g., to reduce network bandwidth):
Boolean iFramesOnly = False;

unsigned const VOB_AUDIO = 1<<0;
unsigned const VOB_VIDEO = 1<<1;
unsigned mediaToStream = VOB_AUDIO|VOB_VIDEO; // by default

char const** inputFileNames;
char const** curInputFileName;
Boolean haveReadOneFile = False;

UsageEnvironment* env;
MPEG1or2Demux* mpegDemux;
AC3AudioStreamFramer* audioSource = NULL;
FramedSource* videoSource = NULL;
RTPSink* audioSink = NULL;
RTCPInstance* audioRTCP = NULL;
RTPSink* videoSink = NULL;
RTCPInstance* videoRTCP = NULL;
RTSPServer* rtspServer = NULL;
unsigned short const defaultRTSPServerPortNum = 554;
unsigned short rtspServerPortNum = defaultRTSPServerPortNum;

Groupsock* rtpGroupsockAudio;
Groupsock* rtcpGroupsockAudio;
Groupsock* rtpGroupsockVideo;
Groupsock* rtcpGroupsockVideo;

void usage() {
  *env << "usage: " << programName << " [-i] [-a|-v] "
	  "[-p <RTSP-server-port-number>] "
	  "<VOB-file>...<VOB-file>\n";
  exit(1);
}

void play(); // forward

int main(int argc, char const** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Parse command-line options:
  // (Unfortunately we can't use getopt() here; Windoze doesn't have it)
  programName = argv[0];
  while (argc > 2) {
    char const* const opt = argv[1];
    if (opt[0] != '-') break;
    switch (opt[1]) {

    case 'i': { // transmit video I-frames only
      iFramesOnly = True;
      break;
    }

    case 'a': { // transmit audio, but not video
      mediaToStream &=~ VOB_VIDEO;
      break;
    }

    case 'v': { // transmit video, but not audio
      mediaToStream &=~ VOB_AUDIO;
      break;
    }

    case 'p': { // specify port number for built-in RTSP server
      int portArg;
      if (sscanf(argv[2], "%d", &portArg) != 1) {
        usage();
      }
      if (portArg <= 0 || portArg >= 65536) {
        *env << "bad port number: " << portArg
	     << " (must be in the range (0,65536))\n";
        usage();
      }
      rtspServerPortNum = (unsigned short)portArg;
      ++argv; --argc;
      break;
    }

    default: {
      usage();
      break;
    }
    }

    ++argv; --argc;
  }
  if (argc < 2) usage();
  if (mediaToStream == 0) {
    *env << "The -a and -v flags cannot both be used!\n";
    usage();
  }
  if (iFramesOnly && (mediaToStream&VOB_VIDEO) == 0) {
    *env << "Warning: Because we're not streaming video, the -i flag has no effect.\n";
  }

  inputFileNames = &argv[1];
  curInputFileName = inputFileNames;

  // Create 'groupsocks' for RTP and RTCP:
  struct in_addr destinationAddress;
  destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);

  const unsigned short rtpPortNumAudio = 4444;
  const unsigned short rtcpPortNumAudio = rtpPortNumAudio+1;
  const unsigned short rtpPortNumVideo = 8888;
  const unsigned short rtcpPortNumVideo = rtpPortNumVideo+1;
  const unsigned char ttl = 255;

  const Port rtpPortAudio(rtpPortNumAudio);
  const Port rtcpPortAudio(rtcpPortNumAudio);
  const Port rtpPortVideo(rtpPortNumVideo);
  const Port rtcpPortVideo(rtcpPortNumVideo);

  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case

  if (mediaToStream&VOB_AUDIO) {
    rtpGroupsockAudio
      = new Groupsock(*env, destinationAddress, rtpPortAudio, ttl);
    rtpGroupsockAudio->multicastSendOnly(); // because we're a SSM source

    // Create an 'AC3 Audio RTP' sink from the RTP 'groupsock':
    audioSink
      = AC3AudioRTPSink::createNew(*env, rtpGroupsockAudio, 96, 0);
    // set the RTP timestamp frequency 'for real' later

    // Create (and start) a 'RTCP instance' for this RTP sink:
    rtcpGroupsockAudio
      = new Groupsock(*env, destinationAddress, rtcpPortAudio, ttl);
    rtcpGroupsockAudio->multicastSendOnly(); // because we're a SSM source
    const unsigned estimatedSessionBandwidthAudio
      = 160; // in kbps; for RTCP b/w share
    audioRTCP = RTCPInstance::createNew(*env, rtcpGroupsockAudio,
					estimatedSessionBandwidthAudio, CNAME,
					audioSink, NULL /* we're a server */,
					True /* we're a SSM source */);
    // Note: This starts RTCP running automatically
  }

  if (mediaToStream&VOB_VIDEO) {
    rtpGroupsockVideo
      = new Groupsock(*env, destinationAddress, rtpPortVideo, ttl);
    rtpGroupsockVideo->multicastSendOnly(); // because we're a SSM source

    // Create a 'MPEG Video RTP' sink from the RTP 'groupsock':
    videoSink = MPEG1or2VideoRTPSink::createNew(*env, rtpGroupsockVideo);

    // Create (and start) a 'RTCP instance' for this RTP sink:
    rtcpGroupsockVideo
      = new Groupsock(*env, destinationAddress, rtcpPortVideo, ttl);
    rtcpGroupsockVideo->multicastSendOnly(); // because we're a SSM source
    const unsigned estimatedSessionBandwidthVideo
      = 4500; // in kbps; for RTCP b/w share
    videoRTCP = RTCPInstance::createNew(*env, rtcpGroupsockVideo,
					estimatedSessionBandwidthVideo, CNAME,
					videoSink, NULL /* we're a server */,
					True /* we're a SSM source */);
    // Note: This starts RTCP running automatically
  }

  if (rtspServer == NULL) {
    rtspServer = RTSPServer::createNew(*env, rtspServerPortNum);
    if (rtspServer == NULL) {
      *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
      *env << "To change the RTSP server's port number, use the \"-p <port number>\" option.\n";
      exit(1);
    }
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, "vobStream", *curInputFileName,
	     "Session streamed by \"vobStreamer\"", True /*SSM*/);
    if (audioSink != NULL) sms->addSubsession(PassiveServerMediaSubsession::createNew(*audioSink, audioRTCP));
    if (videoSink != NULL) sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, videoRTCP));
    rtspServer->addServerMediaSession(sms);

    *env << "Created RTSP server.\n";

    // Display our "rtsp://" URL, for clients to connect to:
    char* url = rtspServer->rtspURL(sms);
    *env << "Access this stream using the URL:\n\t" << url << "\n";
    delete[] url;
  }

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
  if ((audioSource != NULL && audioSource->isCurrentlyAwaitingData()) ||
      (videoSource != NULL && videoSource->isCurrentlyAwaitingData())) {
    return;
  }

  // Now that both sinks have ended, close both input sources,
  // and start playing again:
  *env << "...done reading from file\n";

  if (audioSink != NULL) audioSink->stopPlaying();
  if (videoSink != NULL) videoSink->stopPlaying();
      // ensures that both are shut down
  Medium::close(audioSource);
  Medium::close(videoSource);
  Medium::close(mpegDemux);
  // Note: This also closes the input file that this source read from.

  // Move to the next file name (if any):
  ++curInputFileName;

  // Start playing once again:
  play();
}

void play() {
  if (*curInputFileName == NULL) {
    // We have reached the end of the file name list.
    // Start again, unless we didn't succeed in reading any files:
    if (!haveReadOneFile) exit(1);
    haveReadOneFile = False;
    curInputFileName = inputFileNames;
  }

  // Open the current input file as a 'byte-stream file source':
  ByteStreamFileSource* fileSource
    = ByteStreamFileSource::createNew(*env, *curInputFileName);
  if (fileSource == NULL) {
    *env << "Unable to open file \"" << *curInputFileName
	 << "\" as a byte-stream file source\n";
    // Try the next file instead:
    ++curInputFileName;
    play();
    return;
  }
  haveReadOneFile = True;

  // We must demultiplex Audio and Video Elementary Streams
  // from the input source:
  mpegDemux = MPEG1or2Demux::createNew(*env, fileSource);
  if (mediaToStream&VOB_AUDIO) {
    FramedSource* audioES = mpegDemux->newElementaryStream(0xBD);
      // Because, in a VOB file, the AC3 audio has stream id 0xBD
    audioSource
      = AC3AudioStreamFramer::createNew(*env, audioES, 0x80);
  }
  if (mediaToStream&VOB_VIDEO) {
    FramedSource* videoES = mpegDemux->newVideoStream();

    videoSource
      = MPEG1or2VideoStreamFramer::createNew(*env, videoES, iFramesOnly);
  }

  // Finally, start playing each sink.
  *env << "Beginning to read from \"" << *curInputFileName << "\"...\n";
  if (videoSink != NULL) {
    videoSink->startPlaying(*videoSource, afterPlaying, videoSink);
  }
  if (audioSink != NULL) {
    audioSink->setRTPTimestampFrequency(audioSource->samplingRate());
    audioSink->startPlaying(*audioSource, afterPlaying, audioSink);
  }
}
