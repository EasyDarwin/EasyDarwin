#!python

"""Script for parsing the output of a StreamingLoadTool file.  Can be used both as a script and as a python module.
The output is a commma separated file.  The StreamingLoadTool has to be run with -V 3 or above.

Usage: ParseSLTOut.py [-h] [-b interval] [-s all] [filename]

options:
	-h                show this help message and exit
	-b                Take the packet lengths and categorizes them according to their arrival time.
	                  The packet lengths within the same bucket (of size interval seconds) are summed up
	                  and averaged over the interval, which is useful for seeing the bit rate.
	-s                The streams to parse: one of video, audio, all; default is video

If filename is missing, then the script will read from standard in.  The script writes to standard out."""

"""
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
"""

import re, sys

#the regular expression for matching lines in the SLT output
playRE = re.compile(\
		r"^Receiving track (\d+), trackID=(\d+), (\w+) at time (\d+)")
timeRE = re.compile(\
		r"^Processing incoming packets at time (\d+)")
processedRE = re.compile(\
		r"^Processed packet: track=(\d+), len=(\d+), seq=(\d+), time=(\d+)\((\d+)\); bufferingDelay=(\d+), FBS=(\d+)")


def parseSLTInput(inputFile):
	"""
	Parses a string consisting of outputs from StreamingLoadTool.
	The output is a list, where the index is the track index.  The value is a map that maps from processing time to
	packets.  Each packet is a 3-tuple containing the packet length(in bytes), the sequence number, and the timestamp.
	"""
	videoTrackIndex = None
	audioTrackIndex = None
	localTimeBase = None
	curTime = None

	packetTable = [{}, {}]

	for line in inputFile:
		matchObj = playRE.match(line)
		if matchObj is not None:
			trackIndex, trackID, trackType, startTime = matchObj.groups();
			if trackType == "video":
				videoTrackIndex = int(trackIndex)
			elif trackType == "audio":
				audioTrackIndex = int(trackIndex)
			localTimeBase = int(startTime)
			continue

		matchObj = timeRE.match(line)
		if matchObj is not None:
			processingTime, = matchObj.groups()
			curTime = int(processingTime)
			continue

		matchObj = processedRE.match(line)
		if matchObj is not None:
			trackIndex, packetLen, seqNum, timeStamp, timeStampInMediaTime, playoutDelay, freeBufferSpace = matchObj.groups()
			trackIndex = int(trackIndex);
			while trackIndex >= len(packetTable):
				packetTable.append({})
			packetList = packetTable[trackIndex].setdefault(curTime, [])
			packetList.append( (int(packetLen), int(seqNum), int(timeStamp)) )
			continue

	if localTimeBase is None:
		sys.exit("Parse error: cannot find a track")

	# modify the processing times to be 0-based
	newPacketTable = [{}] * len(packetTable)
	for trackIndex in range(len(packetTable)):
		newPackets = {}
		for (curTime, packet) in packetTable[trackIndex].items():
			curTime -= localTimeBase
			newPackets[curTime] = packet
		packetTable[trackIndex] = newPackets

	return (packetTable, videoTrackIndex, audioTrackIndex)


def calcBitRate(xList, yList, interval):
	"""xList is a list of processing time, and yList is a list of packet size.
	The function will calculate the bitrate, divided into interval-sized buckets, and returns it as a list
	of (processing time, bitrate) pair.
	xList is expected to be a list of time in milliseconds, and yList is expected to be a list of packet size in bytes."""
	maxIndex = max(xList) / interval
	bitsReceived = [0] * (maxIndex + 1)

	for i in range(len(yList)):
		x = xList[i]
		y = yList[i]

		bitsReceived[x / interval] += y * 8
	
	return [ (i * interval, (bitsReceived[i] * 1000) / interval) for i in range(len(bitsReceived))]


if __name__ == "__main__":
	# the file is ran as a script

	# first parse the command line
	import getopt
	from decimal import Decimal

	try:
		optlist, args = getopt.getopt(sys.argv[1:], "hb:s:")
	except getopt.GetoptError:
		sys.exit(__doc__)

	categorize = False
	streams = 'video'
	for opt, optarg in optlist:
		if opt == '-h':
			print __doc__
			sys.exit(0)
		elif opt == '-b':
			categorize = True
			try:
				interval = int(Decimal(optarg) * 1000)
			except:
				sys.exit(__doc__)
		elif opt == '-s':
			streams = optarg
			if streams != 'video' and streams != 'audio' and streams != 'all':
				sys.exit(__doc__)

	if len(args) == 0:
		inputFile = sys.stdin
	elif len(args) == 1:
		inputFile = open(args[0], 'r')
	else:
		sys.exit(__doc__)
	
	(packetTable, videoIndex, audioIndex) = parseSLTInput(inputFile)

	if streams == 'video':
		if videoIndex is None:
			sys.exit("Parse error: Cannot find a video stream")
		packets = packetTable[videoIndex]
	elif streams == 'audio':
		if audioIndex is None:
			sys.exit("Parse error: Cannot find an audio stream")
		packets = packetTable[audioIndex]
	else:
		packets = {}
		for perStreamPackets in packetTable:
			for (time, packetList) in perStreamPackets.items():
				packets.setdefault(time, []).extend(packetList)

	if len(packets) == 0:
		sys.exit("Error: Cannot find a stream")


	def millisecondsToSec(time):
		"""Convert time to 123.456 form as a string"""
		return str(time / 1000) + '.' + str(time % 1000)

	if categorize:
		xList = []
		yList = []

		for processingTime, packetList in packets.items():
			for (packetLen, seqNum, timeStamp) in packetList:
				xList.append(processingTime)
				yList.append(packetLen)
		data = calcBitRate(xList, yList, interval)
		for (time, bitrate) in data:
			print "%s, %i" % (millisecondsToSec(time), bitrate)
	else:
		processingTimes = packets.keys()
		processingTimes.sort()
		for processingTime in processingTimes:
			packetList = packets[processingTime]
			for (packetLen, seqNum, timeStamp) in packets[processingTime]:
				# output.append( (processingTime, packetLen) )
				print ("%s, %i") % ( millisecondsToSec(processingTime), packetLen )
