/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       RTSPRelaySession.cpp
    Contains:   RTSP Relay Client
*/
#include "RTSPRelaySession.h"
#include "OSMemory.h"
#include <stdlib.h>
#include "SafeStdLib.h"

static const SInt64 kIdleTimeoutInMsec		= 30000;	// 30s timeout
static const SInt16 kSanitySeqNumDifference = 3000;		// RTP cseq check

char* ConvertBytesToCHexString( void* inValue, const UInt32 inValueLen)
{
    static const char* kHEXChars={ "0123456789ABCDEF" };
    UInt8* theDataPtr = (UInt8*) inValue;
    UInt32 len = inValueLen *2;

    char *theString = NEW char[len+1];
    char *resultStr = theString;
    if (theString != NULL)
    {
        UInt8 temp;
        UInt32 count = 0;
        for (count = 0; count < inValueLen; count++)
        {
            temp = *theDataPtr++;
            *theString++ = kHEXChars[temp >> 4];
            *theString++ = kHEXChars[temp & 0xF];
        }
        *theString = 0;
    }
    return resultStr;
}


RTSPRelaySession::RTSPRelaySession( UInt32 inAddr, UInt16 inPort, char* inURL,
									ClientType inClientType, UInt32 inRTCPIntervalInSec, UInt32 inOptionsIntervalInSec,
									UInt32 inReadInterval, UInt32 inSockRcvBufSize, Float32 inSpeed, 
									char* inPacketRangePlayHeader, UInt32 inOverbufferWindowSizeInK,
									Bool16 sendOptions,char *name, char *password, const char* streamName)
:   fSocket(NULL),
    fClient(NULL),
    fTimeoutTask(this, kIdleTimeoutInMsec),

    fRTCPIntervalInSec(inRTCPIntervalInSec),
    fOptionsIntervalInSec(inOptionsIntervalInSec),
    
    fOptions(sendOptions),
    fTransactionStartTimeMilli(0),

    fState(kSendingDescribe),
    fDeathReason(kDiedNormally),
    fNumSetups(0),
    fUDPSocketArray(NULL),
    
    fPlayTime(0),
    fTotalPlayTime(0),
    fLastRTCPTime(0),
    fTeardownImmediately(false),
    fReadInterval(inReadInterval),
    fSockRcvBufSize(inSockRcvBufSize),
    
    fSpeed(inSpeed),
    fPacketRangePlayHeader(inPacketRangePlayHeader),
    fStats(NULL),
    fOverbufferWindowSizeInK(inOverbufferWindowSizeInK),
    fCurRTCPTrack(0),
    fNumPacketsReceived(0),
	fReflectorSession(NULL),
	fStreamName(NULL),
	fAddr(inAddr),
	fPort(inPort),
	fName(NULL),
	fPassword(NULL),
	fURL(NULL)
{
    this->SetTaskName("RTSPRelaySession");
    StrPtrLen theURL(inURL);

	fURL = NEW char[::strlen(inURL) + 2];
	::strcpy(fURL, inURL);

	if (streamName != NULL)
    {
        fStreamName.Ptr = NEW char[strlen(streamName) + 1];
		::memset(fStreamName.Ptr,0,strlen(streamName) + 1);
        ::memcpy(fStreamName.Ptr, streamName, strlen(streamName));
        fStreamName.Len = strlen(streamName);
        fRef.Set(fStreamName, this);
    }

    if (true == sendOptions)
       fState = kSendingOptions;
       
	// Construct the appropriate ClientSocket type depending on what type of client we are supposed to be
    switch (inClientType)
    {
        case kRTSPUDPClientType:
        {
            fControlType = kRawRTSPControlType;
            fTransportType = kUDPTransportType;
            fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
            break;
        }
        case kRTSPTCPClientType:
        {
            fControlType = kRawRTSPControlType;
            fTransportType = kTCPTransportType;
            fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
            break;
        }
        default:
        {
            qtss_printf("RTSPRelaySession: Attempt to create unsupported client type.\n");
        }
    }
    
    fSocket->Set(inAddr, inPort);

    fClient = NEW RTSPClient(fSocket);
    fClient->Set(theURL);
	fClient->SetVerboseLevel(3);

    //username:password
    if (name != NULL && password != NULL)
    {
        fClient->SetName(name);
        fClient->SetPassword(password);
    }

	if (name != NULL)
	{
		fName = NEW char[::strlen(name) + 2];
		::strcpy(fName, name);
	}

	if (password != NULL)
	{
		fPassword = NEW char[::strlen(password) + 2];
		::strcpy(fPassword, password);
	}

	qtss_printf("\nNew Connection %s:%s\n",fStreamName.Ptr,fURL);
}

