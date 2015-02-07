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
// A program that reads an existing MPEG-2 Transport Stream file,
// and generates a separate index file that can be used - by our RTSP server
// implementation - to support 'trick play' operations when streaming the
// Transport Stream file.
// main program

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>

void afterPlaying(void* clientData); // forward

UsageEnvironment* env;
char const* programName;

void usage() {
  *env << "usage: " << programName << " <transport-stream-file-name>\n";
  *env << "\twhere <transport-stream-file-name> ends with \".ts\"\n";
  exit(1);
}

int main(int argc, char const** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Parse the command line:
  programName = argv[0];
  if (argc != 2) usage();

  char const* inputFileName = argv[1];
  // Check whether the input file name ends with ".ts":
  int len = strlen(inputFileName);
  if (len < 4 || strcmp(&inputFileName[len-3], ".ts") != 0) {
    *env << "ERROR: input file name \"" << inputFileName
	 << "\" does not end with \".ts\"\n";
    usage();
  }

  // Open the input file (as a 'byte stream file source'):
  FramedSource* input
    = ByteStreamFileSource::createNew(*env, inputFileName, TRANSPORT_PACKET_SIZE);
  if (input == NULL) {
    *env << "Failed to open input file \"" << inputFileName << "\" (does it exist?)\n";
    exit(1);
  }

  // Create a filter that indexes the input Transport Stream data:
  FramedSource* indexer
    = MPEG2IFrameIndexFromTransportStream::createNew(*env, input);

  // The output file name is the same as the input file name, except with suffix ".tsx":
  char* outputFileName = new char[len+2]; // allow for trailing x\0
  sprintf(outputFileName, "%sx", inputFileName);

  // Open the output file (for writing), as a 'file sink':
  MediaSink* output = FileSink::createNew(*env, outputFileName);
  if (output == NULL) {
    *env << "Failed to open output file \"" << outputFileName << "\"\n";
    exit(1);
  }

  // Start playing, to generate the output index file:
  *env << "Writing index file \"" << outputFileName << "\"...";
  output->startPlaying(*indexer, afterPlaying, NULL);

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void afterPlaying(void* /*clientData*/) {
  *env << "...done\n";
  exit(0);
}
