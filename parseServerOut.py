#!python

"""Script for parsing the output of the server when "enable_packet_header_printfs" is turned on in the xml config file.
This script will calculate the instantaneous bitrate of the movie as well as the total bytes sent.

Usage: parseServerOut.py [-h] [-i interval] [filename]

options:
	-h                show this help message and exit
	-i                Interval for the instantaneous bitrate.  Defaults to 1 seconds.

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

import re, sys, decimal
from decimal import Decimal

#the regular expression for matching lines in the SLT output
sendRE = re.compile(\
	r"^<send sess=\d+: RTP UDP xmit_sec=(\S+)  type=(\w+) size=(\d+)")
def parseServerInput(inputFile):
	videoTimeList = []
	videoSizeList = []
	audioTimeList = []
	audioSizeList = []
	otherSizeList = []
	otherTimeList = []
	for line in inputFile:
		matchObj = sendRE.match(line)
		if matchObj is not None:
			(time, streamType, size) = matchObj.groups()
			if streamType == "video":
				videoTimeList.append(int(Decimal(time) * 1000))
				videoSizeList.append(int(size))
			elif streamType == "audio":
				audioTimeList.append(int(Decimal(time) * 1000))
				audioSizeList.append(int(size))
			else:
				otherTimeList.append(int(Decimal(time) * 1000))
				otherSizeList.append(int(size))
			continue

	# modify the times to be 0-based
	if len(videoTimeList) > 0:
		earliestTime = min(videoTimeList)
		for i in range(len(videoTimeList)):
			videoTimeList[i] -= earliestTime

	if len(audioTimeList) > 0:
		earliestTime = min(audioTimeList)
		for i in range(len(audioTimeList)):
			audioTimeList[i] -= earliestTime

	if len(otherTimeList) > 0:
		earliestTime = min(otherSizeList)
		for i in range(len(otherTimeList)):
			otherTimeList[i] -= earliestTime

	return (videoTimeList, videoSizeList, audioTimeList, audioSizeList, otherTimeList, otherSizeList)


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

	try:
		optlist, args = getopt.getopt(sys.argv[1:], "hi:s:")
	except getopt.GetoptError:
		sys.exit(__doc__)

	interval = 1000
	streams = 'video'
	for opt, optarg in optlist:
		if opt == '-h':
			print __doc__
			sys.exit(0)
		elif opt == '-i':
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
	
	
	videoTimeList, videoSizeList, audioTimeList, audioSizeList, otherTimeList, otherSizeList = parseServerInput(inputFile)

	print "%i" % (sum(videoSizeList) + sum(audioSizeList) + sum(otherSizeList)) # in bytes

	def millisecondsToSec(time):
		"""Convert time to 123.456 form as a string"""
		return str(time / 1000) + '.' + str(time % 1000)

	if len(videoTimeList) > 0:
		print "video:"
		videoData = calcBitRate(videoTimeList, videoSizeList, interval)
		for (time, bitrate) in videoData:
			print "%s, %i" % (millisecondsToSec(time), bitrate)
		print ""

	if len(audioTimeList) > 0:
		print "audio:"
		audioData = calcBitRate(audioTimeList, audioSizeList, interval)
		for (time, bitrate) in audioData:
			print "%s, %i" % (millisecondsToSec(time), bitrate)
		print ""

	if len(otherTimeList) > 0:
		print "other:"
		otherData = calcBitRate(otherTimeList, otherSizeList, interval)
		for (time, bitrate) in otherData:
			print "%s, %i" % (millisecondsToSec(time), bitrate)
		print ""

