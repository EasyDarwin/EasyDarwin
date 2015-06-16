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
    File:       RTCPAPPNADUPacket.cpp

    Contains:   RTCPAPPNADUPacket de-packetizing classes

    
*/


#include "RTCPAPPNADUPacket.h"
#include "MyAssert.h"
#include "OS.h"
#include "OSMemory.h"
#include "StrPtrLen.h"





/* RTCPNaduPacket
data: One or more of the following data format blocks may appear

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P| subtype |   PT=APP=204  |             length            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           SSRC/CSRC                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          name (ASCII)                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <-------- data block
|                            SSRC                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
|      Playout Delay            |            NSN                | 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
|  Reserved           |   NUN   |    Free Buffer Space (FBS)    | 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 

*/
char RTCPNaduPacket::sRTCPTestBuffer[];
 

RTCPNaduPacket::RTCPNaduPacket(Bool16 debug = false): 
    RTCPAPPPacket(debug),
    fNaduDataBuffer(NULL),
    fNumBlocks(0)
{
}

void RTCPNaduPacket::GetTestPacket(StrPtrLen* resultPtr)
{
/*
Compound test packet

lengths are 32 bit words, include header, are minus 1

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|    RC   |   PT=RR=201   |             length            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P| subtype |   PT=SDES=202  |             length            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P| subtype |   PT=APP=204  |             length            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           SSRC/CSRC                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          name (ASCII)                         |  PSS0
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+----app specific data PSS0
|                    SSRC                                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
|      Playout Delay            |            NSN                | 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
|  Reserved           |   NUN   |    Free Buffer Space (FBS)    | 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ ----app data may repeat



// rtcp common header
    typedef struct {
       unsigned int version:2;   // protocol version 
       unsigned int p:1;         // padding flag 
       unsigned int count:5;     // varies by packet type 
       unsigned int pt:8;        // RTCP packet type 
       u_int16 length;           // pkt len in words, w/o this word can be 0
   } rtcp_common_t;

 // rtcp compound packet starts with rtcp rr header
 // rr data may be empty or not
 // nadu app header follows rr header and data if any
 

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+       
*/

#if 1 //full receiver report with SDES and Nadu
    UInt32  *theWriterStart = (UInt32*)sRTCPTestBuffer;
    UInt32  *theWriter = (UInt32*)sRTCPTestBuffer;

    *(theWriter++) = htonl(0x81c90007);     // 1 RR  packet header, full report
    *(theWriter++) = htonl(0x2935F2D6);     // 1 Sender SSRC = 691401430
    *(theWriter++) = htonl(0x6078CE22);     // 1 SSRC_1 = 1618529826
    *(theWriter++) = htonl(0x01000001);     // fraction lost | cumulative num packets lost 1% , 1 packet
    *(theWriter++) = htonl(0x0000361A);     // extended highest seq number received = 13850
    *(theWriter++) = htonl(0x00C7ED4D);     // interarrival jitter = 13102413
    *(theWriter++) = htonl(0x00000000);     // LSR last sender report = 0
    *(theWriter++) = htonl(0x04625238);     // Delay since last SR (DLSR) = 73552440 (garbage)
    
    *(theWriter++) = htonl(0x81ca0005);     // 1 SDES  packet header,
    *(theWriter++) = htonl(0x2935F2D6);     // 1 Sender SSRC = 691401430
    *(theWriter++) = htonl(0x010A5344);     // 1 CNAME = 01, len=10, "SD"
    *(theWriter++) = htonl(0x45532043);     // 1 CNAME = "ES C"
    *(theWriter++) = htonl(0x4e414d45);     // 1 CNAME = "NAME"
    *(theWriter++) = htonl(0x00000000);     // NULL item = end of list + 32bit padding
    
    
     
    *(theWriter++) = htonl(0x80CC0000);     // 1 APP packet header, needs len -> assigned beow
    
    UInt32  *appPacketLenStart = theWriter;
    
    *(theWriter++) = htonl(FOUR_CHARS_TO_INT('S', 'S', 'R', 'C')); //nadu ssrc
    *(theWriter++) = htonl(FOUR_CHARS_TO_INT('P', 'S', 'S', '0')); //nadu app packet name
    
    // first (typically only) ssrc block
    *(theWriter++) = htonl(0x423A35C7); //ssrc = 1111111111
    *(theWriter++) = htonl(0x2B6756CE); //delay | nsn = 11111 | 22222
    *(theWriter++) = htonl(0xFFFFAD9C); //nun | fbs= 31 | 44444
    
    // optional 2nd or more ssrc blocks
    *(theWriter++) = htonl(0x84746B8E); //ssrc = 222222222
    *(theWriter++) = htonl(0x2B6756CE); //delay | nsn = 11111 | 22222
    *(theWriter++) = htonl(0xFFFFAD9C); //nun | fbs= 31 | 44444

    UInt16 *packetLenOffsetPtr = &( (UInt16*)theWriterStart)[29];
    UInt16  packetLenInWords = htons( ((UInt32*)theWriter - (UInt32*)appPacketLenStart) ) ;
    
    *packetLenOffsetPtr = packetLenInWords;
    qtss_printf("packetLenInWords =%lu\n", ntohs(packetLenInWords));
    UInt32 len = (char*)theWriter - (char*)theWriterStart;
    if (resultPtr)
        resultPtr->Set(sRTCPTestBuffer, len);

#endif

#if 0 //full receiver report with Nadu
    UInt32  *theWriterStart = (UInt32*)sRTCPTestBuffer;
    UInt32  *theWriter = (UInt32*)sRTCPTestBuffer;

    *(theWriter++) = htonl(0x80c90007);     // 1 RR  packet header, empty len is ok but could be a full report
    *(theWriter++) = htonl(0x2935F2D6);     // 1 SSRC = 691401430
    *(theWriter++) = htonl(0x6078CE22);     // 1 SSRC_1 = 1618529826
    *(theWriter++) = htonl(0x01000001);     // fraction lost | cumulative num packets lost 1% , 1 packet
    *(theWriter++) = htonl(0x0000361A);     // extended highest seq number received = 13850
    *(theWriter++) = htonl(0x00C7ED4D);     // interarrival jitter = 13102413
    *(theWriter++) = htonl(0x00000000);     // LSR last sender report = 0
    *(theWriter++) = htonl(0x04625238);     // Delay since last SR (DLSR) = 73552440 (garbage)
    
    
     
    *(theWriter++) = htonl(0x80CC0000);     // 1 APP packet header, needs len -> assigned beow
    
    UInt32  *appPacketLenStart = theWriter;
    
    *(theWriter++) = htonl(FOUR_CHARS_TO_INT('S', 'S', 'R', 'C')); //nadu ssrc
    *(theWriter++) = htonl(FOUR_CHARS_TO_INT('P', 'S', 'S', '0')); //nadu app packet name
    
    // first (typically only) ssrc block
    *(theWriter++) = htonl(0x423A35C7); //ssrc = 1111111111
    *(theWriter++) = htonl(0x2B6756CE); //delay | nsn = 11111 | 22222
    *(theWriter++) = htonl(0xFFFFAD9C); //nun | fbs= 31 | 44444
    
    // optional 2nd or more ssrc blocks
    *(theWriter++) = htonl(0x84746B8E); //ssrc = 222222222
    *(theWriter++) = htonl(0x2B6756CE); //delay | nsn = 11111 | 22222
    *(theWriter++) = htonl(0xFFFFAD9C); //nun | fbs= 31 | 44444

    UInt16 *packetLenOffsetPtr = &( (UInt16*)theWriterStart)[17];
    UInt16  packetLenInWords = htons( (UInt32*)theWriter - (UInt32*)appPacketLenStart) ;
    
    *packetLenOffsetPtr = packetLenInWords;
    
    UInt32 len = (char*)theWriter - (char*)theWriterStart;
    if (resultPtr)
        resultPtr->Set(sRTCPTestBuffer, len);

#endif

#if 0 //empty receiver report with NADU
    UInt32  *theWriterStart = (UInt32*)sRTCPTestBuffer;
    UInt32  *theWriter = (UInt32*)sRTCPTestBuffer;

    *(theWriter++) = htonl(0x80c90000);     // 1 RR  packet header, empty len is ok but could be a full report
    
    *(theWriter++) = htonl(0x80CC0000);     // 1 APP packet header, needs len -> assigned beow
    
    UInt32  *appPacketLenStart = theWriter;
    
    *(theWriter++) = htonl(FOUR_CHARS_TO_INT('S', 'S', 'R', 'C')); //nadu ssrc
    *(theWriter++) = htonl(FOUR_CHARS_TO_INT('P', 'S', 'S', '0')); //nadu app packet name
    
    // first (typically only) ssrc block
    *(theWriter++) = htonl(0x423A35C7); //ssrc = 1111111111
    *(theWriter++) = htonl(0x2B6756CE); //delay | nsn = 11111 | 22222
    *(theWriter++) = htonl(0xFFFFAD9C); //nun | fbs= 31 | 44444
    
    // optional 2nd or more ssrc blocks
    *(theWriter++) = htonl(0x84746B8E); //ssrc = 222222222
    *(theWriter++) = htonl(0x2B6756CE); //delay | nsn = 11111 | 22222
    *(theWriter++) = htonl(0xFFFFAD9C); //nun | fbs= 31 | 44444

    UInt16 *packetLenOffsetPtr = &( (UInt16*)theWriterStart)[3];
    UInt16  packetLenInWords = htons( (UInt32*)theWriter - (UInt32*)appPacketLenStart) ;
    
    *packetLenOffsetPtr = packetLenInWords;
    
    UInt32 len = (char*)theWriter - (char*)theWriterStart;
    if (resultPtr)
        resultPtr->Set(sRTCPTestBuffer, len);
#endif

/*

sample run of the test packet below:
----------------------------------------
RTPStream::TestRTCPPackets received packet inPacketPtr.Ptr=0xf0080568 inPacketPtr.len =20
testing RTCPNaduPacket using packet inPacketPtr.Ptr=0xe2c38 inPacketPtr.len =40
>recv sess=1: RTCP RR recv_sec=6.812  type=video size=40 H_vers=2, H_pad=0, H_rprt_count=0, H_type=201, H_length=0, H_ssrc=-2134114296
>recv sess=1: RTCP APP recv_sec=6.813  type=video size=36 H_vers=2, H_pad=0, H_rprt_count=0, H_type=204, H_length=8, H_ssrc=1397969475
 
NADU Packet
     Block Index = 0 (h_ssrc = 1111111111, h_playoutdelay = 11111, h_sequence_num = 22222, h_nun_unit_num = 31, h_fbs_free_buf = 44444)
     Block Index = 1 (h_ssrc = 2222222222, h_playoutdelay = 11111, h_sequence_num = 22222, h_nun_unit_num = 31, h_fbs_free_buf = 44444)

Dumping Nadu List (list size = 3  record count=48)
-------------------------------------------------------------
NADU Record: list_index = 2 list_recordID = 48
     Block Index = 0 (h_ssrc = 1111111111, h_playoutdelay = 11111, h_sequence_num = 22222, h_nun_unit_num = 31, h_fbs_free_buf = 44444)
     Block Index = 1 (h_ssrc = 2222222222, h_playoutdelay = 11111, h_sequence_num = 22222, h_nun_unit_num = 31, h_fbs_free_buf = 44444)
NADU Record: list_index = 1 list_recordID = 47
     Block Index = 0 (h_ssrc = 1111111111, h_playoutdelay = 11111, h_sequence_num = 22222, h_nun_unit_num = 31, h_fbs_free_buf = 44444)
     Block Index = 1 (h_ssrc = 2222222222, h_playoutdelay = 11111, h_sequence_num = 22222, h_nun_unit_num = 31, h_fbs_free_buf = 44444)
NADU Record: list_index = 0 list_recordID = 46
     Block Index = 0 (h_ssrc = 1111111111, h_playoutdelay = 11111, h_sequence_num = 22222, h_nun_unit_num = 31, h_fbs_free_buf = 44444)
     Block Index = 1 (h_ssrc = 2222222222, h_playoutdelay = 11111, h_sequence_num = 22222, h_nun_unit_num = 31, h_fbs_free_buf = 44444)


*/

}



