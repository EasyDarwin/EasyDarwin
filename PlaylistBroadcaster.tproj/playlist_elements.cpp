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

#include "playlist_elements.h"
#include "playlist_utils.h"
#include "OS.h"
#include "SocketUtils.h"
#ifndef __Win32__
#include <unistd.h>
#endif

// ************************
//
// MediaStream
//
// ************************
#define DROP_RTCP_TEST 0
#define DROPCOUNT  1// drop count RTCPs 

MediaStream::MediaStream(void)
{
    memset( (char *)&fData, '\0', sizeof(fData));
    fSend = true;
    fData.fSenderReportReady = true;
}
MediaStream::~MediaStream()
{
    if (fData.fSoundDescriptionBuffer) delete[] fData.fSoundDescriptionBuffer;
}


UInt32 MediaStream::GetACName(char* ioCNameBuffer)
{       
    //clear out the whole buffer
    ::memset(ioCNameBuffer, 0, kMaxCNameLen);
    
    //cName identifier
    ioCNameBuffer[0] = 1;
    
    //Unique cname is constructed from the base name and the current time
    qtss_sprintf(&ioCNameBuffer[2], " %s%"_64BITARG_"d", "QTSS", OS::Milliseconds() / 1000);
    UInt32 cNameLen = ::strlen(&ioCNameBuffer[2]);
    //2nd byte of CName should be length
    ioCNameBuffer[1] = (UInt8) cNameLen ;//doesn't count indicator or length byte
    cNameLen += 2; // add the identifier and len bytes to the result len
    //pad length to a 4 byte boundary
    UInt32 padLength = cNameLen % 4;
    if (padLength > 0)
        cNameLen += 4 - padLength;
    
    return cNameLen;
}

void MediaStream::TestAndIncSoundDescriptor(RTpPacket *packetPtr)
{ // currently not executed
    SInt16 test = 0;
    do 
    {
        if (!fData.fIsSoundStream) break;
        if (!packetPtr->HasSoundDescription()) break;
        if (!fData.fSoundDescriptionBuffer) break;

        SoundDescription *packetSDPtr = NULL;
        SoundDescription *savedSDPtr = (SoundDescription *) fData.fSoundDescriptionBuffer;
        (void) packetPtr->GetSoundDescriptionRef(&packetSDPtr);
        SInt32 descSize = packetPtr->fSoundDescriptionLen;
        
        if (descSize == 0) break;
        if (descSize > eMaxSoundDescriptionSize) break;
        
        if (fData.fSavedSoundDescSize == descSize)
        {   test = ::memcmp(packetSDPtr, fData.fSoundDescriptionBuffer, descSize);
        } 
        else test = 1; // they are different sizes so it is a new sample description

        if (test != 0) // they are different
        {   ::memcpy(savedSDPtr, packetSDPtr, descSize); 
            fData.fSavedSoundDescSize = descSize;
            fData.fSavedDataRefIndex ++ ; // it is different than saved so change the index
        }
        packetSDPtr->dataRefIndex = htons(fData.fSavedDataRefIndex);
         
    } while (false);
}


void MediaStream::UpdatePacketInStream(RTpPacket *packetPtr)
{
    UInt32  curSSRC = 0;
    UInt32  curRTpTimeStamp = 0;
    UInt16  curRTpSequenceNumber = 0;
    UInt32  newRTpTimeStamp = 0;
    UInt16  newRTpSequenceNumber = 0;
    UInt32  newSSRC = 0;
    unsigned char curPayload;
    unsigned char newPayload;
    packetPtr->GetHeaderInfo(&curRTpTimeStamp, &curRTpSequenceNumber, &curSSRC, &curPayload);

    newSSRC = fData.fInitSSRC;
    MapToStream(curRTpTimeStamp, curRTpSequenceNumber, curPayload, &newRTpTimeStamp, &newRTpSequenceNumber, &newPayload);
    
    if (fData.fIsVideoStream) { LogStr("video "); }
    if (fData.fIsSoundStream) { LogStr("audio "); }
    
    packetPtr->SetHeaderInfo(newRTpTimeStamp, newRTpSequenceNumber,newSSRC,newPayload);

    //TestAndIncSoundDescriptor(packetPtr); // put in to track QuickTime format sound descriptors and flag change in sample types
}

