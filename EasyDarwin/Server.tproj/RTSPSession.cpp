/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 */
 /*
	 File:       RTSPSession.cpp

	 Contains:   Implemenation of RTSPSession objects
 */
#define __RTSP_HTTP_DEBUG__ 0
#define __RTSP_HTTP_VERBOSE__ 0
#define __RTSP_AUTH_DEBUG__ 0
#define debug_printf if (__RTSP_AUTH_DEBUG__) qtss_printf

#include "RTSPSession.h"
#include "RTSPRequest.h"
#include "QTSServerInterface.h"
#include "RTSPRequest3GPP.h"

#include "MyAssert.h"
#include "OSMemory.h"

#include "QTSS.h"
#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"
#include "md5digest.h"
#include "QTSSDataConverter.h"

#if __FreeBSD__ || __hpux__	
#include <unistd.h>
#endif

#include <errno.h>

#if __solaris__ || __linux__ || __sgi__	|| __hpux__
#include <crypt.h>
#endif

#if __RTSP_HTTP_DEBUG__
#define HTTP_TRACE(s) qtss_printf(s);
#define HTTP_TRACE_SPL(s) PrintfStrPtrLen(s);
#define HTTP_TRACE_ONE(s, one ) qtss_printf(s, one);
#define HTTP_TRACE_TWO(s, one, two ) qtss_printf(s, one, two);
#else
#define HTTP_TRACE(s);
#define HTTP_TRACE_SPL(s);
#define HTTP_TRACE_ONE(s, one );
#define HTTP_TRACE_TWO(s, one, two );
#endif

#if __RTSP_HTTP_VERBOSE__
#define HTTP_VTRACE(s) qtss_printf(s);
#define HTTP_VTRACE_SPL(s) PrintfStrPtrLen(s);
#define HTTP_VTRACE_ONE(s, one ) qtss_printf(s, one);
#define HTTP_VTRACE_TWO(s, one, two ) qtss_printf(s, one, two);
#else
#define HTTP_VTRACE(s);
#define HTTP_VTRACE_SPL(s);
#define HTTP_VTRACE_ONE(s, one );
#define HTTP_VTRACE_TWO(s, one, two );
#endif

#if  __RTSP_HTTP_DEBUG__ || __RTSP_HTTP_VERBOSE__
static void PrintfStrPtrLen(StrPtrLen *splRequest)
{
	char    buff[1024];

	memcpy(buff, splRequest->Ptr, splRequest->Len);

	buff[splRequest->Len] = 0;

	HTTP_TRACE_ONE("%s\n", buff)
		//qtss_printf( "%s\n", buff );
}
#endif

// hack stuff
static char*                    sBroadcasterSessionName = "QTSSReflectorModuleBroadcasterSession";
static QTSS_AttributeID         sClientBroadcastSessionAttr = qtssIllegalAttrID;

static StrPtrLen    sVideoStr("video");
static StrPtrLen    sAudioStr("audio");
static StrPtrLen    sRtpMapStr("rtpmap");
static StrPtrLen    sControlStr("control");
static StrPtrLen    sBufferDelayStr("x-bufferdelay");
static StrPtrLen    sContentType("application/x-random-data");

static StrPtrLen    sAuthAlgorithm("md5");
static StrPtrLen    sAuthQop("auth");
static StrPtrLen    sEmptyStr("");

// static class member  initialized in RTSPSession ctor
OSRefTable* RTSPSession::sHTTPProxyTunnelMap = NULL;

char        RTSPSession::sHTTPResponseHeaderBuf[kMaxHTTPResponseLen];
StrPtrLen   RTSPSession::sHTTPResponseHeaderPtr(sHTTPResponseHeaderBuf, kMaxHTTPResponseLen);

char        RTSPSession::sHTTPResponseNoServerHeaderBuf[kMaxHTTPResponseLen];
StrPtrLen   RTSPSession::sHTTPResponseNoServerHeaderPtr(sHTTPResponseNoServerHeaderBuf, kMaxHTTPResponseLen);

// stock reponse with place holder for server header and optional "x-server-ip-address" header ( %s%s%s for  "x-server-ip-address" + ip address + \r\n )
// the optional version must be generated at runtime to include a valid IP address for the actual interface
char*       RTSPSession::sHTTPResponseFormatStr = "HTTP/1.0 200 OK\r\n%s%s%s%s\r\nConnection: close\r\nDate: Thu, 19 Aug 1982 18:30:00 GMT\r\nCache-Control: no-store\r\nPragma: no-cache\r\nContent-Type: application/x-rtsp-tunnelled\r\n\r\n";
char*       RTSPSession::sHTTPNoServerResponseFormatStr = "HTTP/1.0 200 OK\r\n%s%s%s%sConnection: close\r\nDate: Thu, 19 Aug 1982 18:30:00 GMT\r\nCache-Control: no-store\r\nPragma: no-cache\r\nContent-Type: application/x-rtsp-tunnelled\r\n\r\n";

// RTSPSession全局初始化,在QTSServer中调用一次
void RTSPSession::Initialize()
{
	sHTTPProxyTunnelMap = new OSRefTable(OSRefTable::kDefaultTableSize);

	// Construct premade HTTP response for HTTP proxy tunnel
	qtss_sprintf(sHTTPResponseHeaderBuf, sHTTPResponseFormatStr, "", "", "", QTSServerInterface::GetServerHeader().Ptr);
	sHTTPResponseHeaderPtr.Len = ::strlen(sHTTPResponseHeaderBuf);
	Assert(sHTTPResponseHeaderPtr.Len < kMaxHTTPResponseLen);

	qtss_sprintf(sHTTPResponseNoServerHeaderBuf, sHTTPNoServerResponseFormatStr, "", "", "", "");
	sHTTPResponseNoServerHeaderPtr.Len = ::strlen(sHTTPResponseNoServerHeaderBuf);
	Assert(sHTTPResponseNoServerHeaderPtr.Len < kMaxHTTPResponseLen);
}

RTSPSession::RTSPSession(bool doReportHTTPConnectionAddress)
	: RTSPSessionInterface(),
	fRequest(NULL),
	fRTPSession(NULL),
    fRTSPSessionHandler(NULL),
	fReadMutex(),
	fHTTPMethod(kHTTPMethodInit),
	fWasHTTPRequest(false),
	fFoundValidAccept(false),
	fDoReportHTTPConnectionAddress(doReportHTTPConnectionAddress),
	fCurrentModule(0),
	fState(kReadingFirstRequest),
    fMsgCount(0)
{
	this->SetTaskName("RTSPSession");

	// must guarantee this map is present
	Assert(sHTTPProxyTunnelMap != NULL);

	QTSServerInterface::GetServer()->AlterCurrentRTSPSessionCount(1);

	// Setup the QTSS param block, as none of these fields will change through the course of this session.
	fRoleParams.rtspRequestParams.inRTSPSession = this;
	fRoleParams.rtspRequestParams.inRTSPRequest = NULL;
	fRoleParams.rtspRequestParams.inClientSession = NULL;

	fModuleState.curModule = NULL;
	fModuleState.curTask = this;
	fModuleState.curRole = 0;
	fModuleState.globalLockRequested = false;

	fProxySessionID[0] = 0;
	fProxySessionIDPtr.Set(fProxySessionID, 0);

	fLastRTPSessionID[0] = 0;
	fLastRTPSessionIDPtr.Set(fLastRTPSessionID, 0);
	Assert(fLastRTPSessionIDPtr.Ptr == &fLastRTPSessionID[0]);

	(void)QTSS_IDForAttr(qtssClientSessionObjectType, sBroadcasterSessionName, &sClientBroadcastSessionAttr);

    fRTSPSessionHandler = NEW RTSPSessionHandler(this);
}

RTSPSession::~RTSPSession()
{
	// Invoke the session closing modules
	QTSS_RoleParams theParams;
	theParams.rtspSessionClosingParams.inRTSPSession = this;

	// Invoke modules
	for (UInt32 x = 0; x < QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPSessionClosingRole); x++)
		(void)QTSServerInterface::GetModule(QTSSModule::kRTSPSessionClosingRole, x)->CallDispatch(QTSS_RTSPSessionClosing_Role, &theParams);

	fLiveSession = false; //used in Clean up request to remove the RTP session.
	this->CleanupRequest();// Make sure that all our objects are deleted
	if (fSessionType == qtssRTSPSession)
		QTSServerInterface::GetServer()->AlterCurrentRTSPSessionCount(-1);
	else
		QTSServerInterface::GetServer()->AlterCurrentRTSPHTTPSessionCount(-1);

	if (*fProxySessionID != '\0')
	{
#if DEBUG
		char * str = "???";

		if (fSessionType == qtssRTSPHTTPInputSession)
			str = "input session";
		else if (fSessionType == qtssRTSPHTTPSession)
			str = "input session";

		HTTP_VTRACE_TWO("~RTSPSession, was a fProxySessionID (%s), %s\n", fProxySessionID, str)
#endif      
			sHTTPProxyTunnelMap->UnRegister(&fProxyRef);
	}
	if (fRequest)
	{
		delete fRequest;
		fRequest = NULL;
	}
}