// use if you don't know what kind of packet this is
Bool16 RTCPNaduPacket::ParseNaduPacket(UInt8* inPacketBuffer, UInt32 inPacketLength)
{
        
    if (!this->ParseAPPPacket(inPacketBuffer, inPacketLength))
        return false;
        
    if (this->GetAppPacketName() != RTCPNaduPacket::kNaduPacketName)
        return false;
          
    return true;
}


Bool16 RTCPNaduPacket::ParseAPPData(UInt8* inPacketBuffer, UInt32 inPacketLength)
{

    if (!this->ParseNaduPacket(inPacketBuffer, inPacketLength) )
        return false;

    UInt32 *naduDataBuffer = (UInt32 *) (this->GetPacketBuffer()+kNaduDataOffset);
   
    int wordsLen = this->GetPacketLength() - 2;
    if (wordsLen < 3) // min is 3
        return false;
     
    if (0 !=(wordsLen % 3))// blocks are 3x32bits so there is a bad block somewhere.
        return false;
        
    fNumBlocks = wordsLen / 3;
    
    if (0 == fNumBlocks)
        return false;
        
    if (fNumBlocks > 100) // too many
        return false;
        
    fNaduDataBuffer = naduDataBuffer;

    if (0) //testing 
        this->DumpNaduPacket();
    
   return true;
 
 }

