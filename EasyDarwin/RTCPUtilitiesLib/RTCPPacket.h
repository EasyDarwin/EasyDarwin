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
	File:       RTCPPacket.h

	Contains:   RTCPReceiverPacket de-packetizing classes


*/

//#define DEBUG_RTCP_PACKETS 1


#ifndef _RTCPPACKET_H_
#define _RTCPPACKET_H_

#include <stdlib.h>
#ifndef __Win32__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include "OSHeaders.h"

class RTCPPacket
{
public:

	// Packet types
	enum
	{
		kReceiverPacketType = 201,  //UInt32
		kSDESPacketType = 202,  //UInt32
		kAPPPacketType = 204   //UInt32
	};


	RTCPPacket() : fReceiverPacketBuffer(NULL) {}
	virtual ~RTCPPacket() {}

	//Call this before any accessor method. Returns true if successful, false otherwise
	bool ParsePacket(UInt8* inPacketBuffer, UInt32 inPacketLen);

	inline int GetVersion();
	inline bool GetHasPadding();
	inline int GetReportCount();
	inline UInt8 GetPacketType();
	inline UInt16 GetPacketLength();    //in 32-bit words
	inline UInt32 GetPacketSSRC();
	inline SInt16 GetHeader();
	UInt8* GetPacketBuffer() { return fReceiverPacketBuffer; }

	//bool IsValidPacket();

	virtual void Dump();

	enum
	{
		kRTCPPacketSizeInBytes = 8,     //All are UInt32s
		kRTCPHeaderSizeInBytes = 4
	};

protected:

	UInt8* fReceiverPacketBuffer;

	enum
	{
		kVersionOffset = 0,
		kVersionMask = 0xC0000000UL,
		kVersionShift = 30,
		kHasPaddingOffset = 0,
		kHasPaddingMask = 0x20000000UL,
		kReportCountOffset = 0,
		kReportCountMask = 0x1F000000UL,
		kReportCountShift = 24,
		kPacketTypeOffset = 0,
		kPacketTypeMask = 0x00FF0000UL,
		kPacketTypeShift = 16,
		kPacketLengthOffset = 0,
		kPacketLengthMask = 0x0000FFFFUL,
		kPacketSourceIDOffset = 4,  //packet sender SSRC
		kPacketSourceIDSize = 4,    //
		kSupportedRTCPVersion = 2
	};

};




class SourceDescriptionPacket : public RTCPPacket

{

public:

	SourceDescriptionPacket() : RTCPPacket() {}

	bool ParseSourceDescription(UInt8* inPacketBuffer, UInt32 inPacketLength)
	{
		return ParsePacket(inPacketBuffer, inPacketLength);
	}

private:
};




class RTCPReceiverPacket : public RTCPPacket
{
public:

	RTCPReceiverPacket() : RTCPPacket(), fRTCPReceiverReportArray(NULL) {}

	//Call this before any accessor method. Returns true if successful, false otherwise
	virtual bool ParseReport(UInt8* inPacketBuffer, UInt32 inPacketLength);

	inline UInt32 GetReportSourceID(int inReportNum);
	UInt8 GetFractionLostPackets(int inReportNum);
	UInt32 GetTotalLostPackets(int inReportNum);
	inline UInt32 GetHighestSeqNumReceived(int inReportNum);
	inline UInt32 GetJitter(int inReportNum);
	inline UInt32 GetLastSenderReportTime(int inReportNum);
	inline UInt32 GetLastSenderReportDelay(int inReportNum);    //expressed in units of 1/65536 seconds

	UInt32 GetCumulativeFractionLostPackets();
	UInt32 GetCumulativeTotalLostPackets();
	UInt32 GetCumulativeJitter();

	//bool IsValidPacket();

	virtual void Dump(); //Override

protected:
	inline int RecordOffset(int inReportNum);

	UInt8* fRTCPReceiverReportArray;    //points into fReceiverPacketBuffer

	enum
	{
		kReportBlockOffsetSizeInBytes = 24,     //All are UInt32s

		kReportBlockOffset = kPacketSourceIDOffset + kPacketSourceIDSize,

		kReportSourceIDOffset = 0,  //SSRC for this report
		kFractionLostOffset = 4,
		kFractionLostMask = 0xFF000000UL,
		kFractionLostShift = 24,
		kTotalLostPacketsOffset = 4,
		kTotalLostPacketsMask = 0x00FFFFFFUL,
		kHighestSeqNumReceivedOffset = 8,
		kJitterOffset = 12,
		kLastSenderReportOffset = 16,
		kLastSenderReportDelayOffset = 20
	};
};

class RTCPSenderReportPacket : public RTCPReceiverPacket
{
public:
	bool ParseReport(UInt8* inPacketBuffer, UInt32 inPacketLength);
	SInt64 GetNTPTimeStamp()
	{
		UInt32* fieldPtr = (UInt32*)&fReceiverPacketBuffer[kSRPacketNTPTimeStampMSW];
		SInt64 timestamp = ntohl(*fieldPtr);
		fieldPtr = (UInt32*)&fReceiverPacketBuffer[kSRPacketNTPTimeStampLSW];
		return (timestamp << 32) | ntohl(*fieldPtr);
	}
	UInt32 GetRTPTimeStamp()
	{
		UInt32* fieldPtr = (UInt32*)&fReceiverPacketBuffer[kSRPacketRTPTimeStamp];
		return ntohl(*fieldPtr);
	}
protected:
	enum
	{
		kRTCPSRPacketSenderInfoInBytes = 20,
		kSRPacketNTPTimeStampMSW = 8,
		kSRPacketNTPTimeStampLSW = 12,
		kSRPacketRTPTimeStamp = 16
	};
};


