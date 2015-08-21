/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include "EasyDeviceCenter.h"
#include "EasyProtocol.h"
#include "ClientSystemCommonDef.h"
#include "CommonDef.h"

using namespace EasyDarwin::Protocol;

namespace EasyDarwin { namespace libEasyCMS
{

EasyDeviceSession::EasyDeviceSession()
	:Task(),
	fSocket(NULL),
    m_httpClient(NULL),
    fClientTimeoutTask(this, 10*1000),
	fLiveSession(true),
	m_state(EasyDeviceOffline)
{
	//Client Socket
	fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
	//HTTPClient
	m_httpClient = new CHttpClient(fSocket);

	m_strCMSHost.empty();
	m_nCMSPort = 0;
}

EasyDeviceSession::~EasyDeviceSession()
{
	if (m_httpClient)
	{
		delete m_httpClient;
		m_httpClient = NULL;
	}
	if(fSocket)
	{
		delete fSocket;
		fSocket = NULL;
	}
}

void EasyDeviceSession::ResetClient()
{
	//AUTIO_GUARD_MUTEX(GetClientMutex());
	m_strSessionID.clear();
	m_iTerminalType = -1;
	m_iAppType = -1;
	m_iDeviceType = -1;
	m_strXMLVersion.clear();

	m_iAccessType = -1;
	//m_strAccess.clear();
	//m_strPassword.clear();

	//m_strCMSHost.clear();
	//m_nCMSPort = 0;

	//this->Disconnect();
	//fSocket->GetSocket()->Cleanup();
	this->m_httpClient->ResetRecvBuf();
	
	printf("reset client\n");
	m_state = EasyDeviceOffline;
}

void EasyDeviceSession::SetRecvDataCallback(HttpClientRecv_Callback func, void *vptr)
{
	m_recvCallback = func;
	m_recvCallbackParam = vptr;
}

SInt64 EasyDeviceSession::Run()
{
    EventFlags events = this->GetEvents();
    QTSS_Error err = QTSS_NoErr;
        
    //check for a timeout or a kill. If so, just consider the session dead
    if ((events & Task::kTimeoutEvent) || (events & Task::kKillEvent))
	{
		//fLiveSession = false;
		//return -1;
	}

	//有新消息
	if (events & Task::kReadEvent)
	{
		if (!fSocket->GetSocket()->IsConnected())
		{
			//连接断开，重置客户端
			ResetClient();
			if (m_recvCallback)
			{
				m_recvCallback(Easy_InvalidSocket, 0, m_recvCallbackParam);
			}
			return 0;
		}

		m_httpClient->ReceivePacket();

		if(m_httpClient->GetContentLength())
		{
			if (m_recvCallback)
			{
				m_recvCallback(0, m_httpClient->GetContentBody(), m_recvCallbackParam);
			}
			m_httpClient->ResetRecvBuf();
		}
		else
		{
			m_httpClient->ResetRecvBuf();
		}

		fSocket->GetSocket()->SetTask(this);
		fSocket->GetSocket()->RequestEvent(fSocket->GetEventMask());
	}


	//如果流程走到这里，Session实际已经无效了，应该被删除，但没有，因为还有其他地方引用了Session对象
    return 0;
}

Easy_Error EasyDeviceSession::SetServer(const char *szHost, UInt16 nPort)
{
	Easy_Error theErr = Easy_NoErr;
	if(szHost && nPort)
	{
		UInt32 inAddr = SocketUtils::ConvertStringToAddr(host2ip(szHost).c_str());
		if(inAddr)
			fSocket->Set(inAddr, nPort);
		else 
			theErr = Easy_BadArgument;
	}
	else
		theErr = Easy_BadArgument;

	return theErr;
}

void EasyDeviceSession::Disconnect()
{
	if(fSocket)
		delete fSocket;
	//构造Socket
	fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);

