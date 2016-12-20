/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
	File:       HTTPSession.cpp
	Contains:
*/

#include "HTTPSession.h"
#include "QTSServerInterface.h"
#include "OSArrayObjectDeleter.h"
#include "QTSSMemoryDeleter.h"
#include "QueryParamList.h"

#include "EasyProtocolDef.h"
#include "EasyProtocol.h"
#include "EasyUtil.h"
#include "Format.h"

using namespace EasyDarwin::Protocol;
using namespace std;

#if __FreeBSD__ || __hpux__	
#include <unistd.h>
#endif

#include <errno.h>

#if __solaris__ || __linux__ || __sgi__	|| __hpux__
#include <crypt.h>
#endif

static StrPtrLen	sEasyHLSModule("api/easyhlsmodule");
static StrPtrLen	sGetHLSSessions("api/gethlssessions");

#define	QUERY_STREAM_NAME		"name"
#define QUERY_STREAM_URL		"url"
#define QUERY_STREAM_CMD		"cmd"
#define QUERY_STREAM_TIMEOUT	"timeout"
#define QUERY_STREAM_CMD_START	"start"
#define QUERY_STREAM_CMD_STOP	"stop"

HTTPSession::HTTPSession()
	: HTTPSessionInterface(),
	fRequest(nullptr),
	fReadMutex(),
	fCurrentModule(0),
	fState(kReadingFirstRequest)
{
	this->SetTaskName("HTTPSession");

	QTSServerInterface::GetServer()->AlterCurrentRTSPHTTPSessionCount(1);

	// Setup the QTSS param block, as none of these fields will change through the course of this session.
	fRoleParams.rtspRequestParams.inRTSPSession = this;
	fRoleParams.rtspRequestParams.inRTSPRequest = nullptr;
	fRoleParams.rtspRequestParams.inClientSession = nullptr;

	fModuleState.curModule = nullptr;
	fModuleState.curTask = this;
	fModuleState.curRole = 0;
	fModuleState.globalLockRequested = false;
}

HTTPSession::~HTTPSession()
{
	char remoteAddress[20] = { 0 };
	StrPtrLen theIPAddressStr(remoteAddress, sizeof(remoteAddress));
	QTSS_GetValue(this, easyHTTPSesRemoteAddrStr, 0, static_cast<void*>(theIPAddressStr.Ptr), &theIPAddressStr.Len);

	char msgStr[2048] = { 0 };
	qtss_snprintf(msgStr, sizeof(msgStr), "HTTPSession offline from ip[%s]", remoteAddress);
	QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);
	QTSServerInterface::GetServer()->AlterCurrentRTSPHTTPSessionCount(-1);

	// Invoke the session closing modules
	QTSS_RoleParams theParams;
	theParams.rtspSessionClosingParams.inRTSPSession = this;

	//会话断开时，调用模块进行一些停止的工作
	for (UInt32 x = 0; x < QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPSessionClosingRole); x++)
		(void)QTSServerInterface::GetModule(QTSSModule::kRTSPSessionClosingRole, x)->CallDispatch(QTSS_RTSPSessionClosing_Role, &theParams);

	fLiveSession = false; //used in Clean up request to remove the RTP session.
	this->CleanupRequest();// Make sure that all our objects are deleted
	//if (fSessionType == qtssServiceSession)
	//    QTSServerInterface::GetServer()->AlterCurrentServiceSessionCount(-1);

}

