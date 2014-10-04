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
    File:       BroadcasterSession.cpp

    Contains:   .  
                    
    
    
*/

#include "BroadcasterSession.h"
#include "OSMemory.h"
#include "StrPtrLen.h"
#include "OSMutex.h"
#include "SDPUtils.h"
#include <stdlib.h>
#include "SafeStdLib.h"
#define BROADCAST_SESSION_DEBUG 0

static const SInt64 kMaxWaitTimeInMsec = 1000;
static const SInt64 kIdleTimeoutInMsec = 60 * 1000; // Time out at 60 seconds
static const SInt16 kSanitySeqNumDifference = 300;

UInt32          BroadcasterSession::sActiveConnections = 0;
UInt32          BroadcasterSession::sBroadcastingConnections = 0;
UInt32          BroadcasterSession::sTotalConnectionAttempts = 0;


char *BroadcasterSession::fPacket = NULL;

BroadcasterSession::BroadcasterSession( UInt32 inAddr, UInt16 inPort, char* inURL,
                                BroadcasterType inClientType,
                                UInt32 inDurationInSec, UInt32 inStartPlayTimeInSec,
                                UInt32 inRTCPIntervalInSec, UInt32 inOptionsIntervalInSec,
                                UInt32 inHTTPCookie, Bool16 inAppendJunkData, UInt32 inReadInterval,
                                UInt32 inSockRcvBufSize,
                                StrPtrLen *sdpSPLPtr,
                                char *namePtr,
                                char *passwordPtr,
                                Bool16  deepDebug,
                                Bool16 burst)
:   fSocket(NULL),
    fRTSPClient(NULL),
    fTimeoutTask(NULL, kIdleTimeoutInMsec),

    fDurationInSec(inDurationInSec),
    fStartPlayTimeInSec(inStartPlayTimeInSec),
    fRTCPIntervalInSec(inRTCPIntervalInSec),
    fOptionsIntervalInSec(inOptionsIntervalInSec),
    
    fState(kSendingAnnounce),
    fDeathState(kSendingAnnounce),
    fDeathReason(kDiedNormally),
    fNumSetups(0),
    fUDPSocketArray(NULL),
    
    fPlayTime(0),
    fTotalPlayTime(0),
    fLastRTCPTime(0),
    fTeardownImmediately(false),
    fAppendJunk(inAppendJunkData),
    fReadInterval(inReadInterval),
    fSockRcvBufSize(inSockRcvBufSize),
    fBurst(burst),
    fBurstTime(10),
    fStats(NULL),
    fPacketLen(0),
    fChannel(0)
//  fPacket(NULL)

{
    fTimeoutTask.SetTask(this);
    StrPtrLen theURL(inURL);
    fSDPParser.Parse(sdpSPLPtr->Ptr, sdpSPLPtr->Len);
    if (fBurst && deepDebug)
        printf("Burst Mode enabled: broadcast will be delayed for %"_U32BITARG_" seconds before starting\n", fBurstTime);
        
#if BROADCAST_SESSION_DEBUG

    qtss_printf("Connecting to: %s, port %d\n", inURL, inPort);

#endif  
    //
    // Construct the appropriate ClientSocket type depending on what type of client we are supposed to be
    switch (inClientType)
    {
        case kRTSPUDPBroadcasterType:
        {
            fControlType = kRawRTSPControlType;
            fTransportType = kUDPTransportType;
            fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
            break;
        }
        case kRTSPTCPBroadcasterType:
        {
            fControlType = kRawRTSPControlType;
            fTransportType = kTCPTransportType;
            fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
            break;
        }
        case kRTSPHTTPBroadcasterType:
        {
            fControlType = kRTSPHTTPControlType;
            fTransportType = kTCPTransportType;
            fSocket = NEW HTTPClientSocket(theURL, inHTTPCookie, Socket::kNonBlockingSocketType);
            break;
        }
        case kRTSPHTTPDropPostBroadcasterType:
        {
            fControlType = kRTSPHTTPDropPostControlType;
            fTransportType = kTCPTransportType;
            fSocket = NEW HTTPClientSocket(theURL, inHTTPCookie, Socket::kNonBlockingSocketType);
            break;
        }
        case kRTSPReliableUDPBroadcasterType:
        {
            Assert(0);
            break;
        }
        default:
        {
            qtss_printf("BroadcasterSession: Attempt to create unsupported client type.\n");
            ::exit(-1);
        }
    }
    
    fSocket->Set(inAddr, inPort);
    fSocket->GetSocket()->SetTask(this);

    int sndBufSize = 32 * 1024;
    int rcvBufSize=1024;
    ((TCPClientSocket*)fSocket)->SetOptions(sndBufSize,rcvBufSize);
    //
    // Construct the client object using this socket.
    Bool16 verbose = deepDebug;
    fRTSPClient = NEW RTSPClient(fSocket, verbose);
    fRTSPClient->Set(theURL);
    fRTSPClient->SetTransportMode(RTSPClient::kPushMode);
    fRTSPClient->SetName(namePtr);
    fRTSPClient->SetPassword(passwordPtr);

    //
    // Start the connection process going
    this->Signal(Task::kStartEvent);
}

