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

#ifndef __Win32__
    #include <unistd.h>
#endif

static FILE* fTest;

// PREFS
static UInt32                   sDefaultM3U8Version				= 3; 
static Bool16                   sDefaultAllowCache					= false; 
static UInt32                   sDefaultTargetDuration				= 4;
static UInt32                   sDefaultPlaylistCapacity			= 4;
static char*					sDefaultLocalRootDir				= "./HLS/";
static char*					sDefaultHTTPRootDir					= "http://hls.easydarwin.org/";

UInt32                          EasyHLSSession::sM3U8Version		= 3;
Bool16                          EasyHLSSession::sAllowCache			= false;
UInt32                          EasyHLSSession::sTargetDuration		= 4;
UInt32                          EasyHLSSession::sPlaylistCapacity	= 4;
char*							EasyHLSSession::sLocalRootDir		= "./HLS/";
char*							EasyHLSSession::sHTTPRootDir		= "http://hls.easydarwin.org/";


void EasyHLSSession::Initialize(QTSS_ModulePrefsObject inPrefs)
{
	delete [] sLocalRootDir;
    sLocalRootDir = QTSSModuleUtils::GetStringAttribute(inPrefs, "LOCAL_ROOT_DIR", sDefaultLocalRootDir);

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
int CALLBACK __NVSourceCallBack( int _chid, int *_chPtr, int _mediatype, char *pbuf, NVS_FRAME_INFO *frameinfo)
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
	fNVSHandle(NULL),
	fHLSHandle(NULL)
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

	fHLSHandle = HLSSession_Create(sPlaylistCapacity, sAllowCache, sM3U8Version);
	ResetStreamCache(fHLSHandle, sLocalRootDir, fHLSSessionID.Ptr, fHLSSessionID.Ptr, sTargetDuration);

	fTest = ::fopen("./aaa.264","wb");
}


EasyHLSSession::~EasyHLSSession()
{
    fHLSSessionID.Delete();
}

QTSS_Error EasyHLSSession::ProcessData(int _chid, int mediatype, char *pbuf, NVS_FRAME_INFO *frameinfo)
{
	if (mediatype == MEDIA_TYPE_VIDEO)
	{
		//printf("Get %s Video Len:%d tm:%d rtp:%d\n",frameinfo->type==FRAMETYPE_I?"I":"P", frameinfo->length, frameinfo->timestamp_sec, frameinfo->rtptimestamp);
		::fwrite(pbuf, 1, frameinfo->length, fTest);
		VideoMux(fHLSHandle, frameinfo->type, (unsigned char*)pbuf, frameinfo->length, frameinfo->rtptimestamp,frameinfo->rtptimestamp,frameinfo->rtptimestamp);
	}
	else if (mediatype == MEDIA_TYPE_AUDIO)
	{
		//printf("Get Audio Len:%d tm:%d rtp:%d\n", frameinfo->length, frameinfo->timestamp_sec, frameinfo->rtptimestamp);
	}
	else if (mediatype == MEDIA_TYPE_EVENT)
	{
		printf("Get MEDIA_TYPE_EVENT\n");
		if (NULL == pbuf && NULL == frameinfo)
		{
			printf("Connecting:%s ...\n", fHLSSessionID.Ptr);
			//_TRACE("[ch%d]连接中...\n", _chid);
			//MEDIA_FRAME_INFO	frameinfo;
			//memset(&frameinfo, 0x00, sizeof(MEDIA_FRAME_INFO));
			//frameinfo.length = 1;
			//frameinfo.type   = 0xFF;
			//SSQ_AddData(pRealtimePlayThread[_chid].pAVQueue, _chid, MEDIA_TYPE_EVENT, (MEDIA_FRAME_INFO*)&frameinfo, "1");
		}
		else if (NULL!=frameinfo && frameinfo->type==0xF1)
		{
			printf("Lose Packet:%s ...\n", fHLSSessionID.Ptr);
			//_TRACE("[ch%d]掉包[%.2f]...\n", _chid, frameinfo->losspacket);
			//frameinfo->length = 1;
			//SSQ_AddData(pRealtimePlayThread[_chid].pAVQueue, _chid, MEDIA_TYPE_EVENT, (MEDIA_FRAME_INFO*)frameinfo, "1");
		}
	}

	return QTSS_NoErr;
}

QTSS_Error	EasyHLSSession::HLSSessionCreate(char* rtspUrl)
{
	NVS_Init(&fNVSHandle);

	if (NULL == fNVSHandle) return QTSS_Unimplemented;

	unsigned int mediaType = MEDIA_TYPE_VIDEO;
	mediaType |= MEDIA_TYPE_AUDIO;	//换为NVSource, 屏蔽声音

	NVS_SetCallback(fNVSHandle, __NVSourceCallBack);
	NVS_OpenStream(fNVSHandle, 0, rtspUrl,RTP_OVER_TCP, mediaType, 0, 0, this, 1000, 0);

	return QTSS_NoErr;
}

QTSS_Error	EasyHLSSession::HLSSessionRelease()
{
	if(NULL == fNVSHandle)	return QTSS_BadArgument;
	NVS_CloseStream(fNVSHandle);
	NVS_Deinit(&fNVSHandle);

	::fclose(fTest);
	return QTSS_NoErr;
}