/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _Easy_RTSPClient_API_H
#define _Easy_RTSPClient_API_H

#include "EasyAPITypes.h"

/*
	_mediatype:		EASY_SDK_VIDEO_FRAME_FLAG	EASY_SDK_AUDIO_FRAME_FLAG	EASY_SDK_EVENT_FRAME_FLAG	
	如果在EasyRTSP_OpenStream中的参数outRtpPacket置为1, 则回调中的_mediatype为EASY_SDK_RTP_FRAME_FLAG, pbuf为接收到的RTP包(包含rtp头信息), frameinfo->length为包长
*/
typedef int (Easy_APICALL *RTSPSourceCallBack)( int _chid, int *_chPtr, int _mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo);

#ifdef __cplusplus
extern "C"
{
#endif
	/* 获取最后一次错误的错误码 */
	Easy_API int Easy_APICALL EasyRTSP_GetErrCode();

	/* 创建RTSPClient句柄  返回为句柄值 */
	Easy_API int Easy_APICALL EasyRTSP_Init(Easy_RTSP_Handle *handle);

	/* 释放RTSPClient 参数为RTSPClient句柄 */
	Easy_API int Easy_APICALL EasyRTSP_Deinit(Easy_RTSP_Handle *handle);

	/* 设置数据回调 */
	Easy_API int Easy_APICALL EasyRTSP_SetCallback(Easy_RTSP_Handle handle, RTSPSourceCallBack _callback);

	/* 打开网络流 */
	Easy_API int Easy_APICALL EasyRTSP_OpenStream(Easy_RTSP_Handle handle, int _channelid, char *_url, RTP_CONNECT_TYPE _connType, unsigned int _mediaType, char *_username, char *_password, void *userPtr, int _reconn/*1000表示长连接,即如果网络断开自动重连, 其它值为连接次数*/, int outRtpPacket/*默认为0,即回调输出完整的帧, 如果为1,则输出RTP包*/);
	
	/* 关闭网络流 */
	Easy_API int Easy_APICALL EasyRTSP_CloseStream(Easy_RTSP_Handle handle);

#ifdef __cplusplus
}
#endif

#endif