BroadcasterSession::~BroadcasterSession()
{
#if BROADCAST_SESSION_DEBUG
    qtss_printf("BroadcasterSession::~BroadcasterSession() \n");
#endif

    if (fUDPSocketArray != NULL)
    {
        for (UInt32 x = 0; x < fSDPParser.GetNumStreams() * 2; x++)
            delete fUDPSocketArray[x];
    }
            
    delete [] fUDPSocketArray;
    delete fRTSPClient;
    delete fSocket;
}

char*   BroadcasterSession::GetNextPacket(UInt32 *packetLen, UInt8 *channel)
{
    char* thePacketData = NULL;
    *packetLen = 0;
    *channel = 0;
    //qtss_printf("BroadcasterSession::GetNextPacket Q len=%"_U32BITARG_"\n",fPacketQueue.GetLength());
    OSQueueElem *thePacketQElemPtr = fPacketQueue.GetHead();
    if(thePacketQElemPtr != NULL)
    {   
        BroadcasterSession::RTPPacket* thePacket = (BroadcasterSession::RTPPacket*) thePacketQElemPtr->GetEnclosingObject();
        fPacketQueue.Remove(thePacketQElemPtr); 
        
        Assert(thePacket != NULL);
        Assert(thePacket->fData != NULL);
        Assert(thePacket->fLen > 0);
        
        if (NULL == thePacket || NULL == thePacket->fData || 0 == thePacket->fLen)
            return NULL;
            
        thePacketData = thePacket->fData;
        *packetLen = thePacket->fLen;
        *channel = thePacket->fChannel;
        
        delete thePacket;
    }
    
    Assert(*packetLen <= RTSPClient::kReqBufSize);
    
    return thePacketData;
}

OS_Error    BroadcasterSession::SendPacket(char* data, UInt32 len,UInt8 channel)
{   
    OS_Error theErr = 0;
    if (fState != kBroadcasting)
    {   
        switch (fState)
        {   case    kSendingAnnounce:
                    theErr = kConnectionFailed;
            break;
            
            case    kSendingReceive:
            case    kSendingSetup:
                    theErr = kRequestFailed;
            break;
            
            default:
                theErr = kDiedWhileBroadcasting;
        }
        return theErr;
    }
    
    BroadcasterSession::RTPPacket* newPacket = NEW BroadcasterSession::RTPPacket;
    if (newPacket == NULL)
        return kMemoryError;
    
     if (fBurst)
    {
        char *theSendData = NEW char[len + 12];
        memcpy(theSendData,data,len);
        static char* tag="aktt";
        memcpy(&theSendData[len],tag,4);
        SInt64 currentTime= OS::HostToNetworkSInt64(OS::Milliseconds());
        memcpy(&theSendData[len+4],&currentTime,8);
        
        newPacket->fCount = ++fPacketCount;
        newPacket->SetEnclosingObject(newPacket);
        newPacket->SetPacketData(theSendData,len +12,channel);
    }
   
    else
    {
        char *theSendData = NEW char[len];
        memcpy(theSendData,data,len);
        newPacket->fCount = ++fPacketCount;
        newPacket->SetEnclosingObject(newPacket);
        newPacket->SetPacketData(theSendData,len,channel);
    }
    
    fPacketQueue.EnQueue(newPacket->GetQElement());
    return 0;
}   

OS_Error BroadcasterSession::SendWaitingPackets()
{   
    static SInt64 sFirstTime = 0;

    if (fBurst)
    {
        if (sFirstTime == 0)
            sFirstTime = OS::Milliseconds() + (fBurstTime * 1000);
       
        if (sFirstTime > OS::Milliseconds())
        {
            // qtss_printf("BroadcasterSession::GetNextPacket NOT sending packets Q len=%"_U32BITARG_"\n",fPacketQueue.GetLength());
            return QTSS_NoErr;
        }
    }
    
    OSMutexLocker locker(this->GetMutex());
    OS_Error theErr = OS_NoErr;
    
    if (fPacket == NULL)
    {   fPacket = this->GetNextPacket(&fPacketLen, &fChannel);
    }
    
    while (fPacket != NULL)
    {   Bool16 getNext = false;
        //qtss_printf("BroadcasterSession::Run- Broadcasting SendInterleavedWrite \n");
        Assert(fPacketLen <= RTSPClient::kReqBufSize);
        OSMutexLocker locker(fRTSPClient->GetMutex());
        theErr = fRTSPClient->SendInterleavedWrite(fChannel, (UInt16) fPacketLen, fPacket, &getNext);
        if (getNext)
        {   delete [] fPacket;
            fPacket = this->GetNextPacket(&fPacketLen, &fChannel);
        }
        
        if (theErr != OS_NoErr)
        {
            if (theErr == EAGAIN || theErr == EINPROGRESS)
            {   // keep the packet around and try again
                //qtss_printf("BroadcasterSession::SendWaitingPackets- Broadcasting SendInterleavedWrite err=%"_S32BITARG_" Request Write Event\n",theErr);
                break;      
            }
            // some bad error bail.
            delete [] fPacket;
            fPacket = NULL;
            
            break;
        }
        
        // we sent the packet and may or may not have another packet to send
            
        fTimeoutTask.RefreshTimeout();
        //qtss_printf("send channel=%u len=%"_U32BITARG_"\n",(UInt16)fChannel,fPacketLen);
    }
    
    return theErr;              
}   