void MediaStream::MapToStream(UInt32 curRTpTimeStamp, UInt16 curRTpSequenceNumber, unsigned char curPayload, UInt32 *outRTpTimeStampPtr, UInt16 *outRTpSequenceNumberPtr, unsigned char *outPayloadPtr)
{

    if (fData.fNewMovieStarted == true)     // this is the first packet in a new movie
    {   
        fData.fNewMovieStarted = false; 
    }  
            
    fData.fCurStreamRTpSequenceNumber++; //  the stream sequence number
    
    UInt64 curSequenceNumber = (UInt64) fData.fCurStreamRTpSequenceNumber + (UInt64) fData.fSeqRandomOffset;    
    UInt64 curTimeStamp = (UInt64) curRTpTimeStamp + (UInt64) fData.fMediaStartOffsetMediaScale + (UInt64) fData.fRTpRandomOffset;
        
    UInt32 outTime = (UInt32) ( (UInt64) curTimeStamp & (UInt64) 0xFFFFFFFF ); 
    UInt16 outSeq =  (UInt16) ( (UInt64) curSequenceNumber & (UInt64) 0xFFFF ); 
    unsigned char outPayload = curPayload;
    Assert (fData.fMovieMediaTypePtr != NULL);// should always be valid
    PayLoad *firstPayLoadPtr =  (fData.fMovieMediaTypePtr)->fPayLoadTypes.Begin(); // potential problem; assumes first payload per track is this payload
    if (firstPayLoadPtr)
    {   
        outPayload = (unsigned char) ( 0x7F & firstPayLoadPtr->payloadID);
        outPayload |= (curPayload & 0x80);// the movie payload marker
    }

//  qtss_printf("MediaStream::MapToStream outTime = %"_U32BITARG_"\n", outTime);
//  qtss_printf("MediaStream::MapToStream calculated time = %"_U32BITARG_"\n",(UInt32) curTimeInScale); 

    if (outRTpTimeStampPtr) *outRTpTimeStampPtr = outTime;
    if (outRTpSequenceNumberPtr) *outRTpSequenceNumberPtr = outSeq;
    if (outPayloadPtr)  *outPayloadPtr = outPayload;
}

void MediaStream::BuildStaticRTCpReport()
{
    char theTempCName[kMaxCNameLen];
    UInt32 cNameLen = GetACName(theTempCName);
    
    //write the SR & SDES headers
    UInt32* theSRWriter = (UInt32*)&fData.fSenderReportBuffer;
    *theSRWriter = htonl(0x80c80006);
    theSRWriter += 7;
    //SDES length is the length of the CName, plus 2 32bit words
    *theSRWriter = htonl(0x81ca0000 + (cNameLen >> 2) + 1);
    ::memcpy(&fData.fSenderReportBuffer[kSenderReportSizeInBytes], theTempCName, cNameLen);
    fData.fSenderReportSize = kSenderReportSizeInBytes + cNameLen;
}

void MediaStream::InitIfAudio()
{
    if (fData.fStreamMediaTypePtr)
    {
        SimpleString audioStr("audio");
        fData.fIsSoundStream = SimpleParser::Compare(&audioStr, &(fData.fStreamMediaTypePtr->fTheTypeStr), false );
        if (fData.fIsSoundStream)
        {               
            fData.fSoundDescriptionBuffer = new char[eMaxSoundDescriptionSize];
            ::memset(fData.fSoundDescriptionBuffer, 0, eMaxSoundDescriptionSize );
        }
        else
        {
            SimpleString videoStr("video");
            fData.fIsVideoStream = SimpleParser::Compare(&videoStr, &(fData.fStreamMediaTypePtr->fTheTypeStr), false );
        }
    }   

}


