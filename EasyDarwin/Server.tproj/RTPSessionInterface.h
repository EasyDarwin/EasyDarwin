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
	Contains:   API interface for objects to use to get access to attributes,
				data items, whatever, specific to RTP sessions (see RTPSession.h
				for more details on what that object is). This object
				implements the RTP Session Dictionary.



*/


#ifndef _RTPSESSIONINTERFACE_H_
#define _RTPSESSIONINTERFACE_H_

#include "QTSSDictionary.h"

#include "RTCPSRPacket.h"
#include "RTSPSessionInterface.h"
#include "TimeoutTask.h"
#include "Task.h"
#include "RTPBandwidthTracker.h"
#include "RTPOverbufferWindow.h"
#include "QTSServerInterface.h"
#include "OSMutex.h"
#include "RTPSession3GPP.h"

class RTSPRequestInterface;

class RTPSessionInterface : public QTSSDictionary, public Task
{
public:

	// Initializes dictionary resources
	static void Initialize();

	//
	// CONSTRUCTOR / DESTRUCTOR

	RTPSessionInterface();
	virtual ~RTPSessionInterface()
	{
		if (GetQualityLevel() != 0)
			QTSServerInterface::GetServer()->IncrementNumThinned(-1);
		if (fRTSPSession != NULL)
			fRTSPSession->DecrementObjectHolderCount();
		delete[] fSRBuffer.Ptr;
		delete[] fAuthNonce.Ptr;
		delete[] fAuthOpaque.Ptr;
	}

	virtual void SetValueComplete(UInt32 inAttrIndex, QTSSDictionaryMap* inMap,
		UInt32 inValueIndex, void* inNewValue, UInt32 inNewValueLen);

	//Timeouts. This allows clients to refresh the timeout on this session
	void    RefreshTimeout() { fTimeoutTask.RefreshTimeout(); }
	void    RefreshRTSPTimeout() { if (fRTSPSession != NULL) fRTSPSession->RefreshTimeout(); }
	void    RefreshTimeouts() { RefreshTimeout(); RefreshRTSPTimeout(); }

	//
	// ACCESSORS

	bool  IsFirstPlay() { return fIsFirstPlay; }
	SInt64  GetFirstPlayTime() { return fFirstPlayTime; }
	//Time (msec) most recent play was issued
	SInt64  GetPlayTime() { return fPlayTime; }
	SInt64  GetNTPPlayTime() { return fNTPPlayTime; }
	SInt64  GetSessionCreateTime() { return fSessionCreateTime; }
	//Time (msec) most recent play, adjusted for start time of the movie
	//ex: PlayTime() == 20,000. Client said start 10 sec into the movie,
	//so AdjustedPlayTime() == 10,000
	QTSS_PlayFlags GetPlayFlags() { return fPlayFlags; }
	OSMutex*        GetSessionMutex() { return &fSessionMutex; }
	UInt32          GetPacketsSent() { return fPacketsSent; }
	UInt32          GetBytesSent() { return fBytesSent; }
	OSRef*      GetRef() { return &fRTPMapElem; }
	RTSPSessionInterface* GetRTSPSession() { return fRTSPSession; }
	UInt32      GetMovieAvgBitrate() { return fMovieAverageBitRate; }
	QTSS_CliSesTeardownReason GetTeardownReason() { return fTeardownReason; }
	QTSS_RTPSessionState    GetSessionState() { return fState; }
	void    SetUniqueID(UInt32 theID) { fUniqueID = theID; }
	UInt32  GetUniqueID() { return fUniqueID; }
	RTPBandwidthTracker* GetBandwidthTracker() { return &fTracker; }
	RTPOverbufferWindow* GetOverbufferWindow() { return &fOverbufferWindow; }
	UInt32  GetFramesSkipped() { return fFramesSkipped; }

	//
	// MEMORY FOR RTCP PACKETS

	//
	// Class for easily building a standard RTCP SR
	RTCPSRPacket*   GetSRPacket() { return &fRTCPSRPacket; }

	//
	// Memory if you want to build your own
	char*           GetSRBuffer(UInt32 inSRLen);

	//
	// STATISTICS UPDATING

	//The session tracks the total number of bytes sent during the session.
	//Streams can update that value by calling this function
	void            UpdateBytesSent(UInt32 bytesSent) { fBytesSent += bytesSent; }

	//The session tracks the total number of packets sent during the session.
	//Streams can update that value by calling this function                
	void            UpdatePacketsSent(UInt32 packetsSent) { fPacketsSent += packetsSent; }

	void            UpdateCurrentBitRate(const SInt64& curTime)
	{
		if (curTime > (fLastBitRateUpdateTime + 3000)) this->UpdateBitRateInternal(curTime);
	}

	void            SetAllTracksInterleaved(bool newValue) { fAllTracksInterleaved = newValue; }
	//
	// RTSP RESPONSES

