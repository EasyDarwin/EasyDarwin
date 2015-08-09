/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _Easy_NVS_API_H
#define _Easy_NVS_API_H

#define WIN32_LEAN_AND_MEAN


#ifdef _WIN32
#define EasyNVS_API  __declspec(dllexport)
#define Easy_APICALL  __stdcall
#include <winsock2.h>
#else
#define EasyNVS_API
#define Easy_APICALL 
#define CALLBACK
#endif

#define Easy_NVS_Handle void*

//媒体类型
#ifndef MEDIA_TYPE_VIDEO
#define MEDIA_TYPE_VIDEO		0x00000001
#endif
#ifndef MEDIA_TYPE_AUDIO
#define MEDIA_TYPE_AUDIO		0x00000002
#endif
#ifndef MEDIA_TYPE_EVENT
#define MEDIA_TYPE_EVENT		0x00000004
#endif
#ifndef MEDIA_TYPE_RTP
#define MEDIA_TYPE_RTP			0x00000008
#endif
#ifndef MEDIA_TYPE_SDP
#define MEDIA_TYPE_SDP			0x00000010
#endif

/* video codec */
#define	VIDEO_CODEC_H264	0x1C
#define	VIDEO_CODEC_MJPEG	0x08
#define	VIDEO_CODEC_MPEG4	0x0D
/* audio codec */
#define AUDIO_CODEC_MP4A	0x15002		//86018
#define AUDIO_CODEC_PCMU	0x10006		//65542


/* 连接类型 */
typedef enum __RTP_CONNECT_TYPE
{
	RTP_OVER_TCP	=	0x01,
	RTP_OVER_UDP
}RTP_CONNECT_TYPE;

/* 帧类型 */
#ifndef FRAMETYPE_I
#define FRAMETYPE_I		0x01
#endif
#ifndef FRAMETYPE_P
#define FRAMETYPE_P		0x02
#endif
#ifndef FRAMETYPE_B
#define FRAMETYPE_B		0x03
#endif

/* 帧信息 */
typedef struct 
{
	unsigned int	codec;			//编码格式
	unsigned char	type;			//帧类型
	unsigned char	fps;			//帧率
	unsigned char	reserved1;
	unsigned char	reserved2;

	unsigned short	width;			//宽
	unsigned short  height;			//高
	unsigned int	sample_rate;	//采样率
	unsigned int	channels;		//声道
	unsigned int	length;			//帧大小
	unsigned int    rtptimestamp;	//rtp timestamp
	unsigned int	timestamp_sec;	//秒
	
	float			bitrate;
	float			losspacket;
}NVS_FRAME_INFO;

/*
//回调:
_mediatype:		MEDIA_TYPE_VIDEO	MEDIA_TYPE_AUDIO	MEDIA_TYPE_EVENT	
如果在EasyNVS_OpenStream中的参数outRtpPacket置为1, 则回调中的_mediatype为MEDIA_TYPE_RTP, pbuf为接收到的RTP包(包含rtp头信息), frameinfo->length为包长
*/
typedef int (CALLBACK *NVSourceCallBack)( int _chid, int *_chPtr, int _mediatype, char *pbuf, NVS_FRAME_INFO *frameinfo);

extern "C"
{
	/* 获取最后一次错误的错误码 */
	EasyNVS_API int Easy_APICALL EasyNVS_GetErrCode();

	/* 创建NVSource句柄  返回为句柄值 */
	EasyNVS_API int Easy_APICALL EasyNVS_Init(Easy_NVS_Handle *handle);

	/* 释放NVSource 参数为NVSource句柄 */
	EasyNVS_API int Easy_APICALL EasyNVS_Deinit(Easy_NVS_Handle *handle);

	/* 设置数据回调 */
	EasyNVS_API int Easy_APICALL EasyNVS_SetCallback(Easy_NVS_Handle handle, NVSourceCallBack _callback);

	/* 打开网络流 */
	EasyNVS_API int Easy_APICALL EasyNVS_OpenStream(Easy_NVS_Handle handle, int _channelid, char *_url, RTP_CONNECT_TYPE _connType, unsigned int _mediaType, char *_username, char *_password, void *userPtr, int _reconn/*1000表示长连接,即如果网络断开自动重连, 其它值为连接次数*/, int outRtpPacket/*默认为0,即回调输出完整的帧, 如果为1,则输出RTP包*/);
	
	/* 关闭网络流 */
	EasyNVS_API int Easy_APICALL EasyNVS_CloseStream(Easy_NVS_Handle handle);
};

#endif
