/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include "EasyCameraSource.h"
#include "QTSServerInterface.h"
#include "EasyProtocolDef.h"
#include "HCNetSDK.h"

#include <map>
#include <APICommonCode/QTSSModuleUtils.h>

#include <EasyUtil.h>

////////////////////////////////////////////////////////////////////////////////////////////

bool IsIFrame(unsigned char* buf)
{
	unsigned char naltype = (buf[4] & 0x1F);
	switch (naltype)
	{
	case 7:	//sps
	case 8:	// pps
	case 6: // i
	case 5:	//idr
		return true;
	case 1: // slice
	case 9:	// unknown ???
	default:
		return false;
	}
}

bool GetH246FromPS(IN BYTE* pBuffer, IN int nBufLenth, BYTE** pH264, int& nH264Lenth, BOOL& bVideo)
{
	if (!pBuffer || nBufLenth <= 0)
	{
		return FALSE;
	}

	BYTE* pH264Buffer = NULL;
	int nHerderLen = 0;

	if (pBuffer
		&& pBuffer[0] == 0x00
		&& pBuffer[1] == 0x00
		&& pBuffer[2] == 0x01
		&& pBuffer[3] == 0xE0)//E==视频数据(此处E0标识为视频)
	{
		bVideo = TRUE;
		nHerderLen = 9 + (int)pBuffer[8];//9个为固定的数据包头长度，pBuffer[8]为填充头部分的长度
		pH264Buffer = pBuffer + nHerderLen;
		if (*pH264 == NULL)
		{
			*pH264 = new BYTE[nBufLenth];
		}
		if (*pH264&&pH264Buffer && (nBufLenth - nHerderLen)>0)
		{
			memcpy(*pH264, pH264Buffer, (nBufLenth - nHerderLen));
		}
		nH264Lenth = nBufLenth - nHerderLen;

		return TRUE;
	}
	else if (pBuffer
		&& pBuffer[0] == 0x00
		&& pBuffer[1] == 0x00
		&& pBuffer[2] == 0x01
		&& pBuffer[3] == 0xC0) //C==音频数据？
	{
		*pH264 = NULL;
		nH264Lenth = 0;
		bVideo = FALSE;
	}
	else if (pBuffer
		&& pBuffer[0] == 0x00
		&& pBuffer[1] == 0x00
		&& pBuffer[2] == 0x01
		&& pBuffer[3] == 0xBA)//视频流数据包 包头
	{
		bVideo = TRUE;
		*pH264 = NULL;
		nH264Lenth = 0;
		return FALSE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////


static unsigned int sLastVPTS = 0;
static unsigned int sLastAPTS = 0;

// Camera IP
static char*            sCamera_IP = NULL;
static char*            sDefaultCamera_IP_Addr = "127.0.0.1";
// Camera Port
static UInt16			sCameraPort = 80;
static UInt16			sDefaultCameraPort = 80;
// Camera user
static char*            sCameraUser = NULL;
static char*            sDefaultCameraUser = "admin";
// Camera password
static char*            sCameraPassword = NULL;
static char*            sDefaultCameraPassword = "admin";
// Camera stream subtype
static UInt32			sStreamType = 1;
static UInt32			sDefaultStreamType = 1;

static int HSBUFFLEN = 164;
static int BUFFLEN = 160;
static int PTSPER = 20;

int AddHead(char* dst, char* src, int length)
{
	dst[0] = 0x00;
	dst[1] = 0x01;
	dst[2] = 0x50;
	dst[3] = 0x00;
	memcpy(&dst[4], src, length);

	return 0;
}

void EasyCameraSource::Initialize(QTSS_ModulePrefsObject modulePrefs)
{
	delete[] sCamera_IP;
	sCamera_IP = QTSSModuleUtils::GetStringAttribute(modulePrefs, "camera_ip", sDefaultCamera_IP_Addr);

	QTSSModuleUtils::GetAttribute(modulePrefs, "camera_port", qtssAttrDataTypeUInt16, &sCameraPort, &sDefaultCameraPort, sizeof(sCameraPort));

	delete[] sCameraUser;
	sCameraUser = QTSSModuleUtils::GetStringAttribute(modulePrefs, "camera_user", sDefaultCameraUser);

	delete[] sCameraPassword;
	sCameraPassword = QTSSModuleUtils::GetStringAttribute(modulePrefs, "camera_password", sDefaultCameraPassword);

	QTSSModuleUtils::GetAttribute(modulePrefs, "camera_stream_type", qtssAttrDataTypeUInt32, &sStreamType, &sDefaultStreamType, sizeof(sStreamType));
}

int __EasyPusher_Callback(int _id, EASY_PUSH_STATE_T _state, EASY_AV_Frame *_frame, void *_userptr)
{
	if (_state == EASY_PUSH_STATE_CONNECTING)               printf("Connecting...\n");
	else if (_state == EASY_PUSH_STATE_CONNECTED)           printf("Connected\n");
	else if (_state == EASY_PUSH_STATE_CONNECT_FAILED)      printf("Connect failed\n");
	else if (_state == EASY_PUSH_STATE_CONNECT_ABORT)       printf("Connect abort\n");
	//else if (_state == EASY_PUSH_STATE_PUSHING)             printf("P->");
	else if (_state == EASY_PUSH_STATE_DISCONNECTED)        printf("Disconnect.\n");

	return 0;
}

bool myLoginProc(char* strIp, WORD wPort, DWORD dwError, DWORD dwUser)
{
	return true;
}

bool myErrorProc(char* strip, WORD wPort, DWORD dwUser)
{
	return true;
}

bool myMessageProc(char* cmd, char* buf, int size, DWORD dwUser)
{
	return true;
}

bool myPlayErrorProc(DWORD hPlayer, DWORD dwUser)
{
	return true;
}

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	char tempbuf[256] = { 0 };
	switch (dwType)
	{
	case EXCEPTION_RECONNECT: //预览时重连
		printf("----------reconnect--------%d\n", time(NULL));
		break;
	default:
		break;
	}
}

void CALLBACK myStreamProc(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser)
{
	EasyCameraSource* pThis = (EasyCameraSource*)pUser;
	pThis->PushFrame((unsigned char*)pBuffer, dwBufSize, dwDataType);
}

EasyCameraSource::EasyCameraSource()
	: Task(),
	m_u32Handle(-1),
	streamHandle(-1),
	fCameraLogin(false),
	m_bStreamFlag(false),
	m_bForceIFrame(true),
	fPusherHandle(NULL),
	//fTalkbackBuff(NULL),
	fPlayer(0),
	fPusherBuff(NULL),
	fPusherBuffOffset(0)
{
	this->SetTaskName("EasyCameraSource");

	//SDK初始化，全局调用一次
	NET_DVR_Init();

	//设置连接时间与重连时间
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);

	//登陆摄像机
	cameraLogin();

	//---------------------------------------
	//设置异常消息回调函数
	NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

	fCameraSnapPtr = new unsigned char[EASY_SNAP_BUFFER_SIZE];

	fTimeoutTask = new TimeoutTask(this, 30 * 1000);

	fTimeoutTask->RefreshTimeout();

	//fTalkbackBuff = (char*)malloc(164 * sizeof(char));
	fPusherBuff = new unsigned char[1024 * 50];

}