RTSPRelaySession::~RTSPRelaySession()
{
	qtss_printf("\nDisconnect %s:%s\n",fStreamName.Ptr,fURL);
    if (fUDPSocketArray != NULL)
    {
        for (UInt32 x = 0; x < fSDPParser.GetNumStreams() * 2; x++)
        {
            OS_Error theErr = OS_NoErr;
            
            while (theErr == OS_NoErr)
            {
                    UInt32 theRemoteAddr = 0;
                    UInt32 theLength = 0;
                    UInt16 theRemotePort = 0;
                    char thePacketBuf[2048];
            
                    // Get a packet from one of the UDP sockets.
                    theErr = fUDPSocketArray[x]->RecvFrom(&theRemoteAddr, &theRemotePort,
                                                                &thePacketBuf[0], 2048,
                                                                &theLength);
            }
            delete fUDPSocketArray[x];
        }
    }
    
	delete [] fStreamName.Ptr;
    delete [] fUDPSocketArray;
    delete fClient;
    delete fSocket;
	delete fStats;
	delete fName;
	delete fPassword;
}

SInt64 RTSPRelaySession::Run()
{
    EventFlags theEvents = this->GetEvents();

	//timeout event
    if (theEvents & Task::kTimeoutEvent)
    {
        fDeathReason = kSessionTimedout;
        fState = kDone;
        return 0;
    }

    //RTSP teardown event
    if (theEvents & RTSPRelaySession::kTeardownEvent)
    {
        fTeardownImmediately = true;
    }
    
    //Kill event
    if (theEvents & Task::kKillEvent)
    {
        return -1;
    }   
    
    //refresh timeout
    fTimeoutTask.RefreshTimeout();

    OS_Error theErr = OS_NoErr;
    while ((theErr == OS_NoErr) && (fState != kDone))
	{
        switch (fState)
        {
            case kSendingOptions:
            {
					theErr = fClient->SendOptions();
           
                    if (0 == fTransactionStartTimeMilli) 
                       fTransactionStartTimeMilli = OS::Milliseconds();

                if (theErr == OS_NoErr)
                {
                    // Check that the OPTIONS response is a 200 OK. If not, bail
                    if (fClient->GetStatus() != 200)
                    {
                        theErr = ENOTCONN; // Exit the state machine
                        break;
                    }
                    else
                    {
                        if (fClient->IsVerbose())
                        {
                            qtss_printf("--- Options transaction time ms = %qd  ---\n", (SInt64) ( OS::Milliseconds() - fTransactionStartTimeMilli) );
                            SInt32 receivedLength = (SInt32) fClient->GetContentLength();

                            StrPtrLenDel theBody(ConvertBytesToCHexString(fClient->GetContentBody(), receivedLength));
                            theBody.PrintStr("\n");
                        }
                        fState = kSendingDescribe;
                    }
                }
                break;
            }
            case kSendingDescribe:
            {
                theErr = fClient->SendDescribe();
          
                if (theErr == OS_NoErr)
                {
                    // Check that the DESCRIBE response is a 200 OK. If not, bail
                    if (fClient->GetStatus() != 200)
                    {
                        theErr = ENOTCONN; // Exit the state machine
                        break;
                    }
                    else
                    {
                        //赋值
                        fSDPParser.Parse(fClient->GetContentBody(), fClient->GetContentLength());
                        
                        //流数目检测
                        if (fSDPParser.GetNumStreams() == 0)
                            fDeathReason = kBadSDP;

                        //建立UDP端口
                        if (fTransportType == kUDPTransportType)
                            this->SetupUDPSockets();
                            
                        //客户端对收到的数据进行统计
                        fStats = NEW TrackStats[fSDPParser.GetNumStreams()];
                        ::memset(fStats, 0, sizeof(TrackStats) * fSDPParser.GetNumStreams());
                    }
                    fState = kSendingSetup;
				}				
				break;
            }
            case kSendingSetup:
            {
                // The SETUP request is different depending on whether we are interleaving or not
                if (fTransportType == kUDPTransportType)
                {
                    theErr = fClient->SendUDPSetup(fSDPParser.GetStreamInfo(fNumSetups)->fTrackID,
                                                fUDPSocketArray[fNumSetups*2]->GetLocalPort());
                }
                else if (fTransportType == kTCPTransportType)
                {
                    fSocket->SetRcvSockBufSize(fSockRcvBufSize); // Make the rcv buf really big
					theErr = fClient->SendTCPSetup(fSDPParser.GetStreamInfo(fNumSetups)->fTrackID,fNumSetups * 2, (fNumSetups * 2) +1,&fSDPParser.GetStreamInfo(fNumSetups)->fTrackName);
                }

                //循环SETUP，当所有的Track都建立后，进入PLAY
                if (theErr == OS_NoErr)
                {
                    if (fClient->GetStatus() != 200)
                    {
                        theErr = ENOTCONN; // Exit the state machine
                        break;
                    }
                    else
                    {
                        // Record the server port for RTCPs.
                        fStats[fNumSetups].fDestRTCPPort = fClient->GetServerPort() + 1;
                        
                        fNumSetups++;
                        if (fNumSetups == fSDPParser.GetNumStreams())
                            fState = kSendingPlay;
                    }               
                }
                break;
            }
            case kSendingPlay:
            {
                if (fPacketRangePlayHeader != NULL)
                    theErr = fClient->SendPacketRangePlay(fPacketRangePlayHeader, fSpeed);
                else
                    theErr = fClient->SendPlay(0, fSpeed);             
                // If this PLAY request / response is complete, then we are done with setting
                // up all the client crap. Now all we have to do is receive the data until it's
                // time to send the TEARDOWN
                if (theErr == OS_NoErr)
                {
                    if (fClient->GetStatus() != 200)
                    {
                        theErr = ENOTCONN; // Exit the state machine
                        break;
                    }
                        
                    // Mark down the SSRC for each track, if possible. 
                    for (UInt32 ssrcCount = 0; ssrcCount < fSDPParser.GetNumStreams(); ssrcCount++)
                    {
                        fStats[ssrcCount].fSSRC = fClient->GetSSRCByTrack(fSDPParser.GetStreamInfo(ssrcCount)->fTrackID);
                        if (fStats[ssrcCount].fSSRC != 0)
                            fStats[ssrcCount].fIsSSRCValid = true;
                    }
                    fState = kPlaying;                    
                    // Start our duration timer. Use this to figure out when to send a teardown
                    fPlayTime = fLastRTCPTime = OS::Milliseconds();     
                }
                break;
            }
            case kPlaying:
            {
               if (fCurRTCPTrack == fSDPParser.GetNumStreams())
                    theErr = this->ReadMediaData();

                // If we've encountered some fatal error, bail.
                if ((theErr != EINPROGRESS) && (theErr != EAGAIN) && (theErr != OS_NoErr))
                {
					qtss_printf("\nReconnect %s:%s\n",fStreamName.Ptr,fURL);
                    break;
                }
                theErr = OS_NoErr; // Ignore flow control errors here.

                SInt64 curTime = OS::Milliseconds();
                if (fTeardownImmediately)
                {
                    fState = kSendingTeardown;
                    fTotalPlayTime = curTime - fPlayTime;
                }
                else
				{
                    if ((curTime - fLastRTCPTime) > (fRTCPIntervalInSec * 1000))
                    {
                        //
                        // If we are using TCP as our media transport, we only need to
                        // send 1 set of RTCPs to the server, to tell it about overbuffering
                        if (fTransportType != kTCPTransportType)
                        {
							fCurRTCPTrack = 0;
							//(void) fClient->SendOptions();
                        }
                        fLastRTCPTime = curTime;
                    }
                        
                    for ( ; fCurRTCPTrack < fSDPParser.GetNumStreams(); fCurRTCPTrack++)
                    {
                        if (this->SendReceiverReport(fCurRTCPTrack) != OS_NoErr)
                            break;
                    }
                    return fReadInterval;
                }
                break;
            }
            case kSendingTeardown:
            {
                theErr = fClient->SendTeardown();
                if (theErr == OS_NoErr)
                    fState = kDone;
                    
                break;
            }               
        }
    }
    
    if ((theErr == EINPROGRESS) || (theErr == EAGAIN))
    {
        //
        // Request an async event
        fSocket->GetSocket()->SetTask(this);
        fSocket->GetSocket()->RequestEvent(fSocket->GetEventMask());
    }
    else if (theErr != OS_NoErr)
    {
        // We encountered some fatal error with the socket. Record this as a connection failure
        if (fState == kSendingTeardown)
            fDeathReason = kTeardownFailed;
        else if (fState == kPlaying)
            fDeathReason = kDiedWhilePlaying;
        else if (fClient->GetStatus() != 200)
            fDeathReason = kRequestFailed;
        else
            fDeathReason = kConnectionFailed;

        fState = kDone;
    }         

	if (fState == kDone)
	{
		Reset();
		return 100;
	}

    return 0;
}

