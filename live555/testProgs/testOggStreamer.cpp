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
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********/
// Copyright (c) 1996-2015, Live Networks, Inc.  All rights reserved
// A test program that reads a ".ogg" (i.e., Ogg) file, demultiplexes each track
// (audio and/or video), and streams each track using RTP multicast.
// main program

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

UsageEnvironment* env;
char const* inputFileName = "test.ogg";
struct in_addr destinationAddress;
RTSPServer* rtspServer;
ServerMediaSession* sms;
OggFile* oggFile;
OggDemux* oggDemux;
unsigned numTracks;

// A structure representing the state of a track:
struct TrackState {
  u_int32_t trackNumber;
  FramedSource* source;
  RTPSink* sink;
  RTCPInstance* rtcp;
};
TrackState* trackState;

void onOggFileCreation(OggFile* newFile, void* clientData); // forward

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Define our destination (multicast) IP address:
  destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);
    // Note: This is a multicast address.  If you wish instead to stream
    // using unicast, then you should use the "testOnDemandRTSPServer"
    // test program - not this test program - as a model.

  // Create our RTSP server.  (Receivers will need to use RTSP to access the stream.)
  rtspServer = RTSPServer::createNew(*env, 8554);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }
  sms = ServerMediaSession::createNew(*env, "testStream", inputFileName,
                                      "Session streamed by \"testMKVStreamer\"",
                                      True /*SSM*/);

  // Arrange to create an "OggFile" object for the specified file.
  // (Note that this object is not created immediately, but instead via a callback.)
  OggFile::createNew(*env, inputFileName, onOggFileCreation, NULL);

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void play(); // forward

void onOggFileCreation(OggFile* newFile, void* clientData) {
  oggFile = newFile;

  // Create a new demultiplexor for the file:
  oggDemux = oggFile->newDemux();

  // Create source streams, "RTPSink"s, and "RTCPInstance"s for each preferred track;
  unsigned short rtpPortNum = 22222;
  const unsigned char ttl = 255;

  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case

  numTracks = oggFile->numTracks();
  trackState = new TrackState[numTracks];
  for (unsigned i = 0; i < numTracks; ++i) {
    u_int32_t trackNumber;
    FramedSource* baseSource = oggDemux->newDemuxedTrack(trackNumber);
    trackState[i].trackNumber = trackNumber;

    unsigned estBitrate, numFiltersInFrontOfTrack;
    trackState[i].source = oggFile
      ->createSourceForStreaming(baseSource, trackNumber, estBitrate, numFiltersInFrontOfTrack);
    trackState[i].sink = NULL; // by default; may get changed below
    trackState[i].rtcp = NULL; // ditto

    if (trackState[i].source != NULL) {
      Groupsock* rtpGroupsock = new Groupsock(*env, destinationAddress, rtpPortNum, ttl);
      Groupsock* rtcpGroupsock = new Groupsock(*env, destinationAddress, rtpPortNum+1, ttl);
      rtpPortNum += 2;

      trackState[i].sink
        = oggFile->createRTPSinkForTrackNumber(trackNumber, rtpGroupsock, 96+i);
      if (trackState[i].sink != NULL) {
        if (trackState[i].sink->estimatedBitrate() > 0) {
          estBitrate = trackState[i].sink->estimatedBitrate(); // hack
        }
	trackState[i].rtcp
	  = RTCPInstance::createNew(*env, rtcpGroupsock, estBitrate, CNAME,
				    trackState[i].sink, NULL /* we're a server */,
				    True /* we're a SSM source */);
	  // Note: This starts RTCP running automatically

	// Having set up a track for streaming, add it to our RTSP server's "ServerMediaSession":
	sms->addSubsession(PassiveServerMediaSubsession::createNew(*trackState[i].sink, trackState[i].rtcp));
      }
    }
  }

  if (sms->numSubsessions() == 0) {
    *env << "Error: The Ogg file \"" << inputFileName << "\" has no streamable tracks\n";
    *env << "(Perhaps the file does not exist, is not an 'Ogg' file, or has no tracks that we know how to stream.)\n";
    exit(1);
  }

  rtspServer->addServerMediaSession(sms);

  char* url = rtspServer->rtspURL(sms);
  *env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;

  // Start the streaming:
  play();
}

void afterPlaying(void* /*clientData*/) {
  *env << "...done reading from file\n";

  // Stop playing all "RTPSink"s, then close the source streams
  // (which will also close the demultiplexor itself):
  unsigned i;
  for (i = 0; i < numTracks; ++i) {
    if (trackState[i].sink != NULL) trackState[i].sink->stopPlaying();
    Medium::close(trackState[i].source); trackState[i].source = NULL;
  }

  // Create a new demultiplexor from our Ogg file, then new data sources for each track:
  oggDemux = oggFile->newDemux();
  for (i = 0; i < numTracks; ++i) {
    if (trackState[i].trackNumber != 0) {
      FramedSource* baseSource
	= oggDemux->newDemuxedTrack(trackState[i].trackNumber);

      unsigned estBitrate, numFiltersInFrontOfTrack;
      trackState[i].source
	= oggFile->createSourceForStreaming(baseSource, trackState[i].trackNumber,
					    estBitrate, numFiltersInFrontOfTrack);
    }
  }

  // Start playing once again:
  play();
}

void play() {
  *env << "Beginning to read from file...\n";

  // Start playing each track's RTP sink from its corresponding source:
  for (unsigned i = 0; i < numTracks; ++i) {
    if (trackState[i].sink != NULL && trackState[i].source != NULL) {
      trackState[i].sink->startPlaying(*trackState[i].source, afterPlaying, NULL);
    }
  }
}
