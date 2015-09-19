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
#include "EasyRTSPClientAPI.h"
#include "EasyHLSAPI.h"

#include "TimeoutTask.h"

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
		QTSS_Error		ProcessData(int _chid, int mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo);
		QTSS_Error		HLSSessionStart(char* rtspUrl, UInt32 inTimeout);
		QTSS_Error		HLSSessionRelease();
		char*			GetHLSURL();
		char*			GetSourceURL();

		void RefreshTimeout()	{ fTimeoutTask.RefreshTimeout(); }

		//统计
		SInt64          GetTotalPlayTime()		const { return fTotalPlayTime; }
		SInt64			GetNumPacketsReceived() const { return fNumPacketsReceived; }
		SInt64			GetNumBytesReceived()	const { return fNumBytesReceived; }
		UInt32			GetLastStatBitrate()	const { return fLastStatBitrate; }
   
    private:

        //HLSSession列表由EasyHLSModule的sHLSSessionMap维护  
        OSRef       fRef;
        StrPtrLen   fHLSSessionID;
		char		fHLSURL[QTSS_MAX_URL_LENGTH];
		char		fSourceURL[QTSS_MAX_URL_LENGTH];
        OSQueueElem fQueueElem; 

		//RTSPClient Handle
		Easy_RTSP_Handle	fRTSPClientHandle;
		//HLS Handle
		Easy_HLS_Handle fHLSHandle;
		
		//TS timestamp ms，自定义时间戳
		int tsTimeStampMSsec;

		static UInt32	sM3U8Version;
		static Bool16	sAllowCache;
		static UInt32	sTargetDuration;
		static UInt32	sPlaylistCapacity;
		static char*	sHTTPRootDir;

		//统计
		SInt64          fPlayTime;				//起始的时间
		SInt64			fLastStatPlayTime;		//上一次统计的时间

        SInt64          fTotalPlayTime;			//总共播放时间

        SInt64			fNumPacketsReceived;	//收到的数据包的数量
		SInt64			fLastNumPacketsReceived;//上一次统计收到的数据包数量

        SInt64			fNumBytesReceived;		//收到的数据总量
        SInt64			fLastNumBytesReceived;	//上一次统计收到的数据总量

		UInt32			fLastStatBitrate;		//最后一次统计得到的比特率

	protected:
		TimeoutTask		fTimeoutTask;
};

#endif