SInt64 BroadcasterSession::Run()
{

    EventFlags theEvents = this->GetEvents();
    
    if (theEvents & Task::kStartEvent)
    {
        SDPContainer theSDP;
        if ( !theSDP.SetSDPBuffer(this->GetSDPInfo()->GetSDPData()) )
        {
            fDeathReason = kBadSDP;
            fDeathState = fState;
            fState = kDone;
            return 0;
        }
        sActiveConnections++;
        sTotalConnectionAttempts++;
        Assert(theEvents == Task::kStartEvent);
    }
    
    // 
    if (theEvents & Task::kTimeoutEvent)
    {
#if BROADCAST_SESSION_DEBUG
        qtss_printf("Session timing out.\n");
#endif
#if __FreeBSD__ || __MacOSX__
        if (fTransportType != kTCPTransportType)
        {
            fTimeoutTask.RefreshTimeout();
            fSocket->GetSocket()->SetTask(this);
            fSocket->GetSocket()->RequestEvent(fSocket->GetEventMask());                
            return 10000;
        }
#else
        if (fTransportType != kTCPTransportType)
        {   fTimeoutTask.RefreshTimeout();
            return 0;
        }
#endif

        fDeathReason = kSessionTimedout;
        fTeardownImmediately = true;
        fState = kSendingTeardown;
    }
                
    //
    // If we've been told to TEARDOWN, do so.
    if (theEvents & BroadcasterSession::kTeardownEvent && fState != kDone)
    {   

#if BROADCAST_SESSION_DEBUG
        qtss_printf("Session tearing down immediately.\n");
#endif
        fTeardownImmediately = true;
        fState = kSendingTeardown;
    }
    
    // We have been told to delete ourselves. Do so... NOW!!!!!!!!!!!!!!!
    if (theEvents & Task::kKillEvent || fTeardownImmediately)
    {
#if BROADCAST_SESSION_DEBUG
        qtss_printf("Session killed.\n");
#endif
        if (fState != kSendingTeardown )
        {
            
            sActiveConnections--;
            // return -1;
            fTeardownImmediately = true;
            fState = kSendingTeardown;
        }
    }   
    
    // Refresh the timeout. There is some legit activity going on...
    //fTimeoutTask.RefreshTimeout();
    
    OS_Error theErr = OS_NoErr;
    
    while ((theErr == OS_NoErr) && (fState != kDone))
    {
        //
        // Do the appropriate thing depending on our current state
        switch (fState)
        {
            case kSendingAnnounce:
            {
#if BROADCAST_SESSION_DEBUG
                qtss_printf("BroadcasterSession::kSendingAnnounce\n");
#endif              
                
                char *theSDP = (fSDPParser.GetSDPData())->Ptr;
                theErr = fRTSPClient->SendAnnounce(theSDP);
                
#if BROADCAST_SESSION_DEBUG
                qtss_printf("BroadcasterSession::Run Sending ANNOUNCE. Result = %"_U32BITARG_". Response code = %"_U32BITARG_"\n", theErr, fRTSPClient->GetStatus());
#endif                                  
                if (theErr == OS_NoErr)
                {
                    // Check that the ANNOUNCE response is a 200 OK. If not, bail
                    if (fRTSPClient->GetStatus() != 200)
                    {
                        theErr = ENOTCONN; // Exit the state machine
                        break;
                    }
                    else
                    {
                        Assert(fControlType != kReliableUDPTransportType);
                            
                        //                          
                        //
                        // We have valid SDP. If this is a UDP connection, construct a UDP
                        // socket array to act as incoming sockets.
                        if ((fControlType == kUDPTransportType) && (fTransportType != kTCPTransportType))
                            this->SetupUDPSockets();
                        //
                        // Setup client stats
                        fStats = NEW TrackStats[fSDPParser.GetNumStreams()];
                        ::memset(fStats, 0, sizeof(TrackStats) * fSDPParser.GetNumStreams());
                    }
                    
                    fState = kSendingSetup;
                }
#if BROADCAST_SESSION_DEBUG
                else
                {
                    if ( (fRTSPClient->GetStatus() == 0) && ( (theErr == EAGAIN) || (theErr== EINPROGRESS)) )
                    {
                        qtss_printf("BroadcasterSession::Run Sending ANNOUNCE. Result = %"_U32BITARG_". Response code = %"_U32BITARG_"\n", theErr, fRTSPClient->GetStatus());
                    }
                }
#endif                                  
                break;
            }
            case kSendingSetup:
            {
#if BROADCAST_SESSION_DEBUG
                qtss_printf("BroadcasterSession::kSendingSetup\n");
#endif              
                // The SETUP request is different depending on whether we are interleaving or not
                if (fTransportType == kUDPTransportType)
                {
                    theErr = fRTSPClient->SendUDPSetup(fSDPParser.GetStreamInfo(fNumSetups)->fTrackID,
                                                fUDPSocketArray[fNumSetups*2]->GetLocalPort());
                }
                else if (fTransportType == kTCPTransportType)
                {   
                    theErr = fRTSPClient->SendTCPSetup(fSDPParser.GetStreamInfo(fNumSetups)->fTrackID,fNumSetups * 2, (fNumSetups * 2) +1);                 
#if BROADCAST_SESSION_DEBUG
                    qtss_printf("Sending SETUP #%"_U32BITARG_". Result = %"_U32BITARG_". Response code = %"_U32BITARG_"\n", fNumSetups, theErr, fRTSPClient->GetStatus());
                    if (theErr == EAGAIN || theErr == EINPROGRESS)
                    {   
                        if  (theErr == EAGAIN)
                            qtss_printf("BroadcasterSession::kSendingSetup EAGAIN\n");
                        if  (theErr == EINPROGRESS)
                            qtss_printf("BroadcasterSession::kSendingSetup EINPROGRESS\n");
                    
        //              fSocket->GetSocket()->RequestEvent(EV_RE | EV_WR);
        //              return 0;   
                    }
#endif              

                }

                // If this SETUP request / response is complete, check for errors, and if
                // it succeeded, move onto the next SETUP. If we're done setting up all tracks,
                // move onto PLAY.
                if (theErr == OS_NoErr)
                {
                    if (fRTSPClient->GetStatus() != 200)
                    {
                        theErr = ENOTCONN; // Exit the state machine
                        break;
                    }
                    else
                    {
                        // Record the server port for RTCPs.
                        
                        fStats[fNumSetups].fDestRTPPort = fRTSPClient->GetServerPort();
                        fStats[fNumSetups].fDestRTCPPort = fRTSPClient->GetServerPort() + 1;
                        fNumSetups++;
                        if (fNumSetups == fSDPParser.GetNumStreams())
                            fState = kSendingReceive;
                    }               
                }
                break;
            }
            case kSendingReceive:
            {                   
#if BROADCAST_SESSION_DEBUG
                qtss_printf("BroadcasterSession::kSendingReceive\n");
#endif              

                theErr = fRTSPClient->SendReceive(fStartPlayTimeInSec);
                
#if BROADCAST_SESSION_DEBUG
                qtss_printf("BroadcasterSession::fRTSPClient->SendReceive. Result = %"_U32BITARG_". Response code = %"_U32BITARG_"\n", theErr, fRTSPClient->GetStatus());
                if  (theErr == EAGAIN)
                    qtss_printf("BroadcasterSession::SendReceive EAGAIN\n");
                if  (theErr == EINPROGRESS)
                    qtss_printf("BroadcasterSession::SendReceive EINPROGRESS\n");
#endif              
                // If this PLAY request / response is complete, then we are done with setting
                // up all the client crap. Now all we have to do is send the data until it's
                // time to send the TEARDOWN
                if (theErr == OS_NoErr)
                {
                    if (fRTSPClient->GetStatus() != 200)
                    {
                        theErr = ENOTCONN; // Exit the state machine
                        break;
                    }
                        
                    // Mark down the SSRC for each track, if possible. 
                    for (UInt32 ssrcCount = 0; ssrcCount < fSDPParser.GetNumStreams(); ssrcCount++)
                    {
                        fStats[ssrcCount].fSSRC = fRTSPClient->GetSSRCByTrack(fSDPParser.GetStreamInfo(ssrcCount)->fTrackID);
                        if (fStats[ssrcCount].fSSRC != 0)
                            fStats[ssrcCount].fIsSSRCValid = true;
                    }

                    fState = kBroadcasting;
                    sBroadcastingConnections++;
                    
                    //
                    // Start our duration timer. Use this to figure out when to send a teardown
                    fPlayTime = fLastRTCPTime = OS::Milliseconds();     
                }
                break;
            }
#if __FreeBSD__ || __MacOSX__
            case kBroadcasting:
            {   
#if BROADCAST_SESSION_DEBUG
                //qtss_printf("BroadcasterSession::kBroadcasting\n");
#endif              
                theErr = OS_NoErr; // Ignore flow control errors here.          
                static char buffer[256];
                UInt32 outRecvLen = 0;
                theErr = fRTSPClient->GetSocket()->Read(buffer, 256, &outRecvLen);
                
                if (fTransportType == kTCPTransportType)
                {   theErr = SendWaitingPackets();

                    if (theErr == EAGAIN || theErr == EINPROGRESS || theErr == ETIMEDOUT)
                    {   fSocket->GetSocket()->SetTask(this);
                        fSocket->GetSocket()->RequestEvent(EV_WR);              
                        return 0; // important must be 0
                    }
                            
                }   
                else if (theErr == EAGAIN || theErr == EINPROGRESS || theErr == ETIMEDOUT)
                {   fSocket->GetSocket()->SetTask(this);
                    fSocket->GetSocket()->RequestEvent(EV_RE);  
                    return 10000;
                }

                if (theErr != OS_NoErr)
                {   
                    // we've encountered some fatal error, bail.
                    //qtss_printf("kBroadcasting FATAL ERROR err=%"_S32BITARG_"\n",theErr);
                    sBroadcastingConnections--;
                    break;
                }
                
                // no Err and nothing left to do so got to sleep for awhile.
                return fReadInterval;
            }
            
#else
            case kBroadcasting:
            {   
#if BROADCAST_SESSION_DEBUG
                qtss_printf("BroadcasterSession::kBroadcasting\n");
#endif              
                theErr = OS_NoErr; // Ignore flow control errors here.
                
                if (fTransportType == kTCPTransportType)
                    theErr = SendWaitingPackets();  
                else
                {   static char buffer[256];
                    UInt32 outRecvLen = 0;
                    theErr = fRTSPClient->GetSocket()->Read(buffer, 256, &outRecvLen);
                    if (theErr == EAGAIN || theErr == EINPROGRESS)
                    {   
                        fSocket->GetSocket()->SetTask(this);
                        fSocket->GetSocket()->RequestEvent(EV_RE);              
                        return 0;
                    }
                }
            
                if (theErr == EAGAIN || theErr == EINPROGRESS)
                {   
                    fSocket->GetSocket()->SetTask(this);
                    fSocket->GetSocket()->RequestEvent(EV_WR);              
                    return 0;
                }
                    
                if (theErr != OS_NoErr)
                {   
                    // we've encountered some fatal error, bail.
                    //qtss_printf("FATAL ERROR err=%"_S32BITARG_"\n",theErr);
                    sBroadcastingConnections--;
                    break;
                }
                
                // no Err and nothing left to do so got to sleep for awhile.
                return fReadInterval;
            }

#endif

            case kSendingTeardown:
            {   
#if BROADCAST_SESSION_DEBUG
                qtss_printf("BroadcasterSession::kSendingTeardown\n");
#endif              
                UInt16 maxTries = 10;
                do 
                {
                    theErr = fRTSPClient->SendTeardown();
                    OSThread::Sleep((unsigned int) 100);
                    maxTries --;
                }
                while (theErr == EAGAIN && maxTries > 0);
            
                theErr = 2;
            
                sActiveConnections--;
                
#if BROADCAST_SESSION_DEBUG
                qtss_printf("Sending TEARDOWN. Result = %"_U32BITARG_". Response code = %"_U32BITARG_"\n", theErr, fRTSPClient->GetStatus());
#endif              
                // Once the TEARDOWN is complete, we are done, so mark ourselves as dead, and wait
                // for the owner of this object to delete us
                fState = kDone;
                //exit (0);
                break;
            }               
        }
    }
        
    if ((theErr == EINPROGRESS) || (theErr == EAGAIN))
    {
#if BROADCAST_SESSION_DEBUG
        if  (theErr == EAGAIN)
            qtss_printf("BroadcasterSession::EAGAIN\n");
        if  (theErr == EINPROGRESS)
            qtss_printf("BroadcasterSession::EINPROGRESS\n");
#endif              
        //
        // Request an async event
        fSocket->GetSocket()->SetTask(this);
        fSocket->GetSocket()->RequestEvent(fSocket->GetEventMask() );
        return 100; // try again 
    }
    else if (theErr != OS_NoErr)
    {   
        //
#if BROADCAST_SESSION_DEBUG
        qtss_printf("BroadCasterSession::RUN FATAL ERROR err=%"_S32BITARG_"\n",theErr);
#endif              
        // We encountered some fatal error with the socket. Record this as a connection failure
        if (fState == kSendingTeardown)
            fDeathReason = kTeardownFailed;
        else if (fState == kBroadcasting)
            fDeathReason = kConnectionFailed;
        else if (fRTSPClient->GetStatus() >= 300)
            fDeathReason = kRequestFailed;
        else if ((fState == kSendingAnnounce) && (theErr == OS_NotEnoughSpace))
                        fDeathReason = kBadSDP;
                else
            fDeathReason = kConnectionFailed;

        fDeathState = fState;
        fState = kDone;
    }

#if BROADCAST_SESSION_DEBUG
    if (fState == kDone)
        qtss_printf("Client connection complete. Death reason = %"_U32BITARG_" RTSPClientStatus=%"_S32BITARG_"\n", fDeathReason,fRTSPClient->GetStatus());
#endif              
    
    return 0;
}



