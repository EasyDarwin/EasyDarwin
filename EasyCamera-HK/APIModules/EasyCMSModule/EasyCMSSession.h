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

#include "HTTPRequestStream.h"
#include "HTTPResponseStream.h"
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

	// �����ж�Session Socket�Ƿ�������
	Bool16				isConnected() const { return fSocket->GetSocket()->IsConnected(); }

	// transfer error code for http status code
	static size_t		getStatusNo(QTSS_Error errNo);

	void				cleanupRequest();

	// �豸ע�ᵽEasyCMS
	QTSS_Error			doDSRegister();

	// �ϴ�����ͼƬ��EasyCMS
	QTSS_Error			doDSPostSnap();

	// ����HTTPRequest������
	QTSS_Error			processMessage();

	QTSS_Error processStartStreamReq() const;
	QTSS_Error processStopStreamReq() const;

	QTSS_Error processControlPTZReq() const;
	QTSS_Error processControlPresetReq() const;
	QTSS_Error processControlTalkbackReq() const;

	// ���ÿͻ��˲���
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

	// ΪCMSSessionר�Ž����������ݰ���ȡ�Ķ���
	HTTPRequestStream*  fInputStream;
	// ΪCMSSessionר�Ž����������ݰ����͵Ķ���
	HTTPResponseStream* fOutputStream;

	// ��ʼ��ʱΪNULL
	// ��ÿһ�����󷢳����߽�������ʱ,���п�������HTTPRequest���󲢽��д���
	// ÿһ��״̬�������ڴ������kIdle~kCleanUp�����̶���Ҫ����HTTPRequest����
	HTTPRequest*        fRequest;

	// ��ȡ���籨��ǰ����סSession��ֹ�����ȡ
	OSMutex             fReadMutex;

	// Session��
	OSMutex             fMutex;

	// �����ĵ�Content����
	char*				fContentBuffer;

	// �����ĵ�Content��ȡƫ����,�ڶ�ζ�ȡ������Content����ʱ�õ�
	UInt32				fContentBufferOffset;

	// send message count
	unsigned int		fNoneACKMsgCount;

	UInt32				fCSeq;

};

#endif

