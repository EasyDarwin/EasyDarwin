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
    File:       ClientSession.h

    
*/

#ifndef __CLIENT_SESSION__
#define __CLIENT_SESSION__

#include "Task.h"
#include "TimeoutTask.h"

#include "SVector.h"
#include "RTSPClient.h"
#include "ClientSocket.h"
#include "SDPSourceInfo.h"
#include "UDPSocket.h"
#include "PlayerSimulator.h"

using namespace EasyDarwin;

class ClientSession : public Task
{
    public:
    
        enum
        {
            kRTSPUDPClientType          = 0,
            kRTSPTCPClientType          = 1,
            kRTSPHTTPClientType         = 2,
            kRTSPHTTPDropPostClientType = 3,
            kRTSPReliableUDPClientType  = 4
        };
        typedef UInt32 ClientType;
    
        //The constructor will signal itself with Task::kStartEvent
        ClientSession(  UInt32 inAddr, UInt16 inPort, char* inURL,
                        ClientType inClientType,
                        UInt32 inDurationInSec, UInt32 inStartPlayTimeInSec,
                        UInt32 inRTCPIntervalInMS, UInt32 inOptionsIntervalInSec,
                        UInt32 inHTTPCookie, bool inAppendJunkData, UInt32 inReadInterval,
                        UInt32 inSockRcvBufSize, Float32 inLateTolerance, char* inMetaInfoFields,
                        Float32 inSpeed, UInt32 verboseLevel, char* inPacketRangePlayHeader, UInt32 inOverbufferWindowSizeInK,
                        bool sendOptions, bool requestRandomData, SInt32 randomDataSize, bool enable3GPP,
                        UInt32 GBW = 0, UInt32 MBW = 0, UInt32 MTD = 0, bool enableForcePlayoutDelay = false, UInt32 playoutDelay = 0,
                        UInt32 bandwidth = 0, UInt32 bufferSpace = 0, UInt32 delayTime = 0, UInt32 startPlayDelay = 0,
                        char *controlID = NULL, char *name = NULL, char *password = NULL);

