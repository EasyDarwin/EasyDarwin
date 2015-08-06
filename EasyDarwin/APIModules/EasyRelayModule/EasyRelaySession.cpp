/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyRelaySession.cpp
    Contains:   RTSP Relay Client
*/
#include "EasyRelaySession.h"
#include <stdlib.h>

EasyRelaySession::EasyRelaySession(char* inURL, ClientType inClientType, const char* streamName)
:   fReflectorSession(NULL),
	fStreamName(NULL),
	fURL(NULL)
{
    this->SetTaskName("EasyRelaySession");

	//½¨Á¢NVSourceClient

    StrPtrLen theURL(inURL);

	fURL = NEW char[::strlen(inURL) + 2];
	::strcpy(fURL, inURL);

	if (streamName != NULL)
    {
		fStreamName.Ptr = NEW char[strlen(streamName) + 1];
		::memset(fStreamName.Ptr,0,strlen(streamName) + 1);
		::memcpy(fStreamName.Ptr, streamName, strlen(streamName));
		fStreamName.Len = strlen(streamName);
		fRef.Set(fStreamName, this);
    }

	qtss_printf("\nNew Connection %s:%s\n",fStreamName.Ptr,fURL);
}

EasyRelaySession::~EasyRelaySession()
{
	qtss_printf("\nDisconnect %s:%s\n",fStreamName.Ptr,fURL);

	delete [] fStreamName.Ptr;

	qtss_printf("Disconnect complete\n");
}

SInt64 EasyRelaySession::Run()
{
    EventFlags theEvents = this->GetEvents();

	if (theEvents & Task::kKillEvent)
    {
        return -1;
    }

    return 0;
}


QTSS_Error EasyRelaySession::SendDescribe()
{
	QTSS_Error theErr = QTSS_NoErr;
	return theErr;
}