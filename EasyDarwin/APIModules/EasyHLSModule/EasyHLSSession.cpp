/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
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
static UInt32					sDefaultM3U8Version					= 3; 
static bool						sDefaultAllowCache					= false; 
static UInt32					sDefaultTargetDuration				= 4;
static UInt32					sDefaultPlaylistCapacity			= 10;

UInt32                          EasyHLSSession::sM3U8Version		= 3;
bool							EasyHLSSession::sAllowCache			= false;
UInt32                          EasyHLSSession::sTargetDuration		= 4;
UInt32                          EasyHLSSession::sPlaylistCapacity	= 10;

void EasyHLSSession::Initialize(QTSS_ModulePrefsObject inPrefs)
{
	QTSSModuleUtils::GetAttribute(inPrefs, "M3U8_VERSION", qtssAttrDataTypeUInt32,
							  &EasyHLSSession::sM3U8Version, &sDefaultM3U8Version, sizeof(sDefaultM3U8Version));

	QTSSModuleUtils::GetAttribute(inPrefs, "ALLOW_CACHE", qtssAttrDataTypeBool16,
							  &EasyHLSSession::sAllowCache, &sDefaultAllowCache, sizeof(sDefaultAllowCache));

	QTSSModuleUtils::GetAttribute(inPrefs, "TARGET_DURATION", qtssAttrDataTypeUInt32,
							  &EasyHLSSession::sTargetDuration, &sDefaultTargetDuration, sizeof(sDefaultTargetDuration));

	QTSSModuleUtils::GetAttribute(inPrefs, "PLAYLIST_CAPACITY", qtssAttrDataTypeUInt32,
							  &EasyHLSSession::sPlaylistCapacity, &sDefaultPlaylistCapacity, sizeof(sDefaultPlaylistCapacity));

}

int Easy_APICALL __RTSPClientCallBack( int _chid, void *_chPtr, int _mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo)
{
	EasyHLSSession* pSession = (EasyHLSSession *)_chPtr;
	if (NULL == pSession)	return -1;
	pSession->ProcessData(_chid, _mediatype, pbuf, frameinfo);
	return 0;
}

EasyHLSSession::EasyHLSSession(StrPtrLen* inSessionID)
:   fRTSPClientHandle(NULL),
	fHLSHandle(NULL),
	fAAChandle(NULL),
	tsTimeStampMSsec(0),
	fPlayTime(0),
    fTotalPlayTime(0),
	fLastStatPlayTime(0),
	fLastStatBitrate(0),
	fNumPacketsReceived(0),
	fLastNumPacketsReceived(0),
	fNumBytesReceived(0),
	fLastNumBytesReceived(0),
	fTimeoutTask(NULL, 60*1000),
	fLastAudioPTS(0)
{
    fTimeoutTask.SetTask(this);

    if (inSessionID != NULL)
    {
        fHLSSessionID.Ptr = NEW char[inSessionID->Len + 1];
        ::memcpy(fHLSSessionID.Ptr, inSessionID->Ptr, inSessionID->Len);
		fHLSSessionID.Ptr[inSessionID->Len] = '\0';
        fHLSSessionID.Len = inSessionID->Len;
        fRef.Set(fHLSSessionID, this);
    }

	fHLSURL[0] = '\0';
	fSourceURL[0] = '\0';

	this->Signal(Task::kStartEvent);
}


EasyHLSSession::~EasyHLSSession()
{
	HLSSessionRelease();
    fHLSSessionID.Delete();

    if (this->GetRef()->GetRefCount() == 0)
    {   
        qtss_printf("EasyHLSSession::~EasyHLSSession() UnRegister and delete session =%p refcount=%"   _U32BITARG_   "\n", GetRef(), GetRef()->GetRefCount() ) ;       
        QTSServerInterface::GetServer()->GetHLSSessionMap()->UnRegister(GetRef());
    }
}