/*
*	函数名：Run()
*	功能：状态机处理RTSP请求中的各种报文
*/
SInt64 RTSPSession::Run()
{
	EventFlags events = this->GetEvents();
	QTSS_Error err = QTSS_NoErr;
	QTSSModule* theModule = NULL;
	UInt32 numModules = 0;
	Assert(fLastRTPSessionIDPtr.Ptr == &fLastRTPSessionID[0]);

	// Some callbacks look for this struct in the thread object
	OSThreadDataSetter theSetter(&fModuleState, NULL);

	//check for a timeout or a kill. If so, just consider the session dead
	// 检查超时事件和Kill事件，如果为真，则RTSPSession会话已结束
	if ((events & Task::kTimeoutEvent) || (events & Task::kKillEvent))
		fLiveSession = false;

	while (this->IsLiveSession())
	{
		// RTSP Session state machine. There are several well defined points in an RTSP request
		// where this session may have to return from its run function and wait for a new event.
		// Because of this, we need to track our current state and return to it.

		switch (fState)
		{
		case kReadingFirstRequest:// 第一次对请求报文进行处理
			{
				if ((err = fInputStream.ReadRequest()) == QTSS_NoErr)
				{
					// If the RequestStream returns QTSS_NoErr, it means
					// that we've read all outstanding data off the socket,
					// and still don't have a full request. Wait for more data.

					// QTSS_NoErr：已经收到该socket的完整数据，但是没有收集到本次请求的全部数据
					// 这时需要接着请求监听读事件，获取更多数据组成完整的RTSP请求消息。

					//+rt use the socket that reads the data, may be different now.
					fInputSocketP->RequestEvent(EV_RE);
					return 0;
				}

				// QTSS_RequestArrived：接收到完整请求报文，可以进入下一状态
				// E2BIG：缓冲区已溢出，默认缓存区的大小为（ kRequestBufferSizeInBytes = 4096）
				if ((err != QTSS_RequestArrived) && (err != E2BIG))
				{
					// Any other error implies that the client has gone away. At this point,
					// we can't have 2 sockets, so we don't need to do the "half closed" check
					// we do below
					Assert(err > 0);
					Assert(!this->IsLiveSession());
					break;
				}

				// 此时收到完整报文
				if (err == QTSS_RequestArrived)
					fState = kHTTPFilteringRequest;
				// If we get an E2BIG, it means our buffer was overfilled.
				// In that case, we can just jump into the following state, and
				// the code their does a check for this error and returns an error.
				if (err == E2BIG)
					// 进入kHaveNonTunnelMessage状态机，再响应该E2BIG错误
					fState = kHaveNonTunnelMessage;
			}
			continue;

		case kHTTPFilteringRequest://对报文内容进行过滤
			{

				HTTP_TRACE("RTSPSession::Run kHTTPFilteringRequest\n")

					// 初始化状态为非tunnel消息
					fState = kHaveNonTunnelMessage; // assume it's not a tunnel setup message
													// prefilter will set correct tunnel state if it is.

					// 检测是否为RTSP-over-HTTP tunneling
				QTSS_Error  preFilterErr = this->PreFilterForHTTPProxyTunnel();

				// 判断preFilterErr值并输出打印信息
				if (preFilterErr == QTSS_NoErr)
				{
					HTTP_TRACE("RTSPSession::Run kHTTPFilteringRequest\n")
						continue;
				}
				else
				{
					// pre filter error indicates a tunnelling message that could 
					// not join to a session.
					HTTP_TRACE("RTSPSession::Run kHTTPFilteringRequest Tunnel protocol ERROR.\n")
						return -1;

				}
			}

		case kWaitingToBindHTTPTunnel:
			// flush the GET response, if it's there
			err = fOutputStream.Flush();
			if (err == EAGAIN)
			{
				// If we get this error, we are currently flow-controlled and should
				// wait for the socket to become writeable again
				fSocket.RequestEvent(EV_WR);
			}
			return 0;
			//continue;

		case kSocketHasBeenBoundIntoHTTPTunnel:// RTSP-over-HTTP状态处理流程

			// DMS - Can this execute either? I don't think so... this one
			// we may not need...

			// I've been joined, it's time to kill this session.
			Assert(!this->IsLiveSession()); // at least the socket should not report connected any longer
			HTTP_TRACE("RTSPSession has died of snarfage.\n")
				break;


		case kReadingRequest:
			{
				// We should lock down the session while reading in data,
				// because we can't snarf up a POST while reading.
				OSMutexLocker readMutexLocker(&fReadMutex);

				// we should be only reading an RTSP request here, no HTTP tunnel messages

				if ((err = fInputStream.ReadRequest()) == QTSS_NoErr)
				{
					// If the RequestStream returns QTSS_NoErr, it means
					// that we've read all outstanding data off the socket,
					// and still don't have a full request. Wait for more data.

					//+rt use the socket that reads the data, may be different now.
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
				fState = kHaveNonTunnelMessage;
				// fall thru to kHaveNonTunnelMessage
			}

		case kHaveNonTunnelMessage:// 说明请求报文格式是正确的，请求已进入受理状态
			{
				// should only get here when fInputStream has a full message built.

				Assert(fInputStream.GetRequestBuffer());

				// 创建RTSPRequest对象，用于解析RTSP消息
				if (fRequest == NULL)
				{
					//                    printf("RTSPRequest size########## %d\n",sizeof(RTSPRequest));
					fRequest = NEW RTSPRequest(this);
				}
				fRequest->ReInit(this);

				fRoleParams.rtspRequestParams.inRTSPRequest = fRequest;
				fRoleParams.rtspRequestParams.inRTSPHeaders = fRequest->GetHeaderDictionary();

				// We have an RTSP request and are about to begin processing. We need to
				// make sure that anyone sending interleaved data on this session won't
				// be allowed to do so until we are done sending our response
				// We also make sure that a POST session can't snarf in while we're
				// processing the request.
				// 开始处理RTSP请求，禁止在发送RTSP响应报文时，其他人发送interleaved数据
				// 同时在处理请求时，禁止POST会话
				fReadMutex.Lock();
				fSessionMutex.Lock();

				// The fOutputStream's fBytesWritten counter is used to
				// count the # of bytes for this RTSP response. So, at
				// this point, reset it to 0 (we can then just let it increment
				// until the next request comes in)
				// 重置响应缓冲区，并进入下一状态kFilteringRequest
				fOutputStream.ResetBytesWritten();

				// Check for an overfilled buffer, and return an error.
				// 检测缓冲区溢出错误
				if (err == E2BIG)
				{
					(void)QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientBadRequest,
						qtssMsgRequestTooLong);
					fState = kPostProcessingRequest;
					break;
				}
				// Check for a corrupt base64 error, return an error
				// 检测参数错误
				if (err == QTSS_BadArgument)
				{
					(void)QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientBadRequest,
						qtssMsgBadBase64);
					fState = kPostProcessingRequest;
					break;
				}

				Assert(err == QTSS_RequestArrived);
				// 状态机跳转到kFilteringRequest
				fState = kFilteringRequest;

				// Note that there is no break here. We'd like to continue onto the next
				// state at this point. This goes for every case in this case statement
			}

		case kFilteringRequest:
			{
				// We received something so auto refresh
				// The need to auto refresh is because the api doesn't allow a module to refresh at this point
				// 

				// 刷新超时任务fTimeoutTask，运转RTSP会话的超时机制
				fTimeoutTask.RefreshTimeout();

				//
				// Before we even do this, check to see if this is a *data* packet,
				// in which case this isn't an RTSP request, so we don't need to go
				// through any of the remaining steps

				// 判断当前数据是否是一个数据包，而不是一个RTSP请求
				if (fInputStream.IsDataPacket()) // can this interfere with MP3?
				{
                    if(fRTSPSessionHandler)
                    {
                        RTSPMsg* theMsg = fRTSPSessionHandler->GetMsg();
                        if(theMsg)
                        {
                            theMsg->fMsgCountID = this->fMsgCount++;
                            theMsg->SetMsgData(fInputStream.GetRequestBuffer()->Ptr, fInputStream.GetRequestBuffer()->Len);
                            fRTSPSessionHandler->ProcessMsg(OS::Milliseconds(), theMsg);
                        }
                    }
                    else
                    {
                        this->HandleIncomingDataPacket();
                    }

					fState = kCleaningUp;
					break;
				}


				//
				// In case a module wants to replace the request
				char* theReplacedRequest = NULL;
				char* oldReplacedRequest = NULL;

				// Setup the filter param block
				QTSS_RoleParams theFilterParams;
				theFilterParams.rtspFilterParams.inRTSPSession = this;
				theFilterParams.rtspFilterParams.inRTSPRequest = fRequest;
				theFilterParams.rtspFilterParams.outNewRequest = &theReplacedRequest;

				// Invoke filter modules
				// 获取注册kRTSPFilterRole角色的模块数量
				numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPFilterRole);
				for (; (fCurrentModule < numModules) && ((!fRequest->HasResponseBeenSent()) || fModuleState.eventRequested); fCurrentModule++)
				{
					fModuleState.eventRequested = false;
					fModuleState.idleTime = 0;
					if (fModuleState.globalLockRequested)
					{
						fModuleState.globalLockRequested = false;
						fModuleState.isGlobalLocked = true;
					}

					// 获取所有注册RTSP Filter角色的模块并调用，并将RTSP请求对象（theFilterParams）作为参数传递给它
					theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPFilterRole, fCurrentModule);
					(void)theModule->CallDispatch(QTSS_RTSPFilter_Role, &theFilterParams);
					fModuleState.isGlobalLocked = false;

					// If this module has requested an event, return and wait for the event to transpire
					if (fModuleState.globalLockRequested) // call this request back locked
						return this->CallLocked();

					if (fModuleState.eventRequested)
					{
						this->ForceSameThread();    // We are holding mutexes, so we need to force
													// the same thread to be used for next Run()
						return fModuleState.idleTime; // If the module has requested idle time...
					}

					//
					// Check to see if this module has replaced the request. If so, check
					// to see if there is an old replacement that we should delete
					if (theReplacedRequest != NULL)
					{
						if (oldReplacedRequest != NULL)
							delete[] oldReplacedRequest;

						// 改变qtssRTSPReqFullRequest属性的值
						fRequest->SetVal(qtssRTSPReqFullRequest, theReplacedRequest, ::strlen(theReplacedRequest));
						oldReplacedRequest = theReplacedRequest;
						theReplacedRequest = NULL;
					}
				}

				fCurrentModule = 0;
				if (fRequest->HasResponseBeenSent())
				{
					fState = kPostProcessingRequest;
					break;
				}

				if (fSentOptionsRequest && this->ParseOptionsResponse())
				{
					fRoundTripTime = (SInt32)(OS::Milliseconds() - fOptionsRequestSendTime);
					//qtss_printf("RTSPSession::Run RTT time = %" _S32BITARG_ " msec\n", fRoundTripTime);
					fState = kSendingResponse;
					break;
				}
				else
					// Otherwise, this is a normal request, so parse it and get the RTPSession.
					this->SetupRequest();// 普通请求，创建RTPSession会话

				// This might happen if there is some syntax or other error,
				// or if it is an OPTIONS request
				// 服务器根据被调用的模块是否对请求做了应答来决定后面的调用
				if (fRequest->HasResponseBeenSent())
				{
					fState = kPostProcessingRequest;
					break;
				}
				fState = kRoutingRequest;// 转到kRoutingRequest状态
			}
		case kRoutingRequest:
			{
				// Invoke router modules
				// 所有注册了kRTSPRouteRole角色的模块数量
				numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPRouteRole);
				{
					// Manipulation of the RTPSession from the point of view of
					// a module is guaranteed to be atomic by the API.
					Assert(fRTPSession != NULL);
					OSMutexLocker   locker(fRTPSession->GetSessionMutex());

					for (; (fCurrentModule < numModules) && ((!fRequest->HasResponseBeenSent()) || fModuleState.eventRequested); fCurrentModule++)
					{
						fModuleState.eventRequested = false;
						fModuleState.idleTime = 0;
						if (fModuleState.globalLockRequested)
						{
							fModuleState.globalLockRequested = false;
							fModuleState.isGlobalLocked = true;
						}

						// 调用所有注册了RTSP Route角色的模块
						theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPRouteRole, fCurrentModule);
						(void)theModule->CallDispatch(QTSS_RTSPRoute_Role, &fRoleParams);
						fModuleState.isGlobalLocked = false;

						if (fModuleState.globalLockRequested) // call this request back locked
							return this->CallLocked();

						// If this module has requested an event, return and wait for the event to transpire
						if (fModuleState.eventRequested)
						{
							this->ForceSameThread();    // We are holding mutexes, so we need to force
														// the same thread to be used for next Run()
							return fModuleState.idleTime; // If the module has requested idle time...
						}
					}
				}
				fCurrentModule = 0;

				// SetupAuthLocalPath must happen after kRoutingRequest and before kAuthenticatingRequest
				// placed here so that if the state is shifted to kPostProcessingRequest from a response being sent
				// then the AuthLocalPath will still be set.
				fRequest->SetupAuthLocalPath();

				if (fRequest->HasResponseBeenSent())
				{
					fState = kPostProcessingRequest;
					break;
				}

				if (fRequest->SkipAuthorization())
				{
					// Skip the authentication and authorization states

					// The foll. normally gets executed at the end of the authorization state 
					// Prepare for kPreprocessingRequest state.
					fState = kPreprocessingRequest;

					if (fRequest->GetMethod() == qtssSetupMethod)
						// Make sure to erase the session ID stored in the request at this point.
						// If we fail to do so, this same session would be used if another
						// SETUP was issued on this same TCP connection.
						fLastRTPSessionIDPtr.Len = 0;
					else if (fLastRTPSessionIDPtr.Len == 0)
						fLastRTPSessionIDPtr.Len = ::strlen(fLastRTPSessionIDPtr.Ptr);

					break;
				}
				else
					fState = kAuthenticatingRequest;
			}

		case kAuthenticatingRequest:
			{
				bool      allowedDefault = QTSServerInterface::GetServer()->GetPrefs()->GetAllowGuestDefault();
				bool      allowed = allowedDefault; //server pref?
				bool      hasUser = false;
				bool      handled = false;
				bool      wasHandled = false;

				StrPtrLenDel prefRealm(QTSServerInterface::GetServer()->GetPrefs()->GetAuthorizationRealm());
				if (prefRealm.Ptr != NULL)
				{
					fRequest->SetValue(qtssRTSPReqURLRealm, 0, prefRealm.Ptr, prefRealm.Len, kDontObeyReadOnly);
				}


				QTSS_RTSPMethod method = fRequest->GetMethod();
				if (method != qtssIllegalMethod) do
				{   //Set the request action before calling the authentication module

					if ((method == qtssAnnounceMethod) || ((method == qtssSetupMethod) && fRequest->IsPushRequest()))
					{
						fRequest->SetAction(qtssActionFlagsWrite);
						break;
					}

					void* theSession = NULL;
					UInt32 theLen = sizeof(theSession);
					if (QTSS_NoErr == fRTPSession->GetValue(sClientBroadcastSessionAttr, 0, &theSession, &theLen))
					{
						fRequest->SetAction(qtssActionFlagsWrite); // an incoming broadcast session
						break;
					}

					fRequest->SetAction(qtssActionFlagsRead);
				} while (false);
				else
				{
					Assert(0);
				}

				if (fRequest->GetAuthScheme() == qtssAuthNone)
				{
					QTSS_AuthScheme scheme = QTSServerInterface::GetServer()->GetPrefs()->GetAuthScheme();
					if (scheme == qtssAuthBasic)
						fRequest->SetAuthScheme(qtssAuthBasic);
					else if (scheme == qtssAuthDigest)
						fRequest->SetAuthScheme(qtssAuthDigest);

					if (scheme == qtssAuthDigest)
						debug_printf("RTSPSession.cpp:kAuthenticatingRequest  scheme == qtssAuthDigest\n");
				}

				// Setup the authentication param block
				QTSS_RoleParams theAuthenticationParams;
				theAuthenticationParams.rtspAthnParams.inRTSPRequest = fRequest;

				fModuleState.eventRequested = false;
				fModuleState.idleTime = 0;

				fRequest->SetAllowed(allowed);
				fRequest->SetHasUser(hasUser);
				fRequest->SetAuthHandled(handled);

				StrPtrLen* lastUsedDigestChallengePtr = this->GetValue(qtssRTSPSesLastDigestChallenge);
				if (lastUsedDigestChallengePtr != NULL)
					(void) fRequest->SetValue(qtssRTSPReqDigestChallenge, (UInt32)0, (void *)lastUsedDigestChallengePtr->Ptr, lastUsedDigestChallengePtr->Len, QTSSDictionary::kDontObeyReadOnly);


				numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPAthnRole);
				for (fCurrentModule = 0; (fCurrentModule < numModules) && ((!fRequest->HasResponseBeenSent()) || fModuleState.eventRequested); fCurrentModule++)
				{

					fRequest->SetAllowed(allowedDefault);
					fRequest->SetHasUser(false);
					fRequest->SetAuthHandled(false);
					debug_printf("RTSPSession.cpp:kAuthenticatingRequest  fCurrentModule = %lu numModules=%lu\n", fCurrentModule, numModules);


					fModuleState.eventRequested = false;
					fModuleState.idleTime = 0;
					if (fModuleState.globalLockRequested)
					{
						fModuleState.globalLockRequested = false;
						fModuleState.isGlobalLocked = true;
					}


					theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPAthnRole, fCurrentModule);
					if (NULL == theModule)
						continue;

					if (__RTSP_AUTH_DEBUG__)
					{
						theModule->GetValue(qtssModName)->PrintStr("QTSSModule::CallDispatch ENTER module=", "\n");
					}

					(void)theModule->CallDispatch(QTSS_RTSPAuthenticate_Role, &theAuthenticationParams);
					fModuleState.isGlobalLocked = false;

					if (fModuleState.globalLockRequested) // call this request back locked
						return this->CallLocked();

					// If this module has requested an event, return and wait for the event to transpire
					if (fModuleState.eventRequested)
					{
						this->ForceSameThread();    // We are holding mutexes, so we need to force
													// the same thread to be used for next Run()
						return fModuleState.idleTime; // If the module has requested idle time...
					}

					allowed = fRequest->GetAllowed();
					hasUser = fRequest->GetHasUser();
					handled = fRequest->GetAuthHandled();
					debug_printf("RTSPSession::Run Role(kAuthenticatingRequest) allowedDefault =%d allowed= %d hasUser = %d handled=%d \n", allowedDefault, allowed, hasUser, handled);
					if (handled)
						wasHandled = handled;

					if (hasUser || handled)
					{
						debug_printf("RTSPSession.cpp::Run(kAuthenticatingRequest)  skipping other modules fCurrentModule = %lu numModules=%lu\n", fCurrentModule, numModules);
						break;
					}

				}

				if (!wasHandled) //don't check and possibly fail the user if it the user has already been checked.
					this->CheckAuthentication();

				fCurrentModule = 0;
				if (fRequest->HasResponseBeenSent())
				{
					fState = kPostProcessingRequest;
					break;
				}
				fState = kAuthorizingRequest;
			}
		case kAuthorizingRequest:
			{
				// Invoke authorization modules
				numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPAuthRole);
				bool      allowedDefault = QTSServerInterface::GetServer()->GetPrefs()->GetAllowGuestDefault();
				bool      allowed = true;
				bool      hasUser = false;
				bool      handled = false;
				QTSS_Error  theErr = QTSS_NoErr;

				// Invoke authorization modules

				// Manipulation of the RTPSession from the point of view of
				// a module is guaranteed to be atomic by the API.
				Assert(fRTPSession != NULL);
				OSMutexLocker   locker(fRTPSession->GetSessionMutex());

				fRequest->SetAllowed(allowed);
				fRequest->SetHasUser(hasUser);
				fRequest->SetAuthHandled(handled);

				for (; (fCurrentModule < numModules) && ((!fRequest->HasResponseBeenSent()) || fModuleState.eventRequested); fCurrentModule++)
				{
					fRequest->SetHasUser(false);
					fRequest->SetAuthHandled(false);
					debug_printf("RTSPSession.cpp:kAuthorizingRequest  BEFORE DISPATCH fCurrentModule = %lu numModules=%lu\n", fCurrentModule, numModules);

					fModuleState.eventRequested = false;
					fModuleState.idleTime = 0;
					if (fModuleState.globalLockRequested)
					{
						fModuleState.globalLockRequested = false;
						fModuleState.isGlobalLocked = true;
					}

					theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPAuthRole, fCurrentModule);
					if (NULL == theModule)
						continue;

					if (__RTSP_AUTH_DEBUG__)
					{
						theModule->GetValue(qtssModName)->PrintStr("QTSSModule::CallDispatch ENTER module=", "\n");
					}


					(void)theModule->CallDispatch(QTSS_RTSPAuthorize_Role, &fRoleParams);
					fModuleState.isGlobalLocked = false;

					if (fModuleState.globalLockRequested) // call this request back locked
						return this->CallLocked();

					// If this module has requested an event, return and wait for the event to transpire
					if (fModuleState.eventRequested)
					{
						this->ForceSameThread();    // We are holding mutexes, so we need to force
													// the same thread to be used for next Run()
						return fModuleState.idleTime; // If the module has requested idle time...
					}

					// allowed != default means a module has set the result
					// handled means a module wants to be the primary for this request
					// -- example qtaccess says only allow valid user and allowed default is false.  So module says handled, hasUser is false, allowed is false
					// 
					allowed = fRequest->GetAllowed();
					hasUser = fRequest->GetHasUser();
					handled = fRequest->GetAuthHandled();
					debug_printf("RTSPSession::Run Role(kAuthorizingRequest) allowedDefault =%d allowed= %d hasUser = %d handled=%d \n", allowedDefault, allowed, hasUser, handled);

					if (!allowed && !handled)  //old module break on !allowed
					{
						debug_printf("RTSPSession.cpp::Run(kAuthorizingRequest)  skipping other modules fCurrentModule = %lu numModules=%lu\n", fCurrentModule, numModules);
						break;
					}
					if (!allowed && hasUser && handled)  //new module break on !allowed
					{
						debug_printf("RTSPSession.cpp::Run(kAuthorizingRequest)  skipping other modules fCurrentModule = %lu numModules=%lu\n", fCurrentModule, numModules);
						break;
					}


				}
				this->SaveRequestAuthorizationParams(fRequest);

				if (!allowed)
				{
					if (false == fRequest->HasResponseBeenSent())
					{
						QTSS_AuthScheme challengeScheme = fRequest->GetAuthScheme();

						if (challengeScheme == qtssAuthDigest)
						{
							debug_printf("RTSPSession.cpp:kAuthorizingRequest  scheme == qtssAuthDigest)\n");
						}
						else if (challengeScheme == qtssAuthBasic)
						{
							debug_printf("RTSPSession.cpp:kAuthorizingRequest  scheme == qtssAuthBasic)\n");
						}

						if (challengeScheme == qtssAuthBasic) {
							fRTPSession->SetAuthScheme(qtssAuthBasic);
							theErr = fRequest->SendBasicChallenge();
						}
						else if (challengeScheme == qtssAuthDigest) {
							fRTPSession->UpdateDigestAuthChallengeParams(false, false, RTSPSessionInterface::kNoQop);
							theErr = fRequest->SendDigestChallenge(fRTPSession->GetAuthQop(), fRTPSession->GetAuthNonce(), fRTPSession->GetAuthOpaque());
						}
						else {
							// No authentication scheme is given and the request was not allowed,
							// so send a 403: Forbidden message
							theErr = fRequest->SendForbiddenResponse();
						}
						if (QTSS_NoErr != theErr) // We had an error so bail on the request quit the session and post process the request.
						{
							fRequest->SetResponseKeepAlive(false);
							fCurrentModule = 0;
							fState = kPostProcessingRequest;
							break;

						}
					}
				}

				fCurrentModule = 0;
				if (fRequest->HasResponseBeenSent())
				{
					fState = kPostProcessingRequest;
					break;
				}

				// Prepare for kPreprocessingRequest state.
				fState = kPreprocessingRequest;

				if (fRequest->GetMethod() == qtssSetupMethod)
					// Make sure to erase the session ID stored in the request at this point.
					// If we fail to do so, this same session would be used if another
					// SETUP was issued on this same TCP connection.
					fLastRTPSessionIDPtr.Len = 0;
				else if (fLastRTPSessionIDPtr.Len == 0)
					fLastRTPSessionIDPtr.Len = ::strlen(fLastRTPSessionIDPtr.Ptr);
			}

		case kPreprocessingRequest:
			{
				// Invoke preprocessor modules
				// 遍历调用所有注册了QTSS_RTSPPreProcessor_Role角色的模块
				numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPPreProcessorRole);
				{
					// Manipulation of the RTPSession from the point of view of
					// a module is guarenteed to be atomic by the API.
					Assert(fRTPSession != NULL);
					OSMutexLocker   locker(fRTPSession->GetSessionMutex());

					for (; (fCurrentModule < numModules) && ((!fRequest->HasResponseBeenSent()) || fModuleState.eventRequested); fCurrentModule++)
					{
						fModuleState.eventRequested = false;
						fModuleState.idleTime = 0;
						if (fModuleState.globalLockRequested)
						{
							fModuleState.globalLockRequested = false;
							fModuleState.isGlobalLocked = true;
						}

						theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPPreProcessorRole, fCurrentModule);
						(void)theModule->CallDispatch(QTSS_RTSPPreProcessor_Role, &fRoleParams);
						fModuleState.isGlobalLocked = false;

						// The way the API is set up currently, the first module that adds a stream
						// to the session is responsible for sending RTP packets for the session.
						if (fRTPSession->HasAnRTPStream() && (fRTPSession->GetPacketSendingModule() == NULL))
							fRTPSession->SetPacketSendingModule(theModule);

						if (fModuleState.globalLockRequested) // call this request back locked
							return this->CallLocked();

						// If this module has requested an event, return and wait for the event to transpire
						if (fModuleState.eventRequested)
						{
							this->ForceSameThread();    // We are holding mutexes, so we need to force
														// the same thread to be used for next Run()
							return fModuleState.idleTime; // If the module has requested idle time...
						}
					}
				}
				fCurrentModule = 0;
				if (fRequest->HasResponseBeenSent())
				{
					// 进入下一状态kPostProcessingRequest
					fState = kPostProcessingRequest;
					break;
				}
				fState = kProcessingRequest;
			}

		case kProcessingRequest:
			{
				// If no preprocessor sends a response, move onto the request processing module. It
				// is ALWAYS supposed to send a response, but if it doesn't, we have a canned error
				// to send back.
				fModuleState.eventRequested = false;
				fModuleState.idleTime = 0;
				if (QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPRequestRole) > 0)
				{
					// Manipulation of the RTPSession from the point of view of
					// a module is guarenteed to be atomic by the API.
					Assert(fRTPSession != NULL);
					OSMutexLocker   locker(fRTPSession->GetSessionMutex());

					if (fModuleState.globalLockRequested)
					{
						fModuleState.globalLockRequested = false;
						fModuleState.isGlobalLocked = true;
					}

					// RTSP Request角色对请求进行处理之后，服务器就调用注册了kRTSPRequestRole角色的模块
					theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPRequestRole, 0);
					(void)theModule->CallDispatch(QTSS_RTSPRequest_Role, &fRoleParams);
					fModuleState.isGlobalLocked = false;

					// Do the same check as above for the preprocessor
					if (fRTPSession->HasAnRTPStream() && fRTPSession->GetPacketSendingModule() == NULL)
						fRTPSession->SetPacketSendingModule(theModule);

					this->Process3GPPData();

					if (fModuleState.globalLockRequested) // call this request back locked
						return this->CallLocked();

					// If this module has requested an event, return and wait for the event to transpire
					if (fModuleState.eventRequested)
					{
						this->ForceSameThread();    // We are holding mutexes, so we need to force
													// the same thread to be used for next Run()
						return fModuleState.idleTime; // If the module has requested idle time...
					}
				}

				if (!fRequest->HasResponseBeenSent())
				{
					// no modules took this one so send back a parameter error
					if (fRequest->GetMethod() == qtssSetParameterMethod) // keep session
					{
						QTSS_RTSPStatusCode statusCode = qtssSuccessOK; //qtssClientParameterNotUnderstood;
						fRequest->SetValue(qtssRTSPReqStatusCode, 0, &statusCode, sizeof(statusCode));
						fRequest->SendHeader();
					}
					else
					{
						QTSSModuleUtils::SendErrorResponse(fRequest, qtssServerInternal, qtssMsgNoModuleForRequest);
					}
				}

				fState = kPostProcessingRequest;
			}

		case kPostProcessingRequest:
			{
				// Post process the request *before* sending the response. Therefore, we
				// will post process regardless of whether the client actually gets our response
				// or not.

				//if this is not a keepalive request, we should kill the session NOW
				fLiveSession = fRequest->GetResponseKeepAlive();

				if (fRTPSession != NULL)
				{
					// Invoke postprocessor modules only if there is an RTP session. We do NOT want
					// postprocessors running when filters or syntax errors have occurred in the request!
					numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPPostProcessorRole);
					{
						// Manipulation of the RTPSession from the point of view of
						// a module is guarenteed to be atomic by the API.
						OSMutexLocker   locker(fRTPSession->GetSessionMutex());

						// Make sure the RTPSession contains a copy of the realStatusCode in this request
						UInt32 realStatusCode = RTSPProtocol::GetStatusCode(fRequest->GetStatus());
						(void)fRTPSession->SetValue(qtssCliRTSPReqRealStatusCode, (UInt32)0, (void *)&realStatusCode, sizeof(realStatusCode), QTSSDictionary::kDontObeyReadOnly);

						// Make sure the RTPSession contains a copy of the qtssRTSPReqRespMsg in this request
						StrPtrLen* theRespMsg = fRequest->GetValue(qtssRTSPReqRespMsg);
						if (theRespMsg->Len > 0)
							(void)fRTPSession->SetValue(qtssCliRTSPReqRespMsg, 0, theRespMsg->Ptr, theRespMsg->Len, QTSSDictionary::kDontObeyReadOnly);

						// Set the current RTSP session for this RTP session.
						// We do this here because we need to make sure the SessionMutex
						// is grabbed while we do this. Only do this if the RTSP session
						// is still alive, of course.
						if (this->IsLiveSession())
							fRTPSession->UpdateRTSPSession(this);

						for (; (fCurrentModule < numModules) || (fModuleState.eventRequested); fCurrentModule++)
						{
							fModuleState.eventRequested = false;
							fModuleState.idleTime = 0;
							if (fModuleState.globalLockRequested)
							{
								fModuleState.globalLockRequested = false;
								fModuleState.isGlobalLocked = true;
							}

							theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPPostProcessorRole, fCurrentModule);
							(void)theModule->CallDispatch(QTSS_RTSPPostProcessor_Role, &fRoleParams);
							fModuleState.isGlobalLocked = false;

							if (fModuleState.globalLockRequested) // call this request back locked
								return this->CallLocked();

							// If this module has requested an event, return and wait for the event to transpire
							if (fModuleState.eventRequested)
							{
								this->ForceSameThread();    // We are holding mutexes, so we need to force
															// the same thread to be used for next Run()
								return fModuleState.idleTime; // If the module has requested idle time...
							}
						}
					}
				}
				fCurrentModule = 0;
				fState = kSendingResponse;
			}

		case kSendingResponse:
			{
				// Sending the RTSP response consists of making sure the
				// RTSP request output buffer is completely flushed to the socket.
				Assert(fRequest != NULL);

				// If x-dynamic-rate header is sent with a value of 1, send OPTIONS request
				if ((fRequest->GetMethod() == qtssSetupMethod) && (fRequest->GetStatus() == qtssSuccessOK)
					&& (fRequest->GetDynamicRateState() == 1) && fRoundTripTimeCalculation)
				{
					this->SaveOutputStream();
					this->ResetOutputStream();
					this->SendOptionsRequest();
				}

				if (fSentOptionsRequest && (fRequest->GetMethod() == qtssIllegalMethod))
				{
					this->ResetOutputStream();
					this->RevertOutputStream();
					fSentOptionsRequest = false;
				}

				err = fOutputStream.Flush();

				if (err == EAGAIN)
				{
					// If we get this error, we are currently flow-controlled and should
					// wait for the socket to become writeable again
					fSocket.RequestEvent(EV_WR);
					this->ForceSameThread();    // We are holding mutexes, so we need to force
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
					err = this->DumpRequestData();

					if (err == EAGAIN)
					{
						fInputSocketP->RequestEvent(EV_RE);
						this->ForceSameThread();    // We are holding mutexes, so we need to force
													// the same thread to be used for next Run()
						return 0;
					}
				}

				// If we've gotten here, we've flushed all the data. Cleanup,
				// and wait for our next request!
				this->CleanupRequest();
				fState = kReadingRequest;
			}
		}
	}

    printf("RTSPSession fObjectHolders:%d !\n", fObjectHolders);

    // Release所有RTSPSessionHandler对RTSPSession的引用
    if(fRTSPSessionHandler)
    {
        fRTSPSessionHandler->Release();
        fRTSPSessionHandler->Signal(Task::kKillEvent);
        fRTSPSessionHandler = NULL;
    }

	//fObjectHolders--  
	if (!IsLiveSession() && fObjectHolders > 0) {
		OSRefTable* theMap = QTSServerInterface::GetServer()->GetRTPSessionMap();
		OSRef* theRef = theMap->Resolve(&fLastRTPSessionIDPtr);
		if (theRef != NULL) {
			fRTPSession = (RTPSession*)theRef->GetObject();
			if (fRTPSession) fRTPSession->Teardown();
			while (theRef->GetRefCount() > 0)
				theMap->Release(fRTPSession->GetRef());
			fRTPSession = NULL;
		}
	}

	// Make absolutely sure there are no resources being occupied by the session
	// at this point.
	this->CleanupRequest();

	// Only delete if it is ok to delete!
	if (fObjectHolders == 0)
    {
        printf("RTSPSesion Run Return -1\n");
		return -1;
    }

	// If we are here because of a timeout, but we can't delete because someone
	// is holding onto a reference to this session, just reschedule the timeout.
	//
	// At this point, however, the session is DEAD.
	return 0;
}

