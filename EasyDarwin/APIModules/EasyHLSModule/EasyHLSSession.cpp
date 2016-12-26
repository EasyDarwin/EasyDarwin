/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
	File:       EasyHLSSession.cpp
	Contains:   EasyHLSSession
*/
#include "EasyHLSSession.h"

int Easy_APICALL EasyRTSPClientCallBack(int _chid, void *_chPtr, int _mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo)
{
	auto pSession = static_cast<EasyHLSSession*>(_chPtr);

	if (NULL == pSession)	return -1;

	return pSession->ProcessData(_chid, _mediatype, pbuf, frameinfo);
}

int EasyRTMP_Callback(int _frameType, char *pBuf, EASY_RTMP_STATE_T _state, void *_userPtr)
{
	//if (_state == EASY_RTMP_STATE_CONNECTING)				Logger::Instance()->info("Connecting...");
	//else if (_state == EASY_RTMP_STATE_CONNECTED)           Logger::Instance()->error("Connected");
	//else if (_state == EASY_RTMP_STATE_CONNECT_FAILED)      Logger::Instance()->error("Connect failed");
	//else if (_state == EASY_RTMP_STATE_CONNECT_ABORT)       Logger::Instance()->error("Connect abort");
	//else if (_state == EASY_RTMP_STATE_DISCONNECTED)        Logger::Instance()->info("Disconnect.");

	return 0;
}

EasyHLSSession::EasyHLSSession(StrPtrLen* inName, StrPtrLen* inSourceURL, UInt32 inChannel)
	: fSessionName(inName->GetAsCString()),
	fSourceURL(inSourceURL->GetAsCString()),
	fChannelNum(inChannel),
	fTimeoutTask(nullptr, 60 * 1000),
	fRTSPClientHandle(nullptr),
	fRTMPHandle(nullptr),
	fAAChandle(nullptr)
{
	this->SetTaskName("EasyHLSSession");
	fTimeoutTask.SetTask(this);
	fTimeoutTask.SetTimeout(90 * 1000);

	if (inName != nullptr)
	{
		char streamID[QTSS_MAX_NAME_LENGTH + 10] = { 0 };
		if (inName->Len > QTSS_MAX_NAME_LENGTH)
			inName->Len = QTSS_MAX_NAME_LENGTH;

		sprintf(streamID, "%s%s%d", inName->Ptr, EASY_KEY_SPLITER, fChannelNum);
		fSourceID.Ptr = NEW char[::strlen(streamID) + 1];
		::strncpy(fSourceID.Ptr, streamID, strlen(streamID));
		fSourceID.Ptr[strlen(streamID)] = '\0';
		fSourceID.Len = strlen(streamID);
		fRef.Set(fSourceID, this);
	}

	fHLSURL[0] = '\0';
}

EasyHLSSession::~EasyHLSSession()
{
	this->SessionRelease();

	fSourceID.Delete();
	fSessionName.Delete();
	fSourceURL.Delete();

	if (this->GetRef()->GetRefCount() == 0)
	{
		qtss_printf("EasyHLSSession::~EasyHLSSession() UnRegister and delete session =%p refcount=%"   _U32BITARG_   "\n", GetRef(), GetRef()->GetRefCount());
		QTSServerInterface::GetServer()->GetHLSSessionMap()->UnRegister(GetRef());
	}
}

SInt64 EasyHLSSession::Run()
{
	EventFlags theEvents = this->GetEvents();

	if (theEvents & Task::kKillEvent)
	{
		return -1;
	}

	if (theEvents & Task::kTimeoutEvent)
	{
		return -1;
	}

	return 0;
}

QTSS_Error EasyHLSSession::EasyInitAACEncoder(int codec)
{
	if (fAAChandle == nullptr)
	{
		InitParam initParam;
		initParam.u32AudioSamplerate = 8000;
		initParam.ucAudioChannel = 1;
		initParam.u32PCMBitSize = 16;

		if (codec == EASY_SDK_AUDIO_CODEC_G711A)
			initParam.ucAudioCodec = Law_ALaw;
		else if (codec == EASY_SDK_AUDIO_CODEC_G711U)
			initParam.ucAudioCodec = Law_ULaw;
		else
			return QTSS_UnknowAudioCoder;

		fAAChandle = Easy_AACEncoder_Init(initParam);
	}
	return QTSS_NoErr;

}

