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
    File:       RTCPAPPQTSSPacket.h

    Contains:   RTCPAPPQTSSPacket de-packetizing classes


    
*/

#ifndef _RTCPAPPQTSSPACKET_H_
#define _RTCPAPPQTSSPACKET_H_

#include "RTCPAPPPacket.h"
#include "StrPtrLen.h"

/****** RTCPCompressedQTSSPacket is the packet type that the client actually sends ******/
class RTCPCompressedQTSSPacket : public RTCPAPPPacket
{
public:

    RTCPCompressedQTSSPacket(Bool16 debug = false);
    virtual ~RTCPCompressedQTSSPacket() {}
 
     //Call this before any accessor method. Returns true if successful, false otherwise
    virtual Bool16 ParseAPPData(UInt8* inPacketBuffer, UInt32 inPacketLength);

    // Call to parse if you don't know what kind of packet this is
    Bool16 ParseCompressedQTSSPacket(UInt8* inPacketBuffer, UInt32 inPacketLength);
    inline UInt32 GetQTSSReportSourceID();
    inline UInt16 GetQTSSPacketVersion();
    inline UInt16 GetQTSSPacketLength(); //In 'UInt32's

    
    inline UInt32 GetReceiverBitRate() {return fReceiverBitRate;}
    inline UInt16 GetAverageLateMilliseconds()  {return fAverageLateMilliseconds;}
    inline UInt16 GetPercentPacketsLost()   {return fPercentPacketsLost;}
    inline UInt16 GetAverageBufferDelayMilliseconds()   {return fAverageBufferDelayMilliseconds;}
    inline Bool16 GetIsGettingBetter()  {return fIsGettingBetter;}
    inline Bool16 GetIsGettingWorse()   {return fIsGettingWorse;}
    inline UInt32 GetNumEyes()  {return fNumEyes;}
    inline UInt32 GetNumEyesActive()    {return fNumEyesActive;}
    inline UInt32 GetNumEyesPaused()    {return fNumEyesPaused;}
    inline UInt32 GetOverbufferWindowSize() {return fOverbufferWindowSize;}
    
    //Proposed - are these there yet?
    inline UInt32 GetTotalPacketReceived()  {return fTotalPacketsReceived;}
    inline UInt16 GetTotalPacketsDropped()  {return fTotalPacketsDropped;}
    inline UInt16 GetTotalPacketsLost() {return fTotalPacketsLost;}
    inline UInt16 GetClientBufferFill() {return fClientBufferFill;}
    inline UInt16 GetFrameRate()    {return fFrameRate;}
    inline UInt16 GetExpectedFrameRate()    {return fExpectedFrameRate;}
    inline UInt16 GetAudioDryCount()    {return fAudioDryCount;}
    
    virtual void Dump(); //Override

    static void GetTestPacket(StrPtrLen* resultPtr) {}
    
    UInt32 fReceiverBitRate;
    UInt16 fAverageLateMilliseconds;
    UInt16 fPercentPacketsLost;
    UInt16 fAverageBufferDelayMilliseconds;
    Bool16 fIsGettingBetter;
    Bool16 fIsGettingWorse;
    UInt32 fNumEyes;
    UInt32 fNumEyesActive;
    UInt32 fNumEyesPaused;
    UInt32 fOverbufferWindowSize;
    
    //Proposed - are these there yet?
    UInt32 fTotalPacketsReceived;
    UInt16 fTotalPacketsDropped;
    UInt16 fTotalPacketsLost;
    UInt16 fClientBufferFill;
    UInt16 fFrameRate;
    UInt16 fExpectedFrameRate;
    UInt16 fAudioDryCount;
    
    enum // QTSS App Header offsets
    {

       kQTSSDataOffset = 20, // in bytes from packet start
       kQTSSReportSourceIDOffset = 3,  //in 32 bit words SSRC for this report
       kQTSSPacketVersionOffset = 4, // in 32bit words
            kQTSSPacketVersionMask = 0xFFFF0000UL,
            kQTSSPacketVersionShift = 16,
       kQTSSPacketLengthOffset = 4, // in 32bit words
            kQTSSPacketLengthMask = 0x0000FFFFUL,

    };

    enum // QTSS App Data Offsets
    {
    
    //Individual item offsets/masks
        kQTSSItemTypeOffset = 0,    //SSRC for this report
            kQTSSItemTypeMask = 0xFFFF0000UL,
            kQTSSItemTypeShift = 16,
        kQTSSItemVersionOffset = 0,
            kQTSSItemVersionMask = 0x0000FF00UL,
            kQTSSItemVersionShift = 8,
        kQTSSItemLengthOffset = 0,
            kQTSSItemLengthMask = 0x000000FFUL,
        kQTSSItemDataOffset = 4,
    
        //version we support currently
        kSupportedCompressedQTSSVersion = 0
    };
    
    enum //The 4 character name in the APP packet
    {   
        kCompressedQTSSPacketName = FOUR_CHARS_TO_INT('Q', 'T', 'S', 'S') //QTSS
    };
    
 
private:
    void ParseAndStore();

};