/*
*	功能：识别出请求中GET或者POST请求，如果是HTTP请求，
*	则查找是否有SessionCookie字段和Accept字段
*	返回值：bool型，返回是否是HTTP请求
*/
bool RTSPSession::ParseProxyTunnelHTTP()
{
	/*
		if it's an HTTP request
		parse the interesing parts from the request

		- check for GET or POST, set fHTTPMethod
		- checck for HTTP protocol, set fWasHTTPRequest
		- check for SessionID header, set fProxySessionID char array
		- check for accept "application/x-rtsp-tunnelled.
	*/

	bool          isHTTPRequest = false;
	StrPtrLen       *splRequest;

	HTTP_VTRACE("ParseProxyTunnelHTTP\n")
		// 获取请求缓冲区指针
		splRequest = fInputStream.GetRequestBuffer();


	fFoundValidAccept = true;

	Assert(splRequest);

	if (splRequest)
	{
		// 初始化fHTTPMethod
		fHTTPMethod = kHTTPMethodUnknown;

#if __RTSP_HTTP_DEBUG__ 
		{
			char    buff[1024];

			memcpy(buff, splRequest->Ptr, splRequest->Len);

			buff[splRequest->Len] = 0;

			HTTP_VTRACE(buff)
		}
#endif

		StrPtrLen       theParsedData;
		StringParser    parser(splRequest);

		parser.ConsumeWord(&theParsedData);

		HTTP_VTRACE("request method: \n")
			HTTP_VTRACE_SPL(&theParsedData)

			// 识别是GET请求还是POST请求
			if (theParsedData.EqualIgnoreCase("post", 4))
			{
				fHTTPMethod = kHTTPMethodPost;
			}
			else if (theParsedData.EqualIgnoreCase("get", 3))
			{
				fHTTPMethod = kHTTPMethodGet;
			}

		if (fHTTPMethod != kHTTPMethodUnknown)
		{
			HTTP_VTRACE("IsAHTTPProxyTunnelPostRequest found POST or GET\n")
				parser.ConsumeWhitespace(); // skip over ws past method

			parser.ConsumeUntilWhitespace(&theParsedData); // theParsedData now contains the URL and CGI params ( we don't need yet );

			parser.ConsumeWhitespace(); // skip over ws past url

			parser.ConsumeWord(&theParsedData); // now should contain "HTTP"

			HTTP_VTRACE("should be HTTP/1.* next: \n")
				HTTP_VTRACE_SPL(&theParsedData)

				// DMS - why use NumEqualIgnoreCase? Wouldn't EqualIgnoreCase do the trick here?
				// 识别是否有http字段
				if (theParsedData.NumEqualIgnoreCase("http", 4))
				{
					HTTP_TRACE("ParseProxyTunnelHTTP found HTTP\n")
						fWasHTTPRequest = true;
				}

		}

		// fWasHTTPRequest为true
		if (fWasHTTPRequest)
		{
			// it's HTTP and one of the methods we like....
			// now, find the Session ID and Accept headers
			const char* kSessionHeaderName = "X-SessionCookie:";// SessionCookie字段
			const int   kSessionHeaderNameLen = ::strlen(kSessionHeaderName);
			const char* kAcceptHeaderName = "Accept:";// Accept字段
			const int   kAcceptHeaderNameLen = ::strlen(kAcceptHeaderName);
			//const char* kAcceptData = "application/x-rtsp-tunnelled";
			//const int kAcceptDataLen = ::strlen(kAcceptData);

			while (parser.GetDataRemaining() > 0)
			{
				parser.GetThruEOL(&theParsedData); // we don't need this, but there is not a ComsumeThru...

				parser.ConsumeUntilWhitespace(&theParsedData); // theParsedData now contains the URL and CGI params ( we don't need yet );

				// 匹配SessionCookie字段
				if (theParsedData.EqualIgnoreCase(kSessionHeaderName, kSessionHeaderNameLen))
				{
					// we got a weener!
					if (parser.GetDataRemaining() > 0)
						parser.ConsumeWhitespace();// 去掉空格

					if (parser.GetDataRemaining() > 0)
					{
						StrPtrLen   sessionID;

						parser.ConsumeUntil(&sessionID, StringParser::sEOLMask);

						// cache the ID so we can use it to remove ourselves from the map
						if (sessionID.Len < QTSS_MAX_SESSION_ID_LENGTH)
						{
							// 将sessionID内容拷贝到fProxySessionID
							::memcpy(fProxySessionID, sessionID.Ptr, sessionID.Len);
							fProxySessionID[sessionID.Len] = 0;
							fProxySessionIDPtr.Set(fProxySessionID, ::strlen(fProxySessionID));
							HTTP_VTRACE_ONE("found session id: %s\n", fProxySessionID)
						}
					}
				}
				// 匹配Accept字段
				else if (theParsedData.EqualIgnoreCase(kAcceptHeaderName, kAcceptHeaderNameLen))
				{
					StrPtrLen   hTTPAcceptHeader;

					// we got another weener!
					if (parser.GetDataRemaining() > 0)
						parser.ConsumeWhitespace();

					if (parser.GetDataRemaining() > 0)
					{
						parser.ConsumeUntil(&hTTPAcceptHeader, StringParser::sEOLMask);

#if __RTSP_HTTP_DEBUG__ 
						{
							char    buff[1024];

							memcpy(buff, hTTPAcceptHeader.Ptr, hTTPAcceptHeader.Len);

							buff[hTTPAcceptHeader.Len] = 0;

							HTTP_VTRACE_ONE("client will accept: %s\n", buff)
						}
#endif

						// we really don't need to check thisif ( theParsedData.EqualIgnoreCase( kAcceptData, kAcceptDataLen ) ) 
						{
							// 标识找到Accept字段
							fFoundValidAccept = true;

							HTTP_VTRACE("found valid accept\n")
						}

					}

				}
			}

		}

	}

	// we found all that we were looking for
	if (fFoundValidAccept && *fProxySessionID  && fWasHTTPRequest)
		isHTTPRequest = true;

	return isHTTPRequest;
}