void MediaStream::StreamStart(SInt64 startTime)
{

    fData.fStreamStartTime = startTime;
    fData.fMovieEndTime = startTime;
    fData.fLastSenderReportTime = 0;

    //for RTCp SRs, we also need to store the play time in NTP
//  fData.fNTPPlayTime = OS::TimeMilli_To_1900Fixed64Secs(startTime);
    fData.fNTPPlayTime = PlayListUtils::TimeMilli_To_1900Fixed64Secs(startTime);
    fData.fCurStreamRTpSequenceNumber =  0;
    fData.fMovieStartOffset = 0;    
    
    fData.fNewStreamStarted = true;
    fData.fSeqRandomOffset = PlayListUtils::Random();
    fData.fRTpRandomOffset = PlayListUtils::Random();

    
//  fData.fSeqRandomOffset = -1000; // test roll over
//  fData.fRTpRandomOffset = -100000; // test roll over
    
    InitIfAudio();
    
    //Build a static RTCp sender report (this way, only the info that changes
    //on the fly will have to be written)
    BuildStaticRTCpReport();    
}

void MediaStream::MovieStart(SInt64 startTime)
{
    fData.fNewMovieStarted = true;
    fData.fMovieStartTime = startTime;
    
    if (fData.fMovieMediaTypePtr && fData.fRTPFilePtr)
    {   UInt32 trackID = fData.fMovieMediaTypePtr->fTrackID;
        fData.fMovieMediaTypePtr->fTimeScale = fData.fRTPFilePtr->GetTrackTimeScale(trackID);
        
        UInt64 lastMovieduration = (UInt64) ( (Float64)  (fData.fLastMovieDurationSecs * (Float64) PlayListUtils::eMilli) ) ; // add the length of the last movie
        fData.fMovieStartOffset += lastMovieduration; 
        
        if (fData.fNewStreamStarted)
        {   fData.fNewStreamStarted = false;
            fData.fMovieEndTime = startTime; // first movie in stream has 0 movieInterval time.
        }
                
        Float64 mediaOffSet =   (Float64) (SInt64)fData.fMovieStartOffset / (Float64)  PlayListUtils::eMilli; // convert to float seconds
        mediaOffSet = mediaOffSet  * (Float64)  fData.fMovieMediaTypePtr->fTimeScale; // mediaOffset in media time scale
        fData.fMediaStartOffsetMediaScale =  (UInt64) mediaOffSet; // convert float time units to UInt64

        fData.fLastMovieDurationSecs = fData.fRTPFilePtr->GetMovieDuration();
    }


}

void MediaStream::MovieEnd(SInt64 endTime)
{
    fData.fMovieEndTime = endTime;
    fData.fMovieMediaTypePtr = NULL;
    fData.fRTPFilePtr = NULL;
}

SInt16  MediaStream::Accounting(RTpPacket *packetPtr)
{
    SInt16 result = -1;
    
    fData.fBytesSent += packetPtr->fLength;
    fData.fPacketsSent ++;
        
    UInt32 lastTime = fData.fTimeStamp;     
    unsigned char payload;
    packetPtr->GetHeaderInfo(&fData.fTimeStamp, &fData.fLastSequenceNumber, &fData.fSsrc, &payload);
    
    fData.fLastTimeStampOffset = fData.fTimeStamp - lastTime;   
    
    result = 0;
    
    return result;
}

SInt16  MediaStream::Send(RTpPacket *packetPtr)
{

    SInt16 result = -1;
    
    do 
    {
        if (fData.fMovieMediaTypePtr == NULL) break;
        
        UpdatePacketInStream(packetPtr);
        
        result = Accounting(packetPtr);
        if (result) break;
        
        if (fSend)
        {   result = fData.fSocketPair->SendRTp(packetPtr->fThePacket, packetPtr->fLength);
        }
    }
    while (false);
    
    return result;
}

void MediaStream::ReceiveOnPorts()
{
    fData.fSocketPair->RecvRTp(fData.fPortRTpReadBuff.fReadBuffer, ReceiveBuffer::kReadBufferSize, &fData.fPortRTpReadBuff.fReceiveLen);
    fData.fSocketPair->RecvRTCp(fData.fPortRTCpReadBuff.fReadBuffer, ReceiveBuffer::kReadBufferSize, &fData.fPortRTCpReadBuff.fReceiveLen);
}

#if DROP_RTCP_TEST
static int numRTCPPackets = 0;
static int numStartDropPackets = 0;
#endif