	if(m_httpClient)
		m_httpClient->SetSocket(fSocket);
	return ;
}

//发送XML报文
OS_Error EasyDeviceSession::SendXML(char* data)
{
	OS_Error theErr = OS_NoErr;
	int loopTimes = 0;
	while(loopTimes < 10)
	{
		theErr = m_httpClient->SendPacket(data);

		if(theErr == EAGAIN || theErr == EINPROGRESS)
		{
			loopTimes++;
			OSThread::Sleep(100);
			continue;
		}
		break;
	}
	if(theErr == OS_NoErr)
	{
		fSocket->GetSocket()->SetTask(this);
		fSocket->GetSocket()->RequestEvent(fSocket->GetEventMask());
	}
	else
	{
		if (m_recvCallback)
		{
			m_recvCallback(Easy_SendError, 0, m_recvCallbackParam);
		}
	}
	return theErr;
}

void EasyDeviceSession::SetVersion(const char *szVersion)
{
	m_strXMLVersion = szVersion;
}

void EasyDeviceSession::SetSessionID(const char *szSessionID)
{
	m_strSessionID = szSessionID;
}

void EasyDeviceSession::SetTerminalType(int eTermissionType)
{
	m_iTerminalType = eTermissionType;
}

void EasyDeviceSession::SetAppType(int eAppType)
{
	m_iAppType = eAppType;
}
void EasyDeviceSession::SetDeviceType(int eDeviceType)
{
	m_iDeviceType = eDeviceType;
}

void EasyDeviceSession::SetCMSHost(const char *szHost, UInt16 nPort)
{
	m_strCMSHost = szHost;
	m_nCMSPort = nPort;
}

void EasyDeviceSession::SetSessionStatus(SessionStatus status)
{
	m_state = status;
}

void EasyDeviceSession::SetAccessType(int eLoginType)
{
	m_iAccessType = eLoginType;
}

void EasyDeviceSession::SetAccess(const char *szAccess)
{
	m_strAccess = szAccess;
}

void EasyDeviceSession::SetPassword(const char *szPassword)
{
	m_strPassword = szPassword;
}

int EasyDeviceSession::GetAccessType()
{
	return m_iAccessType;
}

const char* EasyDeviceSession::GetAccess()
{
	return m_strAccess.c_str();
}

const char* EasyDeviceSession::GetPassword()
{
	return m_strPassword.c_str();
}

const char* EasyDeviceSession::GetVersion()
{
	return m_strXMLVersion.c_str();
}

const char* EasyDeviceSession::GetSessionID()
{
	return m_strSessionID.c_str();
}

int EasyDeviceSession::GetTerminalType()
{
	return m_iTerminalType;
}

int EasyDeviceSession::GetAppType()
{
	return m_iAppType;
}

int EasyDeviceSession::GetDeviceType()
{
	return m_iDeviceType;
}

const char* EasyDeviceSession::GetCMSHost() 
{
	return m_strCMSHost.c_str();
}

UInt16 EasyDeviceSession::GetCMSHostPort() 
{
	return m_nCMSPort;
}

SessionStatus EasyDeviceSession::GetSessionStatus()
{
	return m_state;
} 

//class EasyDeviceSession end
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class EasyDeviceCenter begin

#define MSG_THREAD_COUNT 5
#define MSG_WAITTING_COUNT 30
#define MSG_CACHE_COUNT  50
#define MSG_CACHE_SIZE   1024*20
#define EASY_SNAP_BUFFER_SIZE 1024*1024

EasyDeviceCenter::EasyDeviceCenter()
:	Task(),
	fHeartbeatFailTimes(0),
	fMsgSendStats(0),
	fState(Task::kIdleEvent),
	fEvent(NULL),
	fEventUser(NULL)
{
///////////////START--放到SDK的Init中////////////////
#ifdef _WIN32
	// Start Win32 DLLs
	WORD wsVersion = MAKEWORD(1, 1);
	WSADATA wsData;
	(void)::WSAStartup(wsVersion, &wsData);
#endif
	printf("1");
	//建立Reactor基础环境
	OS::Initialize();
	OSThread::Initialize();
	OSMemory::SetMemoryError(ENOMEM);
	printf("2");
	Socket::Initialize();
	printf("3");
	SocketUtils::Initialize();
//#if !MACOSXEVENTQUEUE
// Windows上啥都不干，Linux下初始化FD_ReadSet、FD_WriteSet
//	::select_startevents();
//#endif
printf("4");
	TaskThreadPool::AddThreads(3);
printf("5");
	IdleTask::Initialize();
	TimeoutTask::Initialize();
	Socket::StartThread();
	printf("6");
	OSThread::Sleep(1000);
	printf("7");
//START--放到SDK的Init中///////////////////////////
	m_deviceSession = new EasyDeviceSession();

	m_dispatchTackHandler = libDispatch_CreateInstance(MSG_THREAD_COUNT, MSG_WAITTING_COUNT, MSG_CACHE_COUNT, MSG_CACHE_SIZE,
		EasyDeviceCenter::DispatchMsgCenter,(unsigned long long)this);
	libDispatch_Start(m_dispatchTackHandler);

	callback_param_ptr_t param = new callback_param_t;
	param->pSessionData_ = m_deviceSession;
	param->pMsgCenterData_ = this;

	m_deviceSession->SetRecvDataCallback(EasyDeviceCenter::HttpRecvData, param);
	
	//建立心跳时间，默认30s
	fHeartbeatTask = new TimeoutTask(NULL,30);
	fHeartbeatTask->SetTask(this);
}

EasyDeviceCenter::~EasyDeviceCenter(void)
{
	if (m_deviceSession)
	{
		delete m_deviceSession;
		m_deviceSession = NULL;
	}

	if (fHeartbeatTask != NULL)
	{
		delete fHeartbeatTask;
		fHeartbeatTask = NULL;
	}
}

int EasyDeviceCenter::DispatchMsgCenter(unsigned long long ulParam, TinySysMsg_T *pMsg)
{
	EasyDeviceCenter *pTask = (EasyDeviceCenter*)ulParam;
	NetMessagePtr_T pNetMsg = NULL;

	switch (pMsg->msgHead_.nMsgType_)
	{

	case SYSTEM_MSG_ERROR:
		break;

	case SYSTEM_MSG_NET_MESSAGE:
		pNetMsg = (NetMessagePtr_T)(pMsg->msgBody_);
		pTask->DispatchNetMsgCenter_(pNetMsg);
		break;

	case SYSTEM_MSG_LOGIN:
		pTask->ExecMsgLogin_(pMsg);
		break;

	case SYSTEM_MSG_HEARTBEAT:
		pTask->ExecMsgHeartBeat_(pMsg);
		break;

	case SYSTEM_MSG_PUBLISH_STREAM:
		pTask->ExecMsgPublishStream_(pMsg);
		break;
	}

	return 0;
}

//执行消息中心投递的登录指令
void EasyDeviceCenter::ExecMsgLogin_(TinySysMsg_T *pMsg)
{
	int nRet = 0;
	EasyDeviceSession *pDeviceSession = (EasyDeviceSession *)(pMsg->msgHead_.customHandle_);

	do{
		if (pDeviceSession == NULL || (pDeviceSession->GetSessionStatus() != EasyDeviceOffline))
		{
			printf("EasyDeviceCenter::ExecMsgLogin_ => The device session status not Offline ");
			nRet = Easy_OutOfState;
			break;
		}
		OSMutexLocker locker (pDeviceSession->GetMutex());

		string strServerHost = pDeviceSession->GetCMSHost();
		int nServerPort = pDeviceSession->GetCMSHostPort();
		switch (pDeviceSession->GetTerminalType())
		{
		case EASYDARWIN_TERMINAL_TYPE_CAMERA:
		//case EASYDARWIN_TERMINAL_TYPE_DEVICE:
			strServerHost = pDeviceSession->GetCMSHost();
			nServerPort   = pDeviceSession->GetCMSHostPort();
			break;

		default:
			nRet = -1;
			break;
		}
		
		if (nRet == -1)
		{
			printf("EasyDeviceCenter::ExecMsgLogin_ => unknown terminal type \n");
			break;
		}		

		pDeviceSession->Disconnect();
		nRet = pDeviceSession->SetServer(strServerHost.c_str(), nServerPort);

		if (nRet < 0)
		{
			printf("EasyDeviceCenter::ExecMsgLogin_ => The device session connect server error.");
			break;
		}

		OS_Error theErr = pDeviceSession->SendXML(pMsg->msgBody_);
		if (theErr != 0)
		{
			nRet = Easy_RequestFailed;

			char msg[2048] = { 0 };
			sprintf(msg,"EasyDeviceCenter::ExecMsgLogin_ => The device session send msg error:%d.", theErr);
			printf(msg);
			break;
		}
		else
		{	
			//Need config
			////printf(pMsg->msgBody_);
			nRet = Easy_NoErr;
			//pDeviceSession->SetSessionStatus(easyClientLogging);
		}
	}while(false);
}

void EasyDeviceCenter::ExecMsgPublishStream_(TinySysMsg_T *pMsg)
{
	int nRet = 0;
	EasyDeviceSession *pDeviceSession = (EasyDeviceSession *)(pMsg->msgHead_.customHandle_);

	do{
		OS_Error osErr = pDeviceSession->SendXML(pMsg->msgBody_);
		if (osErr != 0)
		{
			nRet = Easy_Unimplemented;
			//AUTIO_GUARD_MUTEX(pDeviceSession->GetClientMutex());
			pDeviceSession->SetSessionStatus(EasyDeviceOffline);			
			char msg[2048] = { 0 };
			sprintf(msg, "EasyDeviceCenter::ExecMsgHeartBeat_ => The device session send msg error:%d.", osErr);
			printf(msg);
			
			break;
		}
		else
		{
			nRet = Easy_NoErr;
			printf(pMsg->msgBody_);

			//publish stream
			/*nRet = m_mediaSession->StartSMSStreaming(
														pDeviceSession->GetSMSHost(), 
														pDeviceSession->GetSMSHostPort(),
														pDeviceSession->GetPublishStreamID().c_str(),
														pDeviceSession->GetSessionID());*/
		}

	} while (false);

	if (nRet != 0)
	{	
	}
}

//处理网络报文类消息
void EasyDeviceCenter::DispatchNetMsgCenter_(NetMessagePtr_T pNetMsg)
{
	char *szXML = pNetMsg->szMsgXML;
	EasyProtocol protocol(szXML);
	int nNetMsg = protocol.GetMessageType();

	EasyDeviceSession *pDeviceSession = (EasyDeviceSession*)(pNetMsg->header_.customHandle_);
	if (pDeviceSession == NULL)
	{
		printlog("EasyDeviceCenter::DispatchNetMsgCenter_ => pDeviceSession==NULL.\n");
		return;
	}

	switch (nNetMsg)
	{
	case  MSG_DEV_CMS_REGISTER_RSP:
		LoginAck(CLIENT_MSG_LOGIN, pNetMsg);		
		break;

	//case MSG_CS_HEARTBEAT_ACK:
	//	HeartbeatAck(CLIENT_MSG_HEARTBEAT, pNetMsg);
	//	break;

	//case MSG_SC_PUBLISH_STREAM_REQ:
	//	PublishStreamReq(CLIENT_MSG_PUBLISH_STREAM, pNetMsg);
	//	break;

	//case MSG_CS_PUBLISH_STREAM_START_ACK:
	//	PublishStreamStartAck(CLIENT_MSG_PUBLISH_STREAM_START, pNetMsg);
	//	break;
	}
	
}

//处理Session传递给消息中心的消息
void EasyDeviceCenter::HttpRecvData(Easy_Error nErrorNum, char const* szMsg, void *pData)
{
	callback_param_ptr_t pCallbackParam = (callback_param_ptr_t)pData;
	if (pCallbackParam == NULL)
	{
		printlog("EasyDeviceCenter::HttpRecvData => pCallbackParam == NULL .\n");
		return;
	}
	
	EasyDeviceCenter* pMsgCenter = (EasyDeviceCenter*)(pCallbackParam->pMsgCenterData_);
	EasyDeviceSession* pDeviceSession = (EasyDeviceSession*)(pCallbackParam->pSessionData_);

	if (pMsgCenter==NULL || pDeviceSession==NULL)
	{
		printlog("EasyDeviceCenter::HttpRecvData => pMsgCenter==NULL(%d) || pDeviceSession==NULL(%d) .\n", pMsgCenter==NULL, pDeviceSession==NULL);
		return;
	}
	
	switch(nErrorNum)
	{
	case Easy_NoErr:	//正常的消息报文处理
		{
			TinySysMsg_T *pMsg = pMsgCenter->GetMsgBuffer();
			if (pMsg != NULL)
			{
				//发送网络消息
				pMsg->msgHead_.nMsgType_ = SYSTEM_MSG_NET_MESSAGE;
				NetMessagePtr_T pNetMsg = (NetMessagePtr_T)(pMsg->msgBody_);
				pNetMsg->header_.sockHandle_ = NULL;
				pNetMsg->header_.customHandle_ = pDeviceSession;

				char *xmlBody = pNetMsg->szMsgXML;
				sprintf(xmlBody, "%s", szMsg);

				qtss_printf("recv Msg:%s \n", xmlBody);
				//将报文消息投递到消息中心
				int nRet = pMsgCenter->SendMsg(pMsg);
				if (nRet < 0)
				{
					printlog("EasyDeviceCenter::HttpRecvData => fail to send msg ret=%d\n",nRet);
				}
			}
			break;
		}
	case Easy_InvalidSocket:
		{
			//掉线
			printf("*********HttpRecvData:Easy_InvalidSocket!*********\n");
			pMsgCenter->InvalidSocketHandler();
			break;
		}
	case Easy_RequestFailed:
		{
			//连接失败错误
			printf("HttpRecvData:Easy_RequestFailed!\n");
			break;
		}
	case Easy_ConnectError:
		{
			//连接失败错误
			printf("HttpRecvData:Easy_ConnectError!\n");
			
			break;
		}
	case Easy_SendError:
		{
			//报文发送失败
			printf("HttpRecvData:Easy_SendError!\n");
			break;
		}
	default:
		break;
	}
	if (nErrorNum != 0)
	{
		printlog("EasyDeviceCenter::HttpRecvData => Error (%d) .\n", nErrorNum);
		//pMsgCenter->ClientLogPrint("Client Offline .");
		return;
	}
}

void EasyDeviceCenter::InvalidSocketHandler()
{
	;
}

TinySysMsg_T* EasyDeviceCenter::GetMsgBuffer()
{
	return libDispatch_GetMsgBuffer(m_dispatchTackHandler);
}

int EasyDeviceCenter::SendMsg(TinySysMsg_T* pMsg)
{
	return libDispatch_SendMsg(m_dispatchTackHandler, pMsg);
}

void EasyDeviceCenter::LoginAck(ClientMsgType msgType, NetMessagePtr_T pNetMsg)
{
	Easy_Error nRet = Easy_NoErr;

	do{
		EasyDeviceSession *pDeviceSession = (EasyDeviceSession*)(pNetMsg->header_.customHandle_);
		if (pDeviceSession == NULL)
		{
			printf("EasyDeviceCenter::LoginAck => The device session <null> ");
			nRet = Easy_OutOfState;
			break;
		}

		const char *szXML = pNetMsg->szMsgXML;
		
		//Need config
		////printf(szXML);

		//EasyDarwinLoginAck parse(szXML);
		EasyDarwinRegisterRsp parse(szXML);
		
		//对Login返回进行解析,确定是否上线,保存Session等参数
		//TODO:

		//这里默认设置以上线		
		
		if(pDeviceSession->GetSessionStatus() != EasyDeviceOnline)
		{
			pDeviceSession->SetSessionStatus(EasyDeviceOnline);
			pDeviceSession->SetSessionID(parse.GetHeaderValue(EASYDARWIN_TAG_SESSION_ID).c_str());
			this->Signal(Task::kTimeoutEvent);
			if(fEvent != NULL)
			{
				fEvent(Easy_CMS_Event_Login, NULL, fEventUser);
			}
		}
		else
		{
			//开始心跳保活流程
			//TODO:

			//publish stream
			//nRet = m_mediaSession->StartSMSStreaming("121.40.50.44", 20014, "E82AEA8B9913", "E82AEA8B9913");
			fHeartbeatFailTimes = 0;
		}

	}while(false);
}

void EasyDeviceCenter::HeartbeatAck(ClientMsgType msgType, NetMessagePtr_T pNetMsg)
{
	EasyDeviceSession *pDeviceSession = (EasyDeviceSession*)(pNetMsg->header_.customHandle_);
	if (pDeviceSession == NULL)
	{
		printf("EasyDeviceCenter::HeartbeatAck => pDeviceSession == NULL.");
		return;
	}

	const char *szXML = pNetMsg->szMsgXML;
	//EasyDarwinHeartBeatAck parse(szXML);
	
	//Need config
	////printf(szXML);

	//解析心跳返回，如果有重新上线、重定向要求等，需要进行处理
	//TODO:

	//如果心跳返回报文格式正确,重置失败计数
	fHeartbeatFailTimes = 0;

	return;
}

/*
	设备发送上线请求：
	状态条件必须为EasyDeviceOffline
*/
Easy_Error EasyDeviceCenter::Login(const char *szHost, int nPort, const char *szAccess, const char *szPassword)
{
	int nRet = Easy_NoErr;
	do{

		OSMutexLocker locker (m_deviceSession->GetMutex());
		if (m_deviceSession->GetSessionStatus() != EasyDeviceOffline)
		{
			nRet = Easy_OutOfState;
			char msg[2048] = { 0 };
			sprintf(msg,"EasyDeviceCenter::Login => The device session wrong status (device status = %d).", m_deviceSession->GetSessionStatus());
			printf(msg);
			break;
		}

		printf("EasyDeviceCenter::Login %s:%d\n", szHost, nPort);

		m_deviceSession->SetTerminalType(EASYDARWIN_TERMINAL_TYPE_CAMERA);
		/*m_deviceSession->SetAppType(EASYDARWIN_APP_TYPE_PC);
		m_deviceSession->SetDeviceType(EASYDARWIN_DEVICE_TYPE_CAMERA);
		m_deviceSession->SetAccessType(EASYDARWIN_LOGIN_TYPE_SERIAL);*/
		m_deviceSession->SetAccess(szAccess);
		m_deviceSession->SetPassword(szPassword);

		//SessionID由CMS分配,在EasyDarwinLoginAck里面携带
		//m_deviceSession->SetSessionID("字母与数字组合的12位MAC");

		m_deviceSession->SetCMSHost(szHost, nPort);

		TinySysMsg_T *pMsg = GetMsgBuffer();
		if (pMsg!=NULL)
		{
			pMsg->msgHead_.nMsgType_ = SYSTEM_MSG_LOGIN;
			pMsg->msgHead_.customHandle_ = m_deviceSession;

			char *xmlBody = pMsg->msgBody_;
			EasyDarwinRegisterReq req;
			req.SetHeaderValue(EASYDARWIN_TAG_VERSION, "1.0");
			req.SetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(EASYDARWIN_TERMINAL_TYPE_CAMERA).c_str());
			req.SetHeaderValue(EASYDARWIN_TAG_CSEQ, "1");	
			
			req.SetBodyValue("SerialNumber", m_deviceSession->GetAccess());
			req.SetBodyValue("AuthCode", m_deviceSession->GetPassword());
		
			/*EasyDarwinLoginReq req;

			req.Pack(0, m_deviceSession->GetSessionID(),
				EASYDARWIN_TERMINAL_TYPE_DEVICE,
				EASYDARWIN_TERMINAL_TYPE_CAMERA,
				EASYDARWIN_LOGIN_TYPE_SERIAL,
				m_deviceSession->GetAccess(), 
				m_deviceSession->GetPassword());*/

			string buffer = req.GetMsg();
			sprintf(xmlBody, "%s", buffer.c_str());

			printf("Send Msg:\n%s\n", xmlBody);
			//向DispatchCenter消息队列发送消息
			nRet = SendMsg(pMsg);
						
			if (nRet < 0)
			{
				char msg[2048] = { 0 };
				sprintf(msg, "EasyDeviceCenter::Login => fail to send msg to DispatchCenter ret=%d .",nRet);
				printf(msg);
				break;
			}
		}
	}while(false);
	