QTSS_Error EasyHLSSession::ProcessData(int _chid, int mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo)
{
	if (mediatype == EASY_SDK_VIDEO_FRAME_FLAG)
	{
		//printf("Get video Len:%d tm:%u.%u\n", frameinfo->length, frameinfo->timestamp_sec, frameinfo->timestamp_usec);
		//RTMP ONLY SUPPORT H.264 YET
		if (frameinfo->codec == EASY_SDK_VIDEO_CODEC_H264)
		{
			if (fRTMPHandle == nullptr) return 0;

			if (frameinfo && frameinfo->length)
			{
				EASY_AV_Frame  avFrame;
				memset(&avFrame, 0x00, sizeof(EASY_AV_Frame));
				avFrame.u32AVFrameFlag = EASY_SDK_VIDEO_FRAME_FLAG;
				avFrame.u32AVFrameLen = frameinfo->length;
				avFrame.pBuffer = reinterpret_cast<unsigned char*>(pbuf);
				avFrame.u32VFrameType = (frameinfo->type == EASY_SDK_VIDEO_FRAME_I) ? EASY_SDK_VIDEO_FRAME_I : EASY_SDK_VIDEO_FRAME_P;
				EasyRTMP_SendPacket(fRTMPHandle, &avFrame);
			}
		}
	}
	else if (mediatype == EASY_SDK_AUDIO_FRAME_FLAG)
	{
		//printf("Get Audio Len:%d tm:%u.%u\n", frameinfo->length, frameinfo->timestamp_sec, frameinfo->timestamp_usec);

		if (fRTMPHandle == nullptr) return 0;

		if (frameinfo && frameinfo->length && (frameinfo->codec == EASY_SDK_AUDIO_CODEC_AAC))
		{
			EASY_AV_Frame  avFrame;
			memset(&avFrame, 0x00, sizeof(EASY_AV_Frame));
			avFrame.u32AVFrameLen = frameinfo->length;
			avFrame.pBuffer = reinterpret_cast<unsigned char*>(pbuf);
			avFrame.u32VFrameType = frameinfo->type;
			avFrame.u32AVFrameFlag = EASY_SDK_AUDIO_FRAME_FLAG;
			avFrame.u32TimestampSec = frameinfo->timestamp_sec;
			avFrame.u32TimestampUsec = frameinfo->timestamp_usec;
			EasyRTMP_SendPacket(fRTMPHandle, &avFrame);
		}

		//printf("Get Audio \tLen:%d \ttm:%u.%u \t%u\n", frameinfo->length, frameinfo->timestamp_sec, frameinfo->timestamp_usec, llPTS);
		if (frameinfo->codec == EASY_SDK_AUDIO_CODEC_G711A || frameinfo->codec == EASY_SDK_AUDIO_CODEC_G711U)
		{
			if (EasyInitAACEncoder(frameinfo->codec) == QTSS_NoErr)
			{
				memset(pbAACBuffer, 0, EASY_ACCENCODER_BUFFER_SIZE_LEN);
				unsigned int iAACBufferLen = 0;
				if (Easy_AACEncoder_Encode(fAAChandle, reinterpret_cast<unsigned char*>(pbuf), frameinfo->length, pbAACBuffer, &iAACBufferLen) > 0)
				{
					EASY_AV_Frame avFrame;
					memset(&avFrame, 0x00, sizeof(EASY_AV_Frame));
					avFrame.u32AVFrameLen = iAACBufferLen;
					avFrame.pBuffer = reinterpret_cast<unsigned char*>(pbAACBuffer);
					avFrame.u32VFrameType = frameinfo->type;
					avFrame.u32AVFrameFlag = EASY_SDK_AUDIO_FRAME_FLAG;
					avFrame.u32TimestampSec = frameinfo->timestamp_sec;
					avFrame.u32TimestampUsec = frameinfo->timestamp_usec;
					EasyRTMP_SendPacket(fRTMPHandle, &avFrame);
				}
			}
		}

	}
	else if (mediatype == EASY_SDK_MEDIA_INFO_FLAG)
	{
		//if((NULL == fRTMPHandle) && (pbuf != NULL) )
		//{
		//	fRTMPHandle = EasyRTMP_Create();
		//	EASY_MEDIA_INFO_T mediainfo;
		//	memset(&mediainfo, 0x00, sizeof(EASY_MEDIA_INFO_T));
		//	mediainfo.u32VideoFps = 25;
		//	mediainfo.u32AudioSamplerate = 8000;

		//	EasyRTMP_SetCallback(fRTMPHandle, EasyRTMP_Callback, nullptr);

		//	auto bRet = EasyRTMP_Connect(fRTMPHandle, "rtmp://127.0.0.1:10035/live/test");
		//	if (!bRet)
		//	{
		//		printf("Fail to EasyRTMP_Connect channel {}");
		//	}
		//	auto iRet = EasyRTMP_InitMetadata(fRTMPHandle, &mediainfo, 1024);
		//	if (iRet < 0)
		//	{
		//		printf("Fail to InitMetadata channel {}");
		//	}
		//	qtss_sprintf(fHLSURL, "%s/%s", QTSServerInterface::GetServer()->GetPrefs()->GetNginxRTMPPath(), fSourceID.Ptr);
		//	//fLocalRTMPAutoStop.ResetTimeout();
		//}

	}
	else if (mediatype == EASY_SDK_EVENT_FRAME_FLAG)
	{
		;
	}

	return QTSS_NoErr;
}