SInt64 HTTPSession::Run()
{
	//获取事件类型
	EventFlags events = this->GetEvents();
	QTSS_Error err = QTSS_NoErr;
	// Some callbacks look for this struct in the thread object
	OSThreadDataSetter theSetter(&fModuleState, nullptr);

	//超时事件或者Kill事件，进入释放流程：清理 & 返回-1
	if (events & Task::kKillEvent)
		fLiveSession = false;

	if (events & Task::kTimeoutEvent)
	{
		// Session超时,释放Session 
		return -1;
	}

	//正常事件处理流程
	while (this->IsLiveSession())
	{
		//报文处理以状态机的形式，可以方便多次处理同一个消息
		switch (fState)
		{
		case kReadingFirstRequest://首次对Socket进行读取
			{
				if ((err = fInputStream.ReadRequest()) == QTSS_NoErr)
				{
					//如果RequestStream返回QTSS_NoErr，就表示已经读取了目前所到达的网络数据
					//但，还不能构成一个整体报文，还要继续等待读取...
					fInputSocketP->RequestEvent(EV_RE);
					return 0;
				}

				if ((err != QTSS_RequestArrived) && (err != E2BIG))
				{
					// Any other error implies that the client has gone away. At this point,
					// we can't have 2 sockets, so we don't need to do the "half closed" check
					// we do below
					Assert(err > 0);
					Assert(!this->IsLiveSession());
					break;
				}

				if ((err == QTSS_RequestArrived) || (err == E2BIG))
					fState = kHaveCompleteMessage;
			}
			continue;
		case kReadingRequest://读取请求报文
			{
				//读取锁，已经在处理一个报文包时，不进行新网络报文的读取和处理
				OSMutexLocker readMutexLocker(&fReadMutex);

				//网络请求报文存储在fInputStream中
				if ((err = fInputStream.ReadRequest()) == QTSS_NoErr)
				{
					//如果RequestStream返回QTSS_NoErr，就表示已经读取了目前所到达的网络数据
					//但，还不能构成一个整体报文，还要继续等待读取...
					fInputSocketP->RequestEvent(EV_RE);
					return 0;
				}

				if ((err != QTSS_RequestArrived) && (err != E2BIG) && (err != QTSS_BadArgument))
				{
					//Any other error implies that the input connection has gone away.
					// We should only kill the whole session if we aren't doing HTTP.
					// (If we are doing HTTP, the POST connection can go away)
					Assert(err > 0);
					if (fOutputSocketP->IsConnected())
					{
						// If we've gotten here, this must be an HTTP session with
						// a dead input connection. If that's the case, we should
						// clean up immediately so as to not have an open socket
						// needlessly lingering around, taking up space.
						Assert(fOutputSocketP != fInputSocketP);
						Assert(!fInputSocketP->IsConnected());
						fInputSocketP->Cleanup();
						return 0;
					}
					else
					{
						Assert(!this->IsLiveSession());
						break;
					}
				}
				fState = kHaveCompleteMessage;
			}
		case kHaveCompleteMessage://读取到完整的请求报文
			{
				Assert(fInputStream.GetRequestBuffer());

				Assert(fRequest == nullptr);
				//根据具体请求报文构造HTTPRequest请求类
				fRequest = new HTTPRequest(&QTSServerInterface::GetServerHeader(), fInputStream.GetRequestBuffer());

				//在这里，我们已经读取了一个完整的Request，并准备进行请求的处理，直到响应报文发出
				//在此过程中，此Session的Socket不进行任何网络数据的读/写；
				fReadMutex.Lock();
				fSessionMutex.Lock();

				//清空发送缓冲区
				fOutputStream.ResetBytesWritten();

				//网络请求超过了缓冲区，返回Bad Request
				if (err == E2BIG)
				{
					//返回HTTP报文，错误码408
					//(void)QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientBadRequest, qtssMsgRequestTooLong);
					fState = kSendingResponse;
					break;
				}
				// Check for a corrupt base64 error, return an error
				if (err == QTSS_BadArgument)
				{
					//返回HTTP报文，错误码408
					//(void)QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientBadRequest, qtssMsgBadBase64);
					fState = kSendingResponse;
					break;
				}

				Assert(err == QTSS_RequestArrived);
				fState = kFilteringRequest;
			}

		case kFilteringRequest:
			{
				//刷新Session保活时间
				fTimeoutTask.RefreshTimeout();

				//对请求报文进行解析
				QTSS_Error theErr = SetupRequest();
				//当SetupRequest步骤未读取到完整的网络报文，需要进行等待
				if (theErr == QTSS_WouldBlock)
				{
					this->ForceSameThread();
					fInputSocketP->RequestEvent(EV_RE);
					// We are holding mutexes, so we need to force
					// the same thread to be used for next Run()
					return 0;//返回0表示有事件才进行通知，返回>0表示规定事件后调用Run

				}

				//每一步都检测响应报文是否已完成，完成则直接进行回复响应
				if (fOutputStream.GetBytesWritten() > 0)
				{
					fState = kSendingResponse;
					break;
				}

				fState = kPreprocessingRequest;
				break;
			}

		case kPreprocessingRequest:
			{
				//请求预处理过程
				//TODO:报文处理过程
				fState = kCleaningUp;
				break;
			}

		case kProcessingRequest:
			{
				if (fOutputStream.GetBytesWritten() == 0)
				{
					// 响应报文还没有形成
					//QTSSModuleUtils::SendErrorResponse(fRequest, qtssServerInternal, qtssMsgNoModuleForRequest);
					fState = kCleaningUp;
					break;
				}

				fState = kSendingResponse;
			}
		case kSendingResponse:
			{
				//响应报文发送，确保完全发送
				Assert(fRequest != nullptr);

				//发送响应报文
				err = fOutputStream.Flush();

				if (err == EAGAIN)
				{
					// If we get this error, we are currently flow-controlled and should
					// wait for the socket to become writeable again
					//如果收到Socket EAGAIN错误，那么我们需要等Socket再次可写的时候再调用发送
					fSocket.RequestEvent(EV_WR);
					this->ForceSameThread();
					// We are holding mutexes, so we need to force
					// the same thread to be used for next Run()
					return 0;
				}
				else if (err != QTSS_NoErr)
				{
					// Any other error means that the client has disconnected, right?
					Assert(!this->IsLiveSession());
					break;
				}

				fState = kCleaningUp;
			}

		case kCleaningUp:
			{
				// Cleaning up consists of making sure we've read all the incoming Request Body
				// data off of the socket
				if (this->GetRemainingReqBodyLen() > 0)
				{
					err = this->dumpRequestData();

					if (err == EAGAIN)
					{
						fInputSocketP->RequestEvent(EV_RE);
						this->ForceSameThread();    // We are holding mutexes, so we need to force
													// the same thread to be used for next Run()
						return 0;
					}
				}

				//一次请求的读取、处理、响应过程完整，等待下一次网络报文！
				this->CleanupRequest();
				fState = kReadingRequest;
			}
		}
	}

	//清空Session占用的所有资源
	this->CleanupRequest();

	//Session引用数为0，返回-1后，系统会将此Session删除
	if (fObjectHolders == 0)
		return -1;

	//如果流程走到这里，Session实际已经无效了，应该被删除，但没有，因为还有其他地方引用了Session对象
	return 0;
}