inline UInt32 RTCPCompressedQTSSPacket::GetQTSSReportSourceID()
{
    return (UInt32) ntohl( ((UInt32*)this->GetPacketBuffer())[kQTSSReportSourceIDOffset] ) ;
}


inline UInt16 RTCPCompressedQTSSPacket::GetQTSSPacketVersion()
{   
    UInt32 field = ((UInt32*)this->GetPacketBuffer())[kQTSSPacketVersionOffset];
    UInt16 vers = (UInt16)   ( ( ntohl(field) & kQTSSPacketVersionMask) >> kQTSSPacketVersionShift );
    return vers;
}

inline UInt16 RTCPCompressedQTSSPacket::GetQTSSPacketLength()
{  
    UInt32  field = ((UInt32*)this->GetPacketBuffer())[kQTSSPacketLengthOffset];
    return (UInt16) ( (UInt32)  ntohl(field)  & kQTSSPacketLengthMask );
}

/*
QTSS APP: QTSS Application-defined RTCP packet

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P| subtype |  PT=APP=204  |             length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           SSRC/CSRC                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                          name (ASCII)                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <---- app data start
   |                           SSRC/CSRC                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |        version                |             length            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      field name='ob' other    |   version=0   |   length=4    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               Over-buffer window size in bytes                |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

fieldnames = rr, lt, ls, dl, :), :|, :(, ey, pr, pd, pl, bl, fr, xr, d#, ob
   
 */





/****** RTCPqtssPacket is apparently no longer sent by the client ******/
class RTCPqtssPacket : public RTCPAPPPacket
{
public:
    
    RTCPqtssPacket() : RTCPAPPPacket() {}
    virtual ~RTCPqtssPacket() {}
    
    //Call this before any accessor method. Returns true if successful, false otherwise
    virtual Bool16 ParseAPPData(UInt8* inPacketBuffer, UInt32 inPacketLength);

    //Call this before any accessor method. Returns true if successful, false otherwise
    Bool16 ParseQTSSPacket(UInt8* inPacketBuffer, UInt32 inPacketLength);

    
    inline UInt32 GetReceiverBitRate() {return fReceiverBitRate;}
    inline UInt32 GetAverageLateMilliseconds()  {return fAverageLateMilliseconds;}
    inline UInt32 GetPercentPacketsLost()   {return fPercentPacketsLost;}
    inline UInt32 GetAverageBufferDelayMilliseconds()   {return fAverageBufferDelayMilliseconds;}
    inline Bool16 GetIsGettingBetter()  {return fIsGettingBetter;}
    inline Bool16 GetIsGettingWorse()   {return fIsGettingWorse;}
    inline UInt32 GetNumEyes()  {return fNumEyes;}
    inline UInt32 GetNumEyesActive()    {return fNumEyesActive;}
    inline UInt32 GetNumEyesPaused()    {return fNumEyesPaused;}
    
    //Proposed - are these there yet?
    inline UInt32 GetTotalPacketReceived()  {return fTotalPacketsReceived;}
    inline UInt32 GetTotalPacketsDropped()  {return fTotalPacketsDropped;}
    inline UInt32 GetClientBufferFill() {return fClientBufferFill;}
    inline UInt32 GetFrameRate()    {return fFrameRate;}
    inline UInt32 GetExpectedFrameRate()    {return fExpectedFrameRate;}
    inline UInt32 GetAudioDryCount()    {return fAudioDryCount;}

    
   
private:
 
    void ParseAndStore();

    UInt32 fReportSourceID;
    UInt16 fAppPacketVersion;
    UInt16 fAppPacketLength;    //In 'UInt32's
    
    UInt32 fReceiverBitRate;
    UInt32 fAverageLateMilliseconds;
    UInt32 fPercentPacketsLost;
    UInt32 fAverageBufferDelayMilliseconds;
    Bool16 fIsGettingBetter;
    Bool16 fIsGettingWorse;
    UInt32 fNumEyes;
    UInt32 fNumEyesActive;
    UInt32 fNumEyesPaused;
    
    //Proposed - are these there yet?
    UInt32 fTotalPacketsReceived;
    UInt32 fTotalPacketsDropped;
    UInt32 fClientBufferFill;
    UInt32 fFrameRate;
    UInt32 fExpectedFrameRate;
    UInt32 fAudioDryCount;
    
    enum
    {    
        //Individual item offsets/masks
        kQTSSItemTypeOffset = 0,    //SSRC for this report
        kQTSSItemVersionOffset = 4,
            kQTSSItemVersionMask = 0xFFFF0000UL,
            kQTSSItemVersionShift = 16,
        kQTSSItemLengthOffset = 4,
            kQTSSItemLengthMask = 0x0000FFFFUL,
        kQTSSItemDataOffset = 8,

        //version we support currently
        kSupportedQTSSVersion = 0
    };
    

};


#endif //_RTCPAPPQTSSPACKET_H_
