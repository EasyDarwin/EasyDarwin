/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       RTSPRelaySession.cpp
    Contains:   RTSP Relay Client
*/
#include "RTSPRelaySession.h"
#include <stdlib.h>

RTSPRelaySession::RTSPRelaySession(char* inURL, ClientType inClientType, const char* streamName)
:   fReflectorSession(NULL),
	fStreamName(NULL),
	fURL(NULL)
{
    this->SetTaskName("RTSPRelaySession");

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

RTSPRelaySession::~RTSPRelaySession()
{
	qtss_printf("\nDisconnect %s:%s\n",fStreamName.Ptr,fURL);

	delete [] fStreamName.Ptr;

	qtss_printf("Disconnect complete\n");
}

SInt64 RTSPRelaySession::Run()
{
    EventFlags theEvents = this->GetEvents();

	if (theEvents & Task::kKillEvent)
    {
        return -1;
    }

    return 0;
}


QTSS_Error RTSPRelaySession::SendDescribe()
{
	QTSS_Error theErr = QTSS_NoErr;
	return theErr;
}