QTSS_Error HTTPSession::SendHTTPPacket(StrPtrLen* contentXML, bool connectionClose, bool decrement)
{
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(contentXML->Len ? httpOK : httpNotImplemented);
	if (contentXML->Len)
		httpAck.AppendContentLengthHeader(contentXML->Len);

	if (connectionClose)
		httpAck.AppendConnectionCloseHeader();

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

	RTSPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (contentXML->Len > 0)
		pOutputStream->Put(contentXML->Ptr, contentXML->Len);

	if (pOutputStream->GetBytesWritten() != 0)
	{
		pOutputStream->Flush();
	}

	//将对HTTPSession的引用减少一
	if (fObjectHolders && decrement)
		DecrementObjectHolderCount();

	if (connectionClose)
		this->Signal(Task::kKillEvent);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::SetupRequest()
{
	//解析请求报文
	QTSS_Error theErr = fRequest->Parse();
	if (theErr != QTSS_NoErr)
		return QTSS_BadArgument;


	if (fRequest->GetRequestPath())
	{
		StrPtrLen theFullPath(fRequest->GetRequestPath());

		if (theFullPath.Equal(sEasyHLSModule))
		{
			return ExecNetMsgEasyHLSModuleReq(fRequest->GetQueryString(), nullptr);
		}

		if (theFullPath.Equal(sGetHLSSessions))
		{
			return ExecNetMsgGetHlsSessionsReq(fRequest->GetQueryString(), nullptr);
		}
	}

	if (fRequest->GetRequestPath() != nullptr)
	{
		string sRequest(fRequest->GetRequestPath());

		if (!sRequest.empty())
		{
			boost::to_lower(sRequest);

			vector<string> path;
			if (boost::ends_with(sRequest, "/"))
			{
				boost::erase_tail(sRequest, 1);
			}
			boost::split(path, sRequest, boost::is_any_of("/"), boost::token_compress_on);
			if (path.size() == 3)
			{

				if (path[0] == "api" && path[1] == "v1" && path[2] == "login")
				{
					return execNetMsgCSLoginReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == "v1" && path[2] == "logout")
				{
					return execNetMsgCSLogoutReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == "v1" && path[2] == "getserverinfo")
				{
					return execNetMsgCSGetServerVersionReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == "v1" && path[2] == "getbaseconfig")
				{
					return execNetMsgCSGetBaseConfigReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == "v1" && path[2] == "setbaseconfig")
				{
					return execNetMsgCSSetBaseConfigReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == "v1" && path[2] == "restart")
				{
					return execNetMsgCSRestartServiceRESTful(fRequest->GetQueryString());
				}

				//if (path[0] == "api" && path[1] == "v1" && path[2] == "getchannels")
				//{
				//	return execNetMsgCSGetChannelsRESTful(fRequest->GetQueryString());
				//}

				if (path[0] == "api" && path[1] == "v1" && path[2] == "getdevicestream")
				{
					return execNetMsgCSGetDeviceStreamReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == "v1" && path[2] == "livedevicestream")
				{
					return execNetMsgCSLiveDeviceStreamReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == "v1" && path[2] == "getrtsplivesessions")
				{
					return execNetMsgCSGetRTSPLiveSessionsRESTful(fRequest->GetQueryString());
				}
				if (path[0] == "api" && path[1] == "v1" && path[2] == "getrecordlist")
				{
					return execNetMsgCSGetRTSPRecordSessionsRESTful(fRequest->GetQueryString());
				}
			}

			EasyMsgExceptionACK rsp;
			string msg = rsp.GetMsg();
			StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
			this->SendHTTPPacket(&theValue, false, false);

			return QTSS_NoErr;
		}
	}

	//获取具体Content json数据部分

	//1、获取json部分长度
	StrPtrLen* lengthPtr = fRequest->GetHeaderValue(httpContentLengthHeader);
	StringParser theContentLenParser(lengthPtr);
	theContentLenParser.ConsumeWhitespace();
	UInt32 content_length = theContentLenParser.ConsumeInteger(nullptr);

	if (content_length)
	{
		qtss_printf("HTTPSession read content-length:%d \n", content_length);
		// Check for the existence of 2 attributes in the request: a pointer to our buffer for
		// the request body, and the current offset in that buffer. If these attributes exist,
		// then we've already been here for this request. If they don't exist, add them.
		UInt32 theBufferOffset = 0;
		char* theRequestBody = nullptr;
		UInt32 theLen = sizeof(theRequestBody);
		theErr = QTSS_GetValue(this, easyHTTPSesContentBody, 0, &theRequestBody, &theLen);

		if (theErr != QTSS_NoErr)
		{
			// First time we've been here for this request. Create a buffer for the content body and
			// shove it in the request.
			theRequestBody = new char[content_length + 1];
			memset(theRequestBody, 0, content_length + 1);
			theLen = sizeof(theRequestBody);
			theErr = QTSS_SetValue(this, easyHTTPSesContentBody, 0, &theRequestBody, theLen);// SetValue creates an internal copy.
			Assert(theErr == QTSS_NoErr);

			// Also store the offset in the buffer
			theLen = sizeof(theBufferOffset);
			theErr = QTSS_SetValue(this, easyHTTPSesContentBodyOffset, 0, &theBufferOffset, theLen);
			Assert(theErr == QTSS_NoErr);
		}

		theLen = sizeof(theBufferOffset);
		QTSS_GetValue(this, easyHTTPSesContentBodyOffset, 0, &theBufferOffset, &theLen);

		// We have our buffer and offset. Read the data.
		//theErr = QTSS_Read(this, theRequestBody + theBufferOffset, content_length - theBufferOffset, &theLen);
		theErr = fInputStream.Read(theRequestBody + theBufferOffset, content_length - theBufferOffset, &theLen);
		Assert(theErr != QTSS_BadArgument);

		if (theErr == QTSS_RequestFailed)
		{
			OSCharArrayDeleter charArrayPathDeleter(theRequestBody);
			//
			// NEED TO RETURN HTTP ERROR RESPONSE
			return QTSS_RequestFailed;
		}

		qtss_printf("Add Len:%d \n", theLen);
		if ((theErr == QTSS_WouldBlock) || (theLen < (content_length - theBufferOffset)))
		{
			//
			// Update our offset in the buffer
			theBufferOffset += theLen;
			(void)QTSS_SetValue(this, easyHTTPSesContentBodyOffset, 0, &theBufferOffset, sizeof(theBufferOffset));
			// The entire content body hasn't arrived yet. Request a read event and wait for it.

			Assert(theErr == QTSS_NoErr);
			return QTSS_WouldBlock;
		}

		Assert(theErr == QTSS_NoErr);

		OSCharArrayDeleter charArrayPathDeleter(theRequestBody);

		////报文处理，不进入队列
		//EasyDarwin::Protocol::EasyProtocol protocol(theRequestBody);
		//int nNetMsg = protocol.GetMessageType();

		//switch (nNetMsg)
		//{
		//	case MSG_DEV_CMS_REGISTER_REQ:
		//		ExecNetMsgDevRegisterReq(theRequestBody);
		//		break;
		//	case MSG_NGX_CMS_NEED_STREAM_REQ:
		//		ExecNetMsgNgxStreamReq(theRequestBody);
		//		break;
		//	default:
		//		ExecNetMsgDefaultReqHandler(theRequestBody);
		//		break;
		//}

		UInt32 offset = 0;
		(void)QTSS_SetValue(this, easyHTTPSesContentBodyOffset, 0, &offset, sizeof(offset));
		char* content = nullptr;
		(void)QTSS_SetValue(this, easyHTTPSesContentBody, 0, &content, 0);

	}

	qtss_printf("get complete http msg:%s QueryString:%s \n", fRequest->GetRequestPath(), fRequest->GetQueryString());

	return QTSS_NoErr;
}

void HTTPSession::CleanupRequest()
{
	if (fRequest != nullptr)
	{
		// nullptr out any references to the current request
		delete fRequest;
		fRequest = nullptr;
		fRoleParams.rtspRequestParams.inRTSPRequest = nullptr;
		fRoleParams.rtspRequestParams.inRTSPHeaders = nullptr;
	}

	fSessionMutex.Unlock();
	fReadMutex.Unlock();

	// Clear out our last value for request body length before moving onto the next request
	this->SetRequestBodyLength(-1);
}

bool HTTPSession::OverMaxConnections(UInt32 buffer)
{
	QTSServerInterface* theServer = QTSServerInterface::GetServer();
	SInt32 maxConns = theServer->GetPrefs()->GetMaxConnections();
	bool overLimit = false;

	if (maxConns > -1) // limit connections
	{
		UInt32 maxConnections = static_cast<UInt32>(maxConns) + buffer;
		if (theServer->GetNumRTSPSessions() > maxConnections)
		{
			overLimit = true;
		}
	}
	return overLimit;
}

QTSS_Error HTTPSession::dumpRequestData()
{
	char theDumpBuffer[QTSS_MAX_REQUEST_BUFFER_SIZE];

	QTSS_Error theErr = QTSS_NoErr;
	while (theErr == QTSS_NoErr)
		theErr = this->Read(theDumpBuffer, QTSS_MAX_REQUEST_BUFFER_SIZE, nullptr);

	return theErr;
}

QTSS_Error HTTPSession::ExecNetMsgEasyHLSModuleReq(char* queryString, char* json)
{
	QTSS_Error theErr = QTSS_NoErr;

	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	bool bStop = false;

	char decQueryString[QTSS_MAX_URL_LENGTH] = { 0 };
	EasyUtil::Urldecode(reinterpret_cast<unsigned char*>(queryString), reinterpret_cast<unsigned char*>(decQueryString));

	char* hlsURL = new char[QTSS_MAX_URL_LENGTH];
	hlsURL[0] = '\0';
	QTSSCharArrayDeleter theHLSURLDeleter(hlsURL);
	do
	{

		if (strlen(decQueryString) == 0)
			break;

		StrPtrLen theQueryString(decQueryString);

		QueryParamList parList(decQueryString);

		const char* sName = parList.DoFindCGIValueForParam(QUERY_STREAM_NAME);
		if (sName == nullptr)
		{
			theErr = QTSS_Unimplemented;
			break;
		}

		const char* sURL = parList.DoFindCGIValueForParam(QUERY_STREAM_URL);
		const char* sCMD = parList.DoFindCGIValueForParam(QUERY_STREAM_CMD);
		const char* sTIMEOUT = parList.DoFindCGIValueForParam(QUERY_STREAM_TIMEOUT);
		int iTIMEOUT = 0;

		if (sCMD)
		{
			if (::strcmp(sCMD, QUERY_STREAM_CMD_STOP) == 0)
				bStop = true;
		}

		if (bStop)
		{
			Easy_StopHLSession(sName);
		}
		else
		{
			if (sURL == nullptr)
			{
				theErr = QTSS_Unimplemented;
				break;
			}

			//TODO::这里需要对URL进行过滤
			//


			if (sTIMEOUT)
				iTIMEOUT = ::atoi(sTIMEOUT);

			char msgStr[2048] = { 0 };
			qtss_snprintf(msgStr, sizeof(msgStr), "HTTPSession::ExecNetMsgEasyHLSModuleReq name=%s,url=%s,time=%d", sName, sURL, iTIMEOUT);
			QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);

			Easy_StartHLSession(sName, sURL, iTIMEOUT, hlsURL);
		}
	} while (0);

	//构造MSG_SC_START_HLS_ACK响应报文
	EasyMsgSCStartHLSACK ack;
	ack.SetHeaderValue(EASY_TAG_VERSION, "1.0");
	if (strlen(hlsURL))
		ack.SetStreamURL(hlsURL);

	string msg = ack.GetMsg();
	StrPtrLen msgJson(const_cast<char*>(msg.c_str()));

	//构造响应报文(HTTP头)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(msgJson.Len ? httpOK : httpNotImplemented);
	if (msgJson.Len)
		httpAck.AppendContentLengthHeader(msgJson.Len);

	//响应完成后断开连接
	httpAck.AppendConnectionCloseHeader();

	//Push MSG to OutputBuffer
	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

	RTSPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (msgJson.Len > 0)
		pOutputStream->Put(msgJson.Ptr, msgJson.Len);

	return theErr;
}