	//到这里已经将SYSTEM_MSG_LOGIN提交给DispatchMsgCenter进行处理
	return nRet;
}

void EasyDeviceCenter::PublishStreamReq(ClientMsgType msgType, NetMessagePtr_T pNetMsg)
{
	const char *szXML = pNetMsg->szMsgXML;
	EasyDeviceSession *pDeviceSession = (EasyDeviceSession*)(pNetMsg->header_.customHandle_);
	int nRet = 0;	

	do{
		printf(szXML);

		//EasyDarwinPublishStreamReq req(szXML);

		////AUTIO_GUARD_MUTEX(pDeviceSession->GetClientMutex());

		//pDeviceSession->SetSMSHost(req.GetServerAddress().c_str(), req.GetServerPort());
		//pDeviceSession->SetProtocol(req.GetProtocol());
		//pDeviceSession->SetPublishStreamID(req.GetPublishStreamID());

		TinySysMsg_T *pMsg = GetMsgBuffer();
		if (pMsg != NULL)
		{
			pMsg->msgHead_.nMsgType_ = SYSTEM_MSG_PUBLISH_STREAM;
			pMsg->msgHead_.customHandle_ = pDeviceSession;

			char *xmlBody = pMsg->msgBody_;

			/*EasyDarwinPublishStreamAck ack;

			ack.SetMediaInfo(req.GetPublishStreamID(), EASYDARWIN_PROTOCOL_TYPE_RTSP, EASYDARWIN_MEDIA_ENCODE_AUDIO_AAC, EASYDARWIN_MEDIA_ENCODE_VIDEO_H264, 25);
			ack.Pack(req.GetSessionID(), req.GetCSeq(), EASYDARWIN_APP_TYPE_PC, EASYDARWIN_TERMINAL_TYPE_CAMERA);

			string buffer = ack.GetMsg();

			sprintf(xmlBody, "%s", buffer.c_str());*/

			nRet = SendMsg(pMsg);
			if (nRet<0)
			{
				char msg[2048] = { 0 };
				sprintf(msg, "EasyDeviceCenter::PublishStreamReq => fail to send msg ret=%d .", nRet);
				printf(msg);
				break;
			}
			else
			{
				//TODO
				//需要在这里加入TimeoutTask开始进行请求超时检测
				nRet = 0;
			}
		}

	} while (false);
}

