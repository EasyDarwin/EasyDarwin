/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include "EasyCameraSource.h"
#include "QTSServerInterface.h"
#include "EasyProtocolDef.h"

static unsigned int sLastVPTS = 0;
static unsigned int sLastAPTS = 0;

// Camera IP
static char*            sCamera_IP				= NULL;
static char*            sDefaultCamera_IP_Addr	= "127.0.0.1";
// Camera Port
static UInt16			sCameraPort				= 80;
static UInt16			sDefaultCameraPort		= 80;
// Camera user
static char*            sCameraUser				= NULL;
static char*            sDefaultCameraUser		= "admin";
// Camera password
static char*            sCameraPassword			= NULL;
static char*            sDefaultCameraPassword	= "admin";
// Camera stream subtype
static UInt32			sStreamType				= 1;
static UInt32			sDefaultStreamType		= 1;


// 初始化读取配置文件中各项配置
void EasyCameraSource::Initialize(QTSS_ModulePrefsObject modulePrefs)
{
	delete [] sCamera_IP;
    sCamera_IP = QTSSModuleUtils::GetStringAttribute(modulePrefs, "camera_ip", sDefaultCamera_IP_Addr);

	QTSSModuleUtils::GetAttribute(modulePrefs, "camera_port", qtssAttrDataTypeUInt16, &sCameraPort, &sDefaultCameraPort, sizeof(sCameraPort));

	delete [] sCameraUser;
    sCameraUser = QTSSModuleUtils::GetStringAttribute(modulePrefs, "camera_user", sDefaultCameraUser);
	
	delete [] sCameraPassword;
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

//摄像机音视频数据回调
HI_S32 NETSDK_APICALL OnStreamCallback(	HI_U32 u32Handle,		/* 句柄 */
										HI_U32 u32DataType,     /* 数据类型，视频或音频数据或音视频复合数据 */
										HI_U8*  pu8Buffer,      /* 数据包含帧头 */
										HI_U32 u32Length,		/* 数据长度 */
										HI_VOID* pUserData		/* 用户数据*/
									)						
{
	HI_S_AVFrame* pstruAV = HI_NULL;
	HI_S_SysHeader* pstruSys = HI_NULL;
	EasyCameraSource* pThis = (EasyCameraSource*)pUserData;

	if (u32DataType == HI_NET_DEV_AV_DATA)
	{
		pstruAV = (HI_S_AVFrame*)pu8Buffer;

		if (pstruAV->u32AVFrameFlag == HI_NET_DEV_VIDEO_FRAME_FLAG)
		{
			//强制要求第一帧为I关键帧
			if(pThis->m_bForceIFrame)
			{
				if(pstruAV->u32VFrameType == HI_NET_DEV_VIDEO_FRAME_I)
					pThis->m_bForceIFrame = false;
				else
					return HI_SUCCESS;
			}

			unsigned int vInter = pstruAV->u32AVFramePTS - sLastVPTS;

			sLastVPTS = pstruAV->u32AVFramePTS;
			pThis->PushFrame((unsigned char*)pu8Buffer, u32Length);
		}
		else if (pstruAV->u32AVFrameFlag == HI_NET_DEV_AUDIO_FRAME_FLAG)
		{
			pThis->PushFrame((unsigned char*)pu8Buffer, u32Length);
		}
	}	

	return HI_SUCCESS;
}

	
HI_S32 NETSDK_APICALL OnEventCallback(	HI_U32 u32Handle,	/* 句柄 */
										HI_U32 u32Event,	/* 事件 */
										HI_VOID* pUserData  /* 用户数据*/
                                )
{
	return HI_SUCCESS;
}

HI_S32 NETSDK_APICALL OnDataCallback(	HI_U32 u32Handle,		/* 句柄 */
										HI_U32 u32DataType,		/* 数据类型*/
										HI_U8*  pu8Buffer,      /* 数据 */
										HI_U32 u32Length,		/* 数据长度 */
										HI_VOID* pUserData		/* 用户数据*/
                                )
{
	return HI_SUCCESS;
}


EasyCameraSource::EasyCameraSource()
:	Task(),
	m_u32Handle(0),
	fCameraLogin(false),
	m_bStreamFlag(false),
	m_bForceIFrame(true),
	fTimeoutTask(this, 30 * 1000),
	fPusherHandle(NULL)
{
	this->SetTaskName("EasyCameraSource");

	//SDK初始化，全局调用一次
	HI_NET_DEV_Init();

	fTimeoutTask.RefreshTimeout();
}

EasyCameraSource::~EasyCameraSource(void)
{
	//先停止Stream，内部有是否在Stream的判断
	NetDevStopStream();

	if(fCameraLogin)
		HI_NET_DEV_Logout(m_u32Handle);

	//SDK释放，全局调用一次
	HI_NET_DEV_DeInit();
}

bool EasyCameraSource::CameraLogin()
{
	//如果已登录，返回true
	if(fCameraLogin) return true;
	//登录到摄像机
	HI_S32 s32Ret = HI_SUCCESS;
	s32Ret = HI_NET_DEV_Login(	&m_u32Handle, sCameraUser, sCameraPassword, sCamera_IP, sCameraPort);

	if (s32Ret != HI_SUCCESS)
	{
		qtss_printf("HI_NET_DEV_Login Fail\n");
		m_u32Handle = 0;
		return false;
	}
	else
	{
		HI_NET_DEV_SetReconnect(m_u32Handle, 5000);
		fCameraLogin = true;
	}

	return true;
}

QTSS_Error EasyCameraSource::NetDevStartStream()
{
	//如果未登录,返回失败
	if(!CameraLogin()) return QTSS_RequestFailed;
	
	//已经在流传输中，返回Easy_AttrNameExists
	if(m_bStreamFlag) return QTSS_AttrNameExists;

	OSMutexLocker locker(this->GetMutex());

	QTSS_Error theErr = QTSS_NoErr;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_S_STREAM_INFO struStreamInfo;

	HI_NET_DEV_SetEventCallBack(m_u32Handle, (HI_ON_EVENT_CALLBACK)OnEventCallback, this);
	HI_NET_DEV_SetStreamCallBack(m_u32Handle, (HI_ON_STREAM_CALLBACK)OnStreamCallback, this);
	HI_NET_DEV_SetDataCallBack(m_u32Handle, (HI_ON_DATA_CALLBACK)OnDataCallback, this);

	struStreamInfo.u32Channel = HI_NET_DEV_CHANNEL_1;
	struStreamInfo.blFlag = sStreamType?HI_TRUE:HI_FALSE;
	struStreamInfo.u32Mode = HI_NET_DEV_STREAM_MODE_TCP;
	struStreamInfo.u8Type = HI_NET_DEV_STREAM_ALL;
	s32Ret = HI_NET_DEV_StartStream(m_u32Handle, &struStreamInfo);
	if (s32Ret != HI_SUCCESS)
	{
		qtss_printf("HI_NET_DEV_StartStream Fail\n");
		return QTSS_RequestFailed;
	}

	m_bStreamFlag = true;
	m_bForceIFrame = true;
	qtss_printf("HI_NET_DEV_StartStream SUCCESS\n");

	return QTSS_NoErr;
}

void EasyCameraSource::NetDevStopStream()
{
	if( m_bStreamFlag )
	{
		qtss_printf("HI_NET_DEV_StopStream\n");
		HI_NET_DEV_StopStream(m_u32Handle);
		m_bStreamFlag = false;
	}
}

void EasyCameraSource::handleClosure(void* clientData) 
{
	if(clientData != NULL)
	{
		EasyCameraSource* source = (EasyCameraSource*)clientData;
		source->handleClosure();
	}
}

void EasyCameraSource::handleClosure() 
{
	if (fOnCloseFunc != NULL) 
	{
		(*fOnCloseFunc)(fOnCloseClientData);
	}
}

void EasyCameraSource::stopGettingFrames() 
{
	OSMutexLocker locker(this->GetMutex());
	fOnCloseFunc = NULL;
	doStopGettingFrames();
}

void EasyCameraSource::doStopGettingFrames() 
{
	qtss_printf("doStopGettingFrames()\n");
	NetDevStopStream();
}

bool EasyCameraSource::GetSnapData(unsigned char* pBuf, UInt32 uBufLen, int* uSnapLen)
{
	//如果摄像机未登录，返回false
	if(!CameraLogin()) return false;

	//调用SDK获取数据
	HI_S32 s32Ret = HI_FAILURE; 
	s32Ret = HI_NET_DEV_SnapJpeg(m_u32Handle, (HI_U8*)pBuf, uBufLen, uSnapLen);
	if(s32Ret == HI_SUCCESS)
	{
		return true;
	}

	return false;
}

SInt64 EasyCameraSource::Run()
{
	QTSS_Error theErr = QTSS_NoErr;
	EventFlags events = this->GetEvents();

	if(events & Task::kTimeoutEvent)
	{
		//向设备获取快照数据
		unsigned char *sData = (unsigned char*)malloc(EASY_SNAP_BUFFER_SIZE);
		int snapBufLen = 0;

		do{
			//如果获取到摄像机快照数据，Base64编码/发送
			if(!GetSnapData(sData, EASY_SNAP_BUFFER_SIZE, &snapBufLen))
			{
				//未获取到数据
				qtss_printf("EasyCameraSource::Run::GetSnapData() => Get Snap Data Fail \n");
				theErr = QTSS_ValueNotFound;
				break;
			}

			QTSS_RoleParams params;
			params.postSnapParams.snapLen = snapBufLen;
			params.postSnapParams.snapPtr = sData;
			params.postSnapParams.snapType = EASY_SNAP_TYPE_JPEG;
			UInt32 fCurrentModule = 0;
			UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kPostSnapRole);
			for (; fCurrentModule < numModules; fCurrentModule++)
			{
				qtss_printf("EasyCameraSource::Run::kPostSnapRole\n");
				QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kPostSnapRole, fCurrentModule);
				(void)theModule->CallDispatch(Easy_PostSnap_Role, &params);	
				break;
			}
			break;

		}while(0);

		free((void*)sData);
		sData = NULL;

		fTimeoutTask.RefreshTimeout();
	}

	return 0;
}