int MediaStream::UpdateSenderReport(SInt64 theTime)
{

    if (NULL == fData.fMovieMediaTypePtr)
        return 0;
            
    int result = 0;
    
    SInt64 timeToSend = fData.fLastSenderReportTime + (kSenderReportIntervalInSecs * PlayListUtils::eMilli);

#if DROP_RTCP_TEST
    if (theTime > timeToSend )
    {   
        if (fData.fIsSoundStream)
        {   numRTCPPackets ++;
            if ( (numRTCPPackets <= numStartDropPackets + DROPCOUNT) )
            {   qtss_printf("skip sound RTCP #%d  time=%qd\n",numRTCPPackets, theTime / PlayListUtils::eMilli);
                fData.fLastSenderReportTime = theTime;
                return 0;
            }
            else
            {   qtss_printf("send sound RTCP #%d time=%qd\n",numRTCPPackets, theTime / PlayListUtils::eMilli);
                numStartDropPackets = numRTCPPackets;           
            }
        }
    }
#endif
    
    if (theTime > timeToSend)
    {
        fData.fLastSenderReportTime = theTime;
        UInt32* theReport = (UInt32*) fData.fSenderReportBuffer;
        
        theReport++;
        *theReport = htonl(fData.fSsrc);
        
        theReport++;
        SInt64* theNTPTimestampP = (SInt64*)theReport;      
        *theNTPTimestampP = OS::HostToNetworkSInt64(fData.fNTPPlayTime +
                                PlayListUtils::TimeMilli_To_Fixed64Secs(theTime - fData.fStreamStartTime));
                                
        theReport += 2;
        *theReport = htonl(fData.fTimeStamp);

        Float64 curTimeInScale =   (Float64) (SInt64) PlayListUtils::Milliseconds() / (Float64)  PlayListUtils::eMilli; // convert to float seconds
        curTimeInScale = curTimeInScale  * (Float64)  fData.fMovieMediaTypePtr->fTimeScale; // curTime in media time scale
        curTimeInScale +=(Float64) fData.fRTpRandomOffset;
        curTimeInScale = (UInt32) ( (UInt64) curTimeInScale & (UInt64) 0xFFFFFFFF ); 

        //qtss_printf("MediaStream::UpdateSenderReport RTCP timestamp = %"_U32BITARG_"\n",(UInt32) curTimeInScale);
        *theReport = htonl((UInt32) curTimeInScale);
        
        theReport++;        
        fData.fPacketCount = (UInt32) fData.fPacketsSent;
        *theReport = htonl(fData.fPacketCount);
    
        theReport++;
        fData.fByteCount = (UInt32)  fData.fBytesSent; 
        *theReport = htonl(fData.fByteCount);
        
        theReport += 2;
        *theReport = htonl(fData.fSsrc);
        
        LogStr("Sender Report\n");
        LogUInt("NTP ",(UInt32) ((*theNTPTimestampP) >> 32)," ");
        LogUInt(" ",(UInt32) ((*theNTPTimestampP) & 0xFFFFFFFF), "\n" );
        LogUInt("time stamp = ", fData.fTimeStamp, "\n");
        LogInt("SSRC = ", fData.fSsrc, "\n");
        LogUInt("Packets sent = ", fData.fPacketCount, "\n");
        LogUInt("Bytes sent = ", fData.fByteCount, "\n");
        LogBuffer();
        result = fData.fSocketPair->SendRTCp(fData.fSenderReportBuffer, fData.fSenderReportSize);

    }

    return result;
}

// ************************
//
// UDPSOCKETPAIR
//
// ************************


void UDPSocketPair::Close()
{
    if (fMultiCastJoined)
        this->LeaveMulticast();
        
#ifdef __Win32__
    if (fSocketRTp != 0) ::closesocket(fSocketRTp);
    if (fSocketRTCp != 0) ::closesocket(fSocketRTCp);
#else   
    if (fSocketRTp != 0) ::close(fSocketRTp);
    if (fSocketRTCp != 0) ::close(fSocketRTCp);
#endif
    fSocketRTp = 0;
    fSocketRTCp = 0;
}