        virtual ~ClientSession();
        
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
            kSendingOptions     = 0,
            kSendingDescribe    = 1,
            kSendingSetup       = 2,
            kSendingPlay        = 3,
            kPlaying            = 4,
            kSendingTeardown    = 5,
            kDone               = 6
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
            kDiedWhilePlaying   = 6     // Connection was forceably closed while playing the movie
        };
        
        //
        // Once this client session is completely done with the TEARDOWN and ready to be
        // destructed, this will return true. Until it returns true, this object should not
        // be deleted. When it does return true, this object should be deleted.
        bool  IsDone()        { return fState == kDone; }
        
        //
        // ACCESSORS
    
        RTSPClient*             GetClient()         { return fClient; }
        ClientSocket*           GetSocket()         { return fSocket; }
        SDPSourceInfo*          GetSDPInfo()        { return &fSDPParser; }
        UInt32                  GetState()          { return fState; }
        
        // When this object is in the kDone state, this will tell you why the session died.
        UInt32                  GetReasonForDying() { return fDeathReason; }
        UInt32                  GetRequestStatus()  { return fClient->GetStatus(); }
        
        // Tells you the total time we were receiving packets. You can use this
        // for computing bit rate
        SInt64                  GetTotalPlayTimeInMsec() { return fTotalPlayTime; }
        
        QTSS_RTPPayloadType     GetTrackType(UInt32 inTrackIndex)
                                    { return fSDPParser.GetStreamInfo(inTrackIndex)->fPayloadType; }
        UInt32                  GetNumPacketsReceived(UInt32 inTrackIndex)					{ return fStats[inTrackIndex].fNumPacketsReceived; }
        UInt32                  GetNumBytesReceived(UInt32 inTrackIndex)					{ return fStats[inTrackIndex].fNumBytesReceived; }
        UInt32                  GetNumPacketsOutOfOrder(UInt32 inTrackIndex)				{ return fStats[inTrackIndex].fNumOutOfOrderPackets; }
        UInt32                  GetNumOutOfBoundPackets(UInt32 inTrackIndex)				{ return fStats[inTrackIndex].fNumOutOfBoundPackets; }
        UInt32                  GetNumAcks(UInt32 inTrackIndex)								{ return fStats[inTrackIndex].fNumAcks; }
        UInt32                  Get3gNumPacketsLost(UInt32 inTrackIndex)                    { return fPlayerSimulator.GetNumPacketsLost(inTrackIndex); }
        UInt32                  Get3gNumDuplicates(UInt32 inTrackIndex)                     { return fPlayerSimulator.GetNumDuplicates(inTrackIndex); }
        UInt32                  Get3gNumLatePackets(UInt32 inTrackIndex)                    { return fPlayerSimulator.GetNumLatePackets(inTrackIndex); }
        UInt32                  Get3gNumBufferOverflowedPackets(UInt32 inTrackIndex)        { return fPlayerSimulator.GetNumBufferOverflowedPackets(inTrackIndex); }
		//include packets with bad SSRC
        UInt32                  GetNumMalformedPackets(UInt32 inTrackIndex)
                                    { return fStats[inTrackIndex].fNumMalformedPackets; }
									
		//Will reset the counter everytime it is called
        UInt32   GetSessionPacketsReceived()  { UInt32 result = fNumPacketsReceived; fNumPacketsReceived = 0; return result; }
        //
        // Global stats
        static UInt32   GetActiveConnections()          { return sActiveConnections; }
        static UInt32   GetPlayingConnections()         { return sPlayingConnections; }
        static UInt32   GetConnectionAttempts()         { return sTotalConnectionAttempts; }
		//The following two functions will reset the global counter every time it is called
        static UInt32   GetConnectionBytesReceived()    { UInt32 result = sBytesReceived; sBytesReceived = 0; return result; }
        static UInt32   GetConnectionPacketsReceived()  { UInt32 result = sPacketsReceived; sPacketsReceived = 0; return result; }
        
        
    private:
    
        enum
        {
            kRawRTSPControlType         = 0,
            kRTSPHTTPControlType        = 1,
            kRTSPHTTPDropPostControlType= 2
        };
        typedef UInt32 ControlType;
        
        enum
        {
            kUDPTransportType           = 0,
            kReliableUDPTransportType   = 1,
            kTCPTransportType           = 2
        };
        typedef UInt32 TransportType;

		//Returns kUInt32_Max if there is no track with such trackID
		UInt32 TrackID2TrackIndex(UInt32 trackID)
		{
			for (UInt32 trackIndex = 0; trackIndex < fSDPParser.GetNumStreams(); trackIndex++)
			{
				if (fSDPParser.GetStreamInfo(trackIndex)->fTrackID == trackID)
					return trackIndex;
			}	
			return kUInt32_Max;
		}
        
        ClientSocket*   fSocket;    // Connection object
        RTSPClient*     fClient;    // Manages the client connection
        SDPSourceInfo   fSDPParser; // Parses the SDP in the DESCRIBE response
        TimeoutTask     fTimeoutTask; // Kills this connection in the event the server isn't responding
        
        ControlType     fControlType;
        TransportType   fTransportType;
        UInt32          fDurationInSec;
        UInt32          fStartPlayTimeInSec;
        UInt32          fRTCPIntervalInMs;
        UInt32          fOptionsIntervalInSec;
        
        bool          fOptions;
        bool          fOptionsRequestRandomData;
        SInt32          fOptionsRandomDataSize;
        SInt64          fTransactionStartTimeMilli;

        UInt32          fState;     // For managing the state machine
        UInt32          fDeathReason;
        UInt32          fNumSetups;
        UDPSocket**     fUDPSocketArray;
        
		//these values starts as soon as the RTSP Play is completed; does not corresonds to actual media play time
        SInt64          fPlayTime;
        SInt64          fTotalPlayTime;
        SInt64          fLastRTCPTime;
        
        bool          fTeardownImmediately;
        bool          fAppendJunk;
        UInt32          fReadInterval;
        UInt32          fSockRcvBufSize;
        
        Float32         fSpeed;
        char*           fPacketRangePlayHeader;

        //These values are for the wireless links only -- not end-to-end
        //Units are in kbps, milliseconds, and bytes
        UInt32 fGuarenteedBitRate;
        UInt32 fMaxBitRate;
        UInt32 fMaxTransferDelay;
		bool fEnableForcePlayoutDelay;
		UInt32 fPlayoutDelay;
        UInt32 fBandwidth;				//bps
		//the buffer space is per stream, not total space
		UInt32 fBufferSpace;
		UInt32 fDelayTime;				//target buffering delay
		UInt32 fStartPlayDelay;			//how much buffer should we keep before we start playing? in milliseconds
		bool fEnable3GPP;

        // Client stats
        struct TrackStats
        {
            //Modified by ClientSession
            UInt32				fNumPacketsReceived;                //track only good packets(but include late and duplicates)
            UInt32				fNumBytesReceived;                  //includes RTP header
			UInt32				fNumOutOfOrderPackets;				//excludes duplicates
            UInt32				fNumOutOfBoundPackets;
            UInt32				fNumMalformedPackets;				//include packets with bad SSRC
            UInt32				fNumAcks;							//cumulative; counts ACK packets with masks as 1 ACK
			
            UInt16				fDestRTCPPort;			
			UInt32				fServerSSRC;						//0 for not available
			UInt32				fClientSSRC;
			
			//Used for the DLSR and LSR field of the RTCP
			SInt64				fLastSenderReportNTPTime;
			SInt64				fLastSenderReportLocalTime;

			//These values are used to calculate the fraction lost and cumulative number of packets lost field in the RTCP RR packet.
			//See RFC 3550 6.4.1 and A.3
	
            //fHighestSeqNum is the highest valid sequence number received; note that this is 32 bits so that it never overflows.
            //An initial value of kUInt32_Max is used as an invalid marker(such that no valid sequence number has been received yet).
            UInt32				fHighestSeqNum;
			UInt32				fBaseSeqNum;
			UInt32				fExpectedPrior;
			UInt32				fReceivedPrior;

            SVector<UInt32> fPacketsToAck;
            TrackStats() : fNumPacketsReceived(0), fNumBytesReceived(0), fNumOutOfOrderPackets(0), fNumOutOfBoundPackets(0),
					fNumMalformedPackets(0), fNumAcks(0), fDestRTCPPort(0),	fServerSSRC(0), fClientSSRC(0), fLastSenderReportNTPTime(0),
					fLastSenderReportLocalTime(0), fHighestSeqNum(kUInt32_Max), fBaseSeqNum(0), fExpectedPrior(0), fReceivedPrior(0)
            { }
        };

        /* Client stats
        struct TrackStats
        {
            enum
            {
                kSeqNumMapSize = 100,
                kHalfSeqNumMap = 50
            };
        
            UInt16          fDestRTCPPort;
            UInt32          fNumPacketsReceived;
            UInt32          fNumBytesReceived;
            UInt32          fNumLostPackets;
            UInt32          fNumOutOfOrderPackets;
            UInt32          fNumThrownAwayPackets;
            UInt8           fSequenceNumberMap[kSeqNumMapSize];
            UInt16          fWrapSeqNum;
            UInt32          fSSRC;
            bool          fIsSSRCValid;
            
            UInt16          fHighestSeqNum;
            UInt16          fLastAckedSeqNum;
            bool          fHighestSeqNumValid;
            
            UInt32          fNumAcks;
            UInt32          fNumDuplicates;
            
        };
        */
        UInt32              fOverbufferWindowSizeInK;
        UInt32              fCurRTCPTrack;						//track index not track id
        UInt32              fNumPacketsReceived;                //track only good packets(but include late and duplicates; see RFC3550 6.4.1)
		UInt32				fNumBytesReceived;                  //includes RTP header

        UInt32              fVerboseLevel;

        SVector<TrackStats> fStats;             //the index of this vector is the same as fSDPParser.GetStreamInfo

        PlayerSimulator     fPlayerSimulator;

        //TrackStats*         fStats;

        //
        // Global stats
        static UInt32           sActiveConnections;
        static UInt32           sPlayingConnections;
        static UInt32           sTotalConnectionAttempts;
        static UInt32           sBytesReceived;
        static UInt32           sPacketsReceived;
		
        //
        // Helper functions for Run()
        void    SetupUDPSockets();
        void    ProcessRTPPacket(char* inPacket, UInt32 inLength, UInt32 inTrackID);
		void    ProcessRTCPPacket(char* inPacket, UInt32 inLength, UInt32 inTrackID);
        OS_Error    ReadMediaData();
		OS_Error    SendRTCPPackets(UInt32 trackIndex);
        void    SendAckPackets(UInt32 inTrackIndex);

		//Calculates the RTCP RR's fraction lost and cumulative number of packets lost field info.
		void CalcRTCPRRPacketsLost(UInt32 trackIndex, UInt8 &outFracLost, SInt32 &outCumLostPackets);

        //Returns kUInt32_Max if newSeqNum is out of bound, otherwise returns the corresponding 32 bit sequence number.
        static UInt32 CalcSeqNum(UInt32 referenceSeqNum, UInt16 newSeqNum);
};

#endif //__CLIENT_SESSION__
