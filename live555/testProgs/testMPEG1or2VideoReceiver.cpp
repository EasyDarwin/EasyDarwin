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
// A test program that receives a RTP/RTCP multicast MPEG video stream,
// and outputs the resulting MPEG file stream to 'stdout'
// main program

#include "liveMedia.hh"
#include "GroupsockHelper.hh"

#include "BasicUsageEnvironment.hh"

// To receive a "source-specific multicast" (SSM) stream, uncomment this:
//#define USE_SSM 1

void afterPlaying(void* clientData); // forward

// A structure to hold the state of the current session.
// It is used in the "afterPlaying()" function to clean up the session.
struct sessionState_t {
  RTPSource* source;
  MediaSink* sink;
  RTCPInstance* rtcpInstance;
} sessionState;

UsageEnvironment* env;

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Create the data sink for 'stdout':
  sessionState.sink = FileSink::createNew(*env, "stdout");
  // Note: The string "stdout" is handled as a special case.
  // A real file name could have been used instead.

  // Create 'groupsocks' for RTP and RTCP:
  char const* sessionAddressStr
#ifdef USE_SSM
    = "232.255.42.42";
#else
    = "239.255.42.42";
  // Note: If the session is unicast rather than multicast,
  // then replace this string with "0.0.0.0"
#endif
  const unsigned short rtpPortNum = 8888;
  const unsigned short rtcpPortNum = rtpPortNum+1;
#ifndef USE_SSM
  const unsigned char ttl = 1; // low, in case routers don't admin scope
#endif

  struct in_addr sessionAddress;
  sessionAddress.s_addr = our_inet_addr(sessionAddressStr);
  const Port rtpPort(rtpPortNum);
  const Port rtcpPort(rtcpPortNum);

#ifdef USE_SSM
  char* sourceAddressStr = "aaa.bbb.ccc.ddd";
                           // replace this with the real source address
  struct in_addr sourceFilterAddress;
  sourceFilterAddress.s_addr = our_inet_addr(sourceAddressStr);

  Groupsock rtpGroupsock(*env, sessionAddress, sourceFilterAddress, rtpPort);
  Groupsock rtcpGroupsock(*env, sessionAddress, sourceFilterAddress, rtcpPort);
  rtcpGroupsock.changeDestinationParameters(sourceFilterAddress,0,~0);
      // our RTCP "RR"s are sent back using unicast
#else
  Groupsock rtpGroupsock(*env, sessionAddress, rtpPort, ttl);
  Groupsock rtcpGroupsock(*env, sessionAddress, rtcpPort, ttl);
#endif

  // Create the data source: a "MPEG Video RTP source"
  sessionState.source = MPEG1or2VideoRTPSource::createNew(*env, &rtpGroupsock);

  // Create (and start) a 'RTCP instance' for the RTP source:
  const unsigned estimatedSessionBandwidth = 160; // in kbps; for RTCP b/w share
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case
  sessionState.rtcpInstance
    = RTCPInstance::createNew(*env, &rtcpGroupsock,
			      estimatedSessionBandwidth, CNAME,
			      NULL /* we're a client */, sessionState.source);
  // Note: This starts RTCP running automatically

  // Finally, start receiving the multicast stream:
  *env << "Beginning receiving multicast stream...\n";
  sessionState.sink->startPlaying(*sessionState.source, afterPlaying, NULL);

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}


void afterPlaying(void* /*clientData*/) {
  *env << "...done receiving\n";

  // End by closing the media:
  Medium::close(sessionState.rtcpInstance); // Note: Sends a RTCP BYE
  Medium::close(sessionState.sink);
  Medium::close(sessionState.source);
}
