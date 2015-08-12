/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyHLSSession.cpp
    Contains:   Implementation of object defined in EasyHLSSession.h. 
*/
#include "EasyHLSSession.h"
#include "SocketUtils.h"
#include "EventContext.h"
#include "OSMemory.h"
#include "OS.h"
#include "atomic.h"
#include "QTSSModuleUtils.h"
#include <errno.h>
#include "QTSServerInterface.h"

#ifndef __Win32__
    #include <unistd.h>
#endif

// PREFS
static UInt32                   sDefaultM3U8Version					= 3; 
static Bool16                   sDefaultAllowCache					= false; 
static UInt32                   sDefaultTargetDuration				= 4;
static UInt32                   sDefaultPlaylistCapacity			= 4;
static char*					sDefaultHTTPRootDir					= "http://hls.easydarwin.org/";

UInt32                          EasyHLSSession::sM3U8Version		= 3;
Bool16                          EasyHLSSession::sAllowCache			= false;
UInt32                          EasyHLSSession::sTargetDuration		= 4;
UInt32                          EasyHLSSession::sPlaylistCapacity	= 4;
char*							EasyHLSSession::sHTTPRootDir		= NULL;

void EasyHLSSession::Initialize(QTSS_ModulePrefsObject inPrefs)
{
	delete [] sHTTPRootDir;
    sHTTPRootDir = QTSSModuleUtils::GetStringAttribute(inPrefs, "HTTP_ROOT_DIR", sDefaultHTTPRootDir);

	QTSSModuleUtils::GetAttribute(inPrefs, "M3U8_VERSION", qtssAttrDataTypeUInt32,
							  &EasyHLSSession::sM3U8Version, &sDefaultM3U8Version, sizeof(sDefaultM3U8Version));

	QTSSModuleUtils::GetAttribute(inPrefs, "ALLOW_CACHE", qtssAttrDataTypeBool16,
							  &EasyHLSSession::sAllowCache, &sDefaultAllowCache, sizeof(sDefaultAllowCache));

	QTSSModuleUtils::GetAttribute(inPrefs, "TARGET_DURATION", qtssAttrDataTypeUInt32,
							  &EasyHLSSession::sTargetDuration, &sDefaultTargetDuration, sizeof(sDefaultTargetDuration));

	QTSSModuleUtils::GetAttribute(inPrefs, "PLAYLIST_CAPACITY", qtssAttrDataTypeUInt32,
							  &EasyHLSSession::sPlaylistCapacity, &sDefaultPlaylistCapacity, sizeof(sDefaultPlaylistCapacity));

}

/* NVSource从RTSPClient获取数据后回调给上层 */
int Easy_APICALL __RTSPClientCallBack( int _chid, int *_chPtr, int _mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo)
{
	EasyHLSSession* pHLSSession = (EasyHLSSession *)_chPtr;

	if (NULL == pHLSSession)	return -1;

	if (NULL != frameinfo)
	{
		if (frameinfo->height==1088)		frameinfo->height=1080;
		else if (frameinfo->height==544)	frameinfo->height=540;
	}

	//投递到具体的EasyHLSSession进行处理
	pHLSSession->ProcessData(_chid, _mediatype, pbuf, frameinfo);

	return 0;
}

EasyHLSSession::EasyHLSSession(StrPtrLen* inSessionID)
:   fQueueElem(),
	fRTSPClientHandle(NULL),
	fHLSHandle(NULL),
	tsTimeStampMSsec(0)
{

    fQueueElem.SetEnclosingObject(this);

    if (inSessionID != NULL)
    {
        fHLSSessionID.Ptr = NEW char[inSessionID->Len + 1];
        ::memcpy(fHLSSessionID.Ptr, inSessionID->Ptr, inSessionID->Len);
		fHLSSessionID.Ptr[inSessionID->Len] = '\0';
        fHLSSessionID.Len = inSessionID->Len;
        fRef.Set(fHLSSessionID, this);
    }

	fHLSURL[0] = '\0';
}


EasyHLSSession::~EasyHLSSession()
{
	HLSSessionRelease();
    fHLSSessionID.Delete();
}

SInt64 EasyHLSSession::Run()
{
    EventFlags theEvents = this->GetEvents();

	if (theEvents & Task::kKillEvent)
    {
        return -1;
    }

    return 0;
}

