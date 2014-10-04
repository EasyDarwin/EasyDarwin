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
    File:       RTPPacket.h

    Some useful things for parsing RTPPackets.
*/
#ifndef _RTPPACKET_H_
#define _RTPPACKET_H_
#ifndef __Win32__
#include "arpa/inet.h"
#endif
#include "OSHeaders.h"
#include "StrPtrLen.h"

class RTPPacket
{
    public:
        enum {
           RTP_VERSION = 2,

           RTCP_SR   = 200,
           RTCP_RR   = 201,
           RTCP_SDES = 202,
           RTCP_BYE  = 203,
           RTCP_APP  = 204
        };

        /*
         * RTP data header
         */
        struct RTPHeader {
 #if 0
           //rtp header
           unsigned int version:2;          /* protocol version */
           unsigned int p:1;                /* padding flag */
           unsigned int x:1;                /* header extension flag */
           unsigned int cc:4;               /* CSRC count */
           unsigned int m:1;                /* marker bit */
           unsigned int pt:7;               /* payload type */
 #endif
           UInt16 rtpheader;
		   UInt16 seq;				        /* sequence number */
           UInt32 ts;                       /* timestamp */
           UInt32 ssrc;                     /* synchronization source */
           //UInt32 csrc[1];                /* optional CSRC list */
        };
        
        RTPPacket(StrPtrLen inPacket)
        :   fPacket(reinterpret_cast<RTPHeader *>(inPacket.Ptr)), fLen(inPacket.Len)
        {}
        RTPPacket(char *inPacket = NULL, UInt32 inLen = NULL)
        :   fPacket(reinterpret_cast<RTPHeader *>(inPacket)), fLen(inLen)
        {}

		UInt8		GetPayloadType() const									{ return ntohs(fPacket->rtpheader) & 0x007F; }
		UInt8		GetCSRCCount() const									{ return (ntohs(fPacket->rtpheader) & 0x0F00 ) >> 8; }

        //The following get functions will convert from network byte order to host byte order.
        //Conversely the set functions will convert from host byte order to network byte order.
        UInt16		GetSeqNum() const                                       { return ntohs(fPacket->seq); }
        void		SetSeqNum(UInt16 seqNum)                                { fPacket->seq = htons(seqNum); }

        UInt32		GetTimeStamp() const                                    { return ntohl(fPacket->ts); }
        void		SetTimeStamp(UInt32 timeStamp)                          { fPacket->ts = htonl(timeStamp); }

        UInt32		GetSSRC() const                                         { return ntohl(fPacket->ssrc); }
        void		SetSSRC(UInt32 SSRC)                                    { fPacket->ssrc = htonl(SSRC); }
		
		//Includes the variable CSRC portion
		UInt32		GetHeaderLen() const									{ return sizeof(RTPHeader) + GetCSRCCount() * 4; }
		
		StrPtrLen	GetBody() const											{ return StrPtrLen(reinterpret_cast<char *>(fPacket) + GetHeaderLen(), fLen - GetHeaderLen()); }

        //Returns true if the header is not bad; do some very basic checking
        Bool16  HeaderIsValid() const
        {
			Assert(sizeof(RTPHeader) == 12);
            if (fLen < sizeof(RTPHeader))
                return false; 
            if ( ( ntohs(fPacket->rtpheader) >> 14)  != RTP_VERSION )
                return false;
			if (GetHeaderLen() > fLen)
				return false;
            return true;
        }

        RTPHeader * fPacket;
        UInt32      fLen;				//total length of the packet, including the header
};

#endif
