/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#define DLL_IMPLEMENT

#include "TinyDispatchCenter.h"
#include "libTinyDispatchCenter.h"
#include <string.h>
namespace libTinyDispatchCenter { 
TinySysMsg_T::TinySysMsg_T()
{
	msgHead_.nMsgType_ = 0;
	bUsing_ = false;
	nBuffLen_ = 0;
	msgBody_ = NULL;
};

void TinySysMsg_T::InitMsg(int nBuffLen)
{
	msgHead_.nMsgType_ = 0;
	bUsing_ = false;
	nBuffLen_ = nBuffLen;
	if (msgBody_==NULL)
	{
		msgBody_ = new char[nBuffLen_];
	}
	memset(msgBody_, 0, nBuffLen_);
}

TinySysMsg_T::~TinySysMsg_T()
{
	if (msgBody_!=NULL)
	{
		delete msgBody_;
	}
}

HDISPATCHTASK libDispatch_CreateInstance(int nDispatchThreadNum,
	int nMaxWaittingMsg, 
	int nMsgCacheNum, 
	int nMsgBuffLen,
	DispatchMsgThread_T func, unsigned long long ulParam)
{
	CTinyDispatchCenter *pInstance = new CTinyDispatchCenter();
	pInstance->InitDispatchCenter(nDispatchThreadNum, nMaxWaittingMsg,
		nMsgCacheNum, nMsgBuffLen);
	pInstance->SetDispatchFunc(func, ulParam);
	return pInstance;
}

int libDispatch_Start(HDISPATCHTASK handle)
{
	CTinyDispatchCenter *task = (CTinyDispatchCenter*)handle;
	task->open();
	return 0;
}

TinySysMsg_T* libDispatch_GetMsgBuffer(HDISPATCHTASK handle)
{
	CTinyDispatchCenter *task = (CTinyDispatchCenter*)handle;
	return task->GetMsgBuffer();
}

int libDispatch_SendMsg(HDISPATCHTASK handle, TinySysMsg_T *pMsg)
{
	CTinyDispatchCenter *task = (CTinyDispatchCenter*)handle;
	return task->PutMsg(pMsg);
}

int libDispatch_SendEmptyMsg(HDISPATCHTASK handle, int nMsgType)
{
	CTinyDispatchCenter *task = (CTinyDispatchCenter*)handle;
	TinySysMsg_T *pMsg = task->GetMsgBuffer();
	pMsg->msgHead_.nMsgType_ = nMsgType;
	return task->PutMsg(pMsg);
}

}