/*
	"pre" filter the request looking for the HTTP Proxy
	tunnel HTTP requests, merge the 2 sessions
	into one, let the donor Session die.
*/
/*
*	函数功能:预先过滤请求,查找HTTP Proxy tunnel 请求,
*	合并两个会话为一个,杀掉donor session会话
*/
QTSS_Error RTSPSession::PreFilterForHTTPProxyTunnel()
{
	// returns true if it's an HTTP request that can tunnel
	if (!this->ParseProxyTunnelHTTP())
		return QTSS_NoErr;

	// This is an RTSP / HTTP session, so decrement the total RTSP sessions
	// and increment the total HTTP sessions
	// 这是一个RTSP / HTTP 会话，所以RTSP会话总数减1，HTTP会话总数+1
	Assert(fSessionType == qtssRTSPSession);
	QTSServerInterface::GetServer()->SwapFromRTSPToHTTP();

	// Setup our ProxyTunnel OSRefTable Ref
	Assert(fProxySessionIDPtr.Len > 0);
	fProxyRef.Set(fProxySessionIDPtr, this);

	// We have to set this here, because IF we are able to put ourselves in the map,
	// the GET may arrive immediately after, and the GET checks this state.
	fState = kWaitingToBindHTTPTunnel;
	QTSS_RTSPSessionType theOtherSessionType = qtssRTSPSession;

	if (fHTTPMethod == kHTTPMethodPost)// HTTP POST请求
	{
		HTTP_TRACE("RTSPSession is a POST request.\n")
			fSessionType = qtssRTSPHTTPInputSession;
		theOtherSessionType = qtssRTSPHTTPSession;
	}
	else if (fHTTPMethod == kHTTPMethodGet)// HTTP GET请求
	{
		HTTP_TRACE("RTSPSession is a GET request.\n")
			// we're session O (outptut)  the POST half is session 1 ( input )
			fSessionType = qtssRTSPHTTPSession;
		theOtherSessionType = qtssRTSPHTTPInputSession;

		bool showServerInfo = QTSServerInterface::GetServer()->GetPrefs()->GetRTSPServerInfoEnabled();
		if (fDoReportHTTPConnectionAddress)
		{
			// contruct a 200 OK header with an "x-server-ip-address" header

			char        responseHeaderBuf[kMaxHTTPResponseLen];
			char        localIPAddrBuf[20] = { 0 };
			StrPtrLen   localIPAddr(localIPAddrBuf, 19);

			// get a copy of the local IP address from the dictionary
			// 从字典中得到本地ip地址

			this->GetValue(qtssRTSPSesLocalAddrStr, 0, localIPAddr.Ptr, &localIPAddr.Len);
			Assert(localIPAddr.Len < sizeof(localIPAddrBuf));
			localIPAddrBuf[localIPAddr.Len] = 0;

			// 使用"x-server-ip-address" 头部字段,构造响应报文
			char *headerFieldPtr = "";
			if (showServerInfo)
			{
				headerFieldPtr = QTSServerInterface::GetServerHeader().Ptr;
				qtss_sprintf(responseHeaderBuf, sHTTPResponseFormatStr, "X-server-ip-address: ", localIPAddrBuf, "\r\n", headerFieldPtr);
			}
			else
			{
				qtss_sprintf(responseHeaderBuf, sHTTPNoServerResponseFormatStr, "X-server-ip-address: ", localIPAddrBuf, "\r\n", headerFieldPtr);
			}
			Assert(::strlen(responseHeaderBuf) < kMaxHTTPResponseLen);
			fOutputStream.Put(responseHeaderBuf);
		}
		else // use the premade stopck version
		{
			// 添加200 OK信息
			if (showServerInfo)
				fOutputStream.Put(sHTTPResponseHeaderPtr);  // 200 OK just means we connected...
			else
				fOutputStream.Put(sHTTPResponseNoServerHeaderPtr);  // 200 OK just means we connected...

		}
	}
	else
		Assert(0);

	// This function attempts to register our Ref into the map. If there is another
	// session with a matching magic number, it resolves it and returns that Ref.
	// If it returns NULL, something bad has happened, and we should just kill the session.

	// 注册RTSPSession到Map中
	OSRef* rtspSessionRef = this->RegisterRTSPSessionIntoHTTPProxyTunnelMap(theOtherSessionType);

	// Something went wrong (usually we get here because there is a session with this magic
	// number, and that session is currently doing something
	if (rtspSessionRef == NULL)
	{
		HTTP_TRACE("RegisterRTSPSessionIntoHTTPProxyTunnelMap returned NULL. Abort.\n");
		return QTSS_RequestFailed;
	}

	// We registered ourselves into the map (we are the first half), so wait for our other half
	// 前一半注册到map中,等待后一半
	if (rtspSessionRef == &fProxyRef)
	{
		HTTP_TRACE("Registered this session into map. Waiting to bind\n");
		return QTSS_NoErr;
	}

	OSRefReleaser theRefReleaser(sHTTPProxyTunnelMap, rtspSessionRef); // auto release this ref
	RTSPSession* theOtherSession = (RTSPSession*)theRefReleaser.GetRef()->GetObject();

	// We must lock down this session, for we (may) be manipulating its socket & input
	// stream, and therefore it cannot be in the process of reading data or processing a request.
	// If it is, well, safest thing to do is probably just deny this attempt to bind.

	// Session加锁,因为在操作其socket和输入流,所以此时不能读取数据或处理请求
	if (!theOtherSession->fReadMutex.TryLock())
	{
		HTTP_TRACE("Found another session in map, but couldn't grab fReadMutex. Abort.\n");
		return QTSS_RequestFailed;
	}

	if (fHTTPMethod == kHTTPMethodPost)
	{
		// take the input session's socket. This also grabs the other session's input stream
		theOtherSession->SnarfInputSocket(this);

		// Attempt to bind to this GET connection
		// this will reset our state on success.
		HTTP_TRACE_ONE("RTSPSession POST snarfed a donor session successfuly (%s).\n", fProxySessionID)
			fState = kSocketHasBeenBoundIntoHTTPTunnel;
		theOtherSession->fState = kReadingRequest;
		theOtherSession->Signal(Task::kReadEvent);
	}
	else if (fHTTPMethod == kHTTPMethodGet)
	{
		Assert(theOtherSession->fState == kWaitingToBindHTTPTunnel);
		HTTP_TRACE_ONE("RTSPSession GET snarfed a donor session successfuly (%s).\n", fProxySessionID)

			// take the input session's socket. This also grabs the other session's input stream
			this->SnarfInputSocket(theOtherSession);

		// we assume the donor's place in the map.
		sHTTPProxyTunnelMap->Swap(&fProxyRef);

		// the 1/2 connections are bound
		// the output Session state goes back to reading a request, this time an RTSP request
		// the socket donor Session(rtspSessionInput) state goes to kSocketHasBeenBoundIntoHTTPTunnel to die
		fState = kReadingRequest;
		theOtherSession->fState = kSocketHasBeenBoundIntoHTTPTunnel;
		theOtherSession->Signal(Task::kKillEvent);
	}

	// Session 解锁
	theOtherSession->fReadMutex.Unlock();
	return QTSS_NoErr;
}