QTSS_Error HTTPSession::ExecNetMsgGetHlsSessionsReq(char* queryString, char* json)
{
	QTSS_Error theErr = QTSS_NoErr;

	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	do
	{
		char* msgContent = static_cast<char*>(Easy_GetHLSessions());

		StrPtrLen msgJson(msgContent);

		//构造响应报文(HTTP头)
		HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
		httpAck.CreateResponseHeader(msgJson.Len ? httpOK : httpNotImplemented);
		if (msgJson.Len)
			httpAck.AppendContentLengthHeader(msgJson.Len);

		//响应完成后断开连接
		httpAck.AppendConnectionCloseHeader();

		//Push MSG to OutputBuffer
		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		RTSPResponseStream *pOutputStream = GetOutputStream();
		pOutputStream->Put(respHeader);
		if (msgJson.Len > 0)
			pOutputStream->Put(msgJson.Ptr, msgJson.Len);

		delete[] msgContent;
	} while (0);

	return theErr;
}

QTSS_Error HTTPSession::execNetMsgCSGetRTSPLiveSessionsRESTful(const char* queryString)
{	
	QTSS_Error theErr = QTSS_NoErr;

	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	do
	{
		// 获取响应Content
		char* msgContent = static_cast<char*>(Easy_GetRTSPPushSessions());

		StrPtrLen msgJson(msgContent);

		// 构造响应报文(HTTP头)
		HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
		httpAck.CreateResponseHeader(msgJson.Len ? httpOK : httpNotImplemented);
		if (msgJson.Len)
			httpAck.AppendContentLengthHeader(msgJson.Len);

		// 响应完成后断开连接
		httpAck.AppendConnectionCloseHeader();

		// HTTP响应Body
		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		// HTTP响应Content
		RTSPResponseStream *pOutputStream = GetOutputStream();
		pOutputStream->Put(respHeader);
		if (msgJson.Len > 0)
			pOutputStream->Put(msgJson.Ptr, msgJson.Len);

		delete[] msgContent;
	} while (false);

	return theErr;
}