void RTCPNaduPacket::DumpNaduPacket()
{       
     char   printName[5];
    (void) this->GetAppPacketName(printName, sizeof(printName));
    qtss_printf(" H_app_packet_name = %s, ", printName );

    qtss_printf("\n");
    SInt32 count = 0;
    for (; count < fNumBlocks ; count ++)
    {
         
        UInt32 ssrc = this->GetSSRC(count);
        UInt32 ssrcIndex = this->GetSSRCBlockIndex(ssrc);
        UInt16 playoutDelay = this->GetPlayOutDelay(count);
        UInt16 nsn = this->GetNSN(count);
        UInt16 nun = this->GetNUN(count);
        UInt16 fbs = this->GetFBS(count);
        qtss_printf("              ");
        qtss_printf("RTCP APP NADU Report[%"_U32BITARG_"] ", ssrcIndex);
        qtss_printf("h_ssrc = %"_U32BITARG_, ssrc);
        qtss_printf(", h_playoutdelay = %u", playoutDelay);
        qtss_printf(", h_sequence_num = %u", nsn);
        qtss_printf(", h_nun_unit_num = %u", nun);
        qtss_printf(", h_fbs_free_buf = %u", fbs);
        
        qtss_printf("\n");
    }
 }




SInt32 RTCPNaduPacket::GetSSRCBlockIndex(UInt32 inSSRC)
{
    UInt32 *blockBuffer = NULL;
    SInt32 count = 0;
    UInt32 ssrc = 0;
    
    if (NULL == fNaduDataBuffer)
        return -1;
                  
    for (; count < fNumBlocks ; count ++)
    {
        blockBuffer = fNaduDataBuffer + (count * 3);            
        ssrc = (UInt32) ntohl(*(UInt32*)&blockBuffer[kOffsetNaduSSRC]);
        
        if (ssrc == inSSRC)
            return count;

    }
    
    return -1;   
}