OS_Error RTSPRelaySession::Start()
{  
	this->Signal(Task::kStartEvent);
	return OS_NoErr;
}

//重置重连属性
OS_Error RTSPRelaySession::Reset()
{
	//先向原连接发送TEARDOWN，不检查错误
	fClient->SendTeardown();

	//重置fUDPSocketArray
	if (fUDPSocketArray != NULL)
	{
		for (UInt32 x = 0; x < fSDPParser.GetNumStreams() * 2; x++)
		{
			OS_Error theErr = OS_NoErr;
	        
			while (theErr == OS_NoErr)
			{
					UInt32 theRemoteAddr = 0;
					UInt32 theLength = 0;
					UInt16 theRemotePort = 0;
					char thePacketBuf[2048];
	        
					// Get a packet from one of the UDP sockets.
					theErr = fUDPSocketArray[x]->RecvFrom(&theRemoteAddr, &theRemotePort,
																&thePacketBuf[0], 2048,
																&theLength);
			}
			delete fUDPSocketArray[x];
		}
	}
	delete [] fUDPSocketArray;
	fUDPSocketArray = NULL;

	//重置fStats
	delete fStats;
	fStats = NULL;

    fTransactionStartTimeMilli = 0;
    fDeathReason = kDiedNormally;
    fNumSetups = 0;
    fPlayTime = 0;
    fTotalPlayTime = 0;
    fLastRTCPTime = 0;
    fTeardownImmediately = false;
    fCurRTCPTrack = 0;
    fNumPacketsReceived = 0;

	//重置Socket
	delete fSocket;
	fSocket = NULL;
    switch (fTransportType)
    {
        case kUDPTransportType:
        {
            fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
            break;
        }
        case kRTSPTCPClientType:
        {
            fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
            break;
        }
        default:
        {
            fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
        }
    }
	fSocket->Set(fAddr,fPort);


	//重置fClient
	delete fClient;
	fClient = NULL;
	fClient = NEW RTSPClient(fSocket);
	fClient->Set(fURL);
	fClient->SetVerboseLevel(3);
	fClient->SetName(fName);
	fClient->SetPassword(fPassword);


	//重置fState
	fState = kSendingDescribe;

	return OS_NoErr;
}

