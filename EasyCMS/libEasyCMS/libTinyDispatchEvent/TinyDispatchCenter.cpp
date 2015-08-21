/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include "dbg.h"
#include "TinyDispatchCenter.h"

#ifndef _WIN32
#define Sleep(x) usleep(x*1000)
#endif

namespace libTinyDispatchCenter { 
CTinyDispatchCenter *CTinyDispatchCenter::m_pInstance = NULL;

CTinyDispatchCenter::CTinyDispatchCenter(void)
{
	m_nCountOfCenterThread = 0;
	m_dispatchMsgFunc = NULL;
	m_bContinue = false;
	m_nDispatchThreadNum = 0;
	m_nMaxWaittingMsg = 0;
	m_nMsgCacheNum = 0;
}

CTinyDispatchCenter::~CTinyDispatchCenter(void)
{
	ReleaseMsgBuffer();
}

void CTinyDispatchCenter::InitDispatchCenter(int nDispatchThreadNum, int nMaxWaittingMsg, int nMsgCacheNum, int nMsgBuffLen)
{
	m_nDispatchThreadNum = nDispatchThreadNum;
	m_nMaxWaittingMsg = nMaxWaittingMsg;	
	m_nMsgCacheNum = nMsgCacheNum;

	m_msgBuffer = new TinySysMsg_T[m_nMsgCacheNum];
	if (m_msgBuffer==NULL)
	{
		return;
	}

	for (int i=0; i<nMsgCacheNum; ++i)
	{
		m_msgBuffer[i].InitMsg(nMsgBuffLen);
	}

	m_nMsgBufferIndex = 0;
	m_nMsgBufferUsed  = 0;
}

void CTinyDispatchCenter::ReleaseMsgBuffer()
{
	if (m_msgBuffer != NULL)
	{
		delete m_msgBuffer;
		m_msgBuffer = NULL;
	}	
	m_nMsgBufferUsed  = 0;
	m_nMsgBufferIndex = 0;
}

TinySysMsg_T *CTinyDispatchCenter::GetMsgBuffer()
{
	TinySysMsg_T *pMsg = NULL;
	TinySysMsg_T *pMsgRes = NULL;
	int nLoopCnt = 0;

	do{
		AUTIO_GUARD_MUTEX(m_msgListMutex);
		m_nMsgBufferIndex = ((m_nMsgBufferIndex>m_nMsgCacheNum-1) || (m_nMsgBufferIndex<0))?0:m_nMsgBufferIndex;
		pMsg = m_msgBuffer+m_nMsgBufferIndex;
		m_nMsgBufferIndex++;

		if (pMsg->bUsing_ == false)
		{
			pMsg->InitMsg(pMsg->nBuffLen_);
			pMsg->bUsing_ = true;
			pMsgRes = pMsg;
			break;
		}
		else
		{
			pMsgRes=NULL;
			Sleep(1);
		}
	}while((pMsg->bUsing_ == true) && (nLoopCnt++<=m_nMsgCacheNum));

	return pMsgRes;
}

int CTinyDispatchCenter::PutMsg(TinySysMsg_T *pMsg)
{
	if (pMsg == NULL || (unsigned long)pMsg<(unsigned long)m_msgBuffer ||
		(unsigned long)pMsg>(unsigned long)(m_msgBuffer+m_nMsgCacheNum-1))
	{
		printlog("PutMsg :: Invalid point 0X%X of TSysMsg .\n", pMsg);
		return -1;
	}

	int nListSize = 0;

	{
		AUTIO_GUARD_MUTEX(m_msgListMutex);
		nListSize = m_msgList.size();

		if (nListSize >= m_nMaxWaittingMsg)
		{
			printlog("PutMsg :: Out of Msg-list .\n");
			list<TinySysMsg_T*>::iterator it;
			for (it = m_msgList.begin(); it != m_msgList.end(); ++it)
			{
				(*it)->bUsing_ = false;
			}
			m_msgList.clear();
		}

		m_msgList.push_back(pMsg);
		nListSize = m_msgList.size();

		sem_post(&m_msgListSemaphore);
	}
	
	if (nListSize >= m_nDispatchThreadNum-2)
	{
		printlog("TRACK-DISPATCH => MSG-ADD-UP %d || THREAD-BUSY %d .\n",
					nListSize, GetCountOfCenterThread());
	}
	else
	{
	}
	
	return 0;
}

int CTinyDispatchCenter::GetCountOfCenterThread()
{
	AUTIO_GUARD_MUTEX(m_mtxCountOfCenterThread);
	int nRet = m_nCountOfCenterThread;
	return nRet;
}

TinySysMsg_T *CTinyDispatchCenter::QueryMsgFromList()
{
	TinySysMsg_T *pMsg = NULL;
	{
		sem_wait(&m_msgListSemaphore);
		AUTIO_GUARD_MUTEX( m_msgListMutex);
		if (m_msgList.size()>0)
		{
			pMsg = m_msgList.front();
			if ((unsigned long) pMsg < (unsigned long) m_msgBuffer || 
				(unsigned long) pMsg > (unsigned long) (m_msgBuffer+m_nMsgCacheNum-1))
			{
				printlog("QueryMsgFromList() => Invalid msg-ptr(%lu). \n ", (unsigned long)pMsg);
				pMsg = NULL;
			}
			m_msgList.pop_front();
		}
	}
	
	return pMsg;
}

int CTinyDispatchCenter::open()
{
#ifdef _WIN32
	m_msgListSemaphore = sem_init(&m_msgListSemaphore, NULL, 0);
#else
    sem_init(&m_msgListSemaphore, NULL, 0);
#endif
	m_bContinue = true;

	for (int i = 0; i< m_nDispatchThreadNum; ++i)
	{
#ifdef _WIN32
		if(::_beginthread(DispatchMsgThread, 0, (void*)this)<0)
#else
        pthread_t thread;
        if(pthread_create(&thread, NULL, DispatchMsgThread, this) < 0)
#endif
		{
			printlog("TRACK-OPEN => Fail to create thread .\n");
		}
		else
		{
		}
	}

	return 0;
}

CTinyDispatchCenter *CTinyDispatchCenter::Instance()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new CTinyDispatchCenter;
	}
	return m_pInstance;
}

