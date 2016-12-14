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

#pragma once
#include "Task.h"
#include "TimeoutTask.h"

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

	ClientSocket* fSocket;

	TimeoutTask fTimeoutTask;

	enum
	{
		kIdle = 0,
		kReadingMessage = 1,
		kProcessingMessage = 2,
		kSendingMessage = 3,
		kCleaningUp = 4
	};

	UInt32 fState;

	void CleanupRequest();

	QTSS_Error CSFreeStream();

	QTSS_Error ProcessMessage();

	HTTPClientRequestStream   fInputStream;
	HTTPClientResponseStream  fOutputStream;

	HTTPRequest*        fRequest;

	OSMutex             fReadMutex;

	OSMutex             fMutex;

	char*				fContentBuffer;

	UInt32				fContentBufferOffset;

	QTSS_Error FreeStream(const char * streamName);

private:

	SInt64 Run() override;

	char*	fStreamName;
	UInt32	fEasyMsgType;
	bool	fLiveSession;

	bool IsConnected() const { return fSocket->GetSocket()->IsConnected(); }
};

#endif

