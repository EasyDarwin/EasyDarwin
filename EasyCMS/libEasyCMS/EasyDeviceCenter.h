/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _EASY_DEVICE_CENTER_H_
#define _EASY_DEVICE_CENTER_H_

#include "HttpClient.h"
#include "libTinyDispatchCenter.h"
#include <map>
#include "MutexLock.h"
#include "libClientCommondef.h"
#include "ClientSystemCommonDef.h"
#include "QTSS.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#ifndef __Win32__
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#endif

#include "OSMemory.h"
#include "OSArrayObjectDeleter.h"
#include "SocketUtils.h"
#include "StringFormatter.h"
#include "Socket.h"
#include "OS.h"
#include "Task.h"
#include "TimeoutTask.h"
#include "SVector.h"
#include "EasyProtocol.h"
#include "systemcommdef.h"

#include "base64.h"
#include "EasyCMSAPI.h"

using namespace std;
using namespace libTinyDispatchCenter;

namespace EasyDarwin { namespace libEasyCMS
{
typedef void (*HttpClientRecv_Callback)(int nErrorNum, char const* szXML, void *vptr);

/////////////////////////////////////////////////////////////////////////
//class EasyDeviceSession begin

//客户端状态机
enum
{
	EasyDeviceOffline = 0,
	//easyClientLogging,
	EasyDeviceOnline = 1,
};
typedef UInt32 SessionStatus;

//存储一个客户端的基本信息，处理客户端消息的收发与网络状态维护
class EasyDeviceSession : public Task
{
public:
	EasyDeviceSession();
	virtual ~EasyDeviceSession();

	//传递给上层处理
	void SetRecvDataCallback(HttpClientRecv_Callback func, void *vptr);

	void ResetClient();

	Bool16 IsLiveSession()      { return fSocket->GetSocket()->IsConnected() && fLiveSession; }

	virtual SInt64 Run();

	CHttpClient* GetClient()	{ return m_httpClient; }
	ClientSocket* GetSocket()	{ return fSocket; }

	Easy_Error SetServer(const char *szHost, UInt16 nPort);
	void Disconnect();
	OS_Error SendXML(char* data);

	void SetVersion(const char *szVersion);
	void SetSessionID(const char *szSessionID);
	void SetTerminalType(int eTermissionType);
	void SetAppType(int eAppType);
	void SetDeviceType(int eDeviceType);
	void SetCMSHost(const char *szHost, UInt16 nPort);
	void SetProtocol(EasyDarwinProtocolType protocol) { m_protocol = protocol; }
	void SetSessionStatus(SessionStatus status);
	void SetPublishStreamID(string sStreamID){ m_strPublishStreamID = sStreamID; }

	void SetAccessType(int eLoginType);
	void SetAccess(const char *szAccess);
	void SetPassword(const char *szPassword);

	int GetAccessType();
	const char* GetAccess();
	const char* GetPassword();

	const char* GetVersion();
	const char* GetSessionID();
	int GetTerminalType();
	int GetAppType();
	int GetDeviceType();
	const char* GetCMSHost();
	UInt16 GetCMSHostPort();

	OSMutex*    GetMutex()      { return &fMutex; }
	SessionStatus GetSessionStatus();
	EasyDarwinProtocolType GetProtocol(){ return m_protocol; }
	string GetPublishStreamID(){ return m_strPublishStreamID; }
private:
	//Socket连接
	ClientSocket* fSocket;    // Connection object
	//HTTPClient连接
	CHttpClient* m_httpClient;
	//超时检测
	TimeoutTask fClientTimeoutTask; 

	//会话ID
	string m_strSessionID;

	//客户端是设备或者用户
	int m_iTerminalType;
	
	//作为用户登录时，客户端类型
	int m_iAppType;
	
	//作为设备客户端时（m_strTermissionType 赋值为 Camera或者Device 时）有效
	int m_iDeviceType;

	int m_iAccessType;
	string m_strAccess;
	string m_strPassword;
	