void EasyDeviceCenter::PublishStreamStartAck(ClientMsgType msgType, NetMessagePtr_T pNetMsg)
{

}

SInt64 EasyDeviceCenter::Run()
{
	//获取事件类型
	EventFlags events = this->GetEvents();
	
	//上线与重连事件
	if ((events & Task::kUpdateEvent) || (events & Task::kTimeoutEvent))
	{
		switch (m_deviceSession->GetSessionStatus())
		{
			case EasyDeviceOffline:
				{
					//上线流程
					Login(m_deviceSession->GetCMSHost(), m_deviceSession->GetCMSHostPort(), m_deviceSession->GetAccess(), m_deviceSession->GetPassword());
					fHeartbeatFailTimes = 0;
					fMsgSendStats = 0;
					break;
				}

			case EasyDeviceOnline:
				{
					DeviceHeartBeat();
					//Login(m_deviceSession->GetCMSHost(), m_deviceSession->GetCMSHostPort(), m_deviceSession->GetAccess(), m_deviceSession->GetPassword());					

					fMsgSendStats++;
					//重置心跳计时
					fHeartbeatTask->RefreshTimeout();
					break;
				}
			default:
				break;
		}
	}
		
	return 0;
}

Easy_Error EasyDeviceCenter::UpdateDeviceSnap(const char* sData, unsigned int snapBufLen)
{
	Easy_Error nRet = Easy_NoErr;
	do{
		int base64DataLen = Base64encode_len(snapBufLen);
		char* snapEncodedBuffer = (char*)malloc(base64DataLen);
		::memset(snapEncodedBuffer, 0, base64DataLen);
		//对数据进行Base64编码
		::Base64encode(snapEncodedBuffer, (const char*)sData, snapBufLen); 

		printf("Send JPG Base64 Data Len:%d \n", base64DataLen);

		EasyDarwinDeviceSnapUpdateReq req;
		req.SetHeaderValue(EASYDARWIN_TAG_VERSION, "1.0");
		req.SetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(EASYDARWIN_TERMINAL_TYPE_CAMERA).c_str());
		req.SetHeaderValue(EASYDARWIN_TAG_CSEQ, "1");	

		req.SetBodyValue("Time", "1900"/*EasyDarwinUtil::TimeT2String(EASYDARWIN_TIME_FORMAT_YYYYMMDDHHMMSS, EasyDarwinUtil::NowTime()).c_str()*/);
		req.SetBodyValue("Img", snapEncodedBuffer);

		string buffer = req.GetMsg();
		//发送MSG_CS_SNAP_UPDATE_REQ消息
		OS_Error osErr = m_deviceSession->SendXML((char*)buffer.c_str());
		if (osErr != 0)
		{
			nRet = Easy_Unimplemented;
	
			char msg[2048] = { 0 };
			sprintf(msg, "EasyDeviceCenter::UpdateDeviceSnap => The device session send msg error:%d.", osErr);
			printf(msg);
			
			break;
		}
		else
		{
			nRet = Easy_NoErr;
		}

		free(snapEncodedBuffer);
		snapEncodedBuffer = NULL;

	} while (false);

	//到这里已经将SYSTEM_MSG_HEARTBEAT提交给DispatchMsgCenter进行处理
	return nRet;
	return -1;
}