QTSS_Error HTTPSession::execNetMsgCSGetRTSPRecordSessionsRESTful(const char* queryString)
{
	QTSS_Error theErr = QTSS_NoErr;

	string queryTemp;
	if (queryString != nullptr)
	{
		queryTemp = EasyUtil::Urldecode(queryString);
	}

	QueryParamList parList(const_cast<char *>(queryTemp.c_str()));
	const char* startTime = parList.DoFindCGIValueForParam(EASY_TAG_L_START_TIME);//username
	const char* name = parList.DoFindCGIValueForParam(EASY_TAG_L_NAME);//username
	const char* endTime = parList.DoFindCGIValueForParam(EASY_TAG_L_END_TIME);//username

	do
	{
		// 获取响应Content
		char* msgContent = static_cast<char*>(Easy_GetRTSPRecordSessions((char *)name, atoi(startTime), atoi(endTime)));

		StrPtrLen msgJson(msgContent);

		// 构造响应报文(HTTP头)
		HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
		httpAck.CreateResponseHeader(msgJson.Len ? httpOK : httpNotImplemented);
		if (msgJson.Len)
			httpAck.AppendContentLengthHeader(msgJson.Len);

		// 响应完成后断开连接
		httpAck.AppendConnectionCloseHeader();

		// HTTP响应Body
		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		// HTTP响应Content
		RTSPResponseStream *pOutputStream = GetOutputStream();
		pOutputStream->Put(respHeader);
		if (msgJson.Len > 0)
			pOutputStream->Put(msgJson.Ptr, msgJson.Len);

		delete[] msgContent;
	} while (false);

	return theErr;
}

