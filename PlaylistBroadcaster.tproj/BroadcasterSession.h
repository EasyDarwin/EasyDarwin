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
    File:       BroadcasterSession.h

    
*/

#ifndef __BROADCASTER_SESSION__
#define __BROADCASTER_SESSION__


#include "Task.h"

#include "TimeoutTask.h"


#include "RTSPClient.h"
#include "ClientSocket.h"
#include "SDPSourceInfo.h"
#include "UDPSocket.h"
#include "StrPtrLen.h"
#include "OSMutex.h"

using namespace EasyDarwin;

class BroadcasterSession : public Task
{
    public:
    
        enum
        {
            kRTSPUDPBroadcasterType             = 0,
            kRTSPTCPBroadcasterType             = 1,
            kRTSPHTTPBroadcasterType            = 2,
            kRTSPHTTPDropPostBroadcasterType    = 3,
            kRTSPReliableUDPBroadcasterType     = 4
        };
        typedef UInt32 BroadcasterType;
    
        BroadcasterSession( UInt32 inAddr, UInt16 inPort, char* inURL,
                        BroadcasterType inBroadcasterType,
                        UInt32 inDurationInSec, UInt32 inStartPlayTimeInSec,
                        UInt32 inRTCPIntervalInSec, UInt32 inOptionsIntervalInSec,
                        UInt32 inHTTPCookie, Bool16 inAppendJunkData, UInt32 inReadInterval,
                        UInt32 inSockRcvBufSize, 
                        StrPtrLen *sdpSPLPtr,
                        char *namePtr,
                        char *passwordPtr,
                        Bool16  deepDebug,
                        Bool16 burst);
        virtual ~BroadcasterSession();
        
        //
        // Signals.
        //
        // Send a kKillEvent to delete this object.
        // Send a kTeardownEvent to tell the object to send a TEARDOWN and abort
        
        enum
        {
            kTeardownEvent = 0x00000100
        };
        
        virtual SInt64 Run();
        
        //
        // States. Find out what the object is currently doing
        enum
        {
            kSendingAnnounce    = 0,
            kSendingSetup       = 1,
            kSendingReceive     = 2,
            kBroadcasting       = 3,
            kSendingTeardown    = 4,
            kDone               = 5
        };
        //
        // Why did this session die?
        enum
        {
            kDiedNormally       = 0,    // Session went fine
            kTeardownFailed     = 1,    // Teardown failed, but session stats are all valid
            kRequestFailed      = 2,    // Session couldn't be setup because the server returned an error
            kBadSDP             = 3,    // Server sent back some bad SDP
            kSessionTimedout    = 4,    // Server not responding
            kConnectionFailed   = 5,    // Couldn't connect at all.
            kDiedWhileBroadcasting  = 6,        // Connection was forceably closed while Broadcasting the movie
            kMemoryError        = 7
        };
        
        //
        // Once this client session is completely done with the TEARDOWN and ready to be
        // destructed, this will return true. Until it returns true, this object should not
        // be deleted. When it does return true, this object should be deleted.
        Bool16  IsDone()        { return fState == kDone; }
        
        //
        // ACCESSORS
    
        RTSPClient*             GetBroadcaster()    { return fRTSPClient; }
        ClientSocket*           GetSocket()         { return fSocket; }
        SDPSourceInfo*          GetSDPInfo()        { return &fSDPParser; }
        UInt32                  GetState()          { return fState; }
        UInt32                  GetDeathState()     { return fDeathState; }
        
        // When this object is in the kDone state, this will tell you why the session died.
        UInt32                  GetReasonForDying() { return fDeathReason; }
        UInt32                  GetRequestStatus()  { return fRTSPClient->GetStatus(); }
        
        // Tells you the total time we were receiving packets. You can use this
        // for computing bit rate
        SInt64                  GetTotalPlayTimeInMsec() { return fTotalPlayTime; }
        
        QTSS_RTPPayloadType     GetTrackType(UInt32 inTrackIndex)
                                    { return fSDPParser.GetStreamInfo(inTrackIndex)->fPayloadType; }
        UInt32                  GetNumPacketsReceived(UInt32 inTrackIndex)
                                    { return fStats[inTrackIndex].fNumPacketsReceived; }
        UInt32                  GetNumBytesReceived(UInt32 inTrackIndex)
                                    { return fStats[inTrackIndex].fNumBytesReceived; }
        UInt32                  GetNumPacketsLost(UInt32 inTrackIndex)
                                    { return fStats[inTrackIndex].fNumLostPackets; }
        UInt32                  GetNumPacketsOutOfOrder(UInt32 inTrackIndex)
                                    { return fStats[inTrackIndex].fNumOutOfOrderPackets; }
        UInt32                  GetNumDuplicates(UInt32 inTrackIndex)
                                    { return fStats[inTrackIndex].fNumDuplicates; }
        UInt32                  GetNumAcks(UInt32 inTrackIndex)
                                    { return fStats[inTrackIndex].fNumAcks; }
        UInt32                  GetNumStreams()
                                    { return fSDPParser.GetNumStreams();}
        UInt32                  GetStreamDestPort(UInt32 inIndex)
                                    { return fStats[inIndex].fDestRTPPort;}
        