/**************  RTCPPacket  inlines **************/
inline int RTCPPacket::GetVersion()
{
	UInt32* theVersionPtr = (UInt32*)&fReceiverPacketBuffer[kVersionOffset];
	UInt32 theVersion = ntohl(*theVersionPtr);
	return (int)((theVersion  & kVersionMask) >> kVersionShift);
}

inline bool RTCPPacket::GetHasPadding()
{
	UInt32* theHasPaddingPtr = (UInt32*)&fReceiverPacketBuffer[kHasPaddingOffset];
	UInt32 theHasPadding = ntohl(*theHasPaddingPtr);
	return (bool)(theHasPadding & kHasPaddingMask);
}

inline int RTCPPacket::GetReportCount()
{
	UInt32* theReportCountPtr = (UInt32*)&fReceiverPacketBuffer[kReportCountOffset];
	UInt32 theReportCount = ntohl(*theReportCountPtr);
	return (int)((theReportCount & kReportCountMask) >> kReportCountShift);
}

inline UInt8 RTCPPacket::GetPacketType()
{
	UInt32* thePacketTypePtr = (UInt32*)&fReceiverPacketBuffer[kPacketTypeOffset];
	UInt32 thePacketType = ntohl(*thePacketTypePtr);
	return (UInt8)((thePacketType & kPacketTypeMask) >> kPacketTypeShift);
}

inline UInt16 RTCPPacket::GetPacketLength()
{
	UInt32* fieldPtr = (UInt32*)&fReceiverPacketBuffer[kPacketLengthOffset];
	UInt32 field = ntohl(*fieldPtr);
	return (UInt16)(field & kPacketLengthMask);
}

inline UInt32 RTCPPacket::GetPacketSSRC()
{
	UInt32* fieldPtr = (UInt32*)&fReceiverPacketBuffer[kPacketSourceIDOffset];
	UInt32 field = ntohl(*fieldPtr);
	return field;
}

inline SInt16 RTCPPacket::GetHeader() { return (SInt16)ntohs(*(SInt16*)&fReceiverPacketBuffer[0]); }

/**************  RTCPReceiverPacket  inlines **************/
inline int RTCPReceiverPacket::RecordOffset(int inReportNum)
{
	return inReportNum*kReportBlockOffsetSizeInBytes;
}


inline UInt32 RTCPReceiverPacket::GetReportSourceID(int inReportNum)
{
	return (UInt32)ntohl(*(UInt32*)&fRTCPReceiverReportArray[this->RecordOffset(inReportNum) + kReportSourceIDOffset]);
}

inline UInt8 RTCPReceiverPacket::GetFractionLostPackets(int inReportNum)
{
	return (UInt8)((ntohl(*(UInt32*)&fRTCPReceiverReportArray[this->RecordOffset(inReportNum) + kFractionLostOffset]) & kFractionLostMask) >> kFractionLostShift);
}


inline UInt32 RTCPReceiverPacket::GetTotalLostPackets(int inReportNum)
{
	return (ntohl(*(UInt32*)&fRTCPReceiverReportArray[this->RecordOffset(inReportNum) + kTotalLostPacketsOffset]) & kTotalLostPacketsMask);
}


inline UInt32 RTCPReceiverPacket::GetHighestSeqNumReceived(int inReportNum)
{
	return (UInt32)ntohl(*(UInt32*)&fRTCPReceiverReportArray[this->RecordOffset(inReportNum) + kHighestSeqNumReceivedOffset]);
}

inline UInt32 RTCPReceiverPacket::GetJitter(int inReportNum)
{
	return (UInt32)ntohl(*(UInt32*)&fRTCPReceiverReportArray[this->RecordOffset(inReportNum) + kJitterOffset]);
}


inline UInt32 RTCPReceiverPacket::GetLastSenderReportTime(int inReportNum)
{
	return (UInt32)ntohl(*(UInt32*)&fRTCPReceiverReportArray[this->RecordOffset(inReportNum) + kLastSenderReportOffset]);
}


inline UInt32 RTCPReceiverPacket::GetLastSenderReportDelay(int inReportNum)
{
	return (UInt32)ntohl(*(UInt32*)&fRTCPReceiverReportArray[this->RecordOffset(inReportNum) + kLastSenderReportDelayOffset]);
}


/*
Receiver Report
---------------
 0                   1                   2                   3
 0 0 0 1 1 1 1 1
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|    RC   |   PT=RR=201   |             length            | header
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     SSRC of packet sender                     |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                 SSRC_1 (SSRC of first source)                 | report
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ block
| fraction lost |       cumulative number of packets lost       |   1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           extended highest sequence number received           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      interarrival jitter                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         last SR (LSR)                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   delay since last SR (DLSR)                  |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                 SSRC_2 (SSRC of second source)                | report
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ block
:                               ...                             :   2
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                  profile-specific extensions                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+



*/

#endif //_RTCPPACKET_H_
