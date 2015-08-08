/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyRelaySession.h
*/

#ifndef __EASY_RELAY_SESSION__
#define __EASY_RELAY_SESSION__


#define __SOCKET_H__

#include "Task.h"
#include "TimeoutTask.h"

#include <stdio.h>
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
#include "EasyNVSourceAPI.h"
#include "EasyPusherAPI.h"

class EasyRelaySession : public Task
{
    public:
        enum
        {
            kRTSPUDPClientType          = 0,
            kRTSPTCPClientType          = 1,
        };
        typedef UInt32 ClientType;
    
        EasyRelaySession(char* inURL, ClientType inClientType, const char* streamName);

        virtual ~EasyRelaySession();

        virtual SInt64	Run();

		OSRef*			GetRef()    		{ return &fRef; } 
		OSMutex*		GetMutex()						{ return &fMutex; }

		QTSS_Error		ProcessData(int _chid, int mediatype, char *pbuf, NVS_FRAME_INFO *frameinfo);
		QTSS_Error		RelaySessionStart();
		QTSS_Error		RelaySessionRelease();
  
    private:
		OSRef			fRef;
		StrPtrLen		fStreamName;
		char*			fURL;
		OSMutex			fMutex;

		//NVSource Handle
		Easy_NVS_Handle	fNVSHandle;
		//HLS Handle
		Easy_Pusher_Handle fPusherHandle;
};

#endif //__EASY_RELAY_SESSION__
