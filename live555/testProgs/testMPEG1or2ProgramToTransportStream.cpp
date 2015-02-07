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
// A program that converts a MPEG-1 or 2 Program Stream file into
// a Transport Stream file.
// main program

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

char const* inputFileName = "in.mpg";
char const* outputFileName = "out.ts";

void afterPlaying(void* clientData); // forward

UsageEnvironment* env;

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Open the input file as a 'byte-stream file source':
  FramedSource* inputSource = ByteStreamFileSource::createNew(*env, inputFileName);
  if (inputSource == NULL) {
    *env << "Unable to open file \"" << inputFileName
	 << "\" as a byte-stream file source\n";
    exit(1);
  }

  // Create a MPEG demultiplexor that reads from that source.
  MPEG1or2Demux* baseDemultiplexor = MPEG1or2Demux::createNew(*env, inputSource);

  // Create, from this, a source that returns raw PES packets:
  MPEG1or2DemuxedElementaryStream* pesSource = baseDemultiplexor->newRawPESStream();

  // And, from this, a filter that converts to MPEG-2 Transport Stream frames:
  FramedSource* tsFrames
    = MPEG2TransportStreamFromPESSource::createNew(*env, pesSource);

  // Open the output file as a 'file sink':
  MediaSink* outputSink = FileSink::createNew(*env, outputFileName);
  if (outputSink == NULL) {
    *env << "Unable to open file \"" << outputFileName << "\" as a file sink\n";
    exit(1);
  }

  // Finally, start playing:
  *env << "Beginning to read...\n";
  outputSink->startPlaying(*tsFrames, afterPlaying, NULL);

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void afterPlaying(void* /*clientData*/) {
  *env << "Done reading.\n";
  *env << "Wrote output file: \"" << outputFileName << "\"\n";
  exit(0);
}