/*
*	名称:RegisterRTSPSessionIntoHTTPProxyTunnelMap
*	功能:注册会话到Map表中
*	返回值:
*	1.注册成功,则返回当前会话的fProxyRef
*	2.如果逻辑数和会话类型相同,则返回另一个会话的fProxyRef
*	3.如果一个会话的逻辑数相同,但是无法解析,则返回NULL
*/
OSRef* RTSPSession::RegisterRTSPSessionIntoHTTPProxyTunnelMap(QTSS_RTSPSessionType inSessionType)
{
	// This function attempts to register the current session's fProxyRef into the map, and
	// 1) returns the current session's fProxyRef if registration was successful
	// 2) returns another session's fProxyRef if it has the same magic number and is the right sessionType
	// 3) returns NULL if there is a session with the same magic # but it couldn't be resolved.

	OSMutexLocker locker(sHTTPProxyTunnelMap->GetMutex());
	OSRef* theRef = sHTTPProxyTunnelMap->RegisterOrResolve(&fProxyRef);
	if (theRef == NULL)
		return &fProxyRef;

	RTSPSession* rtspSession = (RTSPSession*)theRef->GetObject();

	// we can be the only user of the object right now
	Assert(theRef->GetRefCount() > 0);
	if (theRef->GetRefCount() > 1 || rtspSession->fSessionType != inSessionType)
	{
		sHTTPProxyTunnelMap->Release(theRef);
		theRef = NULL;
	}
	return theRef;
}

/*
 *	名称:CheckAuthentication
 *	功能:RTSP会话的验证模块,基础验证和MD5验证
 */