Easy_Error EasyDeviceCenter::DeviceHeartBeat()
{
	Easy_Error nRet = Easy_NoErr;
	do{
		OSMutexLocker locker (m_deviceSession->GetMutex());
		if (m_deviceSession->GetSessionStatus() != EasyDeviceOnline )
		{
			this->Signal(Task::kUpdateEvent);
			nRet = Easy_OutOfState;
			break;
		}

		//m_deviceSession->SetTerminalType(EASYDARWIN_TERMINAL_TYPE_DEVICE);
		//m_deviceSession->SetAppType(EASYDARWIN_APP_TYPE_CAMERA);

		TinySysMsg_T *pMsg = GetMsgBuffer();
		if (pMsg != NULL)
		{
			pMsg->msgHead_.nMsgType_ = SYSTEM_MSG_HEARTBEAT;
			pMsg->msgHead_.customHandle_ = m_deviceSession;

			char *xmlBody = pMsg->msgBody_;

			/*EasyDarwinHeartBeatReq msg;
			msg.Pack(0, m_deviceSession->GetSessionID(), EASYDARWIN_TERMINAL_TYPE_DEVICE, EASYDARWIN_TERMINAL_TYPE_CAMERA);

			string buffer = msg.GetMsg();

			sprintf(xmlBody, "%s", buffer.c_str());*/
			EasyDarwinRegisterReq req;
			req.SetHeaderValue(EASYDARWIN_TAG_VERSION, "1.0");
			req.SetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(EASYDARWIN_TERMINAL_TYPE_CAMERA).c_str());
			req.SetHeaderValue(EASYDARWIN_TAG_CSEQ, "1");	
			
			req.SetBodyValue("SerialNumber", m_deviceSession->GetAccess());
			req.SetBodyValue("AuthCode", m_deviceSession->GetPassword());

			string buffer = req.GetMsg();
			sprintf(xmlBody, "%s", buffer.c_str());

			
			nRet = SendMsg(pMsg);
			if (nRet<0)
			{
				char msg[2048] = { 0 };
				sprintf(msg, "EasyDeviceCenter::DeviceHeartBeat => fail to send msg to DispatchCenter ret=%d .", nRet);
				printf(msg);
				break;
			}
		}
	} while (false);

	//到这里已经将SYSTEM_MSG_HEARTBEAT提交给DispatchMsgCenter进行处理
	return nRet;
}