SInt16 UDPSocketPair::Open()
{
    SInt16 result = 0;
    do
    {
        Close();
        
        fSocketRTp = ::socket(PF_INET, SOCK_DGRAM, 0);
        if (fSocketRTp == kInvalidSocket)
        {   result = kInvalidSocket; 
            break;
        }
        
        fSocketRTCp = ::socket(PF_INET, SOCK_DGRAM, 0);
        if (fSocketRTCp == kInvalidSocket)
        {   result = kInvalidSocket; 
            break;
        }
        
#ifdef __Win32__
        u_long one = 1;
        (void)::ioctlsocket(fSocketRTp, FIONBIO, &one);
        (void)::ioctlsocket(fSocketRTCp, FIONBIO, &one);
#else
        int flag; 
        int val;
        flag = ::fcntl(fSocketRTp, F_GETFL, 0);
        val = ::fcntl(fSocketRTp, F_SETFL, flag | O_NONBLOCK);
        if( val < 0 ) { result = -1; break; }


        flag = ::fcntl(fSocketRTCp, F_GETFL, 0);
        val = ::fcntl(fSocketRTCp, F_SETFL, flag | O_NONBLOCK);
        if( val < 0 ) { result = -1; break; }
#endif          
    } while (false);
    
    if (result != 0)
    {   if (fSocketRTp != 0) ::close(fSocketRTp);
        if (fSocketRTCp != 0) ::close(fSocketRTCp);
        fSocketRTp = 0;
        fSocketRTCp = 0;
    }
            
    return result;
}

void UDPSocketPair::InitPorts(UInt32 addr)
{
    ::memset(&fLocalAddrRTp, 0, sizeof(fLocalAddrRTp));
    fLocalAddrRTp.sin_family = PF_INET;
    fLocalAddrRTp.sin_port = htons(0);
    fLocalAddrRTp.sin_addr.s_addr = htonl(addr);

    ::memset(&fLocalAddrRTCp, 0, sizeof(fLocalAddrRTCp));
    fLocalAddrRTCp.sin_family = PF_INET;
    fLocalAddrRTCp.sin_port = htons(0);
    fLocalAddrRTCp.sin_addr.s_addr = htonl(addr);

    ::memset(&fDestAddrRTp, 0, sizeof(fDestAddrRTp));
    fDestAddrRTp.sin_family = PF_INET;
    fDestAddrRTp.sin_port = htons(0);
    fDestAddrRTp.sin_addr.s_addr = htonl(addr);

    ::memset(&fDestAddrRTCp, 0, sizeof(fDestAddrRTCp));
    fDestAddrRTCp.sin_family = PF_INET;
    fDestAddrRTCp.sin_port = htons(0);
    fDestAddrRTCp.sin_addr.s_addr = htonl(addr);
}

SInt16 UDPSocketPair::Bind(UInt32 addr)
{
    int err = -1;
     
    if ( (fSocketRTp == kInvalidSocket) || (fSocketRTCp == kInvalidSocket) )
        return -1;
    
    InitPorts(addr);
        
    UInt32 PortRTp = eSourcePortStart;
    UInt32 PortRTCp = PortRTp + 1; // keep them together for clarity

    for (int count = eSourcePortStart; count < eSourcePortRange; count ++)
    {
        PortRTp = count;
        Assert( (PortRTp & 1) == 0); // must be even
        count += 1;
        PortRTCp = count;
        Assert( (PortRTCp & 1) == 1);// must be odd and one more than rtp port
        
        fLocalAddrRTp.sin_port = htons( (UInt16) PortRTp);
        fLocalAddrRTCp.sin_port = htons( (UInt16) PortRTCp);
            
        //qtss_printf("Attempting to bind to rtp port %d \n",PortRTp);
        
        err = ::bind(fSocketRTp, (sockaddr *)&fLocalAddrRTp, sizeof(fLocalAddrRTp));
        if (err != 0)
        {   
            //qtss_printf("UDPSocketPair::Bind Error binding to rtp port %d \n",PortRTp);
            InitPorts(addr);
            continue;
        }

        err = ::bind(fSocketRTCp, (sockaddr *)&fLocalAddrRTCp, sizeof(fLocalAddrRTCp)); 
        if (err != 0)
        {
            //qtss_printf("UDPSocketPair::Bind Error binding to rtcp port %d \n",PortRTp);
            Close();
            Open();
            InitPorts(addr);
            continue;
        }  
        
        if (err == 0) 
        {   
            //qtss_printf("Bound to rtp port = %d, rtcp port = %d \n",PortRTp, PortRTCp);
            fState |= kBound;
            break;
        }
    }
    
    return err;
}