void RTSPSession::CheckAuthentication() {

	QTSSUserProfile* profile = fRequest->GetUserProfile();
	StrPtrLen* userPassword = profile->GetValue(qtssUserPassword);
	QTSS_AuthScheme scheme = fRequest->GetAuthScheme();
	bool authenticated = true;

	// Check if authorization information returned by the client is for the scheme that the server sent the challenge
	// 客户端和服务器的验证方式是否一致,如不一致则验证失败
	if (scheme != (fRTPSession->GetAuthScheme())) {
		authenticated = false;
	}
	else if (scheme == qtssAuthBasic) {
		// 普通验证,验证模块返回加密后的密码
		// For basic authentication, the authentication module returns the crypt of the password, 
		// so compare crypt of qtssRTSPReqUserPassword and the text in qtssUserPassword
		StrPtrLen* reqPassword = fRequest->GetValue(qtssRTSPReqUserPassword);
		char* userPasswdStr = userPassword->GetAsCString(); // memory allocated
		char* reqPasswdStr = reqPassword->GetAsCString();   // memory allocated

		if (userPassword->Len == 0)
		{
			authenticated = false;
		}
		else
		{
#ifdef __Win32__
			// The password is md5 encoded for win32
			char md5EncodeResult[120];
			// no memory is allocated in this function call
			MD5Encode(reqPasswdStr, userPasswdStr, md5EncodeResult, sizeof(md5EncodeResult));
			if (::strcmp(userPasswdStr, md5EncodeResult) != 0)
				authenticated = false;
#else
			if (::strcmp(userPasswdStr, (char*)crypt(reqPasswdStr, userPasswdStr)) != 0)
				authenticated = false;
#endif
		}

		delete[] userPasswdStr;    // deleting allocated memory
		userPasswdStr = NULL;
		delete[] reqPasswdStr;
		reqPasswdStr = NULL;        // deleting allocated memory
	}
	else if (scheme == qtssAuthDigest) { // md5验证
		// For digest authentication, md5 digest comparison
		// The text returned by the authentication module in qtssUserPassword is MD5 hash of (username:realm:password)

		UInt32 qop = fRequest->GetAuthQop();
		StrPtrLen* opaque = fRequest->GetAuthOpaque();
		StrPtrLen* sessionOpaque = fRTPSession->GetAuthOpaque();
		UInt32 sessionQop = fRTPSession->GetAuthQop();

		do {
			// The Opaque string should be the same as that sent by the server
			// The QoP should be the same as that sent by the server
			if ((sessionOpaque->Len != 0) && !(sessionOpaque->Equal(*opaque))) {
				authenticated = false;
				break;
			}

			if (sessionQop != qop) {
				authenticated = false;
				break;
			}

			// All these are just pointers to existing memory... no new memory is allocated
			//StrPtrLen* userName = profile->GetValue(qtssUserName);
			//StrPtrLen* realm = fRequest->GetAuthRealm();
			StrPtrLen* nonce = fRequest->GetAuthNonce();
			StrPtrLen method = RTSPProtocol::GetMethodString(fRequest->GetMethod());
			StrPtrLen* digestUri = fRequest->GetAuthUri();
			StrPtrLen* responseDigest = fRequest->GetAuthResponse();
			//StrPtrLen hA1;
			StrPtrLen requestDigest;
			StrPtrLen emptyStr;

			StrPtrLen* cNonce = fRequest->GetAuthCNonce();
			// Since qtssUserPassword = md5(username:realm:password)
			// Just convert the 16 bit hash to a 32 bit char array to get HA1
			//HashToString((unsigned char *)userPassword->Ptr, &hA1);
			//CalcHA1(&sAuthAlgorithm, userName, realm, userPassword, nonce, cNonce, &hA1);


			// For qop="auth"
			if (qop == RTSPSessionInterface::kAuthQop) {
				StrPtrLen* nonceCount = fRequest->GetAuthNonceCount();
				UInt32 ncValue = 0;

				// Convert nounce count (which is a string of 8 hex digits) into a UInt32
				if (nonceCount && nonceCount->Len)
				{
					// Convert nounce count (which is a string of 8 hex digits) into a UInt32                 
					UInt32 bufSize = sizeof(ncValue);
					StrPtrLenDel tempString(nonceCount->GetAsCString());
					tempString.ToUpper();
					QTSSDataConverter::ConvertCHexStringToBytes(tempString.Ptr,
						&ncValue,
						&bufSize);
					ncValue = ntohl(ncValue);

				}
				// nonce count must not be repeated by the client
				if (ncValue < (fRTPSession->GetAuthNonceCount())) {
					authenticated = false;
					break;
				}

				// allocates memory for requestDigest.Ptr
				CalcRequestDigest(userPassword, nonce, nonceCount, cNonce, &sAuthQop, &method, digestUri, &emptyStr, &requestDigest);
				// If they are equal, check if nonce used by client is same as the one sent by the server

			}   // For No qop
			else if (qop == RTSPSessionInterface::kNoQop)
			{
				// allocates memory for requestDigest->Ptr
				CalcRequestDigest(userPassword, nonce, &emptyStr, &emptyStr, &emptyStr, &method, digestUri, &emptyStr, &requestDigest);
			}

			// hA1 is allocated memory 
			//delete [] hA1.Ptr;

			if (responseDigest->Equal(requestDigest)) {
				if (!(nonce->Equal(*(fRTPSession->GetAuthNonce()))))
					fRequest->SetStale(true);
				authenticated = true;
			}
			else {
				authenticated = false;
			}

			// delete the memory allocated in CalcRequestDigest above 
			delete[] requestDigest.Ptr;
			requestDigest.Len = 0;

		} while (false);
	}

	// If authenticaton failed, set qtssUserName in the qtssRTSPReqUserProfile attribute
	// to NULL and clear out the password and any groups that have been set.
	if (!fRequest->GetAuthHandled())
	{
		if ((!authenticated) || (authenticated && (fRequest->GetStale()))) {
			debug_printf("erasing username from profile\n");
			(void)profile->SetValue(qtssUserName, 0, sEmptyStr.Ptr, sEmptyStr.Len, QTSSDictionary::kDontObeyReadOnly);
			(void)profile->SetValue(qtssUserPassword, 0, sEmptyStr.Ptr, sEmptyStr.Len, QTSSDictionary::kDontObeyReadOnly);
			(void)profile->SetNumValues(qtssUserGroups, 0);
		}
	}
}

/*
 *	函数名：ParseOptionsResponse
 *	功能：解析响应报文中是否为RTSP协议
 */
bool RTSPSession::ParseOptionsResponse()
{
	StringParser parser(fRequest->GetValue(qtssRTSPReqFullRequest));
	Assert(fRequest->GetValue(qtssRTSPReqFullRequest)->Ptr != NULL);
	static StrPtrLen sRTSPStr("RTSP", 4);
	StrPtrLen theProtocol;
	parser.ConsumeLength(&theProtocol, 4);

	return (theProtocol.Equal(sRTSPStr));
}

void RTSPSession::Process3GPPData()
{
	if (fRTPSession == NULL)
		return;


	RTSPRequest3GPP* request3GPPInfoPtr = fRequest->GetRequest3GPPInfo();

	if (!request3GPPInfoPtr || !request3GPPInfoPtr->Is3GPP())
		return;

	fRTPSession->SetIs3GPPSession(true);

	if (request3GPPInfoPtr->HasLinkChar())
	{
		StrPtrLen dataStr;
		LinkCharDataFields linkCharFieldsParser;
		if (0 == fRequest->GetHeaderDictionary()->GetValuePtr(qtss3GPPLinkCharHeader, 0, (void**)&dataStr.Ptr, &dataStr.Len, true))
		{
			linkCharFieldsParser.SetData(&dataStr);
			fRTPSession->Set3GPPLinkCharData(&linkCharFieldsParser);
		}
	}


	if (request3GPPInfoPtr->HasRateAdaptation())
	{
		StrPtrLen dataStr;
		RateAdapationStreamDataFields fieldsParser;

		for (int numValues = 0; request3GPPInfoPtr->GetValuePtr(qtss3GPPRequestRateAdaptationStreamData, numValues, (void**)&dataStr.Ptr, &dataStr.Len) == QTSS_NoErr; numValues++)
		{
			//dataStr.PrintStr("RTSPSession::Process3GPPData qtss3GPPRequestRateAdaptationStreamData=[","]\n");      
			fieldsParser.SetData(&dataStr);
			fRTPSession->Set3GPPRateAdaptionData(&fieldsParser);
		}
	}

	return;
}

void RTSPSession::SetupRequest()
{
	// First parse the request
	QTSS_Error theErr = fRequest->Parse();
	if (theErr != QTSS_NoErr)
		return;

	// let's also refresh RTP session timeout so that it's kept alive in sync with the RTSP session.
	 //
	 // Attempt to find the RTP session for this request.
	 // 从RTP会话表中查找RTP会话
	OSRefTable* theMap = QTSServerInterface::GetServer()->GetRTPSessionMap();
	theErr = this->FindRTPSession(theMap);

	if (fRTPSession != NULL)
	{
		OSMutexLocker locker(fRTPSession->GetMutex());

		// 刷新RTPSession超时器
		fRTPSession->RefreshTimeout();
		UInt32 headerBits = fRequest->GetBandwidthHeaderBits();
		if (headerBits != 0)
			(void)fRTPSession->SetValue(qtssCliSessLastRTSPBandwidth, (UInt32)0, &headerBits, sizeof(headerBits), QTSSDictionary::kDontObeyReadOnly);


	}
	QTSS_RTSPStatusCode statusCode = qtssSuccessOK;
	char *body = NULL;
	UInt32 bodySizeBytes = 0;

	// If this is an OPTIONS request, don't even bother letting modules see it. Just
	// send a standard OPTIONS response, and be done.
	if (fRequest->GetMethod() == qtssOptionsMethod)// OPTIONS请求
	{



		this->Process3GPPData();

		StrPtrLen* cSeqPtr = fRequest->GetHeaderDictionary()->GetValue(qtssCSeqHeader);
		if (cSeqPtr == NULL || cSeqPtr->Len == 0)
		{
			statusCode = qtssClientBadRequest;
			fRequest->SetValue(qtssRTSPReqStatusCode, 0, &statusCode, sizeof(statusCode));
			fRequest->SendHeader();
			return;
		}

		fRequest->AppendHeader(qtssPublicHeader, QTSServerInterface::GetPublicHeader());

		// DJM PROTOTYPE
		StrPtrLen* requirePtr = fRequest->GetHeaderDictionary()->GetValue(qtssRequireHeader);
		if (requirePtr && requirePtr->EqualIgnoreCase(RTSPProtocol::GetHeaderString(qtssXRandomDataSizeHeader)))
		{
			body = (char*)RTSPSessionInterface::sOptionsRequestBody;
			bodySizeBytes = fRequest->GetRandomDataSize();
			Assert(bodySizeBytes <= sizeof(RTSPSessionInterface::sOptionsRequestBody));
			fRequest->AppendHeader(qtssContentTypeHeader, &sContentType);
			fRequest->AppendContentLength(bodySizeBytes);
		}

		// 发送标准OPTIONS响应报文
		fRequest->SendHeader();

		// now write the body if there is one
		if (bodySizeBytes > 0 && body != NULL)
			fRequest->Write(body, bodySizeBytes, NULL, 0);

		return;
	}

	// If this is a SET_PARAMETER request, don't let modules see it.
	if (fRequest->GetMethod() == qtssSetParameterMethod)// SET_PARAMETER请求
	{


		// Check that it has the CSeq header
		StrPtrLen* cSeqPtr = fRequest->GetHeaderDictionary()->GetValue(qtssCSeqHeader);
		if (cSeqPtr == NULL || cSeqPtr->Len == 0) // keep session
		{
			statusCode = qtssClientBadRequest;
			fRequest->SetValue(qtssRTSPReqStatusCode, 0, &statusCode, sizeof(statusCode));
			fRequest->SendHeader();
			return;
		}


		// If the RTPSession doesn't exist, return error
		if (fRTPSession == NULL) // keep session
		{
			statusCode = qtssClientSessionNotFound;
			fRequest->SetValue(qtssRTSPReqStatusCode, 0, &statusCode, sizeof(statusCode));
			fRequest->SendHeader();
			return;
		}

		// refresh RTP session timeout so that it's kept alive in sync with the RTSP session.
		if (fRequest->GetLateToleranceInSec() != -1)
		{
			OSMutexLocker locker(fRTPSession->GetMutex());
			fRTPSession->SetStreamThinningParams(fRequest->GetLateToleranceInSec());
			fRequest->SendHeader();
			return;
		}
		// let modules handle it if they want it.

	}

	// If this is a DESCRIBE request, make sure there is no SessionID. This is not allowed,
	// and may screw up modules if we let them see this request.
	if (fRequest->GetMethod() == qtssDescribeMethod)
	{
		if (fRequest->GetHeaderDictionary()->GetValue(qtssSessionHeader)->Len > 0)
		{
			(void)QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientHeaderFieldNotValid, qtssMsgNoSesIDOnDescribe);
			return;
		}
	}

	// If we don't have an RTP session yet, create one...
	if (fRTPSession == NULL)
	{
		// 创建新的RTPSession
		theErr = this->CreateNewRTPSession(theMap);
		if (theErr != QTSS_NoErr)
			return;
	}


	OSMutexLocker locker(fRTPSession->GetMutex());
	UInt32 headerBits = fRequest->GetBandwidthHeaderBits();
	if (headerBits != 0)
		(void)fRTPSession->SetValue(qtssCliSessLastRTSPBandwidth, 0, &headerBits, sizeof(headerBits), QTSSDictionary::kDontObeyReadOnly);

	// If it's a play request and the late tolerance is sent in the request use this value
	if ((fRequest->GetMethod() == qtssPlayMethod) && (fRequest->GetLateToleranceInSec() != -1))
		fRTPSession->SetStreamThinningParams(fRequest->GetLateToleranceInSec());

	// Check to see if this is a "ping" PLAY request (a PLAY request while already
	// playing with no Range header). If so, just send back a 200 OK response and do nothing.
	// No need to go to modules to do this, because this is an RFC documented behavior  
	if ((fRequest->GetMethod() == qtssPlayMethod) && (fRTPSession->GetSessionState() == qtssPlayingState)
		&& (fRequest->GetStartTime() == -1) && (fRequest->GetStopTime() == -1))
	{
		fRequest->SendHeader();
		fRTPSession->RefreshTimeout();
		return;
	}

	Assert(fRTPSession != NULL); // At this point, we must have one!
	fRoleParams.rtspRequestParams.inClientSession = fRTPSession;

	// Setup Authorization params;
	fRequest->ParseAuthHeader();
}

