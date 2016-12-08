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
#include "MyAssert.h"
#include "OSMemory.h"
#include "StringParser.h"
#include "StringFormatter.h"
#include "StringTranslator.h"
#include "StrPtrLen.h"
#include "OSRef.h"
#include "EasyRTSPClientAPI.h"
#include "EasyRTMPAPI.h"
#include "QTSServerInterface.h"

class EasyRTMPSession : public Task
{
    public:
        EasyRTMPSession(StrPtrLen* inName, StrPtrLen* inSourceURL, UInt32 inChannel = 0);

        virtual ~EasyRTMPSession();

        virtual SInt64	Run();

		OSRef*			GetRef()	{ return &fRef; } 
		OSMutex*		GetMutex()	{ return &fMutex; }

		StrPtrLen*      GetSourceID() { return &fSourceID; }
		StrPtrLen*      GetStreamName() { return &fSessionName; }
		StrPtrLen*		GetSourceURL() { return &fSourceURL; }
		const char*		GetRTMPURL() { return fRTMPURL; }
		UInt32			GetChannelNum() { return fChannelNum; }

		QTSS_Error		ProcessData(int _chid, int mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo);
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

		Easy_RTSP_Handle	fRTSPClientHandle;
		Easy_RTMP_Handle	fRTMPHandle;
};

#endif //__EASY_RTMP_SESSION__