SInt64 EasyHLSSession::Run()
{
    EventFlags theEvents = this->GetEvents();
	OSRefTable* sHLSSessionMap =  QTSServerInterface::GetServer()->GetHLSSessionMap();
	OSMutexLocker locker (sHLSSessionMap->GetMutex());

	if (theEvents & Task::kKillEvent)
    {
        return -1;
    }

	if (theEvents & Task::kTimeoutEvent)
    {
		char msgStr[2048] = { 0 };
		qtss_snprintf(msgStr, sizeof(msgStr), "EasyHLSSession::Run Timeout SessionID=%s", fHLSSessionID.Ptr);
		QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);

		return -1;
    }

	//统计数据
	{
		SInt64 curTime = OS::Milliseconds();

		UInt64 bytesReceived = fNumBytesReceived - fLastNumBytesReceived;
		UInt64 durationTime	= curTime - fLastStatPlayTime;

		if(durationTime)
			fLastStatBitrate = ((bytesReceived*1000)/(durationTime))/(1024*8);

		fLastNumBytesReceived = fNumBytesReceived;
		fLastStatPlayTime = curTime;

	}

    return 2000;
}
QTSS_Error EasyHLSSession::EasyInitAACEncoder(int codec)
{
	if(fAAChandle==NULL)
	{
		InitParam initParam;
		initParam.u32AudioSamplerate = 8000;
		initParam.ucAudioChannel = 1;
		initParam.u32PCMBitSize = 16;

		if(codec == EASY_SDK_AUDIO_CODEC_G711A)
			initParam.ucAudioCodec = Law_ALaw;
		else if(codec == EASY_SDK_AUDIO_CODEC_G711U)
			initParam.ucAudioCodec = Law_ULaw;
		else
			return QTSS_UnknowAudioCoder;

		fAAChandle = Easy_AACEncoder_Init( initParam);
	}
	return QTSS_NoErr;

}
QTSS_Error EasyHLSSession::ProcessData(int _chid, int mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo)
{
	if(NULL == fHLSHandle) return QTSS_Unimplemented;

	if ((mediatype == EASY_SDK_VIDEO_FRAME_FLAG) || (mediatype == EASY_SDK_AUDIO_FRAME_FLAG))
	{
		fNumPacketsReceived++;
		fNumBytesReceived += frameinfo->length;
	}

	if (mediatype == EASY_SDK_VIDEO_FRAME_FLAG)
	{
		unsigned long long llPTS = (frameinfo->timestamp_sec%1000000)*1000 + frameinfo->timestamp_usec/1000;	

		//printf("Get %s Video \tLen:%d \ttm:%u.%u \t%u\n",frameinfo->type==EASY_SDK_VIDEO_FRAME_I?"I":"P", frameinfo->length, frameinfo->timestamp_sec, frameinfo->timestamp_usec, llPTS);

		unsigned int uiFrameType = 0;
		if (frameinfo->type == EASY_SDK_VIDEO_FRAME_I)
		{
			uiFrameType = TS_TYPE_PES_VIDEO_I_FRAME;
		}
		else if (frameinfo->type == EASY_SDK_VIDEO_FRAME_P)
		{
			uiFrameType = TS_TYPE_PES_VIDEO_P_FRAME;
		}
		else
		{
			return QTSS_OutOfState;
		}

		EasyHLS_VideoMux(fHLSHandle, uiFrameType, (unsigned char*)pbuf, frameinfo->length, llPTS*90, llPTS*90, llPTS*90);
	}
	else if (mediatype == EASY_SDK_AUDIO_FRAME_FLAG)
	{
		unsigned long long llPTS = (frameinfo->timestamp_sec%1000000)*1000 + frameinfo->timestamp_usec/1000;	

		//printf("Get Audio \tLen:%d \ttm:%u.%u \t%u\n", frameinfo->length, frameinfo->timestamp_sec, frameinfo->timestamp_usec, llPTS);
		if (frameinfo->codec == EASY_SDK_AUDIO_CODEC_G711A||frameinfo->codec ==EASY_SDK_AUDIO_CODEC_G711U)
		{
			if(EasyInitAACEncoder(frameinfo->codec) == QTSS_NoErr)
			{
				memset(pbAACBuffer,0,EASY_ACCENCODER_BUFFER_SIZE_LEN);
				unsigned int iAACBufferLen = 0;
				if(Easy_AACEncoder_Encode(fAAChandle, (unsigned char*)pbuf,  frameinfo->length, pbAACBuffer, &iAACBufferLen) > 0)
				{
					EasyHLS_AudioMux(fHLSHandle, pbAACBuffer, iAACBufferLen, fLastAudioPTS*90, fLastAudioPTS*90);
					fLastAudioPTS = 0;
				}
				
				if( fLastAudioPTS == 0)
				{
					fLastAudioPTS = llPTS;
				}
			}
		}

		if (frameinfo->codec == EASY_SDK_AUDIO_CODEC_AAC)
		{	
			EasyHLS_AudioMux(fHLSHandle, (unsigned char*)pbuf, frameinfo->length, llPTS*90, llPTS*90);
		}
	}
	else if (mediatype == EASY_SDK_EVENT_FRAME_FLAG)
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
QTSS_Error	EasyHLSSession::HLSSessionStart(char* rtspUrl, UInt32 inTimeout)
{
	QTSS_Error theErr = QTSS_NoErr;

	do{
		if(NULL == fRTSPClientHandle)
		{
			EasyRTSP_Init(&fRTSPClientHandle);

			if (NULL == fRTSPClientHandle)
			{
				theErr = QTSS_RequestFailed;
				break;
			}

			::sprintf(fSourceURL, "%s", rtspUrl);

			unsigned int mediaType = EASY_SDK_VIDEO_FRAME_FLAG | EASY_SDK_AUDIO_FRAME_FLAG;

			EasyRTSP_SetCallback(fRTSPClientHandle, __RTSPClientCallBack);
			EasyRTSP_OpenStream(fRTSPClientHandle, 0, rtspUrl, EASY_RTP_OVER_TCP, mediaType, 0, 0, this, 1000, 0, 0x01, 0);

			fPlayTime = fLastStatPlayTime = OS::Milliseconds();
			fNumPacketsReceived = fLastNumPacketsReceived = 0;
			fNumBytesReceived = fLastNumBytesReceived = 0;
		}

		if(NULL == fHLSHandle)
		{
			fHLSHandle = EasyHLS_Session_Create(sPlaylistCapacity, sAllowCache, sM3U8Version);

			if (NULL == fHLSHandle)
			{
				theErr = QTSS_Unimplemented;
				break;
			}

			char subDir[QTSS_MAX_NAME_LENGTH] = { 0 };
			qtss_sprintf(subDir,"%s/", fHLSSessionID.Ptr);

			

			char rootDir[QTSS_MAX_NAME_LENGTH] = { 0 };
			qtss_sprintf(rootDir,"%s/", QTSServerInterface::GetServer()->GetPrefs()->GetNginxRootFolder());
			EasyHLS_ResetStreamCache(fHLSHandle, rootDir, subDir, "0", sTargetDuration);

			char msgStr[2048] = { 0 };
			qtss_snprintf(msgStr, sizeof(msgStr), "EasyHLSSession::EasyHLS_ResetStreamCache SessionID=%s,rootDir=%s,subDir=%s", fHLSSessionID.Ptr, rootDir, subDir);
			QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);
					
			qtss_sprintf(fHLSURL, "%s%s/%s.m3u8", QTSServerInterface::GetServer()->GetPrefs()->GetNginxWebPath(), fHLSSessionID.Ptr, "0");
		}
		
		fTimeoutTask.SetTimeout(inTimeout * 1000);
	}while(0);

	char msgStr[2048] = { 0 };
	qtss_snprintf(msgStr, sizeof(msgStr), "EasyHLSSession::HLSSessionStart SessionID=%s,url=%s,return=%d", fHLSSessionID.Ptr, rtspUrl, theErr);
	QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);

	return theErr;
}

QTSS_Error	EasyHLSSession::HLSSessionRelease()
{
	//释放source
	if(fRTSPClientHandle)
	{
		EasyRTSP_CloseStream(fRTSPClientHandle);
		EasyRTSP_Deinit(&fRTSPClientHandle);
		fRTSPClientHandle = NULL;
		fSourceURL[0] = '\0';
	}

	//释放sink
	if(fHLSHandle)
	{
		EasyHLS_Session_Release(fHLSHandle);
		fHLSHandle = NULL;
		fHLSURL[0] = '\0';
 	}
	if(fAAChandle)
	{
		Easy_AACEncoder_Release(fAAChandle);
		fAAChandle=NULL;
		fLastAudioPTS = 0;
	}
	return QTSS_NoErr;
}

char* EasyHLSSession::GetHLSURL()
{
	return fHLSURL;
}

char* EasyHLSSession::GetSourceURL()
{
	return 	fSourceURL;
}