QTSS_Error EasyCameraSource::StartStreaming(Easy_StartStream_Params* inParams)
{
	if(NULL == fPusherHandle)
	{
		// 实际这里应该都是先通过Camera硬件的SDK接口获取到主/子码流具体流媒体参数信息动态设置的
		EASY_MEDIA_INFO_T	mediainfo;
		memset(&mediainfo, 0x00, sizeof(EASY_MEDIA_INFO_T));
		mediainfo.u32VideoCodec =   EASY_SDK_VIDEO_CODEC_H264;
		mediainfo.u32AudioCodec	=	EASY_SDK_AUDIO_CODEC_G711A;
		mediainfo.u32AudioSamplerate = 8000;
		mediainfo.u32AudioChannel = 1;
		mediainfo.u32VideoFps = 25;

		fPusherHandle = EasyPusher_Create();
		EasyPusher_SetEventCallback(fPusherHandle, __EasyPusher_Callback, 0, NULL);

		// 根据接收到的命令生成流信息
		char sdpName[64] = { 0 };
		sprintf(sdpName,"%s/%s/%s.sdp", inParams->inStreamID, inParams->inSerial, inParams->inChannel); 

		// 开始推送流媒体数据
		EasyPusher_StartStream(fPusherHandle, (char*)inParams->inIP, inParams->inPort, sdpName, "", "", &mediainfo, 1024/* 1M Buffer*/, 0);
	}

	NetDevStartStream();

	// 推送成功后，我们需要保留当前正在推送的参数信息
	{
		/*fStartStreamParams.startStreaParams.inSerial = inParams->inSerial;
		fStartStreamParams.startStreaParams.inChannel = inParams->inChannel;
		fStartStreamParams.startStreaParams.inStreamID = inParams->inStreamID;
		fStartStreamParams.startStreaParams.inProtocol = inParams->inProtocol;
		fStartStreamParams.startStreaParams.inIP = inParams->inIP;
		fStartStreamParams.startStreaParams.inPort = inParams->inPort;*/
		fStartStreamParams.startStreaParams = *inParams;
	}

	return QTSS_NoErr;
}

