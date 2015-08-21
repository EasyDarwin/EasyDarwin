/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include "EasyCMSAPI.h"
#include "EasyDeviceCenter.h"

using namespace EasyDarwin::libEasyCMS;

#define CEasyDarwinDevicePtr_CAST(__ptr__) ((EasyDeviceCenter*)(__ptr__))
#define EasyDarwinDeviceStreamPublishPtr_CAST(__ptr__) ((EasyMediaSession*)(__ptr__))

/* 创建CMS Session  返回为句柄值 */
EasyCMS_API Easy_CMS_Handle Easy_APICALL EasyCMS_Session_Create()
{
	return new EasyDeviceCenter();
}

/* 释放CMS Session句柄 */
EasyCMS_API Easy_U32 Easy_APICALL EasyCMS_Session_Release(Easy_CMS_Handle handle)
{
	if(NULL == handle) return 0;
	EasyDeviceCenter *center = (EasyDeviceCenter*)handle;
	delete center;
	center = NULL;
	return 0;
}

/* 设置流传输事件回调 userptr传输自定义对象指针*/
EasyCMS_API Easy_U32 Easy_APICALL EasyCMS_SetEventCallback(Easy_CMS_Handle handle,  EasyCMS_Callback callback, void *userData)
{
	if(NULL == handle) return 0;
	EasyDeviceCenter *center = (EasyDeviceCenter*)handle;
	return 0;
}

/* 登录到CMS，携带具体的用户名密码 */
EasyCMS_API Easy_U32 Easy_APICALL EasyCMS_Login(Easy_CMS_Handle handle, char* serverAddr, Easy_U16 port, char *username, char *password)
{
	if(NULL == handle) return 0;
	EasyDeviceCenter *center = (EasyDeviceCenter*)handle;
	return CEasyDarwinDevicePtr_CAST(center)->Login(serverAddr, port, username, password);
}

/* 上传快照 */
EasyCMS_API Easy_U32 Easy_APICALL EasyCMS_UpdateSnap(Easy_CMS_Handle handle, const char* snapData, unsigned int snapLen)
{
	if(NULL == handle) return 0;
	EasyDeviceCenter *center = (EasyDeviceCenter*)handle;
	return CEasyDarwinDevicePtr_CAST(center)->UpdateDeviceSnap(snapData, snapLen);
}