void    BroadcasterSession::SetupUDPSockets()
{

    static UInt16 sCurrentRTPPortToUse = 6970;
    static const UInt16 kMinRTPPort = 6970;
    static const UInt16 kMaxRTPPort = 36970;

    OS_Error theErr = OS_NoErr;
    
    //
    // Create a UDP socket pair (RTP, RTCP) for each stream
    fUDPSocketArray = NEW UDPSocket*[fSDPParser.GetNumStreams() * 2];
    for (UInt32 x = 0; x < fSDPParser.GetNumStreams() * 2; x++)
    {
        fUDPSocketArray[x] = NEW UDPSocket(this, Socket::kNonBlockingSocketType);
        theErr = fUDPSocketArray[x]->Open();
        if (theErr != OS_NoErr)
        {
            qtss_printf("BroadcasterSession: Failed to open a UDP socket.\n");
            ::exit(-1);
        }
    }
    
    for (UInt32 y = 0; y < fSDPParser.GetNumStreams(); y++)
    {   
        for (UInt32 portCheck = 0; true; portCheck++)
        {
            theErr = fUDPSocketArray[y * 2]->Bind(INADDR_ANY, sCurrentRTPPortToUse);
            if (theErr == OS_NoErr)
                theErr = fUDPSocketArray[(y*2)+1]->Bind(INADDR_ANY, sCurrentRTPPortToUse + 1);

            sCurrentRTPPortToUse += 2;
            if (sCurrentRTPPortToUse > 30000)
                sCurrentRTPPortToUse = 6970;
                
            if (theErr == OS_NoErr)
            {
                // This is a good pair. Set the rcv buf on the RTP socket to be really big
                fUDPSocketArray[y * 2]->SetSocketRcvBufSize(fSockRcvBufSize);
                break;
            }
                
            if (sCurrentRTPPortToUse == kMaxRTPPort)
                sCurrentRTPPortToUse = kMinRTPPort;
            if (portCheck == 5000)
            {
                // Make sure we don't loop forever trying to bind a UDP socket. If we can't
                // after a certain point, just bail...
                qtss_printf("BroadcasterSession: Failed to bind a UDP socket.\n");
                ::exit(-1);
            }
        }
    }                       
#if BROADCAST_SESSION_DEBUG
    qtss_printf("Opened UDP sockets for %"_U32BITARG_" streams\n", fSDPParser.GetNumStreams());
#endif              
}