	// This function appends a session header to the SETUP response, and
	// checks to see if it is a 304 Not Modified. If it is, it sends the entire
	// response and returns an error
	QTSS_Error DoSessionSetupResponse(RTSPRequestInterface* inRequest);

	//
	// RTSP SESSION

	// This object has a current RTSP session. This may change over the
	// life of the RTSPSession, so update it. It keeps an RTSP session in
	// case interleaved data or commands need to be sent back to the client. 
	void            UpdateRTSPSession(RTSPSessionInterface* inNewRTSPSession);

	// let's RTSP Session pass along it's query string
	void            SetQueryString(StrPtrLen* queryString);

	// SETTERS and ACCESSORS for auth information
	// Authentication information that needs to be kept around
	// for the duration of the session      
	QTSS_AuthScheme GetAuthScheme() { return fAuthScheme; }
	StrPtrLen*      GetAuthNonce() { return &fAuthNonce; }
	UInt32          GetAuthQop() { return fAuthQop; }
	UInt32          GetAuthNonceCount() { return fAuthNonceCount; }
	StrPtrLen*      GetAuthOpaque() { return &fAuthOpaque; }
	void            SetAuthScheme(QTSS_AuthScheme scheme) { fAuthScheme = scheme; }
	// Use this if the auth scheme or the qop has to be changed from the defaults 
	// of scheme = Digest, and qop = auth
	void            SetChallengeParams(QTSS_AuthScheme scheme, UInt32 qop, bool newNonce, bool createOpaque);
	// Use this otherwise...if newNonce == true, it will create a new nonce
	// and reset nonce count. If newNonce == false but nonce was never created earlier
	// a nonce will be created. If newNonce == false, and there is an existing nonce,
	// the nounce count will be incremented.
	void            UpdateDigestAuthChallengeParams(bool newNonce, bool createOpaque, UInt32 qop);

	Float32* GetPacketLossPercent() { UInt32 outLen; return  (Float32*) this->PacketLossPercent(this, &outLen); }

	SInt32          GetQualityLevel() { return fSessionQualityLevel; }
	SInt32*         GetQualityLevelPtr() { return &fSessionQualityLevel; }
	void            SetQualityLevel(SInt32 level) {
		if (fSessionQualityLevel == 0 && level != 0)
			QTSServerInterface::GetServer()->IncrementNumThinned(1);
		else if (fSessionQualityLevel != 0 && level == 0)
			QTSServerInterface::GetServer()->IncrementNumThinned(-1);
		fSessionQualityLevel = level;
	}
	SInt64          fLastQualityCheckTime;
	SInt64			fLastQualityCheckMediaTime;
	bool			fStartedThinning;

	// Used by RTPStream to increment the RTCP packet and byte counts.
	void            IncrTotalRTCPPacketsRecv() { fTotalRTCPPacketsRecv++; }
	UInt32          GetTotalRTCPPacketsRecv() { return fTotalRTCPPacketsRecv; }
	void            IncrTotalRTCPBytesRecv(UInt16 cnt) { fTotalRTCPBytesRecv += cnt; }
	UInt32          GetTotalRTCPBytesRecv() { return fTotalRTCPBytesRecv; }

	UInt32          GetLastRTSPBandwithBits() { return fLastRTSPBandwidthHeaderBits; }
	UInt32          GetCurrentMovieBitRate() { return fMovieCurrentBitRate; }

	UInt32          GetMaxBandwidthBits() { UInt32 maxRTSP = GetLastRTSPBandwithBits(); UInt32 maxLink = fRTPSession3GPP.GetLinkCharMaxKBits() * 1000;  return (maxRTSP > maxLink) ? maxRTSP : maxLink; }

	void            SetIs3GPPSession(bool is3GPP) { fIs3GPPSession = is3GPP; }

protected:

	RTPSession3GPP* Get3GPPSessPtr() { return fRTPSession3GPPPtr; }

	// These variables are setup by the derived RTPSession object when
	// Play and Pause get called

	//Some stream related information that is shared amongst all streams
	bool      fIsFirstPlay;
	bool      fAllTracksInterleaved;
	SInt64      fFirstPlayTime;//in milliseconds
	SInt64      fPlayTime;
	SInt64      fAdjustedPlayTime;
	SInt64      fNTPPlayTime;
	SInt64      fNextSendPacketsTime;

	SInt32      fSessionQualityLevel;

	//keeps track of whether we are playing or not
	QTSS_RTPSessionState fState;

	// If we are playing, this are the play flags that were set on play
	QTSS_PlayFlags  fPlayFlags;

	//Session mutex. This mutex should be grabbed before invoking the module
	//responsible for managing this session. This allows the module to be
	//non-preemptive-safe with respect to a session
	OSMutex     fSessionMutex;

