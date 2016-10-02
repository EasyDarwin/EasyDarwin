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
    File:       RTCPPacket.cpp

    Contains:   RTCPReceiverPacket de-packetizing classes
    
*/


#include "RTCPPacket.h"
#include "RTCPAckPacket.h"
#include <stdio.h>

#define RTCP_PACKET_DEBUG 0

//returns true if successful, false otherwise
bool RTCPPacket::ParsePacket(UInt8* inPacketBuffer, UInt32 inPacketLen)
{
    if (inPacketLen < kRTCPPacketSizeInBytes)
        return false;
        
    fReceiverPacketBuffer = inPacketBuffer;
    if (RTCP_PACKET_DEBUG) qtss_printf("RTCPPacket::ParsePacket first 4 bytes of packet=%x \n", ntohl( *(UInt32 *)inPacketBuffer));
    
    //the length of this packet can be no less than the advertised length (which is
    //in 32-bit words, so we must multiply) plus the size of the header (4 bytes)
    if (RTCP_PACKET_DEBUG) qtss_printf("RTCPPacket::ParsePacket len=%"   _U32BITARG_   " min allowed=%"   _U32BITARG_   "\n", inPacketLen,(UInt32)((this->GetPacketLength() * 4) + kRTCPHeaderSizeInBytes));
    if (inPacketLen < (UInt32)((this->GetPacketLength() * 4) + kRTCPHeaderSizeInBytes))
    {   if (RTCP_PACKET_DEBUG) qtss_printf("RTCPPacket::ParsePacket invalid len=%"   _U32BITARG_   "\n", inPacketLen);
        return false;
    }
    
    //do some basic validation on the packet
    if (this->GetVersion() != kSupportedRTCPVersion)
    {   if (RTCP_PACKET_DEBUG) qtss_printf("RTCPPacket::ParsePacket unsupported version\n");
        return false;
    }
        
    return true;
}

void RTCPReceiverPacket::Dump()//Override
{
    RTCPPacket::Dump();
    qtss_printf("\n");
    for (int i = 0;i<this->GetReportCount(); i++)
    {
        qtss_printf( "              RTCP RR Report[%d] H_ssrc=%"   _U32BITARG_   ", H_frac_lost=%d, H_tot_lost=%"   _U32BITARG_   ", H_high_seq=%"   _U32BITARG_   " H_jit=%"   _U32BITARG_   ", H_last_sr_time=%"   _U32BITARG_   ", H_last_sr_delay=%"   _U32BITARG_   " \n",
                             i,
                             this->GetReportSourceID(i),
                             this->GetFractionLostPackets(i),
                             this->GetTotalLostPackets(i),
                             this->GetHighestSeqNumReceived(i),
                             this->GetJitter(i),
                             this->GetLastSenderReportTime(i),
                             this->GetLastSenderReportDelay(i) );
    }


}


bool RTCPReceiverPacket::ParseReport(UInt8* inPacketBuffer, UInt32 inPacketLength)
{
    bool ok = this->ParsePacket(inPacketBuffer, inPacketLength);
    if (!ok)
        return false;
    
    fRTCPReceiverReportArray = inPacketBuffer + kRTCPPacketSizeInBytes;
    
    //this is the maximum number of reports there could possibly be
    int theMaxReports = (inPacketLength - kRTCPPacketSizeInBytes) / kReportBlockOffsetSizeInBytes;

    //if the number of receiver reports is greater than the theoretical limit, return an error.
    if (this->GetReportCount() > theMaxReports)
    {  if (RTCP_PACKET_DEBUG) printf("RTCPReceiverPacket::ParseReport this rtcp report count=%d > max reports=%d\n",this->GetReportCount(), theMaxReports);
        return false;
    }
        
    return true;
}

UInt32 RTCPReceiverPacket::GetCumulativeFractionLostPackets()
{
    float avgFractionLost = 0;
    for (short i = 0; i < this->GetReportCount(); i++)
    {
        avgFractionLost += this->GetFractionLostPackets(i);
        avgFractionLost /= (i+1);
    }
    
    return (UInt32)avgFractionLost;
}


UInt32 RTCPReceiverPacket::GetCumulativeJitter()
{
    float avgJitter = 0;
    for (short i = 0; i < this->GetReportCount(); i++)
    {
        avgJitter += this->GetJitter(i);
        avgJitter /= (i + 1);
    }
    
    return (UInt32)avgJitter;
}


UInt32 RTCPReceiverPacket::GetCumulativeTotalLostPackets()
{
    UInt32 totalLostPackets = 0;
    for (short i = 0; i < this->GetReportCount(); i++)
    {
        totalLostPackets += this->GetTotalLostPackets(i);
    }
    
    return totalLostPackets;
}


bool RTCPSenderReportPacket::ParseReport(UInt8* inPacketBuffer, UInt32 inPacketLength)
{
    bool ok = this->ParsePacket(inPacketBuffer, inPacketLength);
    if (!ok)
        return false;
    if (inPacketLength < kRTCPPacketSizeInBytes + kRTCPSRPacketSenderInfoInBytes)
		return false;
    
	fRTCPReceiverReportArray = inPacketBuffer + kRTCPPacketSizeInBytes + kRTCPSRPacketSenderInfoInBytes;
    
    //this is the maximum number of reports there could possibly be
    int theNumReports = (inPacketLength - kRTCPPacketSizeInBytes - kRTCPSRPacketSenderInfoInBytes) / kReportBlockOffsetSizeInBytes;

    //if the number of receiver reports is greater than the theoretical limit, return an error.
    if (this->GetReportCount() > theNumReports)
        return false;
        
    return true;
}


void RTCPPacket::Dump()
{  
    qtss_printf( "H_vers=%d, H_pad=%d, H_rprt_count=%d, H_type=%d, H_length=%d, H_ssrc=%" _S32BITARG_ "",
             this->GetVersion(),
             (int)this->GetHasPadding(),
             this->GetReportCount(),
             (int)this->GetPacketType(),
             (int)this->GetPacketLength(),
             this->GetPacketSSRC() );
}


