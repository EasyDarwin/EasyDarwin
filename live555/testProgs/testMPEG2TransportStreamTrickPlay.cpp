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
// A program that tests 'trick mode' operations on a MPEG-2 Transport Stream file,
// by generating a new Transport Stream file that represents the result of the
// 'trick mode' operation (seeking and/or fast forward/reverse play).
// For this to work, there must also be an index file present, in the same directory
// as the Transport Stream file, and with the same name prefix.  (The Transport
// Stream file has name suffix ".ts"; the index file has name suffix ".tsx".)
// main program

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>

void afterPlaying(void* clientData); // forward

UsageEnvironment* env;
char const* programName;

void usage() {
  *env << "usage: " << programName << " <input-transport-stream-file-name> <start-time> <scale> <output-transport-stream-file-name>\n";
  *env << "\twhere\t<transport-stream-file-name> ends with \".ts\"\n";
  *env << "\t\t<start-time> is the starting play time in seconds (0 for the start)\n";
  *env << "\t\t<scale> is a non-zero integer, representing the playing speed (use 1 for normal play; use a negative number for reverse play)\n";
  exit(1);
}

int main(int argc, char const** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Parse the command line:
  programName = argv[0];
  if (argc != 5) usage();

  char const* inputFileName = argv[1];
  // Check whether the input file name ends with ".ts":
  int len = strlen(inputFileName);
  if (len < 4 || strcmp(&inputFileName[len-3], ".ts") != 0) {
    *env << "ERROR: input file name \"" << inputFileName
	 << "\" does not end with \".ts\"\n";
    usage();
  }

  // Parse the <start-time> and <scale> parameters:
  float startTime;
  if (sscanf(argv[2], "%f", &startTime) != 1 || startTime < 0.0f) usage();

  int scale;
  if (sscanf(argv[3], "%d", &scale) != 1 || scale == 0) usage();

  // Open the input file (as a 'byte stream file source'):
  FramedSource* input
    = ByteStreamFileSource::createNew(*env, inputFileName, TRANSPORT_PACKET_SIZE);
  if (input == NULL) {
    *env << "Failed to open input file \"" << inputFileName << "\" (does it exist?)\n";
    exit(1);
  }

  // Check whether the corresponding index file exists.
  // The index file name is the same as the input file name, except with suffix ".tsx":
  char* indexFileName = new char[len+2]; // allow for trailing x\0
  sprintf(indexFileName, "%sx", inputFileName);
  MPEG2TransportStreamIndexFile* indexFile
    = MPEG2TransportStreamIndexFile::createNew(*env, indexFileName);
  if (indexFile == NULL) {
    *env << "Failed to open index file \"" << indexFileName << "\" (does it exist?)\n";
    exit(1);
  }

  // Create a filter that generates trick mode data from the input and index files:
  MPEG2TransportStreamTrickModeFilter* trickModeFilter
    = MPEG2TransportStreamTrickModeFilter::createNew(*env, input, indexFile, scale);

  if (startTime > 0.0f) {
    // Seek the input Transport Stream and Index files to the specified start time:
    unsigned long tsRecordNumber, indexRecordNumber;
    indexFile->lookupTSPacketNumFromNPT(startTime, tsRecordNumber, indexRecordNumber);
    if (!trickModeFilter->seekTo(tsRecordNumber, indexRecordNumber)) { // TARFU!
      *env << "Failed to seek trick mode filter to ts #" << (unsigned)tsRecordNumber
	   << ", ix #" << (unsigned)indexRecordNumber
	   << "(for time " << startTime << ")\n";
      exit(1);
    }
  }

  // Generate a new Transport Stream from the Trick Mode filter:
  MPEG2TransportStreamFromESSource* newTransportStream
    = MPEG2TransportStreamFromESSource::createNew(*env);
  newTransportStream->addNewVideoSource(trickModeFilter, indexFile->mpegVersion());

  // Open the output file (for writing), as a 'file sink':
  char const* outputFileName = argv[4];
  MediaSink* output = FileSink::createNew(*env, outputFileName);
  if (output == NULL) {
    *env << "Failed to open output file \"" << outputFileName << "\"\n";
    exit(1);
  }

  // Start playing, to generate the output file:
  *env << "Writing output file \"" << outputFileName
       << "\" (start time " << startTime
       << ", scale " << scale
       << ")...";
  output->startPlaying(*newTransportStream, afterPlaying, NULL);

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void afterPlaying(void* /*clientData*/) {
  *env << "...done\n";
  exit(0);
}
