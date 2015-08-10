/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyHLSSession.h
    Contains:   HLS
*/
#include "QTSS.h"
#include "OSRef.h"
#include "StrPtrLen.h"
#include "ResizeableStringFormatter.h"
#include "MyAssert.h"

#include "ReflectorStream.h"
#include "SourceInfo.h"
#include "OSArrayObjectDeleter.h"
#include "EasyNVSourceAPI.h"
#include "EasyHLSAPI.h"

#ifndef __EASY_HLS_SESSION__
#define __EASY_HLS_SESSION__

class EasyHLSSession : public Task
{
    public:
        EasyHLSSession(StrPtrLen* inSourceID);
        virtual ~EasyHLSSession();
        
        //加载模块配置
        static void Initialize(QTSS_ModulePrefsObject inPrefs);

		virtual SInt64	Run();
        // ACCESSORS

        OSRef*          GetRef()            { return &fRef; }
        OSQueueElem*    GetQueueElem()      { return &fQueueElem; }
	
        StrPtrLen*      GetSessionID()     { return &fHLSSessionID; }
		QTSS_Error		ProcessData(int _chid, int mediatype, char *pbuf, NVS_FRAME_INFO *frameinfo);
		QTSS_Error		HLSSessionStart(char* rtspUrl);
		QTSS_Error		HLSSessionRelease();
		char*		GetHLSURL();
   
    private:

        //HLSSession列表由EasyHLSModule的sHLSSessionMap维护  
        OSRef       fRef;
        StrPtrLen   fHLSSessionID;

		char		fHLSURL[QTSS_MAX_URL_LENGTH];

        OSQueueElem fQueueElem; 

		//NVS Handle
		Easy_NVS_Handle	fNVSHandle;
		//HLS Handle
		Easy_HLS_Handle fHLSHandle;
		
		//TS timestamp ms，自定义时间戳
		int tsTimeStampMSsec;

		static UInt32	sM3U8Version;
		static Bool16	sAllowCache;
		static UInt32	sTargetDuration;
		static UInt32	sPlaylistCapacity;
		static char*	sHTTPRootDir;
};

#endif

