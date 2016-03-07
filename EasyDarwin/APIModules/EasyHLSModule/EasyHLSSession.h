/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
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
#include "EasyAACEncoderAPI.h"
#ifndef __EASY_HLS_SESSION__
#define __EASY_HLS_SESSION__

class EasyHLSSession : public Task
{
    public:
        EasyHLSSession(StrPtrLen* inSourceID);
        virtual ~EasyHLSSession();
        
        //����ģ������
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

		//ͳ��
		SInt64          GetTotalPlayTime()		const { return fTotalPlayTime; }
		SInt64			GetNumPacketsReceived() const { return fNumPacketsReceived; }
		SInt64			GetNumBytesReceived()	const { return fNumBytesReceived; }
		UInt32			GetLastStatBitrate()	const { return fLastStatBitrate; }
   
    private:

        //HLSSession�б���EasyHLSModule��sHLSSessionMapά��  
        OSRef       fRef;
        StrPtrLen   fHLSSessionID;
		char		fHLSURL[QTSS_MAX_URL_LENGTH];
		char		fSourceURL[QTSS_MAX_URL_LENGTH];
        OSQueueElem fQueueElem; 

		//RTSPClient Handle
		Easy_RTSP_Handle	fRTSPClientHandle;
		//HLS Handle
		Easy_HLS_Handle fHLSHandle;
		//AACEncode Handle
		EasyAACEncoder_Handle fAAChandle;
		//Last Audio PTS
		unsigned long long fLastAudioPTS;

		QTSS_Error EasyInitAACEncoder(int codec);
		UInt8 pbAACBuffer[EASY_ACCENCODER_BUFFER_SIZE_LEN];

		//TS timestamp ms���Զ���ʱ���
		int tsTimeStampMSsec;

		static UInt32	sM3U8Version;
		static Bool16	sAllowCache;
		static UInt32	sTargetDuration;
		static UInt32	sPlaylistCapacity;
		static char*	sHTTPRootDir;

		//ͳ��
		SInt64          fPlayTime;				//��ʼ��ʱ��
		SInt64			fLastStatPlayTime;		//��һ��ͳ�Ƶ�ʱ��

        SInt64          fTotalPlayTime;			//�ܹ�����ʱ��

        SInt64			fNumPacketsReceived;	//�յ������ݰ�������
		SInt64			fLastNumPacketsReceived;//��һ��ͳ���յ������ݰ�����

        SInt64			fNumBytesReceived;		//�յ�����������
        SInt64			fLastNumBytesReceived;	//��һ��ͳ���յ�����������

		UInt32			fLastStatBitrate;		//���һ��ͳ�Ƶõ��ı�����

	protected:
		TimeoutTask		fTimeoutTask;
};

#endif