OS_Error    BroadcasterSession::ReadMediaData()
{
    // For iterating over the array of UDP sockets
    UInt32 theUDPSockIndex = 0;
    OS_Error theErr = OS_NoErr;
    
    while (true)
    {
        //
        // If the media data is being interleaved, get it from the control connection
        UInt32 theTrackID = 0;
        UInt32 theLength = 0;
        Bool16 isRTCP = false;
        char* thePacket = NULL;

        if (fTransportType == kTCPTransportType)
        {
            thePacket = NULL;
            theErr = fRTSPClient->GetMediaPacket(&theTrackID, &isRTCP, &thePacket, &theLength);
            if (thePacket == NULL)
                break;
        }
        else
        {
            static const UInt32 kMaxPacketSize = 2048;
            
            UInt32 theRemoteAddr = 0;
            UInt16 theRemotePort = 0;
            char thePacketBuf[kMaxPacketSize];
            
            // Get a packet from one of the UDP sockets.
            theErr = fUDPSocketArray[theUDPSockIndex]->RecvFrom(&theRemoteAddr, &theRemotePort,
                                                                &thePacketBuf[0], kMaxPacketSize,
                                                                &theLength);
            if ((theErr != OS_NoErr) || (theLength == 0))
            {
                
                theUDPSockIndex++;
                if (theUDPSockIndex == fSDPParser.GetNumStreams() * 2)
                    break;
                continue;
            }
            
            theTrackID = fSDPParser.GetStreamInfo(theUDPSockIndex >> 1)->fTrackID;
            isRTCP = (Bool16) (theUDPSockIndex & 1);
            thePacket = &thePacketBuf[0];
        }
        
        //
        // We have a valid packet. Invoke the packet handler function
        this->ProcessMediaPacket(thePacket, theLength, theTrackID, isRTCP);
    }
    return theErr;
}

