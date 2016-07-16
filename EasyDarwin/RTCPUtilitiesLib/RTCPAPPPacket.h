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
	 File:       RTCPAPPPacket.h

	 Contains:   RTCPAPPPacket de-packetizing classes



 */

#ifndef _RTCPAPPPACKET_H_
#define _RTCPAPPPACKET_H_

#include "RTCPPacket.h"
#include "StrPtrLen.h"
#include "ResizeableStringFormatter.h"

#define APPEND_TO_DUMP_ARRAY(f, v) {if (fDebug && mDumpArray != NULL) { (void)qtss_snprintf(mDumpArray,kmDumpArraySize, f, v); fDumpReport.Put(mDumpArray); }   }

class RTCPAPPPacket : public RTCPPacket
{

public:
	RTCPAPPPacket(Bool16 debug = false);
	virtual ~RTCPAPPPacket() {};
	virtual void Dump();
	virtual Bool16 ParseAPPPacket(UInt8* inPacketBuffer, UInt32 inPacketLength); //default app header check
	virtual Bool16 ParseAPPData(UInt8* inPacketBuffer, UInt32 inPacketLength) { return false; }; //derived class implements
	inline FourCharCode GetAppPacketName(char *outName = NULL, UInt32 len = 0);
	inline UInt32 GetAppPacketSSRC();


	UInt8* fRTCPAPPDataBuffer;  //points into RTCPPacket::fReceiverPacketBuffer should be set past the app header
	UInt32 fAPPDataBufferSize;

	enum
	{
		kAppSSRCOffset = 4,
		kAppNameOffset = 8, //byte offset to four char App identifier               //All are UInt32

		kRTCPAPPHeaderSizeInBytes = 4, //
		kmDumpArraySize = 1024
	};

	char*           mDumpArray;
	StrPtrLenDel    mDumpArrayStrDeleter;
	ResizeableStringFormatter fDumpReport;
	Bool16 fDebug;

private:
	virtual Bool16 ParseAPPPacketHeader(UInt8* inPacketBuffer, UInt32 inPacketLength);

};



/****************  RTCPAPPPacket inlines *******************************/

inline FourCharCode RTCPAPPPacket::GetAppPacketName(char *outName, UInt32 len)
{

	UInt32 packetName = (UInt32)(*(UInt32*)&(GetPacketBuffer()[kAppNameOffset]));

	if (outName)
	{
		if (len > 4)
		{
			*((UInt32*)outName) = packetName;
			outName[4] = 0;
		}
		else if (len > 0)
			outName[0] = 0;
	}

	return ntohl(packetName);
}


inline UInt32 RTCPAPPPacket::GetAppPacketSSRC()
{
	return (UInt32)ntohl(*(UInt32*)&(GetPacketBuffer()[kAppSSRCOffset]));
}





/*
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

 */

#endif //_RTCPAPPPACKET_H_