UInt32 RTCPNaduPacket::GetSSRC(SInt32 index)
{

    if (index < 0)
        return 0;
        
    if (NULL == fNaduDataBuffer)
        return 0;
        
   if (index >= fNumBlocks)
        return 0;

    UInt32 *blockBufferPtr = fNaduDataBuffer + (index * 3);
    UInt32 ssrc = (UInt32) ntohl(*(UInt32*)&blockBufferPtr[kOffsetNaduSSRC]);

    return ssrc;

}

UInt16 RTCPNaduPacket::GetPlayOutDelay(SInt32 index)
{
    if (index < 0)
        return 0;
    
    if (NULL == fNaduDataBuffer)
        return 0;
        
    if (index >= fNumBlocks)
        return 0;
        
    UInt32 *blockBufferPtr = fNaduDataBuffer + (index * 3);
    UInt16 delay = (UInt16) ( ( ntohl(*(UInt32*)&blockBufferPtr[kOffsetNaduPlayoutDelay])  & kPlayoutMask) >> 16);

    return delay;
}

UInt16 RTCPNaduPacket::GetNSN(SInt32 index)
{
    if (index < 0)
        return 0;
        
    if (NULL == fNaduDataBuffer)
        return 0;
        
   if (index >= fNumBlocks)
        return 0;
        
    UInt32 *blockBufferPtr = fNaduDataBuffer + (index * 3);
    UInt16 nsn = (UInt16) ( ntohl(blockBufferPtr[kOffsetNSN]) & kNSNMask );

    return nsn;
}

