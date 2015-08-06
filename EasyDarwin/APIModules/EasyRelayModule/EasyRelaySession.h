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
//#include "ReflectorSession.h"
//#include "QTSServerInterface.h"

#include <stdio.h>
#include "OSHeaders.h"
#include "QTSSModuleUtils.h"
#include "MyAssert.h"

#include "OSMutex.h"
#include "MyAssert.h"
#include "OSMemory.h"
#include <time.h>
#include "StringParser.h"
#include "StringFormatter.h"
#include "StringTranslator.h"
#include "StrPtrLen.h"
#include "UserAgentParser.h"
#include "SDPSourceInfo.h"
#include "OSRef.h"

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
        
        
        virtual SInt64 Run();

		QTSS_Error SendDescribe();
        
  
    private:
        
		void*			fReflectorSession;	//Reflect Session

        UInt32          fOptionsIntervalInSec;
        Bool16          fOptions;

		OSRef			fRef;
		StrPtrLen		fStreamName;
		char*			fURL;

		OSMutex			fMutex;

	public:
		OSRef*					GetRef()    		{ return &fRef; } 

		void	SetReflectorSession(void* refSes)	{ fReflectorSession = refSes; }

		void*	GetReflectorSession()				{ return fReflectorSession; }

		OSMutex*	GetMutex()						{ return &fMutex; }
};

#endif //__EASY_RELAY_SESSION__
