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
    File:       RTPSession.h

    Contains:   RTPSession represents an, well, an RTP session. The server creates
                one of these when a new client connects, and it lives for the duration
                of an RTP presentation. It contains all the resources associated with
                that presentation, such as RTPStream objects. It also provides an
                API for manipulating a session (playing it, pausing it, stopping it, etc) 
                
                It is also the active element, ie. it is the object that gets scheduled
                to send out & receive RTP & RTCP packets
                
    Change History (most recent first):

    
    

*/


#ifndef _RTPSESSION_H_
#define _RTPSESSION_H_

#include "RTPSessionInterface.h"
#include "RTSPRequestInterface.h"
#include "RTPStream.h"
#include "QTSSModule.h"


class RTPSession : public RTPSessionInterface
{
    public:
    
        RTPSession();
        virtual ~RTPSession();
        
        //
        //ACCESS FUNCTIONS
        
        QTSSModule* GetPacketSendingModule()                        { return fModule; }
        Bool16      HasAnRTPStream()                                { return fHasAnRTPStream; }
        
        RTPStream*  FindRTPStreamForChannelNum(UInt8 inChannelNum);
        
        //
        // MODIFIERS
        
        //This puts this session into the session map (called by the server! not the module!)
        //If this function fails (it can return QTSS_DupName), it means that there is already
        //a session with this session ID in the map.
        QTSS_Error  Activate(const StrPtrLen& inSessionID);
                
        // The way this object is implemented currently, only one module can send the
        // packets for a session.
        void        SetPacketSendingModule(QTSSModule* inModule)    { fModule = inModule; }

        //Once the session is bound, a module can add streams to it.
        //It must pass in a trackID that uniquely identifies this stream.
        //This call can only be made during an RTSP Setup request, and the
        //RTSPRequestInterface must be provided.
        //You may also opt to attach a codec name and type to this stream.
        QTSS_Error  AddStream(  RTSPRequestInterface* request, RTPStream** outStream,
                                    QTSS_AddStreamFlags inFlags);
        
		//Reset the thinning params for all streams using the late tolerance value
		void SetStreamThinningParams(Float32 inLateTolerance);
		
		//Find the appropriate stream and set the rate adaptation stream data
		void Set3GPPRateAdaptionData(RateAdapationStreamDataFields *rateAdaptStreamData);
		
		// Store the Session Interface's Link Char values
		void Set3GPPLinkCharData(LinkCharDataFields *linkCharData);

        //Begins playing all streams. Currently must be associated with an RTSP Play
        //request, and the request interface must be provided.
        QTSS_Error  Play(RTSPRequestInterface* request, QTSS_PlayFlags inFlags);
        
        //Pauses all streams.
        void            Pause();
        
        // Tears down the session. This will cause QTSS_SessionClosing_Role to run
        void            Teardown();

        //Utility functions. Modules aren't required to use these, but can be useful
        void            SendDescribeResponse(RTSPRequestInterface* request);
        void            SendAnnounceResponse(RTSPRequestInterface* request);
        void            SendPlayResponse(RTSPRequestInterface* request, UInt32 inFlags);
        void            SendPauseResponse(RTSPRequestInterface* request)
                                                                { request->SendHeader(); }
        void            SendTeardownResponse(RTSPRequestInterface* request)
                                    { request->SetKeepAlive(false); request->SendHeader(); }
                                    
        SInt32          GetQualityLevel();
        void            SetQualityLevel(SInt32 level);
        
        void            SetMovieBitRateData();

    private:
    
        //where timeouts, deletion conditions get processed
        virtual SInt64  Run();
        
        // Utility function used by Play
        UInt32 PowerOf2Floor(UInt32 inNumToFloor);
    
		//overbuffer logging function
		void LogOverbufferStats();
	
		enum
		{
            kRTPStreamArraySize     = 20,
            kCantGetMutexIdleTime   = 10
        };

        QTSSModule*         fModule;
        Bool16              fHasAnRTPStream;
        SInt32              fSessionQualityLevel;
        
        char        fRTPStreamArray[kRTPStreamArraySize];
        
        // Module invocation and module state.
        // This info keeps track of our current state so that
        // the state machine works properly.
        enum
        {
            kStart                  = 0,
            kSendingPackets         = 1
        };
        
        UInt32 fCurrentModuleIndex;
        UInt32 fCurrentState;
        
        QTSS_ModuleState    fModuleState;
        QTSS_CliSesClosingReason fClosingReason;
        
        UInt32              fCurrentModule;
        // This is here to give the ability to the ClientSessionClosing role to
        // do asynchronous I/O
        Bool16              fModuleDoingAsyncStuff;
        
#if DEBUG
        Bool16 fActivateCalled;
#endif
        SInt64              fLastBandwidthTrackerStatsUpdate;

};

#endif //_RTPSESSION_H_