UInt16 RTCPNaduPacket::GetNUN(SInt32 index)
{
    if (index < 0)
        return 0;

    if (NULL == fNaduDataBuffer)
        return 0;
        
    if (index >= fNumBlocks)
        return 0;
        
    UInt32 *blockBufferPtr = fNaduDataBuffer + (index * 3);
    UInt16 nun = (UInt16) ((ntohl(blockBufferPtr[kOffsetNUN]) & kNUNMask) >> 16);

    return nun;
}

UInt16 RTCPNaduPacket::GetFBS(SInt32 index)
{
    if (index < 0)
        return 0;

    if (NULL == fNaduDataBuffer)
        return 0;
        
    if (index >= fNumBlocks)
        return 0;
        
    UInt32 *blockBufferPtr = fNaduDataBuffer + (index * 3);
    UInt16 fbs = (UInt16) ntohl(blockBufferPtr[kOffsetFBS]) & kFBSMask;

    return fbs;
}

void   RTCPNaduPacket::Dump()
{
    this->DumpNaduPacket();

}

/* class NaduReport */
NaduReport::NaduReport(UInt8* inPacketBuffer, UInt32 inPacketLength, UInt32 id)
{
    fPacketBuffer = NEW UInt8[inPacketLength+1];
    fPacketBuffer[inPacketLength] = 0;
    fLength = inPacketLength;
    ::memcpy(fPacketBuffer, inPacketBuffer, inPacketLength);
    fNaduPacket.ParseAPPData(fPacketBuffer, inPacketLength);
    fid = id;
}



/* class NaduList */

void NaduList::Initialize(UInt32 listSize)
{

    fNaduReportList = NEW NaduReport *[listSize];    
    ::memset( (void *) fNaduReportList, 0, sizeof(NaduReport*) * listSize); //initialize ptr array with 0.
    fListSize = listSize;

}

NaduReport* NaduList::GetReport(UInt32 id)
{

    if (NULL == fNaduReportList)
        return NULL;
    

    NaduReport *result = fNaduReportList[this->IDtoIndex(id)];
    if (result && result->getID() == id)
        return result;
    return NULL;

}

UInt32 NaduList::GetReportIndex(UInt32 id)
{

    if (NULL == fNaduReportList)
        return 0;
    
    UInt32 index = this->IDtoIndex(id);
    NaduReport *result = fNaduReportList[index];
    if (result && result->getID() == id)
        return index;
    return 0;

}

