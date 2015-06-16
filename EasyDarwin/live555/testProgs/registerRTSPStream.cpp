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
// A demonstration application that uses our custom RTSP "REGISTER" command to register a stream
// (given by "rtsp://" URL) with a RTSP client or proxy server
// splits it into Audio (AC3) and Video (MPEG) Elementary Streams,
// and streams both using RTP.
// main program

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

char const* programName;
UsageEnvironment* env;

Boolean requestStreamingViaTCP = False;
char const* username = NULL;
char const* password = NULL;

void registerResponseHandler(RTSPClient* rtspClient, int resultCode, char* resultString) {
  Medium::close(rtspClient);

  // We're done:
  exit(0);
}

void usage() {
  *env << "usage: " << programName << " [-t] [-u <username> <password>] "
    "<remote-client-or-proxy-server-name-or-address> <remote-client-or-proxy-server-port-number> <rtsp-URL-to-register>"
    " [proxy-URL-suffix]\n";
  exit(1);
}

int main(int argc, char const** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Parse command-line options and arguments:
  // (Unfortunately we can't use getopt() here; Windoze doesn't have it)
  programName = argv[0];
  while (argc > 2) {
    char const* const opt = argv[1];
    if (opt[0] != '-') break;
    switch (opt[1]) {
      case 't': { // ask the remote client to access the stream via TCP instead of UDP
	requestStreamingViaTCP = True;
	break;
      }

      case 'u': { // specify a username and password
	if (argc < 4) usage(); // there's no argv[3] (for the "password")
	username = argv[2];
	password = argv[3];
	argv+=2; argc-=2;
	break;
      }

      default: {
	usage();
	break;
      }
    }

    ++argv; --argc;
  }
  if (argc != 4 && argc != 5) usage();

  char const* remoteClientNameOrAddress = argv[1];

  portNumBits remoteClientPortNum;
  if (sscanf(argv[2], "%hu", &remoteClientPortNum) != 1 || remoteClientPortNum == 0 || remoteClientPortNum == 0xFFFF) usage();

  char const* rtspURLToRegister = argv[3];

  char const* proxyURLSuffix = argc == 5 ? argv[4] : NULL;

  Authenticator* ourAuthenticator = username == NULL ? NULL : new Authenticator(username, password);

  // We have the command-line arguments.  Send the command:

  RTSPRegisterSender::createNew(*env, remoteClientNameOrAddress, remoteClientPortNum, rtspURLToRegister,
				registerResponseHandler, ourAuthenticator,
				requestStreamingViaTCP, proxyURLSuffix, False/*reuseConnection*/,
				1/*verbosityLevel*/, programName);
      // Note: This object will be deleted later, by the response handler

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}