EasyCameraSource::~EasyCameraSource()
{
	//free(fTalkbackBuff);

	if (fCameraSnapPtr)
	{
		delete[] fCameraSnapPtr;
		fCameraSnapPtr = NULL;
	}

	if (fPusherBuff)
	{
		delete[] fPusherBuff;
		fPusherBuff = NULL;
	}

	if (fTimeoutTask)
	{
		delete fTimeoutTask;
		fTimeoutTask = NULL;
	}

	//先停止Stream，内部有是否在Stream的判断
	netDevStopStream();

	if (fCameraLogin)
		NET_DVR_Logout_V30(m_u32Handle);

	//SDK释放，全局调用一次
	NET_DVR_Cleanup();
}

bool EasyCameraSource::cameraLogin()
{
	//如果已登录，返回true
	if (fCameraLogin) return true;

	//登录到摄像机
	NET_DVR_DEVICEINFO_V30 deviceInfoTmp;
	memset(&deviceInfoTmp, 0, sizeof(NET_DVR_DEVICEINFO_V30));
	m_u32Handle = NET_DVR_Login_V30(sCamera_IP, sCameraPort, sCameraUser, sCameraPassword, &deviceInfoTmp);
	if (m_u32Handle < 0)
	{
		qtss_printf("NET_DVR_Login_V30 Fail\n");
		fPlayer = 0;
		return false;
	}
	else
	{
		fCameraLogin = true;
	}

	return true;
}