QTSS_Error EasyHLSSession::ProcessData(int _chid, int mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo)
{
	if(NULL == fHLSHandle) return QTSS_Unimplemented;
	if (mediatype == MEDIA_TYPE_VIDEO)
	{
		printf("Get %s Video Len:%d tm:%d rtp:%d\n",frameinfo->type==FRAMETYPE_I?"I":"P", frameinfo->length, frameinfo->timestamp_sec, frameinfo->rtptimestamp);

		if(frameinfo->fps == 0) frameinfo->fps = 25;
		tsTimeStampMSsec += 1000/frameinfo->fps;
		unsigned long long llPts = tsTimeStampMSsec * 90;
	
		unsigned int uiFrameType = 0;
		if (frameinfo->type == FRAMETYPE_I)
		{
			uiFrameType = TS_TYPE_PES_VIDEO_I_FRAME;
		}
		else if (frameinfo->type == FRAMETYPE_P)
		{
			uiFrameType = TS_TYPE_PES_VIDEO_P_FRAME;
		}
		else
		{
			return QTSS_OutOfState;
		}

		EasyHLS_VideoMux(fHLSHandle, uiFrameType, (unsigned char*)pbuf, frameinfo->length, llPts, llPts, llPts);
	}
	else if (mediatype == MEDIA_TYPE_AUDIO)
	{
		printf("Get Audio Len:%d tm:%d rtp:%d\n", frameinfo->length, frameinfo->timestamp_sec, frameinfo->rtptimestamp);
		// 暂时不对音频进行处理
	}
	else if (mediatype == MEDIA_TYPE_EVENT)
	{
		if (NULL == pbuf && NULL == frameinfo)
		{
			printf("Connecting:%s ...\n", fHLSSessionID.Ptr);
		}
		else if (NULL!=frameinfo && frameinfo->type==0xF1)
		{
			printf("Lose Packet:%s ...\n", fHLSSessionID.Ptr);
		}
	}

	return QTSS_NoErr;
}

/*
	创建HLS直播Session
*/
QTSS_Error	EasyHLSSession::HLSSessionStart(char* rtspUrl)
{
	if(NULL == fRTSPClientHandle)
	{
		//创建NVSource
		EasyRTSP_Init(&fRTSPClientHandle);

		if (NULL == fRTSPClientHandle) return QTSS_Unimplemented;

		unsigned int mediaType = MEDIA_TYPE_VIDEO;
		//mediaType |= MEDIA_TYPE_AUDIO;	//换为NVSource, 屏蔽声音

		EasyRTSP_SetCallback(fRTSPClientHandle, __RTSPClientCallBack);
		EasyRTSP_OpenStream(fRTSPClientHandle, 0, rtspUrl,RTP_OVER_TCP, mediaType, 0, 0, this, 1000, 0);
	}

	if(NULL == fHLSHandle)
	{
		//创建HLSSessioin Sink
		char movieFolder[QTSS_MAX_FILE_NAME_LENGTH] = { 0 };
		UInt32 pathLen = QTSS_MAX_FILE_NAME_LENGTH;
		QTSServerInterface::GetServer()->GetPrefs()->GetMovieFolder(&movieFolder[0], &pathLen);

		fHLSHandle = EasyHLS_Session_Create(sPlaylistCapacity, sAllowCache, sM3U8Version);

		char subDir[QTSS_MAX_FILE_NAME_LENGTH] = { 0 };
		qtss_sprintf(subDir,"%s/",fHLSSessionID.Ptr);
		EasyHLS_ResetStreamCache(fHLSHandle, movieFolder, subDir, fHLSSessionID.Ptr, sTargetDuration);
		
		qtss_sprintf(fHLSURL, "%s%s/%s.m3u8", sHTTPRootDir, fHLSSessionID.Ptr, fHLSSessionID.Ptr);

	}

	return QTSS_NoErr;
}

QTSS_Error	EasyHLSSession::HLSSessionRelease()
{
	qtss_printf("HLSSession Release....\n");
	
	//释放source
	if(fRTSPClientHandle)
	{
		EasyRTSP_CloseStream(fRTSPClientHandle);
		EasyRTSP_Deinit(&fRTSPClientHandle);
		fRTSPClientHandle = NULL;
	}

	//释放sink
	if(fHLSHandle)
	{
		EasyHLS_Session_Release(fHLSHandle);
		fHLSHandle = NULL;
		fHLSURL[0] = '\0';
 	}

	return QTSS_NoErr;
}

char* EasyHLSSession::GetHLSURL()
{
	return fHLSURL;
}