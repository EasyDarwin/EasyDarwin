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
    File:       RTCPAPPNADUPacketFmt.h

    Some useful things for Generating a 3GPP NADU packet
*/

#ifndef _RTCPNADUPACKETFMT_H_
#define _RTCPNADUPACKETFMT_H_

#include "StrPtrLen.h"
#ifndef __Win32__
#include "arpa/inet.h"
#endif
#include "OSHeaders.h"
#include "MyAssert.h"

class RTCPNADUPacketFmt
{
    enum {
       RTP_VERSION = 2,
       RTCP_SR   = 200,
       RTCP_RR   = 201,
       RTCP_SDES = 202,
       RTCP_BYE  = 203,
       RTCP_APP  = 204
    };

    struct NADUHeader {
#if 0     
        //naduheader
        unsigned int version:2;     /* protocol version */
        unsigned int p:1;           /* padding flag */
        unsigned int subtype:5;     /* 0 */
        unsigned int pt:8;          /* RTCP packet type -- which should be RTCP_APP */
#endif

        UInt16 naduHeader;
        UInt16 length;           /* packet length in words, minus 1 word */
        UInt32 SSRC;                /* SSRC of packet sender */
		UInt32 name;				/* in ASCII */
    };

    struct NADUBlock {
        UInt32 SSRC;                /* data source being reported */
		UInt16 delay;				/* the playout delay, in milliseconds */
		UInt16 NSN;					/* the RTP sequence number of the next ADU to be decoded; if the buffer is empty then the next not yet received sequence number */
		UInt16 NUN;			        /* the unit number of the next ADU to be decoded */
		UInt16 FBS;		            /* the free buffer space, in complete 64 byte blocks */
    };

	public:
		RTCPNADUPacketFmt() : fNumNADUBlocks(0)										{}
        RTCPNADUPacketFmt(StrPtrLen &newBuffer) : fNumNADUBlocks(0)					{ SetBuffer(newBuffer); }
        RTCPNADUPacketFmt(char *newBuffer, UInt32 bufLen) : fNumNADUBlocks(0)		{ SetBuffer(newBuffer, bufLen); }

		//Setting the buffer resets the packet content
        void    SetBuffer(StrPtrLen &newBuffer)                                 { return this->SetBuffer(newBuffer.Ptr, newBuffer.Len); }
        void    SetBuffer(char *newBuffer, UInt32 bufLen)
        {
            Assert(sizeof(NADUHeader) == 12);
			Assert(sizeof(NADUBlock) == 12);
            Assert(bufLen >= sizeof(NADUHeader));

            fBuf.Set(newBuffer, bufLen);
            fNumNADUBlocks = 0;

            //fill in the header
            NADUHeader &header = *reinterpret_cast<NADUHeader *>(fBuf.Ptr);
            ::memset(&header, 0, sizeof(header));
            header.naduHeader = htons(0x80CC); //(RTP_VERSION << 14) + RTCP_APP; 
  			header.name = htonl(FOUR_CHARS_TO_INT('P', 'S', 'S', '0'));
            header.length = htons(GetPacketLen() / 4 - 1);
        }
		
		//units are in milliseconds and in bytes; use a playoutDelay of kUInt32_Max if the buffer is empty
		void	AddNADUBlock(UInt32 SSRC, UInt32 nextSeqNum, UInt8 nextUnitNum, UInt32 freeBufferSpace, UInt32 playoutDelay = kUInt32_Max)
		{
			Assert(fBuf.Len >= GetPacketLen() + sizeof(NADUBlock));
            NADUBlock &nadu = *reinterpret_cast<NADUBlock *>(fBuf.Ptr + GetPacketLen());
            ::memset(&nadu, 0, sizeof(NADUBlock));
			
			fNumNADUBlocks++;
			reinterpret_cast<NADUHeader *>(fBuf.Ptr)->length = htons(GetPacketLen() / 4 - 1);

			nadu.SSRC = htonl(SSRC);
			nadu.NSN = htons(static_cast<UInt16>(nextSeqNum));
			nadu.NUN = htons(nextUnitNum & 0x1F);

			//Use reserved value of 0xffff for undefined
			playoutDelay = MIN(0xffff, playoutDelay);
			nadu.delay = htons(static_cast<UInt16>(playoutDelay));

			//the free buffer space is reported in 64 bytes blocks, and maximum value is 0xffff
			freeBufferSpace = MIN(0xffff, freeBufferSpace / 64);
			nadu.FBS = htons(static_cast<UInt16>(freeBufferSpace));
		}

		void		SetSSRC(UInt32 SSRC)										{ reinterpret_cast<NADUHeader *>(fBuf.Ptr)->SSRC = htonl(SSRC); }

        //The length of packet written out
        UInt32      GetPacketLen()                                              { return sizeof(NADUHeader) + sizeof(NADUBlock) * fNumNADUBlocks; }
        StrPtrLen   GetBufferRemaining()										{ return StrPtrLen(fBuf.Ptr + GetPacketLen(), fBuf.Len - GetPacketLen()); }
        StrPtrLen   GetPacket()													{ return StrPtrLen(fBuf.Ptr, GetPacketLen()); }

	private:
        StrPtrLen   fBuf;
        UInt32      fNumNADUBlocks;
};

#endif //_RTCPNADUPACKETFMT_H_
