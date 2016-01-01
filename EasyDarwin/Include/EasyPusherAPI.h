/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef __EasyPusher_H__
#define __EasyPusher_H__

#include "EasyTypes.h"

typedef struct __EASY_AV_Frame
{
    Easy_U32    u32AVFrameFlag;		/* 帧标志  视频 or 音频 */
    Easy_U32    u32AVFrameLen;		/* 帧的长度 */
    Easy_U32    u32VFrameType;		/* 视频的类型，I帧或P帧 */
    Easy_U8     *pBuffer;			/* 数据 */
	Easy_U32	u32TimestampSec;	/* 时间戳(秒)*/
	Easy_U32	u32TimestampUsec;	/* 时间戳(微秒) */
}EASY_AV_Frame;

/* 推送事件类型定义 */
typedef enum __EASY_PUSH_STATE_T
{
    EASY_PUSH_STATE_CONNECTING   =   1,     /* 连接中 */
    EASY_PUSH_STATE_CONNECTED,              /* 连接成功 */
    EASY_PUSH_STATE_CONNECT_FAILED,         /* 连接失败 */
    EASY_PUSH_STATE_CONNECT_ABORT,          /* 连接异常中断 */
    EASY_PUSH_STATE_PUSHING,                /* 推流中 */
    EASY_PUSH_STATE_DISCONNECTED,           /* 断开连接 */
    EASY_PUSH_STATE_ERROR
}EASY_PUSH_STATE_T;

/* 推送回调函数定义 _userptr表示用户自定义数据 */
typedef int (*EasyPusher_Callback)(int _id, EASY_PUSH_STATE_T _state, EASY_AV_Frame *_frame, void *_userptr);

#ifdef __cplusplus
extern "C"
{
#endif

	/* 创建推送句柄  返回为句柄值 */
	Easy_API Easy_Pusher_Handle Easy_APICALL EasyPusher_Create();
	
	/* 释放推送句柄 */
	Easy_API Easy_U32 Easy_APICALL EasyPusher_Release(Easy_Pusher_Handle handle);

    /* 设置流传输事件回调 userptr传输自定义对象指针*/
    Easy_API Easy_U32 Easy_APICALL EasyPusher_SetEventCallback(Easy_Pusher_Handle handle,  EasyPusher_Callback callback, int id, void *userptr);

	/* 开始流传输 serverAddr:流媒体服务器地址、port:流媒体端口、streamName:流名称<xxx.sdp>、username/password:推送携带的用户名密码、pstruStreamInfo:推送的媒体定义、bufferKSize:以k为单位的缓冲区大小<512~2048之间,默认512> bool createlogfile:创建日志文件*/
	Easy_API Easy_U32 Easy_APICALL EasyPusher_StartStream(Easy_Pusher_Handle handle, char* serverAddr, Easy_U16 port, char* streamName, char *username, char *password, EASY_MEDIA_INFO_T*  pstruStreamInfo, Easy_U32 bufferKSize, Easy_Bool createlogfile );

	/* 停止流传输 */
	Easy_API Easy_U32 Easy_APICALL EasyPusher_StopStream(Easy_Pusher_Handle handle);

	/* 推流 frame:具体推送的流媒体帧 */
	Easy_API Easy_U32 Easy_APICALL EasyPusher_PushFrame(Easy_Pusher_Handle handle, EASY_AV_Frame* frame );

#ifdef __cplusplus
}
#endif

#endif