//发送DESCRIBE,获取SDP信息
OS_Error RTSPRelaySession::SendDescribe()
{
    fTimeoutTask.RefreshTimeout();
    
    OS_Error theErr = OS_NoErr;
	while (fState == kSendingDescribe)
	{
        theErr = fClient->SendDescribe();
  
        if (theErr == OS_NoErr)
		{
            if (fClient->GetStatus() != 200)
            {
                theErr = ENOTCONN;
                break;
            }
            else
            {
                fSDPParser.Parse(fClient->GetContentBody(), fClient->GetContentLength());

				//sdp信息错误
                if (fSDPParser.GetNumStreams() == 0)
				{
                    theErr = ENOTCONN;
					break;
				}

                //建立UDP端口
                if (fTransportType == kUDPTransportType)
                    this->SetupUDPSockets();

                // Setup client stats
                fStats = NEW TrackStats[fSDPParser.GetNumStreams()];
                ::memset(fStats, 0, sizeof(TrackStats) * fSDPParser.GetNumStreams());
            }
			fState = kSendingSetup;
			break;
        }
		//重试
		else if ((theErr == EINPROGRESS) || (theErr == EAGAIN))
		{
#ifndef __Win32__
			usleep(250*1000);
#else
			::Sleep(250);
#endif
		}
		//完成退出，返回成功或者失败
		else
			break;
    }
	return theErr;
}

void RTSPRelaySession::SetupUDPSockets()
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
            qtss_printf("ClientSession: Failed to open a UDP socket.\n");
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
                qtss_printf("ClientSession: Failed to bind a UDP socket.\n");
            }
        }
    }                                   
}