QTSS_Error EasyCameraSource::netDevStartStream()
{
	//如果未登录,返回失败
	if (!cameraLogin()) return QTSS_RequestFailed;

	//已经在流传输中，返回QTSS_NoErr
	if (m_bStreamFlag) return QTSS_NoErr;

	OSMutexLocker locker(this->GetMutex());

	NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
	struPlayInfo.hPlayWnd = NULL; //需要 SDK 解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo.lChannel = 1; //预览通道号
	struPlayInfo.dwStreamType = 0; //0-主码流， 1-子码流， 2-码流 3， 3-码流 4，以此类推
	struPlayInfo.dwLinkMode = 0; //0- TCP 方式， 1- UDP 方式， 2- 多播方式， 3- RTP 方式， 4-RTP/RTSP， 5-RSTP/HTTP
	struPlayInfo.bBlocked = 1; //0- 非阻塞取流， 1- 阻塞取流

	streamHandle = NET_DVR_RealPlay_V40(m_u32Handle, &struPlayInfo, myStreamProc, this);
	if (streamHandle < 0)
	{
		qtss_printf("NET_DVR_RealPlay_V40 Fail\n");
		LONG error = NET_DVR_GetLastError();
		return QTSS_RequestFailed;
	}

	m_bStreamFlag = true;
	m_bForceIFrame = true;
	qtss_printf("NET_DVR_RealPlay_V40 SUCCESS\n");

	return QTSS_NoErr;
}

void EasyCameraSource::netDevStopStream()
{
	if (m_bStreamFlag)
	{
		qtss_printf("HI_NET_DEV_StopStream\n");
		NET_DVR_StopRealPlay(streamHandle);
		m_bStreamFlag = false;
	}
}

void EasyCameraSource::stopGettingFrames()
{
	OSMutexLocker locker(this->GetMutex());
	doStopGettingFrames();
}

void EasyCameraSource::doStopGettingFrames()
{
	qtss_printf("doStopGettingFrames()\n");
	netDevStopStream();
}

bool EasyCameraSource::getSnapData(unsigned char* pBuf, UInt32 uBufLen, int* uSnapLen)
{
	//如果摄像机未登录，返回false
	if (!cameraLogin()) return false;

	//调用SDK获取数据
	LPNET_DVR_JPEGPARA jpegPara = new NET_DVR_JPEGPARA;
	jpegPara->wPicQuality = 0;
	jpegPara->wPicSize = 9;

	LPDWORD Ret = 0;

	NET_DVR_SetCapturePictureMode(JPEG_MODE);
	/*{
		cout << "Set Capture Picture Mode error!" << endl;
		cout << "The error code is " << NET_DVR_GetLastError() << endl;
	}*/

	bool capture = NET_DVR_CaptureJPEGPicture_NEW(m_u32Handle, 1, jpegPara, (char*)pBuf, uBufLen, (LPDWORD)uSnapLen);
	if (!capture)
	{
		printf("Error: NET_DVR_CaptureJPEGPicture_NEW = %d", NET_DVR_GetLastError());
		delete jpegPara;
		return false;
	}

	delete jpegPara;
	return true;
}

SInt64 EasyCameraSource::Run()
{
	EventFlags events = this->GetEvents();

	if (events & Task::kTimeoutEvent)
	{
		//do nothing
		fTimeoutTask->RefreshTimeout();
	}

	return 0;
}

