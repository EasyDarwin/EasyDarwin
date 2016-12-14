/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyRTMPSession.h
*/

#ifndef __EASY_RTMP_SESSION__
#define __EASY_RTMP_SESSION__

#include "Task.h"
#include "TimeoutTask.h"
#include "QTSSModuleUtils.h"
#include "MyAssert.h"
#include "OSMutex.h"
#include "OSMemory.h"
#include "StrPtrLen.h"
#include "OSRef.h"
#include "EasyRTMPAPI.h"
#include <EasyRTSPClientAPI.h>

#include "QTSServerInterface.h"

class EasyRTMPSession : public Task
{
    public:
        EasyRTMPSession(StrPtrLen* inName, StrPtrLen* inSourceURL, UInt32 inChannel = 0);

        virtual ~EasyRTMPSession();

		SInt64	Run() override;

		OSRef*			GetRef()	{ return &fRef; } 
		OSMutex*		GetMutex()	{ return &fMutex; }

		StrPtrLen*      GetSourceID() { return &fSourceID; }
		StrPtrLen*      GetStreamName() { return &fSessionName; }
		StrPtrLen*		GetSourceURL() { return &fSourceURL; }
		const char*		GetRTMPURL() const { return fRTMPURL; }
		UInt32			GetChannelNum() const { return fChannelNum; }
		void			RefreshTimeout() { fTimeoutTask.RefreshTimeout(); }

		QTSS_Error		ProcessData(int _chid, int mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo) const;
		QTSS_Error		SessionStart();
		QTSS_Error		SessionRelease();
  
    private:
		// For storage in the session map       
		OSRef       fRef;
		StrPtrLen   fSourceID;
		StrPtrLen	fSessionName;
		StrPtrLen	fSourceURL;
		char		fRTMPURL[QTSS_MAX_URL_LENGTH];
		UInt32		fChannelNum;

		OSMutex		fMutex;
		TimeoutTask	fTimeoutTask;

		Easy_RTSP_Handle	fRTSPClientHandle;
		Easy_RTMP_Handle	fRTMPHandle;
};

#endif //__EASY_RTMP_SESSION__