SInt16 UDPSocketPair::SendTo(int socket, sockaddr *destAddrPtr, char* inBuffer, UInt32 inLength )
{
    SInt16 result = -1;
    do
    {
        if (inBuffer == NULL) break;        
        if (destAddrPtr == NULL) break;         
        if (socket == kInvalidSocket) break;
        
        //qtss_printf("Sending data to %d. Addr = %d inLength = %d\n", ntohs(theAddr->sin_port), ntohl(theAddr->sin_addr.s_addr), inLength);
        ::sendto(socket, inBuffer, inLength, 0, destAddrPtr, sizeof(sockaddr));
        
        result = 0;
    } while (false);
        
    //if (result != 0) qtss_printf("UDP SENDTO ERROR!\n");
    return result;
}


SInt16 UDPSocketPair::SendRTp(char* inBuffer, UInt32 inLength)
{   
    if (fBroadcasterSession != NULL)
    {   OSMutexLocker locker(fBroadcasterSession->GetMutex());
        return (SInt16) fBroadcasterSession->SendPacket(inBuffer,inLength,fChannel);
    }
    else
        return SendTo(fSocketRTp, (sockaddr*)&fDestAddrRTp, inBuffer, inLength );
}

SInt16 UDPSocketPair::SendRTCp(char* inBuffer, UInt32 inLength)
{   
    if (fBroadcasterSession != NULL)
    {   OSMutexLocker locker(fBroadcasterSession->GetMutex());
        return (SInt16) fBroadcasterSession->SendPacket(inBuffer,inLength,fChannel+1);
    }
    else
        return SendTo(fSocketRTCp, (sockaddr*)&fDestAddrRTCp, inBuffer, inLength );
}

SInt16  UDPSocketPair::SetDestination (char *destAddress,UInt16 destPortRTp, UInt16 destPortRTCp)
{
    SInt16 result = -1;

    if (destAddress != NULL)
    {   UInt32 netAddress = inet_addr(destAddress);
    
        fDestAddrRTp = fLocalAddrRTp; 
        fDestAddrRTp.sin_port = htons(destPortRTp); 
        fDestAddrRTp.sin_addr.s_addr = netAddress;
        
        fDestAddrRTCp = fLocalAddrRTCp;
        fDestAddrRTCp.sin_port = htons(destPortRTCp);       
        fDestAddrRTCp.sin_addr.s_addr =  netAddress;
        
        fIsMultiCast = SocketUtils::IsMulticastIPAddr(ntohl(netAddress));

        result = 0;
    }
    return result;
}

SInt16 UDPSocketPair::SetMulticastInterface()
{
    // set the outgoing interface for multicast datagrams on this socket
    in_addr theLocalAddr;
    ::memset(&theLocalAddr, 0, sizeof(theLocalAddr));
    
    theLocalAddr.s_addr = fLocalAddrRTp.sin_addr.s_addr;
    int err = setsockopt(fSocketRTp, IPPROTO_IP, IP_MULTICAST_IF, (char*)&theLocalAddr, sizeof(theLocalAddr));

    return err; 
}

SInt16 UDPSocketPair::JoinMulticast()
{
    int err = 0;

    
    UInt32 localAddr = fLocalAddrRTp.sin_addr.s_addr; // Already in network byte order

#if __solaris__
    if( localAddr == htonl(INADDR_ANY) )
         localAddr = htonl(SocketUtils::GetIPAddr(0));
#endif

    struct ip_mreq  theMulti;
    ::memset(&theMulti, 0, sizeof(theMulti));

    theMulti.imr_multiaddr.s_addr = fDestAddrRTp.sin_addr.s_addr;
    theMulti.imr_interface.s_addr = localAddr;
    err = setsockopt(fSocketRTp, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&theMulti, sizeof(theMulti));
    (void) setsockopt(fSocketRTCp, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&theMulti, sizeof(theMulti));

    if (err == 0)
        fMultiCastJoined = true;
        
    return err;
}


