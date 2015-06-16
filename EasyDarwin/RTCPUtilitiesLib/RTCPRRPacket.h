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
    File:       RTCPRRPacket.h

    Some useful things for Generating Receiver Report Packet
*/

#ifndef _RTCPRRPACKET_H_
#define _RTCPRRPACKET_H_

#include "StrPtrLen.h"
#ifndef __Win32__
#include "arpa/inet.h"
#endif
#include "OSHeaders.h"
#include "MyAssert.h"

class RTCPRRPacket
{
    enum {
       RTP_VERSION = 2,
       RTCP_SR   = 200,
       RTCP_RR   = 201,
       RTCP_SDES = 202,
       RTCP_BYE  = 203,
       RTCP_APP  = 204,
       
       MAX_REPORTS = 0x1F // 5 bits
    };

    struct RTCPRRHeader {
#if 0
        //receiver report header
        unsigned int version:2;     /* protocol version */
        unsigned int p:1;           /* padding flag */
        unsigned int count:5;       /* varies by packet type */
        unsigned int pt:8;          /* RTCP packet type */
#endif
        UInt16 rrheader;
        UInt16 length;           /* packet length in words, minus 1 word */
        UInt32 SSRC;                /* SSRC of packet sender */
    };

    struct RTCPReportBlock {
        UInt32 ssrc;                /* data source being reported */
        unsigned int fraction:8;    /* fraction lost since last SR/RR */
        int lost:24;                /* cumul. no. of pkts lost (signed!) */
        UInt32 last_seq;           /* extended last seq. no. received */
        UInt32 jitter;             /* interarrival jitter */
        UInt32 lsr;                /* last SR packet from this source */
        UInt32 dlsr;               /* delay since last SR packet */
    };

	public:
		RTCPRRPacket() : fNumReportBlocks(0)									{}
        RTCPRRPacket(StrPtrLen &newBuffer) : fNumReportBlocks(0)				{ SetBuffer(newBuffer); }
        RTCPRRPacket(char *newBuffer, UInt32 bufLen) : fNumReportBlocks(0)		{ SetBuffer(newBuffer, bufLen); }

        
		//Setting the buffer resets the packet content
        void    SetBuffer(StrPtrLen &newBuffer)									{ return this->SetBuffer(newBuffer.Ptr, newBuffer.Len); }
        void    SetBuffer(char *newBuffer, UInt32 bufLen)
        {
            Assert(sizeof(RTCPRRHeader) == 8);
			Assert(sizeof(RTCPReportBlock) == 24);
            Assert(bufLen >= sizeof(RTCPRRHeader));

            fBuf.Set(newBuffer, bufLen);
            fNumReportBlocks = 0;

            //fill in the header
            RTCPRRHeader &header = *reinterpret_cast<RTCPRRHeader *>(fBuf.Ptr);
            ::memset(&header, 0, sizeof(header));
            header.rrheader = htons(0x80C9); //(RTP_VERSION << 14) + RTCP_RR;
            header.length = htons(GetPacketLen() / 4 - 1);
        }
		
		void	SetSSRC(UInt32 SSRC)											{ reinterpret_cast<RTCPRRHeader *>(fBuf.Ptr)->SSRC = htonl(SSRC); }
		
		
		void SetCount(UInt16 count) 
		{ 
		  if (count > MAX_REPORTS) //5 bits
		  {   return;
		  }
		  
		  UInt16 newVal = ntohs(*reinterpret_cast<UInt16 *>(fBuf.Ptr));
		  count <<= 8;
		  newVal |= count;
		  *reinterpret_cast<UInt16 *>(fBuf.Ptr) =  htons(newVal);
		  
		}
		
		void	AddReportBlock(UInt32 SSRC, UInt8 fractionLost, SInt32 cumLostPackets, UInt32 highestSeqNum, UInt32 lsr, UInt32 dlsr)
		{
			Assert(fBuf.Len >= GetPacketLen() + sizeof(RTCPReportBlock));
			if (fNumReportBlocks >= MAX_REPORTS)
			{     return;
			}
			
			RTCPReportBlock &reportBlock = *reinterpret_cast<RTCPReportBlock *>(fBuf.Ptr + GetPacketLen());
			::memset(&reportBlock, 0, sizeof(RTCPReportBlock));

			reportBlock.ssrc = htonl(SSRC);
			reportBlock.fraction = fractionLost;
			reportBlock.last_seq = htonl(highestSeqNum);
			reportBlock.lsr = htonl(lsr);
			reportBlock.dlsr = htonl(dlsr);
			
			//since the cumulative packets lost is a 24 bit signed integer, its clamped between 0x7fffff and 0x800000)
			if(cumLostPackets > 0x7fffff)
				reportBlock.lost = htonl(0x7fffff);
			else if (cumLostPackets < static_cast<SInt32>(0xff800000))
				reportBlock.lost = htonl(0x800000);
			else
				reportBlock.lost = htonl(cumLostPackets);
				
			SetCount(++fNumReportBlocks);
			reinterpret_cast<RTCPRRHeader *>(fBuf.Ptr)->length = htons(GetPacketLen() / 4 - 1);
		}

        //The length of packet written out
        UInt32      GetPacketLen()                                              { return sizeof(RTCPRRHeader) + sizeof(RTCPReportBlock) * fNumReportBlocks; }
        StrPtrLen   GetBufferRemaining()										{ return StrPtrLen(fBuf.Ptr + GetPacketLen(), fBuf.Len - GetPacketLen()); }
        StrPtrLen   GetPacket()													{ return StrPtrLen(fBuf.Ptr, GetPacketLen()); }

	private:
        StrPtrLen   fBuf;
        UInt32      fNumReportBlocks;
};

#endif //_RTCPRRPACKET_H_