NaduReport* NaduList::GetLastReport()
{
    if (NULL == fNaduReportList || fcurrentIndexCount == 0)
        return NULL;
        
    UInt32 index =  this->IDtoIndex(fcurrentIndexCount); 
    return fNaduReportList[index];
    
}

NaduReport* NaduList::GetPreviousReport(NaduReport* theReport)
{ 
    if (NULL == theReport)
        return NULL;
        
    return this->GetReport(theReport->getID() - 1);

}


NaduReport* NaduList::GetNextReport(NaduReport* theReport) 
{ 
    if (NULL == theReport)
        return NULL;
        
    return this->GetReport(theReport->getID() + 1);

}

NaduReport* NaduList::GetEarliestReport() 
{ 

    if ( fcurrentIndexCount > fListSize)
        return fNaduReportList[fcurrentIndexCount % fListSize];
    
    return  fNaduReportList[0]; 
}


Bool16 NaduList::AddReport(UInt8* inPacketBuffer, UInt32 inPacketLength, UInt32 *outID)
{
    if (NULL == fNaduReportList)
        return false;
    
    UInt32 resultID = ++fcurrentIndexCount;
    UInt32 index =this->IDtoIndex(fcurrentIndexCount); 
    
    if (fNaduReportList[index] != 0)
        delete fNaduReportList[index];
        
    fNaduReportList[index] = NEW NaduReport(inPacketBuffer,inPacketLength, resultID);
   
    if (outID)
        *outID = resultID;
        
    return true;

}



UInt32 NaduList::LastReportedFreeBuffSizeBytes()
{
   NaduReport* currentReportPtr = this->GetLastReport();
   if (NULL == currentReportPtr)
        return 0;
        
   RTCPNaduPacket *theNADUPacketData = currentReportPtr->GetNaduPacket();
   if (NULL == theNADUPacketData)
        return 0;
        
   return ((UInt32) theNADUPacketData->GetFBS(0)) * 64; //64 byte blocks are in the report
}

UInt32 NaduList::LastReportedTimeDelayMilli()
{
   NaduReport* currentReportPtr = this->GetLastReport();
   if (NULL == currentReportPtr)
        return 0;
        
   RTCPNaduPacket *theNADUPacketData = currentReportPtr->GetNaduPacket();
   if (NULL == theNADUPacketData)
        return 0;
        
   return theNADUPacketData->GetPlayOutDelay(0); 
}

UInt16 NaduList::GetLastReportedNSN()
{
   NaduReport* currentReportPtr = this->GetLastReport();
   if (NULL == currentReportPtr)
        return 0;
		
   RTCPNaduPacket *theNADUPacketData = currentReportPtr->GetNaduPacket();
   if (NULL == theNADUPacketData)
        return 0;
        
   return theNADUPacketData->GetNSN(0); 
}

void NaduList::DumpList()
{
    
    qtss_printf("\nDumping Nadu List (list size = %"_U32BITARG_"  record count=%"_U32BITARG_")\n",fListSize, fcurrentIndexCount);
    qtss_printf("-------------------------------------------------------------\n");
    NaduReport* lastReportPtr = this->GetLastReport();
    NaduReport* earliestReportPtr = this->GetEarliestReport();
    UInt32 thisID = 0;
    UInt32 stopID = 0;
    if (earliestReportPtr)
        stopID =  earliestReportPtr->getID();
        
    while (lastReportPtr) 
    {
        thisID = lastReportPtr->getID();
        qtss_printf("NADU Record: list_index = %"_U32BITARG_" list_recordID = %"_U32BITARG_"\n", this->GetReportIndex(thisID), thisID);
        lastReportPtr->GetNaduPacket()->Dump();
        if (thisID == stopID)
            break;
            
        thisID --;
        lastReportPtr = this->GetReport(thisID);
    }
    
}