QTSS_Error	EasyHLSSession::SessionStart()
{
	if (NULL == fRTSPClientHandle)
	{
		//´´½¨EasyRTSPClient
		EasyRTSP_Init(&fRTSPClientHandle);

		if (NULL == fRTSPClientHandle) return QTSS_Unimplemented;

		unsigned int mediaType = EASY_SDK_VIDEO_FRAME_FLAG | EASY_SDK_AUDIO_FRAME_FLAG;

		EasyRTSP_SetCallback(fRTSPClientHandle, EasyRTSPClientCallBack);
		EasyRTSP_OpenStream(fRTSPClientHandle, 0, fSourceURL.Ptr, EASY_RTP_OVER_TCP, mediaType, nullptr, nullptr, this, 1000, 0, 0x01, 0);
	}

	if (NULL == fRTMPHandle)
	{
		fRTMPHandle = EasyRTMP_Create();
		EASY_MEDIA_INFO_T mediainfo;
		memset(&mediainfo, 0x00, sizeof(EASY_MEDIA_INFO_T));
		mediainfo.u32VideoFps = 25;
		mediainfo.u32AudioSamplerate = 8000;

		EasyRTMP_SetCallback(fRTMPHandle, EasyRTMP_Callback, nullptr);

		char localRTMPURL[QTSS_MAX_URL_LENGTH] = { 0 };
		qtss_sprintf(localRTMPURL, "rtmp://%s:%d/hls/%s", "127.0.0.1", QTSServerInterface::GetServer()->GetPrefs()->GetRTMPWANPort(), fSourceID.Ptr);

		qtss_sprintf(fHLSURL, "%shls/%s.m3u8", QTSServerInterface::GetServer()->GetPrefs()->GetNginxWebPath(), fSourceID.Ptr);

		auto bRet = EasyRTMP_Connect(fRTMPHandle, localRTMPURL);
		if (!bRet)
		{
			printf("Fail to EasyRTMP_Connect channel {}");
		}
		auto iRet = EasyRTMP_InitMetadata(fRTMPHandle, &mediainfo, 1024);
		if (iRet < 0)
		{
			printf("Fail to InitMetadata channel {}");
		}


		//fLocalRTMPAutoStop.ResetTimeout();

	}

	return QTSS_NoErr;
}

QTSS_Error	EasyHLSSession::SessionRelease()
{
	if (fRTSPClientHandle)
	{
		EasyRTSP_CloseStream(fRTSPClientHandle);
		EasyRTSP_Deinit(&fRTSPClientHandle);
		fRTSPClientHandle = nullptr;
	}

	if (fRTMPHandle)
	{
		EasyRTMP_Release(fRTMPHandle);
		fRTMPHandle = nullptr;
		fHLSURL[0] = '\0';
	}

	if (fAAChandle)
	{
		Easy_AACEncoder_Release(fAAChandle);
		fAAChandle = nullptr;
	}

	return QTSS_NoErr;
}