void RTSPSession::CleanupRequest()
{
	if (fRTPSession != NULL)
	{
		// Release the ref.
		OSRefTable* theMap = QTSServerInterface::GetServer()->GetRTPSessionMap();
		theMap->Release(fRTPSession->GetRef());

		// NULL out any references to this RTP session
		fRTPSession = NULL;
		fRoleParams.rtspRequestParams.inClientSession = NULL;
	}

	if (this->IsLiveSession() == false) //clear out the ID so it can't be re-used.
	{
		fLastRTPSessionID[0] = 0;
		fLastRTPSessionIDPtr.Set(fLastRTPSessionID, 0);
	}

	if (fRequest != NULL)
	{
		// Check to see if a filter module has replaced the request. If so, delete
		// their request now.
		if (fRequest->GetValue(qtssRTSPReqFullRequest) && fInputStream.GetRequestBuffer())
		{
			if (fRequest->GetValue(qtssRTSPReqFullRequest)->Ptr != fInputStream.GetRequestBuffer()->Ptr)
				delete[] fRequest->GetValue(qtssRTSPReqFullRequest)->Ptr;
		}

		// NULL out any references to the current request
		//delete fRequest;
		//fRequest = NULL;
		fRoleParams.rtspRequestParams.inRTSPRequest = NULL;
		fRoleParams.rtspRequestParams.inRTSPHeaders = NULL;
	}

	fSessionMutex.Unlock();
	fReadMutex.Unlock();

	// Clear out our last value for request body length before moving onto the next request
	this->SetRequestBodyLength(-1);
}

QTSS_Error  RTSPSession::FindRTPSession(OSRefTable* inRefTable)
{
	// This function attempts to locate the appropriate RTP session for this RTSP
	// Request. It uses an RTSP session ID as a key to finding the correct RTP session,
	// and it looks for this session ID in two places. First, the RTSP session ID header
	// in the RTSP request, and if there isn't one there, in the RTSP session object itself.

	StrPtrLen* theSessionID = fRequest->GetHeaderDictionary()->GetValue(qtssSessionHeader);
	if (theSessionID != NULL && theSessionID->Len > 0)
	{
		OSRef* theRef = inRefTable->Resolve(theSessionID);

		if (theRef != NULL)
			fRTPSession = (RTPSession*)theRef->GetObject();

	}

	// If there wasn't a session ID in the headers, look for one in the RTSP session itself
	if ((theSessionID == NULL || theSessionID->Len == 0) && fLastRTPSessionIDPtr.Len > 0)
	{
		OSRef* theRef = inRefTable->Resolve(&fLastRTPSessionIDPtr);
		if (theRef != NULL)
			fRTPSession = (RTPSession*)theRef->GetObject();
	}

	return QTSS_NoErr;
}

QTSS_Error  RTSPSession::CreateNewRTPSession(OSRefTable* inRefTable)
{
	Assert(fLastRTPSessionIDPtr.Ptr == &fLastRTPSessionID[0]);

	// This is a brand spanking new session. At this point, we need to create
	// a new RTPSession object that will represent this session until it completes.
	// Then, we need to pass the session onto one of the modules

	// First of all, ask the server if it's ok to add a new session
	QTSS_Error theErr = this->IsOkToAddNewRTPSession();
	if (theErr != QTSS_NoErr)
		return theErr;

	// Create the RTPSession object
	Assert(fRTPSession == NULL);
	fRTPSession = NEW RTPSession();

	{
		//
		// Lock the RTP session down so that it won't delete itself in the
		// unusual event there is a timeout while we are doing this.
		OSMutexLocker locker(fRTPSession->GetSessionMutex());

		// Because this is a new RTP session, setup some dictionary attributes
		// pertaining to RTSP that only need to be set once
		this->SetupClientSessionAttrs();

		// So, generate a session ID for this session
		QTSS_Error activationError = EPERM;
		while (activationError == EPERM)
		{
			fLastRTPSessionIDPtr.Len = this->GenerateNewSessionID(fLastRTPSessionID);

			//ok, some module has bound this session, we can activate it.
			//At this point, we may find out that this new session ID is a duplicate.
			//If that's the case, we'll simply retry until we get a unique ID
			activationError = fRTPSession->Activate(fLastRTPSessionID);
		}
		Assert(activationError == QTSS_NoErr);
	}
	Assert(fLastRTPSessionIDPtr.Ptr == &fLastRTPSessionID[0]);

	// Activate adds this session into the RTP session map. We need to therefore
	// make sure to resolve the RTPSession object out of the map, even though
	// we don't actually need to pointer.
	OSRef* theRef = inRefTable->Resolve(&fLastRTPSessionIDPtr);
	Assert(theRef != NULL);

	return QTSS_NoErr;
}

void RTSPSession::SetupClientSessionAttrs()
{
	// get and pass presentation url
	StrPtrLen* theValue = fRequest->GetValue(qtssRTSPReqURI);
	Assert(theValue != NULL);
	(void)fRTPSession->SetValue(qtssCliSesPresentationURL, 0, theValue->Ptr, theValue->Len, QTSSDictionary::kDontObeyReadOnly);

	// get and pass full request url
	theValue = fRequest->GetValue(qtssRTSPReqAbsoluteURL);
	Assert(theValue != NULL);
	(void)fRTPSession->SetValue(qtssCliSesFullURL, 0, theValue->Ptr, theValue->Len, QTSSDictionary::kDontObeyReadOnly);

	// get and pass request host name
	theValue = fRequest->GetHeaderDictionary()->GetValue(qtssHostHeader);
	Assert(theValue != NULL);
	(void)fRTPSession->SetValue(qtssCliSesHostName, 0, theValue->Ptr, theValue->Len, QTSSDictionary::kDontObeyReadOnly);

	// get and pass user agent header
	theValue = fRequest->GetHeaderDictionary()->GetValue(qtssUserAgentHeader);
	Assert(theValue != NULL);
	(void)fRTPSession->SetValue(qtssCliSesFirstUserAgent, 0, theValue->Ptr, theValue->Len, QTSSDictionary::kDontObeyReadOnly);

	// get and pass CGI params
	if (fRequest->GetMethod() == qtssDescribeMethod)
	{
		theValue = fRequest->GetValue(qtssRTSPReqQueryString);
		Assert(theValue != NULL);
		(void)fRTPSession->SetValue(qtssCliSesReqQueryString, 0, theValue->Ptr, theValue->Len, QTSSDictionary::kDontObeyReadOnly);
	}

	// store RTSP session info in the RTPSession.   
	StrPtrLen tempStr;
	tempStr.Len = 0;
	(void) this->GetValuePtr(qtssRTSPSesRemoteAddrStr, (UInt32)0, (void **)&tempStr.Ptr, &tempStr.Len);
	Assert(tempStr.Len != 0);
	(void)fRTPSession->SetValue(qtssCliRTSPSessRemoteAddrStr, (UInt32)0, tempStr.Ptr, tempStr.Len, QTSSDictionary::kDontObeyReadOnly);

	tempStr.Len = 0;
	(void) this->GetValuePtr(qtssRTSPSesLocalDNS, (UInt32)0, (void **)&tempStr.Ptr, &tempStr.Len);
	Assert(tempStr.Len != 0);
	(void)fRTPSession->SetValue(qtssCliRTSPSessLocalDNS, (UInt32)0, (void **)tempStr.Ptr, tempStr.Len, QTSSDictionary::kDontObeyReadOnly);

	tempStr.Len = 0;
	(void) this->GetValuePtr(qtssRTSPSesLocalAddrStr, (UInt32)0, (void **)&tempStr.Ptr, &tempStr.Len);
	Assert(tempStr.Len != 0);
	(void)fRTPSession->SetValue(qtssCliRTSPSessLocalAddrStr, (UInt32)0, tempStr.Ptr, tempStr.Len, QTSSDictionary::kDontObeyReadOnly);
}

UInt32 RTSPSession::GenerateNewSessionID(char* ioBuffer)
{
	//RANDOM NUMBER GENERATOR

	//We want to make our session IDs as random as possible, so use a bunch of
	//current server statistics to generate a random SInt64.

	//Generate the random number in two UInt32 parts. The first UInt32 uses
	//statistics out of a random RTP session.
	SInt64 theMicroseconds = OS::Microseconds();
	::srand((unsigned int)theMicroseconds);
	UInt32 theFirstRandom = ::rand();

	QTSServerInterface* theServer = QTSServerInterface::GetServer();

	{
		OSMutexLocker locker(theServer->GetRTPSessionMap()->GetMutex());
		OSRefHashTable* theHashTable = theServer->GetRTPSessionMap()->GetHashTable();
		if (theHashTable->GetNumEntries() > 0)
		{
			theFirstRandom %= theHashTable->GetNumEntries();
			theFirstRandom >>= 2;

			OSRefHashTableIter theIter(theHashTable);
			//Iterate through the session map, finding a random session
			for (UInt32 theCount = 0; theCount < theFirstRandom; theIter.Next(), theCount++)
				Assert(!theIter.IsDone());

			RTPSession* theSession = (RTPSession*)theIter.GetCurrent()->GetObject();
			theFirstRandom += theSession->GetPacketsSent();
			theFirstRandom += (UInt32)theSession->GetSessionCreateTime();
			theFirstRandom += (UInt32)theSession->GetPlayTime();
			theFirstRandom += (UInt32)theSession->GetBytesSent();
		}
	}
	//Generate the first half of the random number
	::srand((unsigned int)theFirstRandom);
	theFirstRandom = ::rand();

	//Now generate the second half
	UInt32 theSecondRandom = ::rand();
	theSecondRandom += theServer->GetCurBandwidthInBits();
	theSecondRandom += theServer->GetAvgBandwidthInBits();
	theSecondRandom += theServer->GetRTPPacketsPerSec();
	theSecondRandom += (UInt32)theServer->GetTotalRTPBytes();
	theSecondRandom += theServer->GetTotalRTPSessions();

	::srand((unsigned int)theSecondRandom);
	theSecondRandom = ::rand();

	SInt64 theSessionID = (SInt64)theFirstRandom;
	theSessionID <<= 32;
	theSessionID += (SInt64)theSecondRandom;
	qtss_sprintf(ioBuffer, "%" _64BITARG_ "d", theSessionID);
	Assert(::strlen(ioBuffer) < QTSS_MAX_SESSION_ID_LENGTH);
	return ::strlen(ioBuffer);
}

bool RTSPSession::OverMaxConnections(UInt32 buffer)
{
	QTSServerInterface* theServer = QTSServerInterface::GetServer();
	SInt32 maxConns = theServer->GetPrefs()->GetMaxConnections();// 获取最大连接数
	bool overLimit = false;

	if (maxConns > -1) // limit connections
	{
		UInt32 maxConnections = (UInt32)maxConns + buffer;
		if ((theServer->GetNumRTPSessions() > maxConnections)
			||
			(theServer->GetNumRTSPSessions() + theServer->GetNumRTSPHTTPSessions() > maxConnections)
			)
		{
			overLimit = true;
		}
	}

	return overLimit;

}