void    BroadcasterSession::ProcessMediaPacket( char* inPacket, UInt32 inLength,
                                            UInt32 inTrackID, Bool16 isRTCP)
{
    Assert(inLength > 4);
    
    // Currently we do nothing with RTCPs.
    if (isRTCP)
        return;
    
    UInt16* theSeqNumP = (UInt16*)inPacket;
    UInt16 theSeqNum = ntohs(theSeqNumP[1]);
    
//  UInt32* theSsrcP = (UInt32*)inPacket;
//  UInt32 theSSRC = ntohl(theSsrcP[2]);
    
    for (UInt32 x = 0; x < fSDPParser.GetNumStreams(); x++)
    {
        if (fSDPParser.GetStreamInfo(x)->fTrackID == inTrackID)
        {
            // Check if this packet is even for our stream
            //if (!fStats[x].fIsSSRCValid)
            //  fStats[x].fSSRC = theSSRC; // If we don't know SSRC yet, just use first one we get
            //if (theSSRC != fStats[x].fSSRC)
            //  return;
                
            fStats[x].fNumPacketsReceived++;
            fStats[x].fNumBytesReceived += inLength;
            
            // Check if this packet is out of order
            if (fStats[x].fHighestSeqNumValid)
            {                       
                SInt16 theValidationDifference = theSeqNum - fStats[x].fWrapSeqNum;
                if (theValidationDifference < 0)
                    theValidationDifference -= 2 * theValidationDifference; // take the absolute value
                if (theValidationDifference > kSanitySeqNumDifference)
                {
                    //
                    // If this sequence number is really far out of range, then just toss
                    // the packet and increment our count of crazy packets
                    fStats[x].fNumThrownAwayPackets++;
                    return;
                }
            
                SInt16 theSeqNumDifference = theSeqNum - fStats[x].fHighestSeqNum;

                if (theSeqNumDifference > 0)
                {
                    fStats[x].fNumOutOfOrderPackets += theSeqNumDifference - 1;
                    fStats[x].fHighestSeqNum = theSeqNum;
                }
            }
            else
            {
                fStats[x].fHighestSeqNumValid = true;
                fStats[x].fWrapSeqNum = fStats[x].fHighestSeqNum = theSeqNum;
                fStats[x].fLastAckedSeqNum = theSeqNum - 1;
            }
            
            UInt32 debugblah = 0;
            
            // Put this sequence number into the map to track packet loss
            while ((theSeqNum - fStats[x].fWrapSeqNum) > TrackStats::kSeqNumMapSize)
            {
                debugblah++;
                Assert(debugblah < 10);

                // We've cycled through the entire map. Calculate packet
                // loss on the lowest 50 indexes in the map (don't get too
                // close to where we are lest we mistake out of order packets
                // as packet loss)
                UInt32 halfSeqNumMap = TrackStats::kSeqNumMapSize / 2;
                UInt32 curIndex = (fStats[x].fWrapSeqNum + 1) % TrackStats::kSeqNumMapSize;
                UInt32 numPackets = 0;
                
                for (UInt32 y = 0; y < halfSeqNumMap; y++, curIndex++)
                {
                    if (curIndex == TrackStats::kSeqNumMapSize)
                        curIndex = 0;
                    
                    if (fStats[x].fSequenceNumberMap[curIndex] > 0)
                        numPackets++;
                    fStats[x].fSequenceNumberMap[curIndex] = 0;
                }
                
                // We've figured out how many lost packets there are in the lower
                // half of the map. Increment our counters.
                fStats[x].fNumOutOfOrderPackets -= halfSeqNumMap - numPackets;
                fStats[x].fNumLostPackets += halfSeqNumMap - numPackets;
                fStats[x].fWrapSeqNum += (UInt16) halfSeqNumMap;

#if BROADCAST_SESSION_DEBUG
                qtss_printf("Got %"_U32BITARG_" packets for trackID %"_U32BITARG_". %"_U32BITARG_" packets lost, %"_U32BITARG_" packets out of order\n", fStats[x].fNumPacketsReceived, inTrackID, fStats[x].fNumLostPackets, fStats[x].fNumOutOfOrderPackets);
#endif              
            }
            
            //
            // Track duplicate packets
            if (fStats[x].fSequenceNumberMap[theSeqNum % 100])
                fStats[x].fNumDuplicates++;
                
            fStats[x].fSequenceNumberMap[theSeqNum % 100] = 1;
            theSeqNum = 0;
        }
    }
    Assert(theSeqNum == 0); // We should always find a track with this track ID
}

