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
    File:       RTCPAckPacketFmt.h

    Some useful things for Generating Ack Packets
*/

#ifndef _RTCPACKPACKETFMT_H_
#define _RTCPACKPACKETFMT_H_

#include <stdlib.h>
#ifndef __Win32__
#include "arpa/inet.h"
#endif
#include "StrPtrLen.h"
#include "OSHeaders.h"
#include "MyAssert.h"

class RTCPAckPacketFmt
{
    enum {
       RTP_VERSION = 2,
       RTCP_SR   = 200,
       RTCP_RR   = 201,
       RTCP_SDES = 202,
       RTCP_BYE  = 203,
       RTCP_APP  = 204
    };

    struct RTCPAckHeader {
    
#if 0  
        //ackheader
        unsigned int version:2;             /* protocol version */
        unsigned int p:1;                   /* padding flag */
        unsigned int subtype:5;             /* the subtype of ack */
        unsigned int pt:8;                  /* RTCP packet type */
#endif

        UInt16 ackheader;
        UInt16 length;                      /* pkt len in words, w/o this word */
        UInt32 SSRC;                        /* sender generating this packet */
        UInt32 name;                        /* four ASCII chars */
        UInt32 SSRC1;                       /* SSRC of the stream */
        UInt16 reserved;
        UInt16 seqNum;
    };

	public:
        RTCPAckPacketFmt() : fBitMaskSize(0)                                    {}
        RTCPAckPacketFmt(StrPtrLen &newBuffer) : fBitMaskSize(0)                { SetBuffer(newBuffer); }
        RTCPAckPacketFmt(char *newBuffer, UInt32 bufLen) : fBitMaskSize(0)      { SetBuffer(newBuffer, bufLen); }

		//Setting the buffer resets the packet
        void    SetBuffer(StrPtrLen &newBuffer)                                 { return this->SetBuffer(newBuffer.Ptr, newBuffer.Len); }
        void    SetBuffer(char *newBuffer, UInt32 bufLen)
        {
            Assert(sizeof(RTCPAckHeader) == 20);
            Assert(bufLen >= sizeof(RTCPAckHeader));

			fBitMaskSize = 0;
            fBuf.Set(newBuffer, bufLen);

            //fill in the header
            RTCPAckHeader &header = *reinterpret_cast<RTCPAckHeader *>(fBuf.Ptr);
            ::memset(&header, 0, sizeof(header));    
            header.ackheader = htons(0x80CC); //(RTP_VERSION << 14) + RTCP_APP; 
			header.length = htons(GetPacketLen() / 4 - 1);
            header.name = htonl(FOUR_CHARS_TO_INT('q', 't', 'a', 'k'));
        }
		
		void		SetSSRC(UInt32 SSRC)
        {
            RTCPAckHeader &header = *reinterpret_cast<RTCPAckHeader *>(fBuf.Ptr);
            header.SSRC = htonl(SSRC);
        }

        //Can handle duplicates
        void        SetAcks(SVector<UInt32> AckList, UInt32 serverSSRC)
        {
            Assert(!AckList.empty());
            ::qsort(AckList.begin(), AckList.size(), sizeof(UInt32), UInt32Compare);

            RTCPAckHeader &header = *reinterpret_cast<RTCPAckHeader *>(fBuf.Ptr);
			header.SSRC1 = htonl(serverSSRC);
            header.seqNum = htons(static_cast<UInt16>(AckList.front()));

            fBitMaskSize = 0;
            if (AckList.front() == AckList.back())          //no mask is needed
                return;

            //figure out how big the mask is supposed to be
			UInt32 slotsInMaskNeeded = AckList.back() - AckList.front();
			fBitMaskSize = slotsInMaskNeeded % 32 == 0 ? (slotsInMaskNeeded / 32) * 4 : (slotsInMaskNeeded / 32 + 1) * 4;
			
			header.length = htons(GetPacketLen() / 4 - 1);
            Assert(fBuf.Len >= GetPacketLen());

            UInt32 *mask = reinterpret_cast<UInt32 *>(fBuf.Ptr + sizeof(RTCPAckHeader));
            ::memset(mask, 0, fBitMaskSize);

            //calculate where the bit to set is supposed to be and sets the bits
            for(UInt32 i = 1; i < AckList.size(); ++i)
            {
                if (AckList.front() == AckList[i])
                    continue;

                UInt32 diff = AckList[i] - (AckList.front() + 1);
                UInt32 maskIndex = diff / 32;
                UInt32 shiftSize = diff % 32;
                mask[maskIndex] |= 0x80000000 >> shiftSize;
				Assert(maskIndex * 4 < fBitMaskSize);
            }
			
			//restore big-endianess of the mask
			for(UInt32 i = 0; i < fBitMaskSize / 4; ++i)
				mask[i] = htonl(mask[i]);
        }

        //The length of packet written out
        UInt32      GetPacketLen()                                              { return sizeof(RTCPAckHeader) + fBitMaskSize; }
        StrPtrLen   GetBufferRemaining()										{ return StrPtrLen(fBuf.Ptr + GetPacketLen(), fBuf.Len - GetPacketLen()); }
        StrPtrLen   GetPacket()													{ return StrPtrLen(fBuf.Ptr, GetPacketLen()); }

	private:
        static int UInt32Compare(const void *left, const void *right)
        {
            UInt32 l = *reinterpret_cast<const UInt32 *>(left);
            UInt32 r = *reinterpret_cast<const UInt32 *>(right);
            return l < r ? -1 : l > r ? 1 : 0;
        }

        StrPtrLen   fBuf;
        UInt32      fBitMaskSize;               // in bytes, not words
};

#endif //_RTCPACKPACKETFMT_H_
