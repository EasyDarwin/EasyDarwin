/*
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
 */
 /*
	 File:       RTCPAPPNADUPacket.h

	 Contains:   RTCPAPPNADUPacket de-packetizing class



 */

#ifndef _RTCPAPPNADUPACKET_H_
#define _RTCPAPPNADUPACKET_H_

#include "RTCPAPPPacket.h"
#include "StrPtrLen.h"

class RTCPNaduPacket : public RTCPAPPPacket
{
public:

	RTCPNaduPacket(bool debug);
	virtual ~RTCPNaduPacket() {}

	//Call this before any accessor method. Returns true if successful, false otherwise
	virtual bool ParseAPPData(UInt8* inPacketBuffer, UInt32 inPacketLength);

	// Call to parse if you don't know what kind of packet this is
	bool ParseNaduPacket(UInt8* inPacketBuffer, UInt32 inPacketLength);

	UInt32 GetNumReportBlocks() { return fNumBlocks; };

	SInt32 GetSSRCBlockIndex(UInt32 inSSRC);

	UInt32 GetSSRC(SInt32 index);

	UInt16 GetPlayOutDelay(SInt32 index);

	UInt16 GetNSN(SInt32 index);

	UInt16 GetNUN(SInt32 index);

	UInt16 GetFBS(SInt32 index);

	void DumpNaduPacket();

	static void GetTestPacket(StrPtrLen* resultPtr);
	virtual void Dump(); //Override

	enum
	{
		kNaduDataOffset = 12,
		//32 bit word offsets
		kOffsetNaduSSRC = 0,    //SSRC for this report     
		kOffsetNaduPlayoutDelay = 1,
		kOffsetNSN = 1,
		kOffsetNUN = 2,
		kOffsetFBS = 2,


		kPlayoutMask = 0xFFFF0000UL,
		kNSNMask = 0x0000FFFFUL,
		kNUNMask = 0x001F0000UL,
		kFBSMask = 0x0000FFFFUL,
		kReservedPlayoutDelayValue = 0xFFFF,
		kMaximumReportableFreeBufferSpace = (0xFFFF * 64) //the maximum amount of free buffer space reportable in bytes
	};

	enum //The 4 character name in the APP packet
	{
		kNaduPacketName = FOUR_CHARS_TO_INT('P', 'S', 'S', '0') //QTSS
	};

	enum
	{
		kSupportedNaduPacketVersion = 0
	};

private:
	void ParseAndStore();
	UInt32* fNaduDataBuffer;
	SInt32 fNumBlocks;
	static char sRTCPTestBuffer[256];

};