QTSS_Error EasyCameraSource::StopStreaming(Easy_StopStream_Params* inParams)
{
	if(fPusherHandle)
	{
		EasyPusher_StopStream(fPusherHandle);
		EasyPusher_Release(fPusherHandle);
		fPusherHandle = 0;
	}

	stopGettingFrames();

	return QTSS_NoErr;
}

QTSS_Error EasyCameraSource::PushFrame(unsigned char* frame, int len)
{	
	if(fPusherHandle == NULL) return QTSS_Unimplemented;

	HI_S_AVFrame* pstruAV = (HI_S_AVFrame*)frame;

	EASY_AV_Frame  avFrame;
	memset(&avFrame, 0x00, sizeof(EASY_AV_Frame));

	if (pstruAV->u32AVFrameFlag == HI_NET_DEV_VIDEO_FRAME_FLAG)
	{
		if(pstruAV->u32AVFrameLen > 0)
		{
			unsigned char* pbuf = (unsigned char*)frame+sizeof(HI_S_AVFrame);

			EASY_AV_Frame  avFrame;
			memset(&avFrame, 0x00, sizeof(EASY_AV_Frame));
			avFrame.u32AVFrameLen = pstruAV->u32AVFrameLen;
			avFrame.pBuffer = (unsigned char*)pbuf;
			avFrame.u32VFrameType = (pstruAV->u32VFrameType==HI_NET_DEV_VIDEO_FRAME_I)?EASY_SDK_VIDEO_FRAME_I:EASY_SDK_VIDEO_FRAME_P;
			avFrame.u32AVFrameFlag = EASY_SDK_VIDEO_FRAME_FLAG;
			avFrame.u32TimestampSec = pstruAV->u32AVFramePTS/1000;
			avFrame.u32TimestampUsec = (pstruAV->u32AVFramePTS%1000)*1000;
			EasyPusher_PushFrame(fPusherHandle, &avFrame);
		}	
	}
	else if (pstruAV->u32AVFrameFlag == HI_NET_DEV_AUDIO_FRAME_FLAG)
	{
		if(pstruAV->u32AVFrameLen > 0)
		{
			unsigned char* pbuf = (unsigned char*)frame+sizeof(HI_S_AVFrame);

			EASY_AV_Frame  avFrame;
			memset(&avFrame, 0x00, sizeof(EASY_AV_Frame));
			avFrame.u32AVFrameLen = pstruAV->u32AVFrameLen - 4;//去掉厂家自定义的4字节头
			avFrame.pBuffer = (unsigned char*)pbuf+4;
			avFrame.u32AVFrameFlag = EASY_SDK_AUDIO_FRAME_FLAG;
			avFrame.u32TimestampSec = pstruAV->u32AVFramePTS/1000;
			avFrame.u32TimestampUsec = (pstruAV->u32AVFramePTS%1000)*1000;
			EasyPusher_PushFrame(fPusherHandle, &avFrame);
		}			
	}
	return Easy_NoErr;
}