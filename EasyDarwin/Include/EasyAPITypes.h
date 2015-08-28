/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _Easy_API_Types_H
#define _Easy_API_Types_H

#ifdef _WIN32
#define Easy_API  __declspec(dllexport)
#define Easy_APICALL  __stdcall
#define WIN32_LEAN_AND_MEAN
#else
#define Easy_API
#define Easy_APICALL 
#endif

// Handle类型
#define Easy_RTSP_Handle void*
#define Easy_Pusher_Handle void*

typedef unsigned char           Easy_U8;
typedef unsigned char           Easy_UChar;
typedef unsigned short          Easy_U16;
typedef unsigned int            Easy_U32;

enum
{
    Easy_NoErr						= 0,
    Easy_RequestFailed				= -1,
    Easy_Unimplemented				= -2,
    Easy_RequestArrived				= -3,
    Easy_OutOfState					= -4,
    Easy_NotAModule					= -5,
    Easy_WrongVersion				= -6,
    Easy_IllegalService				= -7,
    Easy_BadIndex					= -8,
    Easy_ValueNotFound				= -9,
    Easy_BadArgument				= -10,
    Easy_ReadOnly					= -11,
	Easy_NotPreemptiveSafe			= -12,
    Easy_NotEnoughSpace				= -13,
    Easy_WouldBlock					= -14,
    Easy_NotConnected				= -15,
    Easy_FileNotFound				= -16,
    Easy_NoMoreData					= -17,
    Easy_AttrDoesntExist			= -18,
    Easy_AttrNameExists				= -19,
    Easy_InstanceAttrsNotAllowed	= -20,
	Easy_InvalidSocket				= -21,
	Easy_MallocError				= -22,
	Easy_ConnectError				= -23,
	Easy_SendError					= -24
};
typedef int Easy_Error;

/* 音视频编码 */
#define EASY_SDK_VIDEO_CODEC_H264	0x01000001		/* H264  */
#define	EASY_SDK_VIDEO_CODEC_MJPEG	0x01000002		/* MJPEG */
#define	EASY_SDK_VIDEO_CODEC_MPEG4	0x01000004		/* MPEG4 */

#define EASY_SDK_AUDIO_CODEC_AAC	0x01000011		/* AAC */
#define EASY_SDK_AUDIO_CODEC_G711A	0x01000012		/* G711 alaw*/
#define EASY_SDK_AUDIO_CODEC_G711U	0x01000014		/* G711 ulaw*/
#define EASY_SDK_AUDIO_CODEC_G726	0x01000018		/* G726 */

/* 音视频帧标识 */
#define EASY_SDK_VIDEO_FRAME_FLAG	0x02000001		/* 视频帧标志 */
#define EASY_SDK_AUDIO_FRAME_FLAG	0x02000002		/* 音频帧标志 */
#define EASY_SDK_EVENT_FRAME_FLAG	0x02000004		/* 事件帧标志 */
#define EASY_SDK_RTP_FRAME_FLAG		0x02000008		/* RTP帧标志 */
#define EASY_SDK_SDP_FRAME_FLAG		0x02000010		/* SDP帧标志 */

/* 视频关键字标识 */
#define EASY_SDK_VIDEO_FRAME_I		0x03000001		/* I帧 */
#define EASY_SDK_VIDEO_FRAME_P		0x03000002		/* P帧 */
#define EASY_SDK_VIDEO_FRAME_B		0x03000004		/* B帧 */

/* 连接类型 */
typedef enum __RTP_CONNECT_TYPE
{
	RTP_OVER_TCP	=	0x01,
	RTP_OVER_UDP
}RTP_CONNECT_TYPE;

/* 帧信息 */
typedef struct 
{
	unsigned int	codec;			//编码格式
	unsigned char	type;			//帧类型
	unsigned char	fps;			//帧率
	unsigned int	reserved1;
	unsigned int	reserved2;

	unsigned short	width;			//宽
	unsigned short  height;			//高
	unsigned int	sample_rate;	//采样率
	unsigned int	channels;		//声道
	unsigned int	length;			//帧大小
	unsigned int    timestamp_usec;	//微妙
	unsigned int	timestamp_sec;	//秒
	
	float			bitrate;
	float			losspacket;
}RTSP_FRAME_INFO;

#endif