SInt16 UDPSocketPair::SetTTL(SInt16 timeToLive)
{
    // set the ttl
    int nOptVal = (int)timeToLive;
    int err = 0;
    
    err = setsockopt(fSocketRTp, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&nOptVal, sizeof(nOptVal));
    if (err != 0) return err;

    err = setsockopt(fSocketRTCp, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&nOptVal, sizeof(nOptVal));
    return err; 
}

SInt16 UDPSocketPair::LeaveMulticast()
{ 
   UInt32 localAddr = fLocalAddrRTp.sin_addr.s_addr; // Already in network byte order

#if __solaris__
    if( localAddr == htonl(INADDR_ANY) )
         localAddr = htonl(SocketUtils::GetIPAddr(0));
#endif

    struct ip_mreq  theMulti;
    ::memset(&theMulti, 0, sizeof(theMulti));
    
    theMulti.imr_multiaddr.s_addr = fDestAddrRTp.sin_addr.s_addr;
    theMulti.imr_interface.s_addr = localAddr;
    
    int err = setsockopt(fSocketRTp, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&theMulti, sizeof(theMulti));
    (void) setsockopt(fSocketRTCp, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&theMulti, sizeof(theMulti));
    return err;
}

SInt16 UDPSocketPair::RecvFrom(sockaddr *RecvRTpddrPtr, int socket, char* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen)
{
    SInt16  result = -1;
    
    do
    {   
        if (ioBuffer == NULL) break;
        if (RecvRTpddrPtr == NULL) break;
        if (socket == kInvalidSocket) break;
        
        sockaddr_in theAddr;
#if __Win32__ || __osf__ || __sgi__  || __hpux__ 
        int addrLen = sizeof(theAddr);
#else
        socklen_t addrLen = sizeof(theAddr);
#endif
        
        SInt32 theRecvLen = ::recvfrom(socket, ioBuffer, inBufLen, 0, (sockaddr*)&theAddr, &addrLen);       
        if (theRecvLen == -1) break;
        
        if (outRecvLen) *outRecvLen = (UInt32)theRecvLen;
    } while (false);
    return result;      
}

SInt16 UDPSocketPair::RecvRTp(char* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen)
{
    return RecvFrom( (sockaddr *)&fDestAddrRTp, fSocketRTp, ioBuffer, inBufLen, outRecvLen);
}

SInt16 UDPSocketPair::RecvRTCp(char* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen)
{
    return  RecvFrom( (sockaddr *)&fDestAddrRTCp, fSocketRTCp, ioBuffer, inBufLen, outRecvLen);
}

SInt16 UDPSocketPair::SetMultiCastOptions(SInt16 ttl)
{
    SInt16 err = 0;

    if (this->fIsMultiCast) do// set by SetDestination
    {               
        err = this->SetTTL(ttl);
        WarnV(err == 0 , "failed to set ttl");
        if (err != 0) break;

        err = this->SetMulticastInterface();
        WarnV(err == 0 , "failed to set multicast socket option");
        if (err != 0) break;
        
        err = this->JoinMulticast();
        WarnV(err == 0 , "failed to join multicast");
        if (err != 0) break;
        
    } while (false);
        
    return err;
}

SInt16 UDPSocketPair::OpenAndBind( UInt16 rtpPort,UInt16 rtcpPort,char *destAddress) 
{
    SInt16 err = -1;
        
    do
    {
        err = this->Open();
        if (err != 0) break;
        
        err = this->Bind(INADDR_ANY);
        if (err != 0) break;
        
        err = this->SetDestination (destAddress, rtpPort, rtcpPort);    
        if (err != 0) break;
        
    } while (false);
    
    if (err)
    {   
        this->Close();
    }
    
    return err;
};

// ************************
//
// RTpPacket
//
// ************************

