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
// A test program that splits a MPEG-1 or 2 Program Stream file into
// video and audio output files.
// main program

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include <stdlib.h>

char const* inputFileName = "in.mpg";
char const* outputFileName_video = "out_video.mpg";
char const* outputFileName_audio = "out_audio.mpg";

void afterPlaying(void* clientData); // forward

// A structure to hold the state of the current session.
// It is used in the "afterPlaying()" function to clean up the session.
struct sessionState_t {
  MPEG1or2Demux* baseDemultiplexor;
  MediaSource* videoSource;
  MediaSource* audioSource;
  FileSink* videoSink;
  FileSink* audioSink;
} sessionState;

UsageEnvironment* env;

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Open the input file as a 'byte-stream file source':
  ByteStreamFileSource* inputSource
    = ByteStreamFileSource::createNew(*env, inputFileName);
  if (inputSource == NULL) {
    *env << "Unable to open file \"" << inputFileName
	 << "\" as a byte-stream file source\n";
    exit(1);
  }

  // Create a MPEG demultiplexor that reads from that source.
  sessionState.baseDemultiplexor = MPEG1or2Demux::createNew(*env, inputSource);

  // Create, from this, our own sources (video and audio):
  sessionState.videoSource = sessionState.baseDemultiplexor->newVideoStream();
  sessionState.audioSource = sessionState.baseDemultiplexor->newAudioStream();

  // Create the data sinks (output files):
  sessionState.videoSink = FileSink::createNew(*env, outputFileName_video);
  sessionState.audioSink = FileSink::createNew(*env, outputFileName_audio);

  // Finally, start playing each sink.
  *env << "Beginning to read...\n";
  sessionState.videoSink->startPlaying(*sessionState.videoSource,
				       afterPlaying, sessionState.videoSink);
  sessionState.audioSink->startPlaying(*sessionState.audioSource,
				       afterPlaying, sessionState.audioSink);

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void afterPlaying(void* clientData) {
  Medium* finishedSink = (Medium*)clientData;

  if (finishedSink == sessionState.videoSink) {
    *env << "No more video\n";
    Medium::close(sessionState.videoSink);
    Medium::close(sessionState.videoSource);
    sessionState.videoSink = NULL;
  } else if (finishedSink == sessionState.audioSink) {
    *env << "No more audio\n";
    Medium::close(sessionState.audioSink);
    Medium::close(sessionState.audioSource);
    sessionState.audioSink = NULL;
  }

  if (sessionState.videoSink == NULL && sessionState.audioSink == NULL) {
    *env << "...finished reading\n";

    Medium::close(sessionState.baseDemultiplexor);

    exit(0);
  }
}