	//XML协议版本
	string m_strXMLVersion;

	//客户端信息操作互斥量
	OSMutex fMutex;

	//CMS服务器信息
	string m_strCMSHost;
	UInt16 m_nCMSPort;

	//客户端状态
	SessionStatus  m_state;

	HttpClientRecv_Callback m_recvCallback;
	void *m_recvCallbackParam;

	EasyDarwinProtocolType m_protocol;
	string m_strPublishStreamID;

protected:
	Bool16              fLiveSession;

};
//class EasyDeviceSession end
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//class EasyDeviceCenter begin
typedef struct _callback_param_t 
{
	void *pSessionData_;
	void *pMsgCenterData_;
}callback_param_t, *callback_param_ptr_t;


//客户端的逻辑操作
class EasyDeviceCenter : public Task
{
public:
	EasyDeviceCenter();
	virtual ~EasyDeviceCenter(void);

private:
	//设备与云平台(CMS)对接Session
	EasyDeviceSession* m_deviceSession;
	//设备与流媒体服务(SMS)对接Session
	//EasyMediaSession *m_mediaSession;
	//内部Task事务处理器
	HDISPATCHTASK    m_dispatchTackHandler;

public:
	//初始化
	void InitClient();
	//设备登录
	virtual Easy_Error Login(const char *szHost, int nPort, const char *szAccess, const char *szPassword);
	//设备心跳
	virtual Easy_Error DeviceHeartBeat();
	//上传快照
	virtual Easy_Error UpdateDeviceSnap(const char* sData, unsigned int snapBufLen);
	//设置云服务器入口，也就是CGE地址和端口
	virtual void LoginAck(ClientMsgType msgType, NetMessagePtr_T pNetMsg);
	virtual void HeartbeatAck(ClientMsgType msgType, NetMessagePtr_T pNetMsg);
	virtual void PublishStreamReq(ClientMsgType msgType, NetMessagePtr_T pNetMsg);
	virtual void PublishStreamStartAck(ClientMsgType msgType, NetMessagePtr_T pNetMsg);

	//处理所有SysMsg的总入口
	static int DispatchMsgCenter(unsigned long long ulParam, TinySysMsg_T *pMsg);
	//所有的网络数据类事务都由此回调完成，包括网络断开等
	static void HttpRecvData(Easy_Error nErrorNum, char const* szMsg, void *pData);
	//从DispatchMsgCenter获取空闲Msg Buffer以调用
	TinySysMsg_T* GetMsgBuffer();
	//向DispatchMsgCenter派发消息
	int SendMsg(TinySysMsg_T* pMsg);
	//Easy_InvalidSocket消息处理
	void InvalidSocketHandler();

	EasyDeviceSession* GetDeviceSession() { return m_deviceSession; }
	//EasyMediaSession* GetMediaSession() { return m_mediaSession; }

	void SetEventCallBack(EasyCMS_Callback fCallBack, void *pUserData);

private:
	////处理所有SysMsg的总入口
	//void DispatchMsgCenter_(TinySysMsg_T *pMsg);
	//处理接收到的网络报文类消息
	void DispatchNetMsgCenter_(NetMessagePtr_T pNetMsg);
	//执行SYSTEM_MSG_LOGIN系统消息
	void ExecMsgLogin_(TinySysMsg_T *pMsg);
	//执行SYSTEM_MSG_PUBLISH_STREAM系统消息
	void ExecMsgPublishStream_(TinySysMsg_T *pMsg);
	//执行SYSTEM_MSG_HEARTBEAT系统消息
	void ExecMsgHeartBeat_(TinySysMsg_T *pMsg);

	SInt64 Run();
	TimeoutTask *fHeartbeatTask;//心跳事件

	UInt32 fHeartbeatFailTimes;
	int fMsgSendStats;
	UInt32 fState;
	EasyCMS_Callback fEvent;
	void *fEventUser;
};
//class EasyDeviceCenter end
/////////////////////////////////////////////////////////////////////////
}
}
#endif