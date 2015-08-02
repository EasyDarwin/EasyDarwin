/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _Easy_CMS_API_H
#define _Easy_CMS_API_H
#include "libClientCommondef.h"

#ifdef _WIN32
#define EasyCMS_API  __declspec(dllexport)
#define Easy_APICALL  __stdcall
#else
#define EasyCMS_API
#define Easy_APICALL 
#endif

namespace EasyDarwin { namespace libEasyCMS
{

enum EASYDARWIN_EVENT_TYPE
{
	EASYDARWIN_EVENT_LOGIN,
	EASYDARWIN_EVENT_OFFLINE
};

typedef void (*fEventCallBack)(EASYDARWIN_EVENT_TYPE eEvent, const char* pEventData, unsigned int iDataLen, void* pUserData);

class EasyCMS_API EasyDarwinCMSAPI
{
public:
	EasyDarwinCMSAPI();
	~EasyDarwinCMSAPI(void);

private:
	void *m_callCenter;

public:
	//设置事件回调
	void SetEventCallBack(fEventCallBack fCallBack, void *pUserData);
	//登录
	Easy_Error  Login(const char *szHost, int nPort, const char *szAccess, const char *szPassword);
	//快照上传
	Easy_Error	UpdateSnap(const char* snapData, unsigned int snapLen);
};

}
}
#endif
