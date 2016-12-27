/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
	File:       EasyHLSSession.h
*/

#ifndef __EASY_HLS_SESSION__
#define __EASY_HLS_SESSION__

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
#include "EasyAACEncoderAPI.h"

class EasyHLSSession : public Task
{
public:
	EasyHLSSession(StrPtrLen* inName, StrPtrLen* inSourceURL, UInt32 inChannel = 0);

	virtual ~EasyHLSSession();

	SInt64	Run() override;

	OSRef*			GetRef() { return &fRef; }
	OSMutex*		GetMutex() { return &fMutex; }

	StrPtrLen*      GetSourceID() { return &fSourceID; }
	StrPtrLen*      GetStreamName() { return &fSessionName; }
	StrPtrLen*		GetSourceURL() { return &fSourceURL; }
	const char*		GetHLSURL() const { return fHLSURL; }
	UInt32			GetChannelNum() const { return fChannelNum; }
	void			RefreshTimeout() { fTimeoutTask.RefreshTimeout(); }

	QTSS_Error		ProcessData(int _chid, int mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo);
	QTSS_Error		SessionStart();
	QTSS_Error		SessionRelease();

private:
	QTSS_Error initAACEncoder(int codec);
	void pushVideo(char* pbuf, RTSP_FRAME_INFO* frameinfo) const;
	void pushAudio(char* pbuf, RTSP_FRAME_INFO* frameinfo);

	// For storage in the session map       
	OSRef       fRef;
	StrPtrLen   fSourceID;
	StrPtrLen	fSessionName;
	StrPtrLen	fSourceURL;
	char		fHLSURL[QTSS_MAX_URL_LENGTH];
	UInt32		fChannelNum;

	OSMutex		fMutex;
	TimeoutTask	fTimeoutTask;

	Easy_RTSP_Handle	fRTSPClientHandle;
	Easy_RTMP_Handle	fRTMPHandle;
	EasyAACEncoder_Handle	fAAChandle;

	unsigned char pbAACBuffer[EASY_ACCENCODER_BUFFER_SIZE_LEN];
};

#endif //__EASY_HLS_SESSION__
