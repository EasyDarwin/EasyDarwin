/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       RTSPRelaySession.h
*/

#ifndef __RTSP_RELAY_SESSION__
#define __RTSP_RELAY_SESSION__


#define __SOCKET_H__

#include "Task.h"
#include "TimeoutTask.h"
//#include "ReflectorSession.h"
//#include "QTSServerInterface.h"

#include <stdio.h>
#include "OSHeaders.h"
#include "QTSSModuleUtils.h"
#include "MyAssert.h"

#include "OSMutex.h"
#include "MyAssert.h"
#include "OSMemory.h"
#include <time.h>
#include "StringParser.h"
#include "StringFormatter.h"
#include "StringTranslator.h"
#include "StrPtrLen.h"
#include "UserAgentParser.h"

#include "SDPSourceInfo.h"

#include "OSRef.h"


class live555Thread : public OSThread
{
    public:
        live555Thread(QTSS_Object scheduler, QTSS_Object env);
        virtual ~live555Thread();

		void live555EventLoop(char* watchVariable);

		void shutdownLiveStream(void* rtspClient);
    
    private:
    
        virtual void Entry();

		QTSS_Object fLive555TaskScheaduler;
		QTSS_Object fLive555Env;
		char fLive555EventLoopWatchVariable;
		char* fLive555EventLoopWatchVariablePtr;
		OSMutex fMutex;

};


class RTSPRelaySession : public Task
{
    public:
        enum
        {
            kRTSPUDPClientType          = 0,
            kRTSPTCPClientType          = 1,
        };
        typedef UInt32 ClientType;
    
        RTSPRelaySession(	char* inURL,					//URL
							ClientType inClientType,		//RTP方式,TCP或UDP
							UInt32 inOptionsIntervalInSec,	//OPTIONS保活间隔
							Bool16 sendOptions,				//是否OPTIONS保活
							const char* streamName = NULL);	//StreamName

        virtual ~RTSPRelaySession();
        
        
        virtual SInt64 Run();

		QTSS_Error SendDescribe();
        
  
    private:
        
        QTSS_Object     fLive555Client;		// live555 RTSPClient
        //TimeoutTask     fTimeoutTask;		// 定时器，定时检测
		SDPSourceInfo   fSDPParser;			// Parses the SDP in the DESCRIBE response
		void*			fReflectorSession;	//Reflect Session

        UInt32          fOptionsIntervalInSec;
        Bool16          fOptions;

        
        SInt64          fPlayTime;
        SInt64          fTotalPlayTime;
        SInt64          fLastRTCPTime;
        UInt32          fSockRcvBufSize;
        
        Float32         fSpeed;
        char*           fPacketRangePlayHeader;

		OSRef			fRef;
		StrPtrLen		fStreamName;
		char*			fURL;

        UInt32          fOverbufferWindowSizeInK;
        UInt32          fCurRTCPTrack;
        UInt32          fNumPacketsReceived;


		live555Thread*	fLive555LoopThread;

		OSMutex			fMutex;


public:
        // ACCESSORS
        QTSS_Object             GetClient()         { return fLive555Client; }

		SDPSourceInfo*          GetSDPInfo()        { return &fSDPParser; }

		OSRef*					GetRef()    		{ return &fRef; } 

		void	SetReflectorSession(void* refSes)	{ fReflectorSession = refSes; }

		void*	GetReflectorSession()				{ return fReflectorSession; }

		OSMutex*	GetMutex()						{ return &fMutex; }
};

#endif //__RTSP_RELAY_SESSION__