OS_Error RTSPRelaySession::ReadMediaData()
{
    // For iterating over the array of UDP sockets
    UInt32 theUDPSockIndex = 0;
    OS_Error theErr = OS_NoErr;
    
    while (true)
    {
        // If the media data is being interleaved, get it from the control connection
        UInt32 theTrackID = 0;
        UInt32 theLength = 0;
        Bool16 isRTCP = false;
        char* thePacket = NULL;

        if (fTransportType == kTCPTransportType)
        {
            thePacket = NULL;
            theErr = fClient->GetMediaPacket(&theTrackID, &isRTCP, &thePacket, &theLength);
            if (thePacket == NULL)
                break;
        }
        else
        {
            static const UInt32 kMaxPacketSize = 8888;
            
            UInt32 theRemoteAddr = 0;
            UInt16 theRemotePort = 0;
            char thePacketBuf[kMaxPacketSize];
            
            // Get a packet from one of the UDP sockets.
            theErr = fUDPSocketArray[theUDPSockIndex]->RecvFrom(&theRemoteAddr, &theRemotePort,
                                                                &thePacketBuf[0], kMaxPacketSize,
                                                                &theLength);
            if ((theErr != OS_NoErr) || (theLength == 0))
            {
                if ((fTransportType == kReliableUDPTransportType) &&
                    (!(theUDPSockIndex & 1)))
                {
                    for (UInt32 trackCount = 0; trackCount < fSDPParser.GetNumStreams(); trackCount++)
                    {
                        if (fSDPParser.GetStreamInfo(trackCount)->fTrackID == fSDPParser.GetStreamInfo(theUDPSockIndex >> 1)->fTrackID)
                        {
                            if (fStats[trackCount].fHighestSeqNumValid)
                                // If we are supposed to be sending acks, and we just finished
                                // receiving all packets for this track that are available at this time,
                                // send an ACK packet
                                this->AckPackets(trackCount, 0, false);
                        }
                    }
                }
                
                theUDPSockIndex++;
                if (theUDPSockIndex == fSDPParser.GetNumStreams() * 2)
                    break;
                continue;
            }
            
            theTrackID = fSDPParser.GetStreamInfo(theUDPSockIndex >> 1)->fTrackID;
            isRTCP = (theUDPSockIndex & 1);
            thePacket = &thePacketBuf[0];
        }
        
        //
        // We have a valid packet. Invoke the packet handler function
        this->ProcessMediaPacket(thePacket, theLength, theTrackID, isRTCP);
    }
    return theErr;
}

void RTSPRelaySession::ProcessMediaPacket(  char* inPacket, UInt32 inLength,
                                            UInt32 inTrackID, Bool16 isRTCP)
{
    Assert(inLength > 4);
    
    // Currently we do nothing with RTCPs.
    if (isRTCP) return;
    
    UInt16* theSeqNumP = (UInt16*)inPacket;
    UInt16 theSeqNum = ntohs(theSeqNumP[1]);
    
    //UInt32* theSsrcP = (UInt32*)inPacket;
    //UInt32 theSSRC = ntohl(theSsrcP[2]);
    
    for (UInt32 x = 0; x < fSDPParser.GetNumStreams(); x++)
    {
        if (fSDPParser.GetStreamInfo(x)->fTrackID == inTrackID)
        {
            // Check if this packet is even for our stream
            //if (!fStats[x].fIsSSRCValid)
            //  fStats[x].fSSRC = theSSRC; // If we don't know SSRC yet, just use first one we get
            //if (theSSRC != fStats[x].fSSRC)
            //  return;

			if(fReflectorSession != NULL)
			{
				QTSS_RoleParams packetParams;
				packetParams.rtspRelayingDataParams.inRTSPSession = (QTSS_Object)this;
				packetParams.rtspRelayingDataParams.inPacketData = inPacket;
				packetParams.rtspRelayingDataParams.inPacketLen = inLength;
				packetParams.rtspRelayingDataParams.inChannel = x*2;

				UInt32 fCurrentModule = 0;
				UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPRelayingDataRole);
				for (; fCurrentModule < numModules; fCurrentModule++)
				{
					QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPRelayingDataRole, fCurrentModule);
				    (void)theModule->CallDispatch(QTSS_RTSPRelayingData_Role, &packetParams);
				}
			}

            fNumPacketsReceived ++;
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

                if ((fTransportType == kReliableUDPTransportType) &&
                    (theSeqNumDifference != 1))
                    this->AckPackets(x, theSeqNum, true);

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
            while ( (SInt32)  ( (SInt32) theSeqNum - (SInt32) fStats[x].fWrapSeqNum) > TrackStats::kSeqNumMapSize)
			{
                debugblah++;              
                if (debugblah > 100)
                    break;
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
                fStats[x].fWrapSeqNum += halfSeqNumMap;
            }
           
            //
            // Track duplicate packets
            if (fStats[x].fSequenceNumberMap[theSeqNum % 100])
                fStats[x].fNumDuplicates++;
                
            fStats[x].fSequenceNumberMap[theSeqNum % 100] = 1;
            theSeqNum = 0;
        }
    }
}