        void                    TearDownNow() {fTeardownImmediately = true; this->Signal(Task::kKillEvent);}
        //
        // Global stats
        static UInt32   GetActiveConnections()          { return sActiveConnections; }
        static UInt32   GetBroadcastingConnections()    { return sBroadcastingConnections; }
        static UInt32   GetConnectionAttempts()         { return sTotalConnectionAttempts; }
        char*           GetNextPacket(UInt32 *packetLen, UInt8 *channel);
        OS_Error        SendPacket(char* data, UInt32 len,UInt8 channel);
        OS_Error        SendWaitingPackets();
        
        enum
        {
            kUDPTransportType           = 0,
            kReliableUDPTransportType   = 1,
            kTCPTransportType           = 2
        };
        typedef UInt32 TransportType;

        TransportType       GetTransportType()  { return fTransportType; }
        
        UInt32              GetPacketQLen()     { return fPacketQueue.GetLength(); }
        OSMutex*            GetMutex()      { return &fMutex; }
    private:
    
        enum
        {
            kRawRTSPControlType         = 0,
            kRTSPHTTPControlType        = 1,
            kRTSPHTTPDropPostControlType= 2
        };
        typedef UInt32 ControlType;
        
        
        ClientSocket*   fSocket;    // Connection object
        RTSPClient*     fRTSPClient;    // Manages the client connection
        SDPSourceInfo   fSDPParser; // Parses the SDP in the DESCRIBE response
        TimeoutTask     fTimeoutTask; // Kills this connection in the event the server isn't responding
        
        ControlType     fControlType;
        TransportType   fTransportType;
        UInt32          fDurationInSec;
        UInt32          fStartPlayTimeInSec;
        UInt32          fRTCPIntervalInSec;
        UInt32          fOptionsIntervalInSec;
        
        UInt32          fState;     // For managing the state machine
        UInt32          fDeathState; // state at time of death
        UInt32          fDeathReason;
        UInt32          fNumSetups;
        UDPSocket**     fUDPSocketArray;
        
        SInt64          fPlayTime;
        SInt64          fTotalPlayTime;
        SInt64          fLastRTCPTime;
        
        Bool16          fTeardownImmediately;
        Bool16          fAppendJunk;
        UInt32          fReadInterval;
        UInt32          fSockRcvBufSize;
        Bool16          fBurst;
        UInt32          fBurstTime;
        //
        // Broadcaster stats
        struct TrackStats
        {
            enum
            {
                kSeqNumMapSize = 100,
                kHalfSeqNumMap = 50
            };
        
            UInt16          fDestRTPPort;
            UInt16          fDestRTCPPort;
            UInt32          fNumPacketsReceived;
            UInt32          fNumBytesReceived;
            UInt32          fNumLostPackets;
            UInt32          fNumOutOfOrderPackets;
            UInt32          fNumThrownAwayPackets;
            UInt8           fSequenceNumberMap[kSeqNumMapSize];
            UInt16          fWrapSeqNum;
            UInt16          fLastSeqNum;
            UInt32          fSSRC;
            Bool16          fIsSSRCValid;
            
            UInt16          fHighestSeqNum;
            UInt16          fLastAckedSeqNum;
            Bool16          fHighestSeqNumValid;
            
            UInt32          fNumAcks;
            UInt32          fNumDuplicates;
            
        };
        TrackStats*         fStats;
        
        static  char* fPacket;
        //
        // Global stats
        static UInt32           sActiveConnections;
        static UInt32           sBroadcastingConnections;
        static UInt32           sTotalConnectionAttempts;
        
        //
        // Helper functions for Run()
        void        SetupUDPSockets();
        void        ProcessMediaPacket(char* inPacket, UInt32 inLength, UInt32 inTrackID, Bool16 isRTCP);
        OS_Error    ReadMediaData();
        void        SendReceiverReport();
        void        AckPackets(UInt32 inTrackIndex, UInt16 inCurSeqNum, Bool16 inCurSeqNumValid);

        OSMutex             fMutex;//this data structure is shared!

        struct RTPPacket
        {
            RTPPacket() : fQueueElem(NULL), fData(NULL), fLen(0), fChannel(0),fCount(0){}
            ~RTPPacket() { fData = NULL; fLen = 0; fChannel = 0; }
            void SetEnclosingObject(void *obj) {fQueueElem.SetEnclosingObject(obj);}
            void SetPacketData(char* data, UInt32 len,UInt8 channel) { fData = data; fLen = len; fChannel = channel; }
            OSQueueElem *GetQElement() {return &fQueueElem;}
            OSQueueElem fQueueElem;
            char*   fData;
            UInt32  fLen;
            UInt8   fChannel;
            UInt64  fCount;
        };
        UInt64  fPacketCount;
        OSQueue fPacketQueue;
        
        UInt32 fPacketLen;
        UInt8 fChannel;
//      char* fPacket;
        

};

#endif //__BROADCASTER_SESSION__
