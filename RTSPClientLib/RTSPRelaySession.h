/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       RTSPRelaySession.h
*/

#ifndef __RTSP_RELAY_SESSION__
#define __RTSP_RELAY_SESSION__

#include "Task.h"
#include "TimeoutTask.h"
#include "RTSPClient.h"
#include "ClientSocket.h"
#include "SDPSourceInfo.h"
#include "UDPSocket.h"
#include "ReflectorSession.h"

#include "QTSServerInterface.h"

class RTSPRelaySession : public Task
{
    public:
        enum
        {
            kRTSPUDPClientType          = 0,
            kRTSPTCPClientType          = 1,
        };
        typedef UInt32 ClientType;
    
        RTSPRelaySession(UInt32 inAddr,					//IP
						UInt16 inPort,					//端口
						char* inURL,					//url
                        ClientType inClientType,		//RTP方式,TCP或UDP
                        UInt32 inRTCPIntervalInSec,		//RTCP间隔
						UInt32 inOptionsIntervalInSec,	//OPTIONS保活间隔
                        UInt32 inReadInterval,			//数据读取间隔
                        UInt32 inSockRcvBufSize,		//Socket缓存大小
                        Float32 inSpeed,				//播放速度
						char* inPacketRangePlayHeader,	//Play Range
						UInt32 inOverbufferWindowSizeInK,
                        Bool16 sendOptions,				//是否OPTIONS保活
						char *name = NULL,				//用户名
						char *password = NULL,			//密码
						const char* streamName = NULL);	//StreamName

        virtual ~RTSPRelaySession();
        
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
        Bool16  IsDone()        { return fState == kDone; }
        
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
        UInt32					GetSessionPacketsReceived()  
									{ UInt32 result = fNumPacketsReceived; fNumPacketsReceived = 0; return result; }
		OSRef*					GetRef()            
									{ return &fRef; } 
		void					SetReflectorSession(ReflectorSession* refSes) 
									{ fReflectorSession = refSes;}
		ReflectorSession*		GetReflectorSession() 
									{ return fReflectorSession;}
        OS_Error SendDescribe();
		OS_Error Start();
		OS_Error Reset();
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
        
        ClientSocket*   fSocket;    // Connection object
        RTSPClient*     fClient;    // Manages the client connection
        SDPSourceInfo   fSDPParser; // Parses the SDP in the DESCRIBE response
        TimeoutTask     fTimeoutTask; // Kills this connection in the event the server isn't responding

		ReflectorSession* fReflectorSession; //Reflect Session
        
        ControlType     fControlType;
        TransportType   fTransportType;
        UInt32          fRTCPIntervalInSec;
        UInt32          fOptionsIntervalInSec;
        
        Bool16          fOptions;
        SInt64          fTransactionStartTimeMilli;

        UInt32          fState;     // For managing the state machine
        UInt32          fDeathReason;
        UInt32          fNumSetups;
        UDPSocket**     fUDPSocketArray;
        
        SInt64          fPlayTime;
        SInt64          fTotalPlayTime;
        SInt64          fLastRTCPTime;
        
        Bool16          fTeardownImmediately;
        UInt32          fReadInterval;
        UInt32          fSockRcvBufSize;
        
        Float32         fSpeed;
        char*           fPacketRangePlayHeader;

		OSRef			fRef;
		StrPtrLen		fStreamName;
		UInt32			fAddr;
		UInt16			fPort;
		char*			fURL;
		char*			fName;
		char*			fPassword;
		

        // Client stats
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

        UInt32              fOverbufferWindowSizeInK;
        UInt32              fCurRTCPTrack;
        UInt32              fNumPacketsReceived;

        // Helper functions for Run()
        void    SetupUDPSockets();

        void    ProcessMediaPacket(char* inPacket, UInt32 inLength, UInt32 inTrackID, Bool16 isRTCP);
        OS_Error    ReadMediaData();
        OS_Error    SendReceiverReport(UInt32 inTrackID);
        void    AckPackets(UInt32 inTrackIndex, UInt16 inCurSeqNum, Bool16 inCurSeqNumValid);
};

#endif //__RTSP_RELAY_SESSION__