#ifdef _WIN32
void CTinyDispatchCenter::DispatchMsgThread(void *argv)
#else
void* CTinyDispatchCenter::DispatchMsgThread(void *argv)
#endif
{
	CTinyDispatchCenter *pTask = (CTinyDispatchCenter*) argv;
	if (pTask == NULL)
	{
		goto RET;
	}

	TinySysMsg_T *sysMsg ;
	//printlog("DispatchMsgThread start .............\n");
	while (pTask->ContinueThread() == true)
	{
		sysMsg = pTask->QueryMsgFromList();

		if (sysMsg != NULL)
		{
			printlog("********handle msg(0X%X) start!\n", sysMsg->msgHead_.nMsgType_);
			pTask->IncrCenterThreadCount();
			try
			{
				pTask->DispatchMessage(sysMsg);
			}
			catch(...)
			{
				printlog("********handle msg(0X%X) exception!\n", sysMsg->msgHead_.nMsgType_);
			}	
			printlog("********handle msg(0X%X) end!\n", sysMsg->msgHead_.nMsgType_);
			sysMsg->bUsing_ = false;			
			pTask->DecrCenterThreadCount();
		}
	}
RET:
#ifdef _WIN32
	return ;
#else
    return NULL;
#endif
}

void CTinyDispatchCenter::IncrCenterThreadCount()
{
	AUTIO_GUARD_MUTEX(m_mtxCountOfCenterThread);
	m_nCountOfCenterThread = m_nCountOfCenterThread<0?0:m_nCountOfCenterThread+1;
}

void CTinyDispatchCenter::DecrCenterThreadCount()
{
	AUTIO_GUARD_MUTEX( m_mtxCountOfCenterThread);
	m_nCountOfCenterThread = m_nCountOfCenterThread<0?0:m_nCountOfCenterThread-1;
}

bool CTinyDispatchCenter::ContinueThread()
{
	return m_bContinue;
}

int CTinyDispatchCenter::DispatchMessage(TinySysMsg_T *pMsg)
{
	if (pMsg == NULL)
	{
		return -1;
	}

	if (m_dispatchMsgFunc == NULL)
	{
		printlog("Pointer of Dispatch-Function is invalid .\n");
		return -1;
	}

	return m_dispatchMsgFunc(m_dispatchMsgFuncParam, pMsg);

}
}