void BroadcasterSession::AckPackets(UInt32 inTrackIndex, UInt16 inCurSeqNum, Bool16 inCurSeqNumValid)
{
#if 0
    char theRRBuffer[256];
    UInt32  *theWriterStart = (UInt32*)theRRBuffer;
    UInt32  *theWriter = (UInt32*)theRRBuffer;

    // APP PACKET - QoS info
    *(theWriter++) = htonl(0x80CC0000); 
    //*(ia++) = htonl(trk[i].TrackSSRC);
    *(theWriter++) = htonl(0);
    *(theWriter++) = htonl('ack ');
    *(theWriter++) = htonl(0);
    
    SInt16 theSeqNumDifference = inCurSeqNum - fStats[inTrackIndex].fHighestSeqNum;
    
    if (!inCurSeqNumValid)
    {
        theSeqNumDifference = 1;
        inCurSeqNum = fStats[inTrackIndex].fHighestSeqNum;
    }
#if BROADCAST_SESSION_DEBUG
    qtss_printf("Highest seq num: %d\n", inCurSeqNum);
#endif
        
    if (theSeqNumDifference > 0)
    {
        *(theWriter++) = htonl(fStats[inTrackIndex].fLastAckedSeqNum + 1);
#if BROADCAST_SESSION_DEBUG
        qtss_printf("TrackID: %d Acking: %d\n", fSDPParser.GetStreamInfo(inTrackIndex)->fTrackID, fStats[inTrackIndex].fLastAckedSeqNum + 1);
#endif

        UInt16 maskPosition = fStats[inTrackIndex].fLastAckedSeqNum + 2;
        SInt16 numPacketsInMask = inCurSeqNum - (fStats[inTrackIndex].fLastAckedSeqNum + 2);
        
#if BROADCAST_SESSION_DEBUG
        qtss_printf("NumPacketsInMask: %d\n", numPacketsInMask);
#endif
        for (SInt32 y = 0; y < numPacketsInMask; y+=32)
        {
            UInt32 mask = 0;
            for (UInt32 x = 0; x < 32; x++)
            {
                SInt16 offsetFromHighest = fStats[inTrackIndex].fHighestSeqNum - maskPosition;
                mask <<= 1;
    
                if (offsetFromHighest >= 0)
                {
#if BROADCAST_SESSION_DEBUG
                    qtss_printf("TrackID: %d Acking in mask: %d\n", fSDPParser.GetStreamInfo(inTrackIndex)->fTrackID, maskPosition);
#endif
                    mask |= 1;
                }
                else if (maskPosition == inCurSeqNum)
                {
#if BROADCAST_SESSION_DEBUG
                    qtss_printf("TrackID: %d Acking in mask: %d\n", fSDPParser.GetStreamInfo(inTrackIndex)->fTrackID, inCurSeqNum);
#endif
                    mask |= 1;
                }

                maskPosition++;
            }
            
            // We have 1 completed mask. Add it to the packet
            *(theWriter++) = htonl(mask);
        }
        fStats[inTrackIndex].fLastAckedSeqNum = inCurSeqNum;
    }
    else
    {
        // Just ack cur seq num, this is an out of order packet
        *(theWriter++) = htonl(inCurSeqNum);
    }

    //
    // Set the packet length
    UInt16* lenP = (UInt16*)theRRBuffer;
    lenP[1] = htons((theWriter - theWriterStart) - 1); //length in octets - 1
    
    // Send the packet
    Assert(fStats[inTrackIndex].fDestRTCPPort != 0);
    fUDPSocketArray[(inTrackIndex*2)+1]->SendTo(fSocket->GetHostAddr(), fStats[inTrackIndex].fDestRTCPPort, theRRBuffer,
                                                (theWriter - theWriterStart) * sizeof(UInt32));

    //
    // Update the stats for this track
    fStats[inTrackIndex].fNumAcks++;
#endif
}


