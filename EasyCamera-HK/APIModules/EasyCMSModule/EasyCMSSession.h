/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
	File:       EasyCMSSession.h
	Contains:   CMS Session
*/

#include "Task.h"
#include "TimeoutTask.h"

#include "QTSServerInterface.h"
#include "OSHeaders.h"
#include "QTSS.h"
#include "EasyProtocol.h"

#include "HTTPClientRequestStream.h"
#include "HTTPClientResponseStream.h"
#include "HTTPRequest.h"

using namespace EasyDarwin::Protocol;
using namespace std;

#ifndef __EASY_CMS_SESSION__
#define __EASY_CMS_SESSION__

class EasyCMSSession : public Task
{
public:
	EasyCMSSession();
	virtual ~EasyCMSSession();

	static void			Initialize(QTSS_ModulePrefsObject inPrefs);

	enum
	{
		kSessionOffline = 0,
		kSessionOnline = 1
	};
	typedef UInt32		SessionStatus;

	SessionStatus		GetSessionStatus() const { return fSessionStatus; }
	void				SetSessionStatus(SessionStatus status) { fSessionStatus = status; }

private:
	virtual SInt64		Run();

	void				stopPushStream() const;

	// 初步判断Session Socket是否已连接
	bool				isConnected() const { return fSocket->GetSocket()->IsConnected(); }

	// transfer error code for http status code
	static size_t		getStatusNo(QTSS_Error errNo);

	void				cleanupRequest();

	// 设备注册到EasyCMS
	QTSS_Error			doDSRegister();

	// 上传快照图片到EasyCMS
	QTSS_Error			doDSPostSnap();

	// 处理HTTPRequest请求报文
	QTSS_Error			processMessage();

	QTSS_Error processStartStreamReq() const;
	QTSS_Error processStopStreamReq() const;

	QTSS_Error processControlPTZReq() const;
	QTSS_Error processControlPresetReq() const;
	QTSS_Error processControlTalkbackReq() const;

	// 重置客户端参数
	void				resetClientSocket();

private:
	enum
	{
		kIdle = 0,
		kReadingMessage = 1,
		kProcessingMessage = 2,
		kSendingMessage = 3,
		kCleaningUp = 4
	};
	UInt32				fState;

	SessionStatus		fSessionStatus;

	TimeoutTask*		fTimeoutTask;
	ClientSocket*		fSocket;

	// 为CMSSession专门进行网络数据包读取的对象
	HTTPClientRequestStream*  fInputStream;
	// 为CMSSession专门进行网络数据包发送的对象
	HTTPClientResponseStream* fOutputStream;

	// 初始化时为NULL
	// 在每一次请求发出或者接收命令时,都有可能生成HTTPRequest对象并进行处理
	// 每一次状态机流程在处理完成kIdle~kCleanUp的流程都需要清理HTTPRequest对象
	HTTPRequest*        fRequest;

	// 读取网络报文前先锁住Session防止重入读取
	OSMutex             fReadMutex;

	// Session锁
	OSMutex             fMutex;

	// 请求报文的Content部分
	char*				fContentBuffer;

	// 请求报文的Content读取偏移量,在多次读取到完整Content部分时用到
	UInt32				fContentBufferOffset;

	// send message count
	unsigned int		fNoneACKMsgCount;

	UInt32				fCSeq;

};

#endif

