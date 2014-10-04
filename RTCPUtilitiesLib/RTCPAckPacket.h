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
    File:       RTCPAckPacket.h

    Contains:   RTCPAckPacket de-packetizing class


    
*/

#ifndef _RTCPACKPACKET_H_
#define _RTCPACKPACKET_H_

#include "OSHeaders.h"
#include "RTCPAPPPacket.h"
#include <stdlib.h>
#include "SafeStdLib.h"
#ifndef __Win32__
#include <netinet/in.h>
#endif

class RTCPAckPacket  : public RTCPAPPPacket
{
    public:
    
/*

        RTCP app ACK packet

        # bytes   description
        -------   -----------
        4         rtcp header
        4         SSRC of receiver
        4         app type ('qtak')
        2         reserved (set to 0)
        2         seqNum

*/

        //
        // This class is not derived from RTCPPacket as a performance optimization.
        // Instead, it is assumed that the RTCP packet validation has already been
        // done.
        RTCPAckPacket() : fRTCPAckBuffer(NULL), fAckMaskSize(0) {}
        virtual ~RTCPAckPacket() {}
        
        // Call to parse if you don't know what kind of packet this is
        // Returns true if this is an Ack packet, false otherwise.
        // Assumes that inPacketBuffer is a pointer to a valid RTCP packet header.
        Bool16  ParseAckPacket(UInt8* inPacketBuffer, UInt32 inPacketLength);

        virtual Bool16 ParseAPPData(UInt8* inPacketBuffer, UInt32 inPacketLength);

        inline UInt16 GetAckSeqNum();
        inline UInt32 GetAckMaskSizeInBits() { return fAckMaskSize * 8; }
        inline Bool16 IsNthBitEnabled(UInt32 inBitNumber);
        inline UInt16 GetPacketLength();
        void   Dump();
        static void GetTestPacket(StrPtrLen* resultPtr) {} //todo
        
        enum
        {
            kAckPacketName = FOUR_CHARS_TO_INT('q', 't', 'a', 'k'), // 'qtak'  documented Apple reliable UDP packet type
            kAckPacketAlternateName = FOUR_CHARS_TO_INT('a', 'c', 'k', ' '), // 'ack ' required by QT 5 and earlier
        };                  
    private:
    
        UInt8* fRTCPAckBuffer;
        UInt32 fAckMaskSize;

        Bool16 IsAckPacketType();
       
        enum
        {
            kAppPacketTypeOffset    = 8,
            kAckSeqNumOffset        = 16,
            kAckMaskOffset          = 20,
            kPacketLengthMask = 0x0000FFFFUL,
        };                  
 
        

        inline Bool16 IsAckType(UInt32 theAppType) {    return ( (theAppType == kAckPacketAlternateName) || (theAppType == kAckPacketName) );}
};


Bool16 RTCPAckPacket::IsNthBitEnabled(UInt32 inBitNumber)
{
    // Don't need to do endian conversion because we're dealing with 8-bit numbers
    UInt8 bitMask = 128;
    return *(fRTCPAckBuffer + kAckMaskOffset + (inBitNumber >> 3)) & (bitMask >>= inBitNumber & 7);
}

UInt16 RTCPAckPacket::GetAckSeqNum()
{
    return (UInt16) (ntohl(*(UInt32*)&fRTCPAckBuffer[kAckSeqNumOffset]));
}

inline UInt16 RTCPAckPacket::GetPacketLength()
{
    return (UInt16) ( ntohl(*(UInt32*)fRTCPAckBuffer) & kPacketLengthMask);
}




/*
6.6 Ack Packet format

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P| subtype |   PT=APP=204  |             length            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           SSRC/CSRC                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                          name (ASCII)  = 'qtak'               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           SSRC/CSRC                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          Reserved             |          Seq num              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                        Mask...                                |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   
 */

#endif //_RTCPAPPPACKET_H_