/*

3GPP TS 26.234 V6.4.0 (2005-06)

6.6 APP: Application-defined RTCP packet

	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P| subtype |   PT=APP=204  |             length            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           SSRC/CSRC                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                          name (ASCII)                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                   application-dependent data                  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


For rate adaptation the name and subtype fields must be set to the following values:

name: The NADU APP data format is detected through the name "PSS0", i.e. 0x50535330 and the subtype.

subtype: This field shall be set to 0 for the NADU format.

length: The number of 32 bit words ?, as defined in RFC 3550 [9]. This means that the field will be 2+3*N, where N is the number of sources reported on. The length field will typically be 5, i.e. 24 bytes packets. application-dependent

data: One or more of the following data format blocks (as described in Figure 4) can be included in the application-dependent data location of the APP packet. The APP packets length field is used to detect how many blocks of data are present. The block shall be sent for the SSRCs for which there are a report block, part of either a Receiver Report or a Sender Report, included in the RTCP compound packet. An NADU APP packet shall not contain any other data format than the one described in figure 4 below.

0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    SSRC                                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      Playout Delay            |            NSN                |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Reserved           |   NUN   |    Free Buffer Space (FBS)    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Figure 4: Data format block for NADU reporting

SSRC: The SSRC of the media stream the buffered packets belong to.

Playout delay (16 bits): The difference between the scheduled playout time of the next ADU to be decoded and the time of sending the NADU APP packet, as measured by the media playout clock, expressed in milliseconds. The client may
choose not to indicate this value by using the reserved value (Ox FFFF). In case of an empty buffer, the playout delay is not defined and the client should also use the reserved value 0xFFFF for this field. The playout delay allows the server to have a more precise value of the amount of time before the client will underflow. The playout delay shall be computed until the actual media playout (i.e., audio playback or video display).

NSN (16 bits): The RTP sequence number of the next ADU to be decoded for the SSRC reported on. In the case where the buffer does not contain any packets for this SSRC, the next not yet received sequence number shall be reported, i.e.
an NSN value that is one larger than the least significant 16 bits of the RTCP SR or RR report block's "extended highest sequence number received".

NUN (5 bits): The unit number (within the RTP packet) of the next ADU to be decoded. The first unit in a packet has a unit number equal to zero. The unit number is incremented by one for each ADU in an RTP packet. In the case of an
audio codec, an ADU is defined as an audio frame. In the case of H.264 (AVC), an ADU is defined as a NAL unit. In the case of H.263 and MPEG4 Visual Simple Profile, each packet carries a single ADU and the NUN field shall be thus
set to zero. Future additions of media encoding or transports capable of having more than one ADU in each RTP payload shall define what shall be counted as an ADU for this format.

FBS (16 bit): The amount of free buffer space available in the client at the time of reporting. The reported free buffer space shall all be part of the buffer space that has been reported as available for adaptation by the 3GPP-Adaptation RTSP header, see clause 5.3.2.2. The amount of free buffer space are reported in number of complete 64 byte blocks, thus allowing for up to 4194304 bytes to be reported as free. If more is available, it shall be reported as the maximal amount available, i.e. 4194304 with a field value 0xffff.

Reserved (11 bits): These bits are not used and shall be set to 0 and shall be ignored by the receiver.



 */


class NaduReport
{

public:
	NaduReport(UInt8* inPacketBuffer, UInt32 inPacketLength, UInt32 id);
	~NaduReport() { delete fPacketBuffer; }
	UInt8* getBuffer() { return fPacketBuffer; }
	UInt32 getLength() { return fLength; }
	UInt32 getID() { return fid; }
	RTCPNaduPacket * GetNaduPacket() { return &fNaduPacket; }

	UInt8* fPacketBuffer;
	UInt32 fLength;
	RTCPNaduPacket fNaduPacket;
	UInt32 fid;

};


// Keep track of the last listSize nadu reports and access them as needed.
// DumpList prints each report while walking backward from the most recent.

class NaduList
{
public:
	NaduList() : fNaduReportList(NULL), fcurrentIndexCount(0), fListSize(0) {};
	~NaduList() {
		for (int i = 0; i < fListSize; i++) {
			if (fNaduReportList[i] != 0) {
				delete fNaduReportList[i];
				fNaduReportList[i] = 0;
			}
		}
		delete[] fNaduReportList;
	}
	void Initialize(UInt32 listSize = 3);

	bool AddReport(UInt8* inPacketBuffer, UInt32 inPacketLength, UInt32 *outID);

	NaduReport*     GetReport(UInt32 id);
	NaduReport*     GetLastReport();
	NaduReport*     GetEarliestReport();
	NaduReport*     GetPreviousReport(NaduReport* theReport);
	NaduReport*     GetNextReport(NaduReport* theReport);
	UInt32          LastReportedFreeBuffSizeBytes();
	UInt32          LastReportedTimeDelayMilli();
	UInt16			GetLastReportedNSN();

	void DumpList();

private:
	UInt32 GetReportIndex(UInt32 id);
	UInt32 IDtoIndex(UInt32 id) { return (id - 1) % fListSize; }
	NaduReport**  fNaduReportList;
	UInt32 fcurrentIndexCount;
	UInt32 fListSize;


};



#endif //_RTCPAPPNADUPACKET_H_