QTSS_Error RTSPSession::IsOkToAddNewRTPSession()
{
	QTSServerInterface* theServer = QTSServerInterface::GetServer();
	QTSS_ServerState theServerState = theServer->GetServerState();

	//we may want to deny this connection for a couple of different reasons
	//if the server is refusing new connections
	// 当服务器状态为以下状态时，服务器拒绝新连接，并发送“拒绝连接”信息给客户端
	if ((theServerState == qtssRefusingConnectionsState) ||
		(theServerState == qtssIdleState) ||
		(theServerState == qtssFatalErrorState) ||
		(theServerState == qtssShuttingDownState))
		return QTSSModuleUtils::SendErrorResponse(fRequest, qtssServerUnavailable,
			qtssMsgRefusingConnections);

	//if the max connection limit has been hit 
	// 如果服务器已经达到最大连接数，发送“无足够带宽”信息给客户端
	if (this->OverMaxConnections(0))
		return QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientNotEnoughBandwidth,
			qtssMsgTooManyClients);

	//if the max bandwidth limit has been hit
	// 如果服务器已经达到了最大带宽限制，发送“无足够带宽”信息给客户端
	SInt32 maxKBits = theServer->GetPrefs()->GetMaxKBitsBandwidth();
	if ((maxKBits > -1) && (theServer->GetCurBandwidthInBits() >= ((UInt32)maxKBits * 1024)))
		return QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientNotEnoughBandwidth,
			qtssMsgTooMuchThruput);

	//if the server is too loaded down (CPU too high, whatever)
	// --INSERT WORKING CODE HERE--

	return QTSS_NoErr;
}

/*
 *	函数名：SaveRequestAuthorizationParams
 *	功能：保存请求中的验证相关参数
 */
void RTSPSession::SaveRequestAuthorizationParams(RTSPRequest *theRTSPRequest)
{
	// Set the RTSP session's copy of the user name
	StrPtrLen* tempPtr = theRTSPRequest->GetValue(qtssRTSPReqUserName);
	Assert(tempPtr != NULL);
	if (tempPtr)
	{
		(void)this->SetValue(qtssRTSPSesLastUserName, 0, tempPtr->Ptr, tempPtr->Len, QTSSDictionary::kDontObeyReadOnly);
		(void)fRTPSession->SetValue(qtssCliRTSPSesUserName, (UInt32)0, tempPtr->Ptr, tempPtr->Len, QTSSDictionary::kDontObeyReadOnly);
	}

	// Same thing... user password
	tempPtr = theRTSPRequest->GetValue(qtssRTSPReqUserPassword);
	Assert(tempPtr != NULL);
	if (tempPtr)
	{
		(void)this->SetValue(qtssRTSPSesLastUserPassword, 0, tempPtr->Ptr, tempPtr->Len, QTSSDictionary::kDontObeyReadOnly);
		(void)fRTPSession->SetValue(qtssCliRTSPSesUserPassword, (UInt32)0, tempPtr->Ptr, tempPtr->Len, QTSSDictionary::kDontObeyReadOnly);
	}

	tempPtr = theRTSPRequest->GetValue(qtssRTSPReqURLRealm);
	if (tempPtr)
	{
		if (tempPtr->Len == 0)
		{
			// If there is no realm explicitly specified in the request, then let's get the default out of the prefs
			OSCharArrayDeleter theDefaultRealm(QTSServerInterface::GetServer()->GetPrefs()->GetAuthorizationRealm());
			char *realm = theDefaultRealm.GetObject();
			UInt32 len = ::strlen(theDefaultRealm.GetObject());
			(void)this->SetValue(qtssRTSPSesLastURLRealm, 0, realm, len, QTSSDictionary::kDontObeyReadOnly);
			(void)fRTPSession->SetValue(qtssCliRTSPSesURLRealm, (UInt32)0, realm, len, QTSSDictionary::kDontObeyReadOnly);
		}
		else
		{
			(void)this->SetValue(qtssRTSPSesLastURLRealm, 0, tempPtr->Ptr, tempPtr->Len, QTSSDictionary::kDontObeyReadOnly);
			(void)fRTPSession->SetValue(qtssCliRTSPSesURLRealm, (UInt32)0, tempPtr->Ptr, tempPtr->Len, QTSSDictionary::kDontObeyReadOnly);
		}
	}
}

QTSS_Error RTSPSession::DumpRequestData()
{
	char theDumpBuffer[2048];

	QTSS_Error theErr = QTSS_NoErr;
	while (theErr == QTSS_NoErr)
		theErr = this->Read(theDumpBuffer, 2048, NULL);

	return theErr;
}

/*
 *	函数名：HandleIncomingDataPacket
 *	功能：处理到来的数据包
 */
void RTSPSession::HandleIncomingDataPacket()
{

	// Attempt to find the RTP session for this request.
	UInt8   packetChannel = (UInt8)fInputStream.GetRequestBuffer()->Ptr[1];
	StrPtrLen* theSessionID = this->GetSessionIDForChannelNum(packetChannel);

	if (theSessionID == NULL)
	{
		Assert(0);
		return;
		theSessionID = &fLastRTPSessionIDPtr;
	}

	// RTP会话表中功能查找SessionID对应的引用
	OSRefTable* theMap = QTSServerInterface::GetServer()->GetRTPSessionMap();
	OSRef* theRef = theMap->Resolve(theSessionID);

	if (theRef != NULL)
		fRTPSession = (RTPSession*)theRef->GetObject();

	if (fRTPSession == NULL)
		return;

	StrPtrLen packetWithoutHeaders(fInputStream.GetRequestBuffer()->Ptr + 4, fInputStream.GetRequestBuffer()->Len - 4);

	OSMutexLocker locker(fRTPSession->GetMutex());
	fRTPSession->RefreshTimeout();
	// 通过packetChannel找到对应的RTP流数据
	RTPStream* theStream = fRTPSession->FindRTPStreamForChannelNum(packetChannel);
	//解析RTP流数据
	theStream->ProcessIncomingInterleavedData(packetChannel, this, &packetWithoutHeaders);

	//
	// We currently don't support async notifications from within this role
	QTSS_RoleParams packetParams;
	packetParams.rtspIncomingDataParams.inRTSPSession = this;

	packetParams.rtspIncomingDataParams.inClientSession = fRTPSession;
	packetParams.rtspIncomingDataParams.inPacketData = fInputStream.GetRequestBuffer()->Ptr;
	packetParams.rtspIncomingDataParams.inPacketLen = fInputStream.GetRequestBuffer()->Len;

	//调用所有注册了kRTSPIncomingDataRole角色的模块
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPIncomingDataRole);
	for (; fCurrentModule < numModules; fCurrentModule++)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPIncomingDataRole, fCurrentModule);
		(void)theModule->CallDispatch(QTSS_RTSPIncomingData_Role, &packetParams);
	}
	//将当前模块数置0
	fCurrentModule = 0;
}


RTSPSessionHandler::RTSPSessionHandler(RTSPSession* session)
	: Task(),
    fRTSPSession(NULL),
    fLiveHandler(true)
{
	this->SetTaskName("RTSPSessionHandler");

    fRTSPSession = session;

	for (UInt32 numPackets = 0; numPackets < kNumPreallocatedMsgs; numPackets++)
	{
		RTSPMsg* msg = NEW RTSPMsg();
		fFreeMsgQueue.EnQueue(&msg->fQueueElem);
	}

    this->Signal(Task::kStartEvent);
}

RTSPSessionHandler::~RTSPSessionHandler()
{
    printf("~RTSPSessionHandler()\n");
    fRTSPSession = NULL;

	{
		OSMutexLocker locker(&fQueueMutex);
		while (fMsgQueue.GetLength() > 0)
		{
			RTSPMsg* theMsg = (RTSPMsg*)fMsgQueue.DeQueue()->GetEnclosingObject();
			delete theMsg;
		}
	}

	while (fFreeMsgQueue.GetLength() > 0)
	{
		RTSPMsg* theMsg = (RTSPMsg*)fFreeMsgQueue.DeQueue()->GetEnclosingObject();
		delete theMsg;
	}

}

SInt64 RTSPSessionHandler::Run()
{
	EventFlags events = this->GetEvents();
	QTSS_Error theErr = QTSS_NoErr;

	if ((events & Task::kTimeoutEvent) || (events & Task::kKillEvent))
    {
        return -1;
    }

    while(fLiveHandler)
    {
        OSMutexLocker locker(&fQueueMutex);
        if (fMsgQueue.GetLength())
        {
	        RTSPMsg* theMsg = (RTSPMsg*)fMsgQueue.DeQueue()->GetEnclosingObject();

            //printf("- Msg:%ld fMsgQueue.Length: %d \n",theMsg->fMsgCountID, fMsgQueue.GetLength());
	        //theMsg处理
            HandleDataPacket(theMsg);

            // 处理完成 
            theMsg->Reset();
            OSMutexLocker locker(&fFreeQueueMutex);
            fFreeMsgQueue.EnQueue(&theMsg->fQueueElem); 
        } 
        
        if (fMsgQueue.GetLength() == 0)
        {
            return sMsgHandleInterval;
        }
    }

    return 0;

}

void RTSPSessionHandler::Release()
{
    fLiveHandler = false;
    this->Run();
    printf("RTSPSessionHandler::Release()\n");
}

RTSPMsg* RTSPSessionHandler::GetMsg()
{
    OSMutexLocker locker(&fFreeQueueMutex);
	if (fFreeMsgQueue.GetLength() == 0)
		return NEW RTSPMsg();
	else
		return (RTSPMsg*)fFreeMsgQueue.DeQueue()->GetEnclosingObject();
}

bool RTSPSessionHandler::ProcessMsg(const SInt64& inMilliseconds, RTSPMsg* theMsg)
{
	theMsg->fTimeArrived = inMilliseconds;
	{
		OSMutexLocker locker(&fQueueMutex);
		fMsgQueue.EnQueue(&theMsg->fQueueElem);
	}

	//this->Signal(Task::kUpdateEvent);

    //printf("+ Msg:%ld fMsgQueue.Length:%d \n",/*theMsg->fMsgCountID*/0, fMsgQueue.GetLength());
	return true;
}

void RTSPSessionHandler::HandleDataPacket(RTSPMsg* theMsg)
{
    RTPSession* rtpSession = NULL;
	// Attempt to find the RTP session for this request.
    UInt8   packetChannel = (UInt8)theMsg->fPacketPtr.Ptr[1];
    StrPtrLen* theSessionID = this->fRTSPSession->GetSessionIDForChannelNum(packetChannel);

	if (theSessionID == NULL)
	{
		Assert(0);
		return;
        theSessionID = &(fRTSPSession->fLastRTPSessionIDPtr);
	}

	// RTP会话表中功能查找SessionID对应的引用
	OSRefTable* theMap = QTSServerInterface::GetServer()->GetRTPSessionMap();
	OSRef* theRef = theMap->Resolve(theSessionID);

	if (theRef != NULL)
        rtpSession = (RTPSession*)theRef->GetObject();

	if (rtpSession == NULL)
		return;

    StrPtrLen packetWithoutHeaders(theMsg->fPacketPtr.Ptr + 4, theMsg->fPacketPtr.Len - 4);

	OSMutexLocker locker(rtpSession->GetMutex());
	rtpSession->RefreshTimeout();
	// 通过packetChannel找到对应的RTP流数据
	RTPStream* theStream = rtpSession->FindRTPStreamForChannelNum(packetChannel);
	//解析RTP流数据
    theStream->ProcessIncomingInterleavedData(packetChannel, fRTSPSession, &packetWithoutHeaders);

	//
	// We currently don't support async notifications from within this role
	QTSS_RoleParams packetParams;
    packetParams.rtspIncomingDataParams.inRTSPSession = fRTSPSession;

	packetParams.rtspIncomingDataParams.inClientSession = rtpSession;
    packetParams.rtspIncomingDataParams.inPacketData = theMsg->fPacketPtr.Ptr;
    packetParams.rtspIncomingDataParams.inPacketLen = theMsg->fPacketPtr.Len;

	//调用所有注册了kRTSPIncomingDataRole角色的模块
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPIncomingDataRole);
	for (UInt32 fCurrentModule = 0; fCurrentModule < numModules; fCurrentModule++)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRTSPIncomingDataRole, fCurrentModule);
		(void)theModule->CallDispatch(QTSS_RTSPIncomingData_Role, &packetParams);
	}

    theMap->Release(rtpSession->GetRef());

    // NULL out any references to this RTP session
    rtpSession = NULL;

	//fCurrentModule = 0;
}