QTSS_Error HTTPSession::execNetMsgCSGetServerVersionReqRESTful(const char* queryString)
{
	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	EasyProtocolACK rsp(MSG_SC_SERVER_INFO_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	char* serverHeader = nullptr;
	(void)QTSS_GetValueAsString(QTSServerInterface::GetServer(), qtssSvrRTSPServerHeader, 0, &serverHeader);
	QTSSCharArrayDeleter theFullPathStrDeleter(serverHeader);
	body[EASY_TAG_SERVER_HEADER] = serverHeader;

	SInt64 timeNow = OS::Milliseconds();
	SInt64 startupTime = 0;
	UInt32 startupTimeSize = sizeof(startupTime);
	(void)QTSS_GetValue(QTSServerInterface::GetServer(), qtssSvrStartupTime, 0, &startupTime, &startupTimeSize);
	SInt64 longstTime = (timeNow - startupTime) / 1000;

	unsigned int timeDays = longstTime / (24 * 60 * 60);
	unsigned int timeHours = (longstTime % (24 * 60 * 60)) / (60 * 60);
	unsigned int timeMins = ((longstTime % (24 * 60 * 60)) % (60 * 60)) / 60;
	unsigned int timeSecs = ((longstTime % (24 * 60 * 60)) % (60 * 60)) % 60;

	body[EASY_TAG_SERVER_RUNNING_TIME] = Format("%u Days %u Hours %u Mins %u Secs", timeDays, timeHours, timeMins, timeSecs);

	body[EASY_TAG_SERVER_HARDWARE] = "x86";
	body[EASY_TAG_SERVER_INTERFACE_VERSION] = "v1";

	rsp.SetHead(header);
	rsp.SetBody(body);

	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, true, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSLoginReqRESTful(const char* queryString)
{
	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	string queryTemp;
	if (queryString != nullptr)
	{
		queryTemp = EasyUtil::Urldecode(queryString);
	}

	QueryParamList parList(const_cast<char *>(queryTemp.c_str()));
	const char* chUserName = parList.DoFindCGIValueForParam(EASY_TAG_L_USER_NAME);//username
	const char* chPassword = parList.DoFindCGIValueForParam(EASY_TAG_L_PASSWORD);//password

	QTSS_Error theErr = QTSS_BadArgument;

	if (!chUserName || !chPassword)
	{
		return theErr;
	}

	//Authentication->token->redis->Platform use
	theErr = QTSS_NoErr;

	EasyProtocolACK rsp(MSG_SC_SERVER_LOGIN_ACK);
	EasyJsonValue header, body;

	body[EASY_TAG_TOKEN] = "EasyDarwinTestToken";

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	int errNo = theErr == QTSS_NoErr ? EASY_ERROR_SUCCESS_OK : EASY_ERROR_CLIENT_UNAUTHORIZED;
	header[EASY_TAG_ERROR_NUM] = errNo;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(errNo);

	rsp.SetHead(header);
	rsp.SetBody(body);

	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSLogoutReqRESTful(const char* queryString)
{
	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}
	//cookie->token->clear redis->Platform clear,need login again

	EasyProtocolACK rsp(MSG_SC_SERVER_LOGOUT_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	rsp.SetHead(header);
	rsp.SetBody(body);

	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSGetBaseConfigReqRESTful(const char* queryString)
{
	if(QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	EasyProtocolACK rsp(MSG_SC_SERVER_BASE_CONFIG_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	UInt16 port;
	UInt32 len = sizeof(UInt16);
	(void)QTSS_GetValue(QTSServerInterface::GetServer()->GetPrefs(), qtssPrefsRTSPPorts, 0, static_cast<void*>(&port), &len);
	body[EASY_TAG_CONFIG_RTSP_LAN_PORT] = to_string(port);

	char* lanip = nullptr;
	(void)QTSS_GetValueAsString(QTSServerInterface::GetServer(), qtssSvrDefaultIPAddrStr, 0, &lanip);
	QTSSCharArrayDeleter theWanIPStrDeleter(lanip);
	body[EASY_TAG_CONFIG_SERVICE_LAN_IP] = lanip;

	body[EASY_TAG_CONFIG_RTSP_WAN_PORT] = to_string(QTSServerInterface::GetServer()->GetPrefs()->GetRTSPWANPort());
	body[EASY_TAG_CONFIG_SERVICE_WAN_IP] = QTSServerInterface::GetServer()->GetPrefs()->GetServiceWANIP();

	body[EASY_TAG_CONFIG_NGINX_ROOT_FOLDER] = QTSServerInterface::GetServer()->GetPrefs()->GetNginxRootFolder();
	body[EASY_TAG_CONFIG_NGINX_WEB_PATH] = QTSServerInterface::GetServer()->GetPrefs()->GetNginxWebPath();
	body[EASY_TAG_CONFIG_NGINX_RTMP_PATH] = QTSServerInterface::GetServer()->GetPrefs()->GetNginxRTMPPath();

	body[EASY_TAG_CONFIG_SERVICE_LAN_PORT] = to_string(QTSServerInterface::GetServer()->GetPrefs()->GetServiceLanPort());
	body[EASY_TAG_CONFIG_SERVICE_WAN_PORT] = to_string(QTSServerInterface::GetServer()->GetPrefs()->GetServiceWanPort());

	rsp.SetHead(header);
	rsp.SetBody(body);

	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSSetBaseConfigReqRESTful(const char* queryString)
{
	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	string queryTemp;
	if (queryString)
	{
		queryTemp = EasyUtil::Urldecode(queryString);
	}
	QueryParamList parList(const_cast<char *>(queryTemp.c_str()));

	//1.EASY_TAG_CONFIG_RTSP_LAN_PORT
	const char* chRTSPLanPort = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_RTSP_LAN_PORT);
	if (chRTSPLanPort)
	{
		UInt16 uRTSPLanPort = stoi(chRTSPLanPort);
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), qtssPrefsRTSPPorts, 0, &uRTSPLanPort, sizeof(uRTSPLanPort));
	}

	//2.EASY_TAG_CONFIG_RTSP_WAN_PORT
	const char*	chRTSPWanPort = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_RTSP_WAN_PORT);
	if (chRTSPWanPort)
	{
		UInt16 uRTSPWanPort = stoi(chRTSPWanPort);
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), easyPrefsRTSPWANPort, 0, &uRTSPWanPort, sizeof(uRTSPWanPort));
	}

	//3.EASY_TAG_CONFIG_SERVICE_LAN_IP
	//const char* chLanIP = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_SERVICE_LAN_IP);
	//if (chLanIP)
	//	(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), qtssPrefsRTSPIPAddr, 0, (void*)chLanIP, strlen(chLanIP));

	//4.EASY_TAG_CONFIG_SERVICE_WAN_IP
	const char* chWanIP = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_SERVICE_WAN_IP);
	if (chWanIP)
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), easyPrefsServiceWANIPAddr, 0, (void*)chWanIP, strlen(chWanIP));

	//5.EASY_TAG_CONFIG_NGINX_ROOT_FOLDER
	const char* chNginxRootFolder = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_NGINX_ROOT_FOLDER);
	if (chNginxRootFolder)
	{
		string nginxRootFolder(chNginxRootFolder);
		if (nginxRootFolder.back() != '\\')
		{
			nginxRootFolder.push_back('\\');
		}
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), qtssPrefsNginxRootFolder, 0, (void*)nginxRootFolder.c_str(), nginxRootFolder.size());
	}

	//6.EASY_TAG_CONFIG_NGINX_WEB_PATH
	const char* chNginxWebPath = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_NGINX_WEB_PATH);
	if (chNginxWebPath)
	{
		string nginxWebPath(chNginxWebPath);
		if (nginxWebPath.back() != '\/')
		{
			nginxWebPath.push_back('\/');
		}
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), easyPrefsNginxWebPath, 0, (void*)nginxWebPath.c_str(), nginxWebPath.size());
	}

	//7.EASY_TAG_CONFIG_NGINX_RTMP_PATH
	const char* chNginxRTMPPath = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_NGINX_RTMP_PATH);
	if (chNginxRTMPPath)
	{
		string nginxRTMPPath(chNginxRTMPPath);
		if (nginxRTMPPath.back() != '\/')
		{
			nginxRTMPPath.push_back('\/');
		}
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), easyPrefsNginxRTMPPath, 0, (void*)nginxRTMPPath.c_str(), nginxRTMPPath.size());
	}

	//8.EASY_TAG_CONFIG_SERVICE_LAN_PORT
	const char* chHTTPLanPort = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_SERVICE_LAN_PORT);
	if (chHTTPLanPort)
	{
		UInt16 uHTTPLanPort = stoi(chHTTPLanPort);
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), easyPrefsHTTPServiceLanPort, 0, &uHTTPLanPort, sizeof(uHTTPLanPort));
	}

	//9.EASY_TAG_CONFIG_SERVICE_WAN_PORT
	const char*	chHTTPWanPort = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_SERVICE_WAN_PORT);
	if (chHTTPWanPort)
	{
		UInt16 uHTTPWanPort = stoi(chHTTPWanPort);
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), easyPrefsHTTPServiceWanPort, 0, &uHTTPWanPort, sizeof(uHTTPWanPort));
	}

	EasyProtocolACK rsp(MSG_SC_SERVER_SET_BASE_CONFIG_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	rsp.SetHead(header);
	rsp.SetBody(body);

	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSRestartServiceRESTful(const char* queryString) const
{
	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	exit(-2);
}

QTSS_Error HTTPSession::execNetMsgCSGetDeviceStreamReqRESTful(const char* queryString)
{
	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	string queryTemp;
	if (queryString)
	{
		queryTemp = EasyUtil::Urldecode(queryString);
	}
	QueryParamList parList(const_cast<char *>(queryTemp.c_str()));

	const char* chSerial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);
	const char* chProtocol = parList.DoFindCGIValueForParam(EASY_TAG_L_PROTOCOL);
	//const char* chReserve = parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE);

	UInt32 theChannelNum = 1;
	EasyStreamType streamType = easyIllegalStreamType;

	char* outURL = new char[QTSS_MAX_URL_LENGTH];
	outURL[0] = '\0';
	QTSSCharArrayDeleter theHLSURLDeleter(outURL);

	int theErr = EASY_ERROR_SERVER_NOT_IMPLEMENTED;

	do
	{
		if (!chSerial)
		{
			theErr = EASY_ERROR_NOT_FOUND;
			break;
		}

		const char* chChannel = parList.DoFindCGIValueForParam(EASY_TAG_CHANNEL);
		if (chChannel)
		{
			theChannelNum = stoi(chChannel);
		}

		if (!chProtocol)
		{
			theErr = EASY_ERROR_CLIENT_BAD_REQUEST;
			break;
		}

		StrPtrLen chProtocolPtr(const_cast<char*>(chProtocol));
		streamType = HTTPProtocol::GetStreamType(&chProtocolPtr);

		if (streamType == easyIllegalStreamType)
		{
			theErr = EASY_ERROR_CLIENT_BAD_REQUEST;
			break;
		}

		QTSS_RoleParams params;
		params.easyGetDeviceStreamParams.inDevice = const_cast<char*>(chSerial);
		params.easyGetDeviceStreamParams.inChannel = theChannelNum;
		params.easyGetDeviceStreamParams.inStreamType = streamType;
		params.easyGetDeviceStreamParams.outUrl = outURL;


		UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kGetDeviceStreamRole);
		for (UInt32 fCurrentModule = 0; fCurrentModule < numModules; fCurrentModule++)
		{
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kGetDeviceStreamRole, fCurrentModule);
			QTSS_Error exeErr = theModule->CallDispatch(Easy_GetDeviceStream_Role, &params);
			if (exeErr == QTSS_NoErr)
			{
				theErr = EASY_ERROR_SUCCESS_OK;
				break;
			}
		}


	} while (false);


	EasyProtocolACK rsp(MSG_SC_GET_STREAM_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = theErr;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(theErr);

	if (theErr == EASY_ERROR_SUCCESS_OK)
	{
		body[EASY_TAG_URL] = outURL;
		body[EASY_TAG_PROTOCOL] = HTTPProtocol::GetStreamTypeStream(streamType)->Ptr;
	}

	rsp.SetHead(header);
	rsp.SetBody(body);

	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSLiveDeviceStreamReqRESTful(const char * queryString)
{
	if (QTSServerInterface::GetServer()->GetPrefs()->CloudPlatformEnabled())
	{
		//printf("if cloud platform enabled,we will check platform login token");
		auto cokieTemp = fRequest->GetHeaderValue(httpCookieHeader);
		//if (!hasLogin(cokieTemp))
		//{
		//	return EASY_ERROR_CLIENT_UNAUTHORIZED;
		//}
	}

	string queryTemp;
	if (queryString)
	{
		queryTemp = EasyUtil::Urldecode(queryString);
	}
	QueryParamList parList(const_cast<char *>(queryTemp.c_str()));

	const char* chSerial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);
	const char* chProtocol = parList.DoFindCGIValueForParam(EASY_TAG_L_PROTOCOL);
	//const char* chReserve = parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE);

	UInt32 theChannelNum = 0;
	EasyStreamType streamType = easyIllegalStreamType;

	int theErr = EASY_ERROR_SERVER_NOT_IMPLEMENTED;

	do
	{
		if (!chSerial)
		{
			theErr = EASY_ERROR_NOT_FOUND;
			break;
		}

		const char* chChannel = parList.DoFindCGIValueForParam(EASY_TAG_CHANNEL);
		if (chChannel)
		{
			theChannelNum = stoi(chChannel);
		}

		if (!chProtocol)
		{
			theErr = EASY_ERROR_CLIENT_BAD_REQUEST;
			break;
		}

		StrPtrLen chProtocolPtr(const_cast<char*>(chProtocol));
		streamType = HTTPProtocol::GetStreamType(&chProtocolPtr);

		if (streamType == easyIllegalStreamType)
		{
			theErr = EASY_ERROR_CLIENT_BAD_REQUEST;
			break;
		}

		QTSS_RoleParams params;
		params.easyGetDeviceStreamParams.inDevice = const_cast<char*>(chSerial);
		params.easyGetDeviceStreamParams.inChannel = theChannelNum;
		params.easyGetDeviceStreamParams.inStreamType = streamType;
		params.easyGetDeviceStreamParams.outUrl = nullptr;

		UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kLiveDeviceStreamRole);
		for (UInt32 fCurrentModule = 0; fCurrentModule < numModules; fCurrentModule++)
		{
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kLiveDeviceStreamRole, fCurrentModule);
			QTSS_Error exeErr = theModule->CallDispatch(Easy_LiveDeviceStream_Role, &params);
			if (exeErr == QTSS_NoErr)
			{
				theErr = EASY_ERROR_SUCCESS_OK;
				break;
			}
		}
	} while (false);

	EasyProtocolACK rsp(MSG_SC_GET_STREAM_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = theErr;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(theErr);

	rsp.SetHead(header);
	rsp.SetBody(body);

	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}