void BroadcasterSession::SendReceiverReport()
{
#if 0
    if (fUDPSocketArray == NULL)
        return;
        
    //
    // build the RTCP receiver report.
    char theRRBuffer[256];
    UInt32  *theWriterStart = (UInt32*)theRRBuffer;
    UInt32  *theWriter = (UInt32*)theRRBuffer;

    // RECEIVER REPORT
    *(theWriter++) = htonl(0x81c90007);     // 1 src RR packet
    //*(theWriter++) = htonl(trk[i].rcvrSSRC);
    *(theWriter++) = htonl(0);
    //*(theWriter++) = htonl(trk[i].TrackSSRC);
    *(theWriter++) = htonl(0);
    //if (trk[i].rtp_num_received > 0)
    //  t = ((float)trk[i].rtp_num_lost / (float)trk[i].rtp_num_received) * 256;
    //else
    //  t = 0.0;
    //temp = t;
    //temp = (temp & 0xff) << 24;
    //temp |= (trk[i].rtp_num_received & 0x00ffffff);
    *(theWriter++) = htonl(0);
    //temp = (trk[i].seq_num_cycles & 0xffff0000) | (trk[i].last_seq_num & 0x0000ffff);
    //*(ia++) = toBigEndian_ulong(temp);
    *(theWriter++) = htonl(0);
    *(theWriter++) = 0;                         // don't do jitter yet.
    *(theWriter++) = 0;                         // don't do last SR timestamp
    *(theWriter++) = 0;                         // don't do delay since last SR

    // APP PACKET - QoS info
    *(theWriter++) = htonl(0x80CC000C); 
    //*(ia++) = htonl(trk[i].TrackSSRC);
    *(theWriter++) = htonl(0);
// this QTSS changes after beta to 'qtss'
    *(theWriter++) = htonl('QTSS');
    //*(ia++) = toBigEndian_ulong(trk[i].rcvrSSRC);
    *(theWriter++) = htonl(0);
    *(theWriter++) = htonl(8);          // eight 4-byte quants below
#define RR 0x72720004
#define PR 0x70720004
#define PD 0x70640002
#define PL 0x706C0004
    *(theWriter++) = htonl(RR);
    //unsigned int now, secs;
    //now = microseconds();
    //secs = now - trk[i].last_rtcp_packet_sent_us / USEC_PER_SEC;
    //if (secs)
    //  temp = trk[i].bytes_received_since_last_rtcp / secs;
    //else
    //  temp = 0;
    //*(ia++) = htonl(temp);
    *(theWriter++) = htonl(0);
    *(theWriter++) = htonl(PR);
    //*(ia++) = htonl(trk[i].rtp_num_received);
    *(theWriter++) = htonl(0);
    *(theWriter++) = htonl(PL);
    //*(ia++) = htonl(trk[i].rtp_num_lost);
    *(theWriter++) = htonl(0);
    *(theWriter++) = htonl(PD);
    *(theWriter++) = htonl(0);      // should be a short, but we need to pad to a long for the entire RTCP app packet

#if BROADCAST_SESSION_DEBUG
    qtss_printf("Sending receiver reports.\n");
#endif              
    // Send the packet
    for (UInt32 x = 0; x < fSDPParser.GetNumStreams(); x++)
    {
        Assert(fStats[x].fDestRTCPPort != 0);
        fUDPSocketArray[(x*2)+1]->SendTo(fSocket->GetHostAddr(), fStats[x].fDestRTCPPort, theRRBuffer,
                                                            (theWriter - theWriterStart) * sizeof(UInt32));
    }

#endif
}
