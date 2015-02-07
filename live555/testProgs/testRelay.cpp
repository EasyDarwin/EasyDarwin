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
// A test program that receives a UDP multicast stream
// and retransmits it to another (multicast or unicast) address & port
// main program

#include <liveMedia.hh>
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"

UsageEnvironment* env;

// To receive a "source-specific multicast" (SSM) stream, uncomment this:
//#define USE_SSM 1

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Create a 'groupsock' for the input multicast group,port:
  char const* inputAddressStr
#ifdef USE_SSM
    = "232.255.42.42";
#else
    = "239.255.42.42";
#endif
  struct in_addr inputAddress;
  inputAddress.s_addr = our_inet_addr(inputAddressStr);

  Port const inputPort(8888);
  unsigned char const inputTTL = 0; // we're only reading from this mcast group

#ifdef USE_SSM
  char* sourceAddressStr = "aaa.bbb.ccc.ddd";
                           // replace this with the real source address
  struct in_addr sourceFilterAddress;
  sourceFilterAddress.s_addr = our_inet_addr(sourceAddressStr);

  Groupsock inputGroupsock(*env, inputAddress, sourceFilterAddress, inputPort);
#else
  Groupsock inputGroupsock(*env, inputAddress, inputPort, inputTTL);
#endif

  // Then create a liveMedia 'source' object, encapsulating this groupsock:
  FramedSource* source = BasicUDPSource::createNew(*env, &inputGroupsock);


  // Create a 'groupsock' for the destination address and port:
  char const* outputAddressStr = "239.255.43.43"; // this could also be unicast
    // Note: You may change "outputAddressStr" to use a different multicast
    // (or unicast address), but do *not* change it to use the same multicast
    // address as "inputAddressStr".
  struct in_addr outputAddress;
  outputAddress.s_addr = our_inet_addr(outputAddressStr);

  Port const outputPort(4444);
  unsigned char const outputTTL = 255;

  Groupsock outputGroupsock(*env, outputAddress, outputPort, outputTTL);

  // Then create a liveMedia 'sink' object, encapsulating this groupsock:
  unsigned const maxPacketSize = 65536; // allow for large UDP packets
  MediaSink* sink = BasicUDPSink::createNew(*env, &outputGroupsock, maxPacketSize);


  // Now, start playing, feeding the sink object from the source:
  sink->startPlaying(*source, NULL, NULL);

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}
