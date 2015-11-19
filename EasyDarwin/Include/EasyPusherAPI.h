/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef __EasyPusher_H__
#define __EasyPusher_H__

#include "EasyTypes.h"

typedef struct __EASY_AV_Frame
{
    Easy_U32    u32AVFrameFlag;		/* ֡��־  ��Ƶ or ��Ƶ */
    Easy_U32    u32AVFrameLen;		/* ֡�ĳ��� */
    Easy_U32    u32VFrameType;		/* ��Ƶ�����ͣ�I֡��P֡ */
    Easy_U8     *pBuffer;			/* ���� */
	Easy_U32	u32TimestampSec;	/* ʱ���(��)*/
	Easy_U32	u32TimestampUsec;	/* ʱ���(΢��) */
}EASY_AV_Frame;

/* �����¼����Ͷ��� */
typedef enum __EASY_PUSH_STATE_T
{
    EASY_PUSH_STATE_CONNECTING   =   1,     /* ������ */
    EASY_PUSH_STATE_CONNECTED,              /* ���ӳɹ� */
    EASY_PUSH_STATE_CONNECT_FAILED,         /* ����ʧ�� */
    EASY_PUSH_STATE_CONNECT_ABORT,          /* �����쳣�ж� */
    EASY_PUSH_STATE_PUSHING,                /* ������ */
    EASY_PUSH_STATE_DISCONNECTED,           /* �Ͽ����� */
    EASY_PUSH_STATE_ERROR
}EASY_PUSH_STATE_T;

/* ���ͻص��������� _userptr��ʾ�û��Զ������� */
typedef int (*EasyPusher_Callback)(int _id, EASY_PUSH_STATE_T _state, EASY_AV_Frame *_frame, void *_userptr);

#ifdef __cplusplus
extern "C"
{
#endif

	/* �������;��  ����Ϊ���ֵ */
	Easy_API Easy_Pusher_Handle Easy_APICALL EasyPusher_Create();
	
	/* �ͷ����;�� */
	Easy_API Easy_U32 Easy_APICALL EasyPusher_Release(Easy_Pusher_Handle handle);

    /* �����������¼��ص� userptr�����Զ������ָ��*/
    Easy_API Easy_U32 Easy_APICALL EasyPusher_SetEventCallback(Easy_Pusher_Handle handle,  EasyPusher_Callback callback, int id, void *userptr);

	/* ��ʼ������ serverAddr:��ý���������ַ��port:��ý��˿ڡ�streamName:������<xxx.sdp>��username/password:����Я�����û������롢pstruStreamInfo:���͵�ý�嶨�塢bufferKSize:��kΪ��λ�Ļ�������С<512~2048֮��,Ĭ��512> bool createlogfile:������־�ļ�*/
	Easy_API Easy_U32 Easy_APICALL EasyPusher_StartStream(Easy_Pusher_Handle handle, char* serverAddr, Easy_U16 port, char* streamName, char *username, char *password, EASY_MEDIA_INFO_T*  pstruStreamInfo, Easy_U32 bufferKSize, Easy_Bool createlogfile );

	/* ֹͣ������ */
	Easy_API Easy_U32 Easy_APICALL EasyPusher_StopStream(Easy_Pusher_Handle handle);

	/* ���� frame:�������͵���ý��֡ */
	Easy_API Easy_U32 Easy_APICALL EasyPusher_PushFrame(Easy_Pusher_Handle handle, EASY_AV_Frame* frame );

#ifdef __cplusplus
}
#endif

#endif
