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
	 File:       RTCPSRPacket.h

	 Contains:   A class that writes a RTCP Sender Report

	 Change History (most recent first):


 */

#ifndef __RTCP_SR_PACKET__
#define __RTCP_SR_PACKET__

#include "OSHeaders.h"
#include "OS.h"
#include "MyAssert.h"

#ifndef __Win32__
#include <netinet/in.h> //definition of htonl
#endif

class RTCPSRPacket
{
public:

	enum
	{
		kSRPacketType = 200,    //UInt32
		kByePacketType = 203
	};

	RTCPSRPacket();
	~RTCPSRPacket() {}

	// ACCESSORS

	void*   GetSRPacket() { return &fSenderReportBuffer[0]; }
	UInt32  GetSRPacketLen() { return fSenderReportWithServerInfoSize; }
	UInt32  GetSRWithByePacketLen() { return fSenderReportWithServerInfoSize + kByeSizeInBytes; }

	void*   GetServerInfoPacket() { return &fSenderReportBuffer[fSenderReportSize]; }
	UInt32  GetServerInfoPacketLen() { return kServerInfoSizeInBytes; }

	//
	// MODIFIERS

	//
	// FOR SR
	inline void SetSSRC(UInt32 inSSRC);
	inline void SetClientSSRC(UInt32 inClientSSRC);

	inline void SetNTPTimestamp(SInt64 inNTPTimestamp);
	inline void SetRTPTimestamp(UInt32 inRTPTimestamp);

	inline void SetPacketCount(UInt32 inPacketCount);
	inline void SetByteCount(UInt32 inByteCount);

	//
	// FOR SERVER INFO APP PACKET
	inline void SetAckTimeout(UInt32 inAckTimeoutInMsec);

	//RTCP support requires generating unique CNames for each session.
	//This function generates a proper cName and returns its length. The buffer
	//passed in must be at least kMaxCNameLen
	enum
	{
		kMaxCNameLen = 60   //Uint32
	};
	static UInt32           GetACName(char* ioCNameBuffer);

private:

	enum
	{
		kSenderReportSizeInBytes = 36,
		kServerInfoSizeInBytes = 28,
		kByeSizeInBytes = 8
	};
	char        fSenderReportBuffer[kSenderReportSizeInBytes + kMaxCNameLen + kServerInfoSizeInBytes + kByeSizeInBytes];
	UInt32      fSenderReportSize;
	UInt32      fSenderReportWithServerInfoSize;

};

inline void RTCPSRPacket::SetSSRC(UInt32 inSSRC)
{
	// Set SSRC in SR
	((UInt32*)&fSenderReportBuffer)[1] = htonl(inSSRC);

	// Set SSRC in SDES
	((UInt32*)&fSenderReportBuffer)[8] = htonl(inSSRC);

	// Set SSRC in SERVER INFO
	Assert((fSenderReportSize & 3) == 0);
	((UInt32*)&fSenderReportBuffer)[(fSenderReportSize >> 2) + 1] = htonl(inSSRC);

	// Set SSRC in BYE
	Assert((fSenderReportWithServerInfoSize & 3) == 0);
	((UInt32*)&fSenderReportBuffer)[(fSenderReportWithServerInfoSize >> 2) + 1] = htonl(inSSRC);
}

inline void RTCPSRPacket::SetClientSSRC(UInt32 inClientSSRC)
{
	//
	// Set Client SSRC in SERVER INFO
	((UInt32*)&fSenderReportBuffer)[(fSenderReportSize >> 2) + 3] = htonl(inClientSSRC);
}

inline void RTCPSRPacket::SetNTPTimestamp(SInt64 inNTPTimestamp)
{
#if ALLOW_NON_WORD_ALIGN_ACCESS
	((SInt64*)&fSenderReportBuffer)[1] = OS::HostToNetworkSInt64(inNTPTimestamp);
#else
	SInt64 temp = OS::HostToNetworkSInt64(inNTPTimestamp);
	::memcpy(&((SInt64*)&fSenderReportBuffer)[1], &temp, sizeof(temp));
#endif
}

inline void RTCPSRPacket::SetRTPTimestamp(UInt32 inRTPTimestamp)
{
	((UInt32*)&fSenderReportBuffer)[4] = htonl(inRTPTimestamp);
}

inline void RTCPSRPacket::SetPacketCount(UInt32 inPacketCount)
{
	((UInt32*)&fSenderReportBuffer)[5] = htonl(inPacketCount);
}

inline void RTCPSRPacket::SetByteCount(UInt32 inByteCount)
{
	((UInt32*)&fSenderReportBuffer)[6] = htonl(inByteCount);
}

inline void RTCPSRPacket::SetAckTimeout(UInt32 inAckTimeoutInMsec)
{
	((UInt32*)&fSenderReportBuffer)[(fSenderReportWithServerInfoSize >> 2) - 1] = htonl(inAckTimeoutInMsec);
}

#endif //__RTCP_SR_PACKET__
