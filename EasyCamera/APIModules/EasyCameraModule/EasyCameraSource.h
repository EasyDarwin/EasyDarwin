/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _EASY_MEDIA_SOURCE_H_
#define _EASY_MEDIA_SOURCE_H_

#include "Task.h"
#include "hi_type.h"
#include "hi_net_dev_sdk.h"
#include "hi_net_dev_errors.h"
#include "QTSS.h"
#include "EasyPusherAPI.h"

#define EASY_SNAP_BUFFER_SIZE 1024*1024

class EasyCameraSource : public Task
{
public:
	EasyCameraSource();
	~EasyCameraSource(void);

	bool CameraLogin();

	bool GetSnapData(unsigned char* pBuf, UInt32 uBufLen, int* uSnapLen);

	QTSS_Error NetDevStartStream();
	void NetDevStopStream();



	QTSS_Error StartStreaming(const char* inSerial, const char* inChannel, const char* inStreamID, const char* inProtocol, const char* inIP, UInt16 inPort);
	
	QTSS_Error StopStreaming();

	QTSS_Error PushFrame(unsigned char* frame, int len);

	typedef void (onCloseFunc)(void* clientData);
	static void handleClosure(void* clientData);
	void handleClosure();

	void stopGettingFrames();
	void doStopGettingFrames();

	OSMutex*	GetMutex()      { return &fMutex; }

public:
	bool fCameraLogin;
	bool m_bStreamFlag;
	bool m_bForceIFrame;

private:
	//摄像机操作句柄
	HI_U32 m_u32Handle;

	Easy_Pusher_Handle fPusherHandle;

	onCloseFunc* fOnCloseFunc;
	void* fOnCloseClientData;
	//客户端信息操作互斥量
	OSMutex fMutex;

	SInt64 Run();
};

#endif //_EASY_MEDIA_SOURCE_H_