	//Stores the session ID
	OSRef               fRTPMapElem;
	char                fRTSPSessionIDBuf[QTSS_MAX_SESSION_ID_LENGTH + 4];

	UInt32      fLastBitRateBytes;
	SInt64      fLastBitRateUpdateTime;
	UInt32      fMovieCurrentBitRate;

	// In order to facilitate sending data over the RTSP channel from
	// an RTP session, we store a pointer to the RTSP session used in
	// the last RTSP request.
	RTSPSessionInterface* fRTSPSession;



private:

	//
	// Utility function for calculating current bit rate
	void UpdateBitRateInternal(const SInt64& curTime);

	static void* PacketLossPercent(QTSSDictionary* inSession, UInt32* outLen);
	static void* TimeConnected(QTSSDictionary* inSession, UInt32* outLen);
	static void* CurrentBitRate(QTSSDictionary* inSession, UInt32* outLen);

	// Create nonce
	void CreateDigestAuthenticationNonce();

	// One of the RTP session attributes is an iterated list of all streams.
	// As an optimization, we preallocate a "large" buffer of stream pointers,
	// even though we don't know how many streams we need at first.
	enum
	{
		kStreamBufSize = 4,
		kUserAgentBufSize = 256,
		kPresentationURLSize = 256,
		kFullRequestURLBufferSize = 256,
		kRequestHostNameBufferSize = 80,

		kIPAddrStrBufSize = 20,
		kLocalDNSBufSize = 80,

		kAuthNonceBufSize = 32,
		kAuthOpaqueBufSize = 32,

	};

	void*       fStreamBuffer[kStreamBufSize];


	// theses are dictionary items picked up by the RTSPSession
	// but we need to store copies of them for logging purposes.
	char        fUserAgentBuffer[kUserAgentBufSize];
	char        fPresentationURL[kPresentationURLSize];         // eg /foo/bar.mov
	char        fFullRequestURL[kFullRequestURLBufferSize];     // eg rtsp://yourdomain.com/foo/bar.mov
	char        fRequestHostName[kRequestHostNameBufferSize];   // eg yourdomain.com

	char        fRTSPSessRemoteAddrStr[kIPAddrStrBufSize];
	char        fRTSPSessLocalDNS[kLocalDNSBufSize];
	char        fRTSPSessLocalAddrStr[kIPAddrStrBufSize];

	char        fUserNameBuf[RTSPSessionInterface::kMaxUserNameLen];
	char        fUserPasswordBuf[RTSPSessionInterface::kMaxUserPasswordLen];
	char        fUserRealmBuf[RTSPSessionInterface::kMaxUserRealmLen];
	UInt32      fLastRTSPReqRealStatusCode;

	//for timing out this session
	TimeoutTask fTimeoutTask;
	UInt32      fTimeout;

	// Time when this session got created
	SInt64      fSessionCreateTime;

	//Packet priority levels. Each stream has a current level, and
	//the module that owns this session sets what the number of levels is.
	UInt32      fNumQualityLevels;

	//Statistics
	UInt32 fBytesSent;
	UInt32 fPacketsSent;
	Float32 fPacketLossPercent;
	SInt64 fTimeConnected;
	UInt32 fTotalRTCPPacketsRecv;
	UInt32 fTotalRTCPBytesRecv;
	// Movie size & movie duration. It may not be so good to associate these
	// statistics with the movie, for a session MAY have multiple movies...
	// however, the 1 movie assumption is in too many subsystems at this point
	Float64     fMovieDuration;
	UInt64      fMovieSizeInBytes;
	UInt32      fMovieAverageBitRate;

	QTSS_CliSesTeardownReason fTeardownReason;
	// So the streams can send sender reports
	UInt32      fUniqueID;

	RTCPSRPacket        fRTCPSRPacket;
	StrPtrLen           fSRBuffer;

	RTPBandwidthTracker fTracker;
	RTPOverbufferWindow fOverbufferWindow;

	// Built in dictionary attributes
	static QTSSAttrInfoDict::AttrInfo   sAttributes[];
	static unsigned int sRTPSessionIDCounter;

	// Authentication information that needs to be kept around
	// for the duration of the session      
	QTSS_AuthScheme             fAuthScheme;
	StrPtrLen                   fAuthNonce;
	UInt32                      fAuthQop;
	UInt32                      fAuthNonceCount;
	StrPtrLen                   fAuthOpaque;
	UInt32                      fQualityUpdate;

	UInt32                      fFramesSkipped;

	RTPSession3GPP			fRTPSession3GPP;
	RTPSession3GPP*			fRTPSession3GPPPtr;
	UInt32                  fLastRTSPBandwidthHeaderBits;
	bool                  fIs3GPPSession;

};

#endif //_RTPSESSIONINTERFACE_H_