void RTSPRelaySession::AckPackets(UInt32 inTrackIndex, UInt16 inCurSeqNum, Bool16 inCurSeqNumValid)
{
    char theRRBuffer[256];
    UInt32  *theWriterStart = (UInt32*)theRRBuffer;
    UInt32  *theWriter = (UInt32*)theRRBuffer;

    // APP PACKET - QoS info
    *(theWriter++) = htonl(0x80CC0000); 
    //*(ia++) = htonl(trk[i].TrackSSRC);
    *(theWriter++) = htonl(0);
    *(theWriter++) = htonl(FOUR_CHARS_TO_INT('q', 't', 'a', 'k'));
    *(theWriter++) = htonl(0);
    
    // Watch out for 16 bit seq num rollover. Dont use SInt32 for theSeqNumDifference, this routine will crash from wrong packet counts after a roll-over. 
    SInt16 theSeqNumDifference = (SInt16) (inCurSeqNum - fStats[inTrackIndex].fHighestSeqNum);
    
    if (!inCurSeqNumValid)
    {
        theSeqNumDifference = 1;
        inCurSeqNum = fStats[inTrackIndex].fHighestSeqNum;
    }

    //
    // There may be nothing to do here
    if (inCurSeqNum == fStats[inTrackIndex].fLastAckedSeqNum)
        return;
        
    if (theSeqNumDifference > 0)
    {
        *(theWriter++) = htonl(fStats[inTrackIndex].fLastAckedSeqNum + 1);

        UInt16 maskPosition = fStats[inTrackIndex].fLastAckedSeqNum + 2;
        SInt16 numPacketsInMask = (inCurSeqNum + 1) - (fStats[inTrackIndex].fLastAckedSeqNum + 2);
        
        for (SInt32 y = 0; y < numPacketsInMask; y+=32)
        {
            UInt32 mask = 0;
            for (UInt32 x = 0; x < 32; x++)
            {
                SInt16 offsetFromHighest = fStats[inTrackIndex].fHighestSeqNum - maskPosition;
                mask <<= 1;
    
                if (offsetFromHighest >= 0)
                {
                    mask |= 1;
                }
                else if (maskPosition == inCurSeqNum)
                {
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
}


OS_Error RTSPRelaySession::SendReceiverReport(UInt32 inTrackID)
{
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
    *(theWriter++) = htonl(FOUR_CHARS_TO_INT('Q', 'T', 'S', 'S'));
    //*(ia++) = toBigEndian_ulong(trk[i].rcvrSSRC);
    *(theWriter++) = htonl(0);
    *(theWriter++) = htonl(8);          // number of 4-byte quants below
#define RR 0x72720004
#define PR 0x70720004
#define PD 0x70640002
#define OB 0x6F620004
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
    //*(theWriter++) = htonl(PL);
    //*(ia++) = htonl(trk[i].rtp_num_lost);
    //*(theWriter++) = htonl(0);
    *(theWriter++) = htonl(OB);
    *(theWriter++) = htonl(fOverbufferWindowSizeInK * 1024);
    *(theWriter++) = htonl(PD);
    *(theWriter++) = htonl(0);      // should be a short, but we need to pad to a long for the entire RTCP app packet

#if CLIENT_SESSION_DEBUG
    qtss_printf("Sending receiver reports.\n");
#endif
    // Send the packet
    if (fUDPSocketArray != NULL)
    {
        Assert(fStats[inTrackID].fDestRTCPPort != 0);
        fUDPSocketArray[(inTrackID*2)+1]->SendTo(fSocket->GetHostAddr(), fStats[inTrackID].fDestRTCPPort, theRRBuffer,
                                                            (theWriter - theWriterStart) * sizeof(UInt32));
    }
    else
    {
        OS_Error theErr = fClient->PutMediaPacket(fSDPParser.GetStreamInfo(inTrackID)->fTrackID,
                                        true,
                                        theRRBuffer,
                                        (theWriter - theWriterStart) * sizeof(UInt32));
        if (theErr != OS_NoErr)
            return theErr;

    }
    return OS_NoErr;
}