QTSS_Error EasyCameraSource::StartStreaming(Easy_StartStream_Params* inParams)
{
	QTSS_Error theErr = QTSS_NoErr;

	do
	{
		{
			OSMutexLocker locker(&fStreamingMutex);
			if (NULL == fPusherHandle)
			{
				if (!cameraLogin())
				{
					theErr = QTSS_RequestFailed;
					break;
				}

				NET_DVR_COMPRESSIONCFG_V30 struCompressionCfg;
				memset(&struCompressionCfg, 0, sizeof(struCompressionCfg));
				DWORD dwReturned = 0;
				EASY_MEDIA_INFO_T mediainfo;
				memset(&mediainfo, 0x00, sizeof(EASY_MEDIA_INFO_T));
				mediainfo.u32VideoCodec = EASY_SDK_VIDEO_CODEC_H264;
				mediainfo.u32AudioChannel = 1;
				if (!NET_DVR_GetDVRConfig(m_u32Handle, NET_DVR_GET_COMPRESSCFG_V30, 1, &struCompressionCfg, sizeof(NET_DVR_COMPRESSIONCFG_V30), &dwReturned))
				{
					printf("can't get config！\n");

					mediainfo.u32VideoFps = 25;
					mediainfo.u32AudioCodec = EASY_SDK_AUDIO_CODEC_G711A;				
					mediainfo.u32AudioSamplerate = 8000;
				}
				else
				{
					mediainfo.u32VideoFps = getFrameRateFromHKSDK(struCompressionCfg.struNormHighRecordPara.dwVideoFrameRate);
					mediainfo.u32AudioCodec = getAudioCodecFromHKSDK(struCompressionCfg.struNormHighRecordPara.byAudioEncType);
					mediainfo.u32AudioSamplerate = getAudioSimpleRateFromHKSDK(struCompressionCfg.struNormHighRecordPara.byAudioSamplingRate);
				}

				fPusherHandle = EasyPusher_Create();
				if (fPusherHandle == NULL)
				{
					//EasyPusher初始化创建失败,可能是EasyPusher SDK未授权
					theErr = QTSS_Unimplemented;
					break;
				}

				// 注册流推送事件回调
				EasyPusher_SetEventCallback(fPusherHandle, __EasyPusher_Callback, 0, NULL);

				// 根据接收到的命令生成流信息
				char sdpName[128] = { 0 };
				sprintf(sdpName, "%s/%s.sdp", /*inParams->inStreamID,*/ inParams->inSerial, inParams->inChannel);

				// 开始推送流媒体数据
				EasyPusher_StartStream(fPusherHandle, (char*)inParams->inIP, inParams->inPort, sdpName, "", "", &mediainfo, 1024/* 1M Buffer*/, 0);

				saveStartStreamParams(inParams);
			}
		}

		theErr = netDevStartStream();

	} while (0);


	if (theErr != QTSS_NoErr)
	{
		// 如果推送不成功，需要释放之前已经开启的资源
		StopStreaming(NULL);
	}
	else
	{
		// 推送成功，将当前正在推送的参数信息回调
		inParams->inChannel = fStartStreamInfo.channel;
		inParams->inIP = fStartStreamInfo.ip;
		inParams->inPort = fStartStreamInfo.port;
		inParams->inProtocol = fStartStreamInfo.protocol;
		inParams->inSerial = fStartStreamInfo.serial;
		inParams->inStreamID = fStartStreamInfo.streamId;
	}

	return theErr;
}

void EasyCameraSource::saveStartStreamParams(Easy_StartStream_Params * inParams)
{
	// 保存最新推送参数
	strncpy(fStartStreamInfo.serial, inParams->inSerial, strlen(inParams->inSerial));
	fStartStreamInfo.serial[strlen(inParams->inSerial)] = 0;

	strncpy(fStartStreamInfo.channel, inParams->inChannel, strlen(inParams->inChannel));
	fStartStreamInfo.channel[strlen(inParams->inChannel)] = 0;

	strncpy(fStartStreamInfo.streamId, inParams->inStreamID, strlen(inParams->inStreamID));
	fStartStreamInfo.streamId[strlen(inParams->inStreamID)] = 0;

	strncpy(fStartStreamInfo.protocol, inParams->inProtocol, strlen(inParams->inProtocol));
	fStartStreamInfo.protocol[strlen(inParams->inProtocol)] = 0;

	memcpy(fStartStreamInfo.ip, inParams->inIP, strlen(inParams->inIP));
	fStartStreamInfo.ip[strlen(inParams->inIP)] = 0;

	fStartStreamInfo.port = inParams->inPort;
}

QTSS_Error EasyCameraSource::StopStreaming(Easy_StopStream_Params* inParams)
{
	{
		OSMutexLocker locker(&fStreamingMutex);
		if (fPusherHandle)
		{
			EasyPusher_StopStream(fPusherHandle);
			EasyPusher_Release(fPusherHandle);
			fPusherHandle = 0;
		}
	}

	stopGettingFrames();

	return QTSS_NoErr;
}