void EasyDeviceCenter::ExecMsgHeartBeat_(TinySysMsg_T *pMsg)
{
	int nRet = Easy_NoErr;
	EasyDeviceSession *pDeviceSession = (EasyDeviceSession *)(pMsg->msgHead_.customHandle_);

	if(pDeviceSession == NULL) return;

	if(fHeartbeatFailTimes > 3)
	{
		pDeviceSession->SetSessionStatus(EasyDeviceOffline);
		this->Signal(Task::kUpdateEvent);
		fEvent(Easy_CMS_Event_Offline, NULL, fEventUser);
		return;
	}

	OSMutexLocker locker (pDeviceSession->GetMutex());
	do{
		OS_Error theErr = pDeviceSession->SendXML(pMsg->msgBody_);
		if (theErr != 0)
		{
			nRet = Easy_Unimplemented;
			pDeviceSession->SetSessionStatus(EasyDeviceOffline);
			
			char msg[2048] = { 0 };
			sprintf(msg, "EasyDeviceCenter::ExecMsgHeartBeat_ => The device session send msg error:%d.", theErr);
			printf(msg);
			break;
		}
		//发送成功，增加一次失败次数，到有ACK再重置为0
		fHeartbeatFailTimes++;

		//Need config
		////printf(pMsg->msgBody_);

	} while (false);
}

void EasyDeviceCenter::SetEventCallBack(EasyCMS_Callback fCallBack, void *pUserData)
{
	fEvent = fCallBack;
	if(pUserData != NULL)
	{
		fEventUser = pUserData;
	}
}

}
}