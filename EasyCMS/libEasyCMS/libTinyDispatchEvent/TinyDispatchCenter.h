/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _TINY_DISPATCH_CENTER_H
#define _TINY_DISPATCH_CENTER_H

#include "libTinyDispatchCenter.h"

#include <list>
#include <map>
#include <vector>
#include "commonthread.h"
#include "MutexLock.h"
using namespace std;

namespace libTinyDispatchCenter { 

class CTinyDispatchCenter
{

public:
	CTinyDispatchCenter(void);
	~CTinyDispatchCenter(void);

	virtual int  open();
	void SetDispatchFunc(DispatchMsgThread_T pFunc, unsigned long long uParam)
	{
		m_dispatchMsgFunc      = pFunc;
		m_dispatchMsgFuncParam = uParam;
	}

	static void SetInstance(CTinyDispatchCenter *pInstance)
	{
		m_pInstance = pInstance;
	};

	static CTinyDispatchCenter *GetInstance()
	{
		if (m_pInstance == NULL)
		{
			m_pInstance = new CTinyDispatchCenter();
		}

		return m_pInstance;
	};

	void InitDispatchCenter(int nDispatchThreadNum, 
		int nMaxWaittingMsg,
		int nMsgCacheNum,
		int nMsgBuffLen);

	void ReleaseMsgBuffer();

	TinySysMsg_T *GetMsgBuffer();
	int PutMsg(TinySysMsg_T *pMsg);

	TinySysMsg_T *QueryMsgFromList();

	bool ContinueThread();

	virtual int DispatchMessage(TinySysMsg_T *pMsg);
	
	//center thread count.
	int  GetCountOfCenterThread();
	void IncrCenterThreadCount();
	void DecrCenterThreadCount();

public:
#ifdef _WIN32
    static void DispatchMsgThread(void *argv);
#else
    static void* DispatchMsgThread(void *argv);
#endif
	static CTinyDispatchCenter *Instance();

private:

	unsigned long       m_timerFuncParam;

	DispatchMsgThread_T m_dispatchMsgFunc;
	unsigned long long		m_dispatchMsgFuncParam;

	//message-cache-buffer
	TinySysMsg_T*			m_msgBuffer;
	int				    m_nMsgBufferIndex;
	int				    m_nMsgBufferUsed;

    ////////////////////////////////////////////////////////////////////////////////

	//message-handles
	list<TinySysMsg_T*>		m_msgList;
	CMutexLock              m_msgListMutex;
	sem_t		            m_msgListSemaphore;
	///////////////////////////////////////////////////////////////////////////////
	
	bool					m_bContinue;
	int						m_nCountOfCenterThread;
	CMutexLock				m_mtxCountOfCenterThread;

	int						m_nMsgCacheNum;

	int						m_nDispatchThreadNum;
	int						m_nMaxWaittingMsg;
private:
	static CTinyDispatchCenter   *m_pInstance;
};

typedef CTinyDispatchCenter CTINY_DISPAT_CHCENTER;
}
#endif