QTSS_Error EasyCameraSource::PushFrame(unsigned char* frame, int len, DWORD dataType)
{
	OSMutexLocker locker(&fStreamingMutex);
	if (fPusherHandle == NULL) return QTSS_Unimplemented;

	if (dataType == NET_DVR_STREAMDATA)
	{		
		if (len > 0)
		{
			unsigned char *h264Buf = NULL;
			int h264Len = 0;
			BOOL isVideo;
			GetH246FromPS(frame, len, &h264Buf, h264Len, isVideo);
			if (isVideo)
			{
				if (h264Buf)
				{
					/*printf("size:%04d\t[0]:0x%08X\t[1]:0x%08X\t[2]:0x%08X\t[3]:0x%08X\t[4]:0x%08X \n",
						h264Len, h264Buf[0], h264Buf[1], h264Buf[2], h264Buf[3], h264Buf[4]);*/

					if (h264Buf[0] == 0 && h264Buf[1] == 0 && h264Buf[2] == 0 && h264Buf[3] == 1)
					{
						if (fPusherBuffOffset > 0)
						{
							EASY_AV_Frame avFrameVideo;
							memset(&avFrameVideo, 0x00, sizeof(EASY_AV_Frame));
							avFrameVideo.u32AVFrameLen = fPusherBuffOffset;
							avFrameVideo.pBuffer = (unsigned char*)fPusherBuff;
							bool isKeyFrame = IsIFrame(fPusherBuff);
							avFrameVideo.u32VFrameType = isKeyFrame ? EASY_SDK_VIDEO_FRAME_I : EASY_SDK_VIDEO_FRAME_P;
							avFrameVideo.u32AVFrameFlag = EASY_SDK_VIDEO_FRAME_FLAG;
							//avFrameVideo.u32TimestampSec = pstruAV->u32AVFramePTS / 1000;
							//avFrameVideo.u32TimestampUsec = (pstruAV->u32AVFramePTS % 1000) * 1000;
							Easy_U32 ret = EasyPusher_PushFrame(fPusherHandle, &avFrameVideo);
							//printf("-- Pushing: Frame %s Result %d \n", isKeyFrame ? "I" : "P", ret);
							fPusherBuffOffset = 0;							
						}
						memcpy(fPusherBuff, h264Buf, h264Len);
						fPusherBuffOffset += h264Len;
					}
					else
					{
						if (fPusherBuffOffset > 0)
						{
							memcpy(fPusherBuff + fPusherBuffOffset, h264Buf, h264Len);
							fPusherBuffOffset += h264Len;
						}
					}
				}
				if (h264Buf)
				{
					delete[] h264Buf;
					h264Buf = NULL;
				}
			}
		}
	}
	else if (dataType == NET_DVR_AUDIOSTREAMDATA)
	{
		if (len > 0)
		{
			EASY_AV_Frame avFrameAudio;
			memset(&avFrameAudio, 0x00, sizeof(EASY_AV_Frame));
			avFrameAudio.u32AVFrameLen = len;
			avFrameAudio.pBuffer = (unsigned char*)frame;
			avFrameAudio.u32AVFrameFlag = EASY_SDK_AUDIO_FRAME_FLAG;
			//avFrameAudio.u32TimestampSec = pstruAV->u32AVFramePTS / 1000;
			//avFrameAudio.u32TimestampUsec = (pstruAV->u32AVFramePTS % 1000) * 1000;
			EasyPusher_PushFrame(fPusherHandle, &avFrameAudio);
		}
	}

	return Easy_NoErr;
}

QTSS_Error EasyCameraSource::GetCameraState(Easy_CameraState_Params* params)
{
	params->outIsLogin = cameraLogin();
	params->outIsStreaming = m_bStreamFlag;
	return 0;
}

QTSS_Error EasyCameraSource::GetCameraSnap(Easy_CameraSnap_Params* params)
{
	QTSS_Error theErr = QTSS_NoErr;

	params->outSnapLen = 0;

	int snapBufLen = 0;
	do
	{
		if (!getSnapData(fCameraSnapPtr, EASY_SNAP_BUFFER_SIZE, &snapBufLen))
		{
			//未获取到数据
			qtss_printf("EasyCameraSource::GetCameraSnap::getSnapData() => Get Snap Data Fail \n");
			theErr = QTSS_ValueNotFound;
			break;
		}

		params->outSnapLen = snapBufLen;
		params->outSnapPtr = fCameraSnapPtr;
		params->outSnapType = EASY_SNAP_TYPE_JPEG;
	} while (false);

	return theErr;
}

QTSS_Error EasyCameraSource::ControlPTZ(Easy_CameraPTZ_Params* params)
{
	return QTSS_NoErr;
}

QTSS_Error EasyCameraSource::ControlPreset(Easy_CameraPreset_Params* params)
{
	return QTSS_NoErr;
}

QTSS_Error EasyCameraSource::ControlTalkback(Easy_CameraTalkback_Params* params)
{
	return QTSS_NoErr;
}