SInt16 RTpPacket::GetSoundDescriptionRef(SoundDescription **soundDescriptionPtr)
{
    SInt16 result = -1;
    
    if (fThePacket && soundDescriptionPtr)
    {
        SInt32 minSoundLength = sizeof(SoundHeader) + sizeof(SoundDescription) + kRTpHeaderSize;
        if ( fLength >= minSoundLength  )
        {
            char *offsetPtr = fThePacket + kRTpHeaderSize + sizeof(SoundHeader);
            *soundDescriptionPtr = (SoundDescription *) offsetPtr;
            SInt32 descSize = ntohl( (**soundDescriptionPtr).descSize);
            fSoundDescriptionLen = descSize;
            result = 0;
        }
    }
    
    return result;
}

bool RTpPacket::HasSoundDescription() 
{
    bool result = false;
    
    if (fThePacket)
    {
    
//      WritePacketToLog(fThePacket, fLength);
        SInt32 minSoundLength = sizeof(SoundHeader) + sizeof(SoundDescription) + kRTpHeaderSize;
        if (fLength >= minSoundLength ) 
        {
            char *offsetPtr = fThePacket + kRTpHeaderSize;
            SoundHeader* testHeaderPtr = (SoundHeader*) offsetPtr;
            do
            {
                if (testHeaderPtr->bytes[0] != 0x17) break;
                if (testHeaderPtr->sndtype[0] != 's') break;
                if (testHeaderPtr->sndtype[1] != 'o') break;
                if (testHeaderPtr->sndtype[2] != 'u') break;
                if (testHeaderPtr->sndtype[3] != 'n') break;

                fHasSoundDescription = result = true;
                
            } while (false);
        }
        
        if (result) 
        {   
            LogStr("has sound description soun\n");
        }
        PrintLogBuffer(true);
    }
    
    return result;
}

SInt16 RTpPacket::GetHeaderInfo(UInt32 *timeStampPtr, UInt16 *sequencePtr,UInt32 *SSRCPtr, unsigned char*payloadTypeAndMarkPtr)
{
    SInt16 result = -1;
    
    
    unsigned char *header8Ptr = (unsigned char *) fThePacket;
    UInt16* header16Ptr = (UInt16*)fThePacket;
    UInt32* header32Ptr = (UInt32*)fThePacket;  
    
    if (fThePacket && timeStampPtr && sequencePtr && SSRCPtr && payloadTypeAndMarkPtr)
    {
        *payloadTypeAndMarkPtr = header8Ptr[cPayloadType];
        *sequencePtr = ntohs(header16Ptr[cSequenceNumber]);
        *timeStampPtr = ntohl(header32Ptr[cTimeStamp]);
        *SSRCPtr = ntohl(header32Ptr[cSSRC]);
        result = 0; 
    }
    
    return result;
}



SInt16 RTpPacket::SetHeaderInfo(UInt32 timeStamp, UInt16 sequence, UInt32 SSRC, unsigned char payloadTypeAndMark)
{
    SInt16 result = -1;
    
    unsigned char *header8Ptr = (unsigned char *) fThePacket;
    UInt16* header16Ptr = (UInt16*)fThePacket;
    UInt32* header32Ptr = (UInt32*)fThePacket;  
    
    
    if (fThePacket)
    {
        LogInt("sequence = ", sequence, " ");
        LogUInt("time = ", timeStamp, " ");
//      LogUInt("old payload = ",( UInt32 ) (header8Ptr[cPayloadType] & 0x7F)," ");
        LogUInt("payload = ",( UInt32 ) (payloadTypeAndMark & 0x7F) ," ");
//      LogUInt("old marker = ", ( UInt32 ) (header8Ptr[cPayloadType] & 0x80) ," ");
//      LogUInt("marker = ", ( UInt32 ) (payloadTypeAndMark & 0x80) ," ");
        LogUInt("ssrc = ", SSRC, "\n");

        header8Ptr[cPayloadType] = payloadTypeAndMark;
        header16Ptr[cSequenceNumber] = htons(sequence);
        header32Ptr[cTimeStamp] = htonl(timeStamp);     
        header32Ptr[cSSRC] = htonl(SSRC);       
        result = 0; 

        LogBuffer();
    }

    
    return result;
}