int EasyCameraSource::getFrameRateFromHKSDK(DWORD type)
{
	switch (type)
	{
	case 5:
		return 1;
	case 6:
		return 2;
	case 7:
		return 4;
	case 8:
		return 6;
	case 9:
		return 8;
	case 10:
		return 10;
	case 11:
		return 12;
	case 12:
		return 16;
	case 13:
		return 20;
	case 14:
		return 15;
	case 15:
		return 18;
	case 16:
		return 22;
	case 17:
		return 25;
	case 18:
		return 30;
	case 19:
		return 35;
	case 20:
		return 40;
	case 21:
		return 45;
	case 22:
		return 50;
	case 23:
		return 55;
	case 24:
		return 60;
	case 25:
		return 3;
	case 26:
		return 5;
	case 27:
		return 7;
	case 28:
		return 9;
	case 29:
		return 100;
	case 30:
		return 120;
	case 31:
		return 24;
	case 32:
		return 48;
	default:
		return 25;
	}
}

int EasyCameraSource::getAudioCodecFromHKSDK(unsigned char type)
{
	switch (type)
	{
	case '1':
		return EASY_SDK_AUDIO_CODEC_G711U;
	case '2':
		return EASY_SDK_AUDIO_CODEC_G711A;
	case '6':
		return EASY_SDK_AUDIO_CODEC_G726;
	case '7':
		return EASY_SDK_AUDIO_CODEC_AAC;
	default:
		return EASY_SDK_AUDIO_CODEC_G711A;
	}
}

unsigned int EasyCameraSource::getAudioSimpleRateFromHKSDK(unsigned char type)
{
	switch (type)
	{
	case '1':
		return 16000;
	case '2':
		return 32000;
	case '3':
		return 48000;
	case '4':
		return 44100;
	case '5':
		return 8000;
	default:
		return 8000;
	}
}

//HI_U32 EasyCameraSource::getPTZCMDFromCMDType(int cmdType)
//{
//	switch (cmdType)
//	{
//	case EASY_PTZ_CMD_TYPE_STOP:
//		return HI_NET_DEV_CTRL_PTZ_STOP;
//	case EASY_PTZ_CMD_TYPE_UP:
//		return HI_NET_DEV_CTRL_PTZ_UP;
//	case EASY_PTZ_CMD_TYPE_DOWN:
//		return HI_NET_DEV_CTRL_PTZ_DOWN;
//	case EASY_PTZ_CMD_TYPE_LEFT:
//		return HI_NET_DEV_CTRL_PTZ_LEFT;
//	case EASY_PTZ_CMD_TYPE_RIGHT:
//		return HI_NET_DEV_CTRL_PTZ_RIGHT;
//	case EASY_PTZ_CMD_TYPE_LEFTUP:
//		return 0;
//	case EASY_PTZ_CMD_TYPE_LEFTDOWN:
//		return 0;
//	case EASY_PTZ_CMD_TYPE_RIGHTUP:
//		return 0;
//	case EASY_PTZ_CMD_TYPE_RIGHTDOWN:
//		return 0;
//	case EASY_PTZ_CMD_TYPE_ZOOMIN:
//		return HI_NET_DEV_CTRL_PTZ_ZOOMIN;
//	case EASY_PTZ_CMD_TYPE_ZOOMOUT:
//		return HI_NET_DEV_CTRL_PTZ_ZOOMOUT;
//	case EASY_PTZ_CMD_TYPE_FOCUSIN:
//		return HI_NET_DEV_CTRL_PTZ_FOCUSIN;
//	case EASY_PTZ_CMD_TYPE_FOCUSOUT:
//		return HI_NET_DEV_CTRL_PTZ_FOCUSOUT;
//	case EASY_PTZ_CMD_TYPE_APERTUREIN:
//		return HI_NET_DEV_CTRL_PTZ_APERTUREIN;
//	case EASY_PTZ_CMD_TYPE_APERTUREOUT:
//		return HI_NET_DEV_CTRL_PTZ_APERTUREOUT;
//	default:
//		return 0;
//	}
//}
//
//HI_U32 EasyCameraSource::getPresetCMDFromCMDType(int cmdType)
//{
//	switch (cmdType)
//	{
//	case EASY_PRESET_CMD_TYPE_GOTO:
//		return HI_NET_DEV_CTRL_PTZ_GOTO_PRESET;
//	case EASY_PRESET_CMD_TYPE_SET:
//		return HI_NET_DEV_CTRL_PTZ_SET_PRESET;
//	case EASY_PRESET_CMD_TYPE_REMOVE:
//		return HI_NET_DEV_CTRL_PTZ_CLE_PRESET;
//	default:
//		return 0;
//	}
//}
