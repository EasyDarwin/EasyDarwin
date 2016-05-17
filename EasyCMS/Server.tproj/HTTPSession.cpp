/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
	File:       HTTPSession.cpp
	Contains:   实现对服务单元每一个Session会话的网络报文处理
*/

#include "HTTPSession.h"
#include "QTSServerInterface.h"
#include "OSMemory.h"
#include "EasyUtil.h"

#include "OSArrayObjectDeleter.h"
#include <boost/algorithm/string.hpp>
#include "QueryParamList.h"

#if __FreeBSD__ || __hpux__	
#include <unistd.h>
#endif

#include <errno.h>

#if __solaris__ || __linux__ || __sgi__	|| __hpux__
#include <crypt.h>
#endif

using namespace std;

static const int sWaitDeviceRspTimeout = 10;

HTTPSession::HTTPSession( )
	: HTTPSessionInterface(),
	fRequest(NULL),
	fReadMutex(),
	fCurrentModule(0),
	fState(kReadingFirstRequest)
{
	this->SetTaskName("HTTPSession");

	//所有HTTPSession数量(包括EasyCameraSession/EasyNVRSession/EasyHTTPSession类型)
	QTSServerInterface::GetServer()->AlterCurrentHTTPSessionCount(1);

	fModuleState.curModule = NULL;
	fModuleState.curTask = this;
	fModuleState.curRole = 0;
	fModuleState.globalLockRequested = false;

	//fDeviceSnap = NEW char[EASY_MAX_URL_LENGTH];
	//fDeviceSnap[0] = '\0';

	qtss_printf("Create HTTPSession:%s\n", fSessionID);
}

HTTPSession::~HTTPSession()
{
	fLiveSession = false; //used in Clean up request to remove the RTP session.
	this->CleanupRequest();// Make sure that all our objects are deleted
	
	QTSServerInterface::GetServer()->AlterCurrentHTTPSessionCount(-1);

	//if (fDeviceSnap != NULL)
		//delete [] fDeviceSnap;

	qtss_printf("Release HTTPSession:%s\n", fSessionID);
}

SInt64 HTTPSession::Run()
{
	EventFlags events = this->GetEvents();
	QTSS_Error err = QTSS_NoErr;
	QTSSModule* theModule = NULL;
	UInt32 numModules = 0;
	// Some callbacks look for this struct in the thread object
	OSThreadDataSetter theSetter(&fModuleState, NULL);

	if (events & Task::kKillEvent)
		fLiveSession = false;

	if(events & Task::kTimeoutEvent)
	{
		char msgStr[512];
		qtss_snprintf(msgStr, sizeof(msgStr), "Timeout HTTPSession，Device_serial[%s]\n", fDevice.serial_.c_str());
		QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);
		fLiveSession = false;
	}

	while (this->IsLiveSession())
	{
		switch (fState)
		{
		case kReadingFirstRequest:
			{
				if ((err = fInputStream.ReadRequest()) == QTSS_NoErr)
				{
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

		case kReadingRequest:
			{
				OSMutexLocker readMutexLocker(&fReadMutex);

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
		case kHaveCompleteMessage:
			{
				Assert( fInputStream.GetRequestBuffer() );

				Assert(fRequest == NULL);
				//根据具体请求报文构造HTTPRequest请求类
				fRequest = NEW HTTPRequest(&QTSServerInterface::GetServerHeader(), fInputStream.GetRequestBuffer());

				//在这里，我们已经读取了一个完整的Request，并准备进行请求的处理，直到响应报文发出
				//在此过程中，此Session的Socket不进行任何网络数据的读/写；
				fReadMutex.Lock();
				fSessionMutex.Lock();

				//清空发送缓冲区
				fOutputStream.ResetBytesWritten();

				//网络请求超过了缓冲区，返回Bad Request
				if ((err == E2BIG) || (err == QTSS_BadArgument))
				{
					ExecNetMsgErrorReqHandler(httpBadRequest);
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
				if(theErr == QTSS_WouldBlock)
				{
					this->ForceSameThread();
					fInputSocketP->RequestEvent(EV_RE);
					// We are holding mutexes, so we need to force
					// the same thread to be used for next Run()
					return 0;//返回0表示有事件才进行通知，返回>0表示规定事件后调用Run

				}
				//TODO:应该再就加上theErr的判断！QTSS_NoErr就应该直接断开连接或者返回错误码，否则下面不一定够得到有效的数据
				//

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
				QTSS_Error theErr = ProcessRequest();//处理请求

				if (fOutputStream.GetBytesWritten() > 0)//每一步都检测响应报文是否已完成，完成则直接进行回复响应
				{
					delete[] fRequestBody;//释放数据部分
					fRequestBody = NULL;
					fState = kSendingResponse;
					break;
				}
				//走到这说明没有进行请求处理，应该是等待其他HTTPSession的回应
				if(fInfo.uWaitingTime>0)
				{
					this->ForceSameThread();
					// We are holding mutexes, so we need to force
					// the same thread to be used for next Run()
					UInt32 iTemp=fInfo.uWaitingTime;
					fInfo.uWaitingTime = 0;//下次等待时间需要重新被赋值
					return iTemp;//等待下一个周期被调用
				}
				delete[] fRequestBody;//释放数据部分,注意在此可能指针为空
				fRequestBody = NULL;
				fState = kCleaningUp;
				break;
			}

		case kProcessingRequest:
			{
				if (fOutputStream.GetBytesWritten() == 0)
				{
					//返回HTTP报文
					ExecNetMsgErrorReqHandler(httpInternalServerError);
					fState = kSendingResponse;
					break;
				}

				fState = kSendingResponse;
			}
		case kSendingResponse:
			{
				Assert(fRequest != NULL);

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
					err = this->DumpRequestData();

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

	return 0;
}

QTSS_Error HTTPSession::SendHTTPPacket(StrPtrLen* contentXML, Bool16 connectionClose, Bool16 decrement)
{
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(httpOK);
	if (contentXML->Len)
		httpAck.AppendContentLengthHeader(contentXML->Len);

	if(connectionClose)
		httpAck.AppendConnectionCloseHeader();

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (contentXML->Len > 0) 
		pOutputStream->Put(contentXML->Ptr, contentXML->Len);

	if (pOutputStream->GetBytesWritten() != 0)
	{
		pOutputStream->Flush();
	}

	//将对HTTPSession的引用减少一
	if(fObjectHolders && decrement)
		DecrementObjectHolderCount();

	if(connectionClose)
		this->Signal(Task::kKillEvent);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::SetupRequest()
{
	//解析请求报文
	QTSS_Error theErr = fRequest->Parse();
	if (theErr != QTSS_NoErr)
		return QTSS_BadArgument;

	if (fRequest->GetRequestPath() != NULL)
	{
		string sRequest(fRequest->GetRequestPath());
		//数据是放在Json部分还是放在HTTP头部分
		//1.数据放在头部则是 post http://****/123,则123为数据部分，或者使用post /123则123为数据部分。
		//2.数据放在JSON部分，则是post http://****/,头部不包含数据部分，跳过进入JSON部分的处理。
		if (!sRequest.empty())
		{
			boost::to_lower(sRequest);

			vector<string> path;
			if (boost::ends_with(sRequest, "/"))
			{
				boost::erase_tail(sRequest, 1);
			}
			boost::split(path, sRequest, boost::is_any_of("/"), boost::token_compress_on);
			if (path.size() == 2)
			{
				if(path[0]=="api"&&path[1]=="getdevicelist")
				{
					ExecNetMsgCSGetDeviceListReqRESTful(fRequest->GetQueryString());//获得设备列表
					return 0;
				}
				else if(path[0]=="api"&&path[1]=="getdeviceinfo")
				{
					ExecNetMsgCSGetCameraListReqRESTful(fRequest->GetQueryString());//获得某个设备详细信息
					return 0;
				}
				else if(path[0]=="api"&&path[1]=="getdevicestream")
				{
					ExecNetMsgCSGetStreamReqRESTful(fRequest->GetQueryString());//客户端的直播请求
					return 0;
				}
			}

			// 执行到这的都说明需要进行异常处理
			EasyMsgExceptionACK rsp;
			string msg = rsp.GetMsg();
			// 构造响应报文(HTTP Header)
			HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
			httpAck.CreateResponseHeader(!msg.empty() ? httpOK : httpNotImplemented);
			if (!msg.empty())
				httpAck.AppendContentLengthHeader((UInt32)msg.length());

			httpAck.AppendConnectionCloseHeader();

			//Push HTTP Header to OutputBuffer
			char respHeader[2048] = { 0 };
			StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
			strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

			HTTPResponseStream *pOutputStream = GetOutputStream();
			pOutputStream->Put(respHeader);

			//Push HTTP Content to OutputBuffer
			if (!msg.empty())
				pOutputStream->Put((char*)msg.data(), msg.length());

			return QTSS_NoErr;
		}		
	}

	HTTPStatusCode statusCode = httpOK;
	char *body = NULL;
	UInt32 bodySizeBytes = 0;

	//获取具体Content json数据部分

	//1、获取json部分长度
	StrPtrLen* lengthPtr = fRequest->GetHeaderValue(httpContentLengthHeader);

	StringParser theContentLenParser(lengthPtr);
	theContentLenParser.ConsumeWhitespace();
	UInt32 content_length = theContentLenParser.ConsumeInteger(NULL);

	qtss_printf("HTTPSession read content-length:%d \n", content_length);

	if (content_length <= 0)
	{
		return QTSS_BadArgument;
	}

	// Check for the existence of 2 attributes in the request: a pointer to our buffer for
	// the request body, and the current offset in that buffer. If these attributes exist,
	// then we've already been here for this request. If they don't exist, add them.
	UInt32 theBufferOffset = 0;
	char* theRequestBody = NULL;
	UInt32 theLen = 0;
	theLen = sizeof(theRequestBody);
	theErr = QTSS_GetValue(this, EasyHTTPSesContentBody, 0, &theRequestBody, &theLen);

	if (theErr != QTSS_NoErr)
	{
		// First time we've been here for this request. Create a buffer for the content body and
		// shove it in the request.
		theRequestBody = NEW char[content_length + 1];
		memset(theRequestBody,0,content_length + 1);
		theLen = sizeof(theRequestBody);
		theErr = QTSS_SetValue(this, EasyHTTPSesContentBody, 0, &theRequestBody, theLen);// SetValue creates an internal copy.
		Assert(theErr == QTSS_NoErr);

		// Also store the offset in the buffer
		theLen = sizeof(theBufferOffset);
		theErr = QTSS_SetValue(this, EasyHTTPSesContentBodyOffset, 0, &theBufferOffset, theLen);
		Assert(theErr == QTSS_NoErr);
	}

	theLen = sizeof(theBufferOffset);
	theErr = QTSS_GetValue(this, EasyHTTPSesContentBodyOffset, 0, &theBufferOffset, &theLen);

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

	qtss_printf("HTTPSession read content-length:%d (%d/%d) \n", theLen, theBufferOffset+theLen, content_length);
	if ((theErr == QTSS_WouldBlock) || (theLen < ( content_length - theBufferOffset)))
	{
		//
		// Update our offset in the buffer
		theBufferOffset += theLen;
		(void)QTSS_SetValue(this, EasyHTTPSesContentBodyOffset, 0, &theBufferOffset, sizeof(theBufferOffset));
		// The entire content body hasn't arrived yet. Request a read event and wait for it.

		Assert(theErr == QTSS_NoErr);
		return QTSS_WouldBlock;
	}
	//执行到这说明已经接收了完整的HTTPHeader+JSON部分
	fRequestBody = theRequestBody;//将数据部分保存起来，让ProcessRequest函数去处理请求。
	Assert(theErr == QTSS_NoErr);
	qtss_printf("Recv message: %s\n", fRequestBody);


	UInt32 offset = 0;
	(void)QTSS_SetValue(this, EasyHTTPSesContentBodyOffset, 0, &offset, sizeof(offset));
	char* content = NULL;
	(void)QTSS_SetValue(this, EasyHTTPSesContentBody, 0, &content, 0);

	return QTSS_NoErr;
}

void HTTPSession::CleanupRequest()
{
	if (fRequest != NULL)
	{
		// NULL out any references to the current request
		delete fRequest;
		fRequest = NULL;
	}

	fSessionMutex.Unlock();
	fReadMutex.Unlock();

	// Clear out our last value for request body length before moving onto the next request
	this->SetRequestBodyLength(-1);
}

Bool16 HTTPSession::OverMaxConnections(UInt32 buffer)
{
	QTSServerInterface* theServer = QTSServerInterface::GetServer();
	SInt32 maxConns = theServer->GetPrefs()->GetMaxConnections();
	Bool16 overLimit = false;

	if (maxConns > -1) // limit connections
	{ 
		UInt32 maxConnections = (UInt32) maxConns + buffer;
		if  ( theServer->GetNumServiceSessions() > maxConnections ) 
		{
			overLimit = true;          
		}
	}
	return overLimit;
}

QTSS_Error HTTPSession::DumpRequestData()
{
	char theDumpBuffer[EASY_REQUEST_BUFFER_SIZE_LEN];

	QTSS_Error theErr = QTSS_NoErr;
	while (theErr == QTSS_NoErr)
		theErr = this->Read(theDumpBuffer, EASY_REQUEST_BUFFER_SIZE_LEN, NULL);

	return theErr;
}

QTSS_Error HTTPSession::ExecNetMsgDSPostSnapReq(const char* json)//设备快照请求
{
	if(!fAuthenticated) return httpUnAuthorized;

	EasyDarwin::Protocol::EasyMsgDSPostSnapREQ parse(json);

	string image			=	parse.GetBodyValue(EASY_TAG_IMAGE);	
	string channel			=	parse.GetBodyValue(EASY_TAG_CHANNEL);
	string device_serial	=	parse.GetBodyValue(EASY_TAG_SERIAL);
	string strType			=	parse.GetBodyValue(EASY_TAG_TYPE);//类型就是图片的扩展名
	string strTime			=	parse.GetBodyValue(EASY_TAG_TIME);//时间属性

	if(channel.empty())//为可选项填充默认值
		channel = "01";
	if(strTime.empty())//如果没有时间属性，则服务端自动为其生成一个
		strTime = EasyUtil::NowTime(EASY_TIME_FORMAT_YYYYMMDDHHMMSSEx);
		
	if( (image.size() <= 0) || (device_serial.size() <= 0) || (strType.size() <= 0) || (strTime.size() <= 0) )
		return QTSS_BadArgument;

	//先对数据进行Base64解码
	image = EasyUtil::Base64Decode(image.data(), image.size());

	//文件夹路径，由快照路径+Serial合成
	char jpgDir[512] = { 0 };
	qtss_sprintf(jpgDir,"%s%s", QTSServerInterface::GetServer()->GetPrefs()->GetSnapLocalPath() ,device_serial.c_str());
	OS::RecursiveMakeDir(jpgDir);

	char jpgPath[512] = { 0 };

	//文件全路径，文件名由Serial_Channel_Time.Type合成
	qtss_sprintf(jpgPath,"%s/%s_%s_%s.%s", jpgDir, device_serial.c_str(), channel.c_str(),strTime.c_str(),strType.c_str());

	//保存快照数据
	FILE* fSnap = ::fopen(jpgPath, "wb");
	if(fSnap==NULL)//错误原因有很多，比如扩展名不规范，文件名不规范,该文件已经被其他文件打开
	{
		//DWORD e=GetLastError();
		return QTSS_NoErr;
	}
	fwrite(image.data(), 1, image.size(), fSnap);
	::fclose(fSnap);
	
	char snapURL[512] = { 0 };
	qtss_sprintf(snapURL, "%s/%s/%s_%s.%s",QTSServerInterface::GetServer()->GetPrefs()->GetSnapWebPath(), device_serial.c_str(), device_serial.c_str(),channel.c_str(),strType.c_str());
	fDevice.HoldSnapPath(snapURL,channel);


	EasyProtocolACK rsp(MSG_SD_POST_SNAP_ACK);
	EasyJsonValue header,body;

	header[EASY_TAG_VERSION]		=	EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ]			=	parse.GetHeaderValue(EASY_TAG_CSEQ);
	header[EASY_TAG_ERROR_NUM]		=	EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING]	=	EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	body[EASY_TAG_SERIAL]			=	device_serial;
	body[EASY_TAG_CHANNEL]			=	channel;

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	//构造响应报文(HTTP Header)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(!msg.empty()?httpOK:httpNotImplemented);
	if (!msg.empty())
		httpAck.AppendContentLengthHeader((UInt32)msg.length());

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);

	//将相应报文添加到HTTP输出缓冲区中
	if (!msg.empty()) 
		pOutputStream->Put((char*)msg.data(), msg.length());
	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgErrorReqHandler(HTTPStatusCode errCode)
{
	//构造响应报文(HTTP Header)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(errCode);

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);

	return QTSS_NoErr;
}

/*
	1.获取TerminalType和AppType,进行逻辑验证，不符合则返回400 httpBadRequest;
	2.验证Serial和Token进行权限验证，不符合则返回401 httpUnAuthorized;
	3.获取Name和Tag信息进行本地保存或者写入Redis;
	4.如果是APPType为EasyNVR,获取Channels通道信息本地保存或者写入Redis
*/
QTSS_Error HTTPSession::ExecNetMsgDSRegisterReq(const char* json)
{
	QTSS_Error theErr = QTSS_NoErr;
	EasyMsgDSRegisterREQ regREQ(json);

	while(!fAuthenticated)
	{
		//1.获取TerminalType和AppType,进行逻辑验证，不符合则返回400 httpBadRequest;
		int appType = regREQ.GetAppType();
		int terminalType = regREQ.GetTerminalType();
		switch(appType)
		{
		case EASY_APP_TYPE_CAMERA:
			{
				fSessionType = EasyCameraSession;
				fTerminalType = terminalType;
				break;
			}
		case EASY_APP_TYPE_NVR:
			{
				fSessionType = EasyNVRSession;
				fTerminalType = terminalType;
				break;
			}
		default:
			{
				break;
			}
		}

		if(fSessionType >= EasyHTTPSession)
		{
			//设备注册既不是EasyCamera，也不是EasyNVR，返回错误
			theErr = QTSS_BadArgument;
			break;
		}

		//2.验证Serial和Token进行权限验证，不符合则返回401 httpUnAuthorized;
		string serial = regREQ.GetBodyValue(EASY_TAG_SERIAL);
		string token = regREQ.GetBodyValue(EASY_TAG_TOKEN);

		if(serial.empty())
		{
			theErr = QTSS_AttrDoesntExist;
			break;
		}

		//验证Serial和Token是否合法
		if(false)
		{
			theErr = QTSS_NotPreemptiveSafe;
			break;
		}

		//3.获取Name和Tag信息进行本地保存或者写入Redis;
		if(!fDevice.GetDevInfo(json))
		{
			theErr = QTSS_BadArgument;
			break;
		}

		OS_Error regErr = QTSServerInterface::GetServer()->GetDeviceSessionMap()->Register(fDevice.serial_,this);
		if(regErr == OS_NoErr)
		{
			//在redis上增加设备
			QTSServerInterface::GetServer()->RedisAddDevName(fDevice.serial_.c_str());
			fAuthenticated = true;
		}
		else
		{
			//上线冲突
			theErr =  QTSS_AttrNameExists;
			break;
		}

		break;
	}

	if(theErr != QTSS_NoErr)	return theErr;

	//走到这说明该设备成功注册或者心跳
	EasyProtocol req(json);
	EasyProtocolACK rsp(MSG_SD_REGISTER_ACK);
	EasyJsonValue header,body;
	header[EASY_TAG_VERSION]=EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ]=req.GetHeaderValue(EASY_TAG_CSEQ);
	header[EASY_TAG_ERROR_NUM]=200;
	header[EASY_TAG_ERROR_STRING]=EasyProtocol::GetErrorString(200);

	body[EASY_TAG_SERIAL]=fDevice.serial_;
	body[EASY_TAG_SESSION_ID]=fSessionID;

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	//构造响应报文(HTTP Header)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(!msg.empty()?httpOK:httpNotImplemented);
	if (!msg.empty())
		httpAck.AppendContentLengthHeader((UInt32)msg.length());

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);

	//将相应报文添加到HTTP输出缓冲区中
	if (!msg.empty()) 
		pOutputStream->Put((char*)msg.data(), msg.length());

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSFreeStreamReq(const char* json)//客户端的停止直播请求
{
	//算法描述：查找指定设备，若设备存在，删除set中的当前客户端元素并判断set是否为空，为空则向设备发出停止流请求
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	EasyDarwin::Protocol::EasyProtocol req(json);

	string strDeviceSerial	=	req.GetBodyValue(EASY_TAG_SERIAL);//设备序列号
	string strCameraSerial	=	req.GetBodyValue(EASY_TAG_CHANNEL);//摄像头序列号
	string strStreamID		=   req.GetBodyValue(EASY_TAG_RESERVE);//StreamID
	string strProtocol		=	req.GetBodyValue(EASY_TAG_PROTOCOL);//Protocol

	//为可选参数填充默认值
	if(strCameraSerial.empty())
		strCameraSerial = "01";
	if(strStreamID.empty())
		 strStreamID = "1";

	if( (strDeviceSerial.size()<=0) || (strProtocol.size()<=0) )//参数判断
		return QTSS_BadArgument;

	OSRefTableEx* DeviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = DeviceMap->Resolve(strDeviceSerial);////////////////////////////////++
	if(theDevRef==NULL)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	//走到这说明存在指定设备，则该设备发出停止推流请求
	HTTPSession * pDevSession = (HTTPSession *)theDevRef->GetObjectPtr();//获得当前设备回话
		
	EasyDarwin::Protocol::EasyProtocolACK		reqreq(MSG_SD_STREAM_STOP_REQ);
	EasyJsonValue headerheader,bodybody;

	char chTemp[16] = {0};
	UInt32 uDevCseq = pDevSession->GetCSeq();
	sprintf(chTemp,"%d",uDevCseq);
	headerheader[EASY_TAG_CSEQ]			=	string(chTemp);//注意这个地方不能直接将UINT32->int,因为会造成数据失真
	headerheader[EASY_TAG_VERSION]		=	EASY_PROTOCOL_VERSION;

	bodybody[EASY_TAG_SERIAL]			=	strDeviceSerial;
	bodybody[EASY_TAG_CHANNEL]			=	strCameraSerial;
	bodybody[EASY_TAG_RESERVE]			=   strStreamID;
	bodybody[EASY_TAG_PROTOCOL]			=	strProtocol;

	reqreq.SetHead(headerheader);
	reqreq.SetBody(bodybody);

	string buffer = reqreq.GetMsg();

	//QTSS_SendHTTPPacket(pDevSession,(char*)buffer.c_str(),buffer.size(),false,false);
	DeviceMap->Release(strDeviceSerial);//////////////////////////////////////////////////////////--
	
	//直接对客户端（EasyDarWin)进行正确回应
	EasyDarwin::Protocol::EasyProtocolACK		rsp(MSG_SC_FREE_STREAM_ACK);
	EasyJsonValue header,body;
	header[EASY_TAG_CSEQ]			=	req.GetHeaderValue(EASY_TAG_CSEQ);;
	header[EASY_TAG_VERSION]		=	EASY_PROTOCOL_VERSION;
	header[EASY_TAG_ERROR_NUM]		=	200;
	header[EASY_TAG_ERROR_STRING]	=	EasyProtocol::GetErrorString(200);

	body[EASY_TAG_SERIAL]			=	strDeviceSerial;
	body[EASY_TAG_CHANNEL]			=	strCameraSerial;
	body[EASY_TAG_RESERVE]			=   strStreamID;
	body[EASY_TAG_PROTOCOL]			=	strProtocol;

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	//构造响应报文(HTTP Header)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(!msg.empty()?httpOK:httpNotImplemented);
	if (!msg.empty())
		httpAck.AppendContentLengthHeader((UInt32)msg.length());

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);

	//将相应报文添加到HTTP输出缓冲区中
	if (!msg.empty()) 
		pOutputStream->Put((char*)msg.data(), msg.length());

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgDSStreamStopAck(const char* json)//设备的停止推流回应
{
	if(!fAuthenticated)//没有进行认证请求
		return httpUnAuthorized;

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSGetStreamReqRESTful(char *queryString)//放到ProcessRequest所在的状态去处理，方便多次循环调用
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/
	QueryParamList parList(queryString);
	const char* chSerial	=	parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号
	const char* chChannel	=	parList.DoFindCGIValueForParam(EASY_TAG_L_CHANNEL);//获取通道
	const char* chProtocol  =   parList.DoFindCGIValueForParam(EASY_TAG_L_PROTOCOL);//获取通道
	const char* chReserve	=	parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE);//获取通道

	//为可选参数填充默认值
	if(chChannel==NULL)
		chChannel = "01";
	if(chReserve==NULL)
		chReserve = "1";

	if(chSerial==NULL||chProtocol==NULL)
		return QTSS_BadArgument;

	EasyDarwin::Protocol::EasyProtocolACK req(MSG_CS_GET_STREAM_REQ);//由restful接口合成json格式请求
	EasyJsonValue header,body;

	char chTemp[16] = {0};//如果客户端不提供CSeq,那么我们每次给他生成一个唯一的CSeq
	UInt32 uCseq = GetCSeq();
	sprintf(chTemp,"%d",uCseq);

	header[EASY_TAG_CSEQ]		=		chTemp;
	header[EASY_TAG_VERSION]	=		"1.0";
	body[EASY_TAG_SERIAL]		=		chSerial;
	body[EASY_TAG_CHANNEL]		=		chChannel;
	body[EASY_TAG_PROTOCOL]		=		chProtocol;
	body[EASY_TAG_RESERVE]		=		chReserve;

	req.SetHead(header);
	req.SetBody(body);

	string buffer = req.GetMsg();
	fRequestBody  = new char[buffer.size()+1];
	strcpy(fRequestBody,buffer.c_str());
	return QTSS_NoErr;

}

QTSS_Error HTTPSession::ExecNetMsgCSGetStreamReq(const char* json)//客户端开始流请求
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	EasyDarwin::Protocol::EasyProtocol req(json);
	string strCSeq = req.GetHeaderValue(EASY_TAG_CSEQ);
	UInt32 uCSeq = atoi(strCSeq.c_str());
	string strURL;//直播地址

	string strDeviceSerial	=	req.GetBodyValue(EASY_TAG_SERIAL);//设备序列号
	string strCameraSerial	=	req.GetBodyValue(EASY_TAG_CHANNEL);//摄像头序列号
	string strProtocol		=	req.GetBodyValue(EASY_TAG_PROTOCOL);//协议
	string strStreamID		=	req.GetBodyValue(EASY_TAG_RESERVE);//流类型

	//可选参数如果没有，则用默认值填充
	if(strCameraSerial.empty())//表示为EasyCamera设备
		strCameraSerial = "01";
	if(strStreamID.empty())//没有码流需求时默认为标清
		strStreamID = "1";

	if(strDeviceSerial.size()<=0||strProtocol.size()<=0)//参数判断
		return QTSS_BadArgument;

	if(fInfo.cWaitingState==0)//第一次处理请求
	{
		OSRefTableEx* DeviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
		OSRefTableEx::OSRefEx* theDevRef = DeviceMap->Resolve(strDeviceSerial);////////////////////////////////++
		if(theDevRef==NULL)//找不到指定设备
			return EASY_ERROR_DEVICE_NOT_FOUND;

		//走到这说明存在指定设备
		HTTPSession * pDevSession = (HTTPSession *)theDevRef->GetObjectPtr();//获得当前设备回话
		string strDssIP,strDssPort;
		if(QTSServerInterface::GetServer()->RedisGetAssociatedDarWin(strDeviceSerial,strCameraSerial,strDssIP,strDssPort))//是否存在关联的EasyDarWin转发服务器test,应该用Redis上的数据，因为推流是不可靠的，而EasyDarWin上的数据是可靠的
		{
			//合成直播的RTSP地址，后续有可能根据请求流的协议不同而生成不同的直播地址，如RTMP、HLS等
			string strSessionID;
			bool bReval = QTSServerInterface::GetServer()->RedisGenSession(strSessionID,SessionIDTimeout);
			if(!bReval)//sessionID在redis上的存储失败
			{
				DeviceMap->Release(strDeviceSerial);/////////////////////////////////////////////--
				return EASY_ERROR_SERVER_INTERNAL_ERROR;
			}
			strURL="rtsp://"+strDssIP+':'+strDssPort+'/'
				+strSessionID+'/'
				+strDeviceSerial+'/'
				+strCameraSerial+".sdp";

			//下面已经用不到设备回话了，释放引用
			DeviceMap->Release(strDeviceSerial);/////////////////////////////////////////////--
		}
		else
		{//不存在关联的EasyDarWin
			bool bErr = QTSServerInterface::GetServer()->RedisGetBestDarWin(strDssIP,strDssPort);
			if(!bErr)//不存在DarWin
			{
				DeviceMap->Release(strDeviceSerial);/////////////////////////////////////////////--
				return EASY_ERROR_SERVICE_NOT_FOUND;
			}
			//向指定设备发送开始流请求

			EasyDarwin::Protocol::EasyProtocolACK		reqreq(MSG_SD_PUSH_STREAM_REQ);
			EasyJsonValue headerheader,bodybody;

			char chTemp[16] = {0};
			UInt32 uDevCseq = pDevSession->GetCSeq();
			sprintf(chTemp,"%d",uDevCseq);
			headerheader[EASY_TAG_CSEQ] = string(chTemp);//注意这个地方不能直接将UINT32->int,因为会造成数据失真
			headerheader[EASY_TAG_VERSION]=		EASY_PROTOCOL_VERSION;

			string strSessionID;
			bool bReval = QTSServerInterface::GetServer()->RedisGenSession(strSessionID,SessionIDTimeout);
			if(!bReval)//sessionID再redis上的存储失败
			{
				DeviceMap->Release(strDeviceSerial);/////////////////////////////////////////////--
				return EASY_ERROR_SERVER_INTERNAL_ERROR;
			}

			bodybody[EASY_TAG_STREAM_ID]		=		strSessionID;
			bodybody[EASY_TAG_SERVER_IP]		=		strDssIP;
			bodybody[EASY_TAG_SERVER_PORT]		=		strDssPort;
			bodybody[EASY_TAG_SERIAL]			=		strDeviceSerial;
			bodybody[EASY_TAG_CHANNEL]			=		strCameraSerial;
			bodybody[EASY_TAG_PROTOCOL]			=		strProtocol;
			bodybody[EASY_TAG_RESERVE]			=		strStreamID;

			reqreq.SetHead(headerheader);
			reqreq.SetBody(bodybody);

			string buffer = reqreq.GetMsg();
			//
			strMessage msgTemp;

			msgTemp.iMsgType = MSG_CS_GET_STREAM_REQ;//当前请求的消息
			msgTemp.pObject = this;//当前对象指针
			msgTemp.uCseq = uCSeq;//当前请求的cseq

			pDevSession->InsertToMsgMap(uDevCseq,msgTemp);//加入到Map中等待客户端的回应
			IncrementObjectHolderCount();//增加引用，防止设备回应时当前Session已经终止
			//QTSS_SendHTTPPacket(pDevSession,(char*)buffer.c_str(),buffer.size(),false,false);
			DeviceMap->Release(strDeviceSerial);//////////////////////////////////////////////////////////--

			fInfo.cWaitingState = 1;//等待设备回应
			fInfo.iResponse = 0;//表示设备还没有回应
			fInfo.uTimeoutNum = 0;//开始计算超时
			fInfo.uWaitingTime = 100;//以100ms为周期循环等待，这样不占用CPU
			return QTSS_NoErr;
		}
	}
	else//等待设备回应 
	{
		if(fInfo.iResponse==0)//设备还没有回应
		{
			fInfo.uTimeoutNum++;
			if(fInfo.uTimeoutNum>CliStartStreamTimeout/100)//超时了
			{
				fInfo.cWaitingState = 0;//恢复状态
				return httpRequestTimeout;
			}
			else//没有超时，继续等待
			{
				fInfo.uWaitingTime = 100;//以100ms为周期循环等待，这样不占用CPU
				return QTSS_NoErr;
			}
		}
		else if(fInfo.uCseq!=uCSeq)//这个不是我想要的，可能是第一次请求时超时，第二次请求时返回了第一个的回应，这时我们应该继续等待第二个的回应直到超时
		{
			fInfo.iResponse = 0;//继续等待，这一个可能和另一个线程同时赋值，加锁也不能解决，只不过影响不大。
			fInfo.uTimeoutNum++;
			fInfo.uWaitingTime = 100;//以100ms为周期循环等待，这样不占用CPU
			return QTSS_NoErr;
		}
		else if(fInfo.iResponse==200)//正确回应
		{
			fInfo.cWaitingState = 0;//恢复状态
			strStreamID = fInfo.strStreamID;//使用设备的流类型和推流协议
			strProtocol = fInfo.strProtocol;
			//合成直播地址

			string strSessionID;
			bool bReval=QTSServerInterface::GetServer()->RedisGenSession(strSessionID,SessionIDTimeout);
			if(!bReval)//sessionID在redis上的存储失败
			{
				return EASY_ERROR_SERVER_INTERNAL_ERROR;
			}

			strURL = "rtsp://"+fInfo.strDssIP+':'+fInfo.strDssPort+'/'
				+strSessionID+'/'
				+strDeviceSerial+'/'
				+strCameraSerial+".sdp";
		}
		else//设备错误回应
		{
			fInfo.cWaitingState = 0;//恢复状态
			return fInfo.iResponse;
		}
	}

	//走到这说明对客户端的正确回应,因为错误回应直接返回。
	EasyDarwin::Protocol::EasyProtocolACK rsp(MSG_SC_GET_STREAM_ACK);
	EasyJsonValue header,body;
	body[EASY_TAG_URL]			=	strURL;
	body[EASY_TAG_SERIAL]		=	strDeviceSerial;
	body[EASY_TAG_CHANNEL]		=	strCameraSerial;
	body[EASY_TAG_PROTOCOL]		=	strProtocol;//如果当前已经推流，则返回请求的，否则返回实际推流类型
	body[EASY_TAG_RESERVE]		=	strStreamID;//如果当前已经推流，则返回请求的，否则返回实际推流类型

	header[EASY_TAG_VERSION]	=	EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ]		=	strCSeq;
	header[EASY_TAG_ERROR_NUM]	=	200;
	header[EASY_TAG_ERROR_STRING] = EasyDarwin::Protocol::EasyProtocol::GetErrorString(200);


	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	//构造响应报文(HTTP Header)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(!msg.empty()?httpOK:httpNotImplemented);
	if (!msg.empty())
		httpAck.AppendContentLengthHeader((UInt32)msg.length());

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);

	//将相应报文添加到HTTP输出缓冲区中
	if (!msg.empty()) 
		pOutputStream->Put((char*)msg.data(), msg.length());
	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgDSPushStreamAck(const char* json)//设备的开始流回应
{
	if(!fAuthenticated)//没有进行认证请求
		return httpUnAuthorized;

	//对于设备的推流回应是不需要在进行回应的，直接解析找到对应的客户端Session，赋值即可	
	EasyDarwin::Protocol::EasyProtocol req(json);


	string strDeviceSerial	=	req.GetBodyValue(EASY_TAG_SERIAL);//设备序列号
	string strCameraSerial	=	req.GetBodyValue(EASY_TAG_CHANNEL);//摄像头序列号
	//string strProtocol		=	req.GetBodyValue("Protocol");//协议,终端仅支持RTSP推送
	string strStreamID		=	req.GetBodyValue(EASY_TAG_RESERVE);//流类型
	string strDssIP         =   req.GetBodyValue(EASY_TAG_SERVER_IP);//设备实际推流地址
	string strDssPort       =   req.GetBodyValue(EASY_TAG_SERVER_PORT);//和端口

	string strCSeq			=	req.GetHeaderValue(EASY_TAG_CSEQ);//这个是关键字
	string strStateCode     =   req.GetHeaderValue(EASY_TAG_ERROR_NUM);//状态码

	UInt32 uCSeq = atoi(strCSeq.c_str());
	int iStateCode = atoi(strStateCode.c_str());

	strMessage strTempMsg;
	if(!FindInMsgMap(uCSeq,strTempMsg))
	{//天啊，竟然找不到，一定是设备发送的CSeq和它接收的不一样
		return QTSS_BadArgument;
	}
	else
	{
		HTTPSession * pCliSession = (HTTPSession *)strTempMsg.pObject;//这个对象指针是有效的，因为之前我们给他加了回命草
		if(strTempMsg.iMsgType==MSG_CS_GET_STREAM_REQ)//客户端的开始流请求
		{
			if(iStateCode==200)//只有正确回应才进行一些信息的保存
			{
				pCliSession->fInfo.strDssIP = strDssIP;
				pCliSession->fInfo.strDssPort = strDssPort;
				pCliSession->fInfo.strStreamID = strStreamID;
				//pCliSession->fInfo.strProtocol=strProtocol;
			}
			pCliSession->fInfo.uCseq = strTempMsg.uCseq;
			pCliSession->fInfo.iResponse = iStateCode;//这句之后开始触发客户端对象
			pCliSession->DecrementObjectHolderCount();//现在可以放心的安息了
		}
		else
		{

		}
	}
	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSGetDeviceListReqRESTful(char *queryString)//客户端获得设备列表
{
	//queryString在这个函数中是没有用的，仅为了保持接口的一致性。
	/*
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	EasyDarwin::Protocol::EasyProtocolACK		rsp(MSG_SC_DEVICE_LIST_ACK);
	EasyJsonValue header,body;

	header[EASY_TAG_VERSION]		=	EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ]			=	1;
	header[EASY_TAG_ERROR_NUM]		=	200;
	header[EASY_TAG_ERROR_STRING]	=	EasyDarwin::Protocol::EasyProtocol::GetErrorString(200);


	OSMutex *mutexMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMutex();
	OSHashMap  *deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMap();
	OSRefIt itRef;
	Json::Value *proot = rsp.GetRoot();

	mutexMap->Lock();
	body[EASY_TAG_DEVICE_COUNT] = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetEleNumInMap();
	for(itRef=deviceMap->begin();itRef!=deviceMap->end();itRef++)
	{
		Json::Value value;
		strDevice *deviceInfo		= ((HTTPSession*)(itRef->second->GetObjectPtr()))->GetDeviceInfo();
		value[EASY_TAG_SERIAL]		=	deviceInfo->serial_;
		value[EASY_TAG_NAME]		=	deviceInfo->name_;
		value[EASY_TAG_TAG]			=	deviceInfo->tag_;
		value[EASY_TAG_APP_TYPE]	=	EasyProtocol::GetAppTypeString(deviceInfo->eAppType);
		value[EASY_TAG_TERMINAL_TYPE]	=	EasyProtocol::GetTerminalTypeString(deviceInfo->eDeviceType);
		(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES].append(value);
	}
	mutexMap->Unlock();

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	//构造响应报文(HTTP头)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(!msg.empty()?httpOK:httpNotImplemented);
	if (!msg.empty())
		httpAck.AppendContentLengthHeader((UInt32)msg.length());

	//响应完成后断开连接
	httpAck.AppendConnectionCloseHeader();

	//Push MSG to OutputBuffer
	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (!msg.empty())
		pOutputStream->Put((char*)msg.data(), msg.length());

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSDeviceListReq(const char *json)//客户端获得设备列表
{
	/*
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/
	EasyDarwin::Protocol::EasyProtocol		req(json);


	EasyDarwin::Protocol::EasyProtocolACK		rsp(MSG_SC_DEVICE_LIST_ACK);
	EasyJsonValue header,body;

	header[EASY_TAG_VERSION]		=	EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ]			=	req.GetHeaderValue(EASY_TAG_CSEQ);
	header[EASY_TAG_ERROR_NUM]		=	200;
	header[EASY_TAG_ERROR_STRING]	=	EasyDarwin::Protocol::EasyProtocol::GetErrorString(200);


	OSMutex *mutexMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMutex();
	OSHashMap  *deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMap();
	OSRefIt itRef;
	Json::Value *proot = rsp.GetRoot();

	mutexMap->Lock();
	body[EASY_TAG_DEVICE_COUNT] = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetEleNumInMap();
	for(itRef=deviceMap->begin();itRef!=deviceMap->end();itRef++)
	{
		Json::Value value;
		strDevice *deviceInfo=((HTTPSession*)(itRef->second->GetObjectPtr()))->GetDeviceInfo();
		value[EASY_TAG_SERIAL]			=	deviceInfo->serial_;
		value[EASY_TAG_NAME]			=	deviceInfo->name_;
		value[EASY_TAG_TAG]				=	deviceInfo->tag_;
		value[EASY_TAG_APP_TYPE]		=	EasyProtocol::GetAppTypeString(deviceInfo->eAppType);
		value[EASY_TAG_TERMINAL_TYPE]	=	EasyProtocol::GetTerminalTypeString(deviceInfo->eDeviceType);
		(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES].append(value);
	}
	mutexMap->Unlock();

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	//构造响应报文(HTTP头)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(!msg.empty()?httpOK:httpNotImplemented);
	if (!msg.empty())
		httpAck.AppendContentLengthHeader((UInt32)msg.length());

	//Push MSG to OutputBuffer
	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (!msg.empty())
		pOutputStream->Put((char*)msg.data(), msg.length());

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSGetCameraListReqRESTful(char* queryString)
{
	/*	
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	QueryParamList parList(queryString);
	const char* device_serial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号

	if(device_serial==NULL)
		return QTSS_BadArgument;

	EasyDarwin::Protocol::EasyProtocolACK		rsp(MSG_SC_DEVICE_INFO_ACK);
	EasyJsonValue header,body;

	header[EASY_TAG_VERSION]		= EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ]		    = 1;

	body[EASY_TAG_SERIAL]			= device_serial;

	OSRefTableEx* DeviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = DeviceMap->Resolve(device_serial);////////////////////////////////++
	if(theDevRef==NULL)//不存在指定设备
	{
		header[EASY_TAG_ERROR_NUM]    = 603;
		header[EASY_TAG_ERROR_STRING] = EasyDarwin::Protocol::EasyProtocol::GetErrorString(EASY_ERROR_DEVICE_NOT_FOUND);
	}
	else//存在指定设备，则获取这个设备的摄像头信息
	{
		header[EASY_TAG_ERROR_NUM]    = 200;
		header[EASY_TAG_ERROR_STRING] = EasyDarwin::Protocol::EasyProtocol::GetErrorString(200);

		Json::Value *proot=rsp.GetRoot();
		strDevice *deviceInfo= ((HTTPSession*)theDevRef->GetObjectPtr())->GetDeviceInfo();
		if(deviceInfo->eAppType==EASY_APP_TYPE_CAMERA)
		{
			body[EASY_TAG_SNAP_URL] = deviceInfo->snapJpgPath_;
		}
		else
		{
			EasyDevices *camerasInfo = &(deviceInfo->channels_);
			EasyDevicesIterator itCam;
			body[EASY_TAG_CHANNEL_COUNT] = ((HTTPSession*)theDevRef->GetObjectPtr())->GetDeviceInfo()->channelCount_;
			for(itCam=camerasInfo->begin();itCam!=camerasInfo->end();itCam++)
			{
				Json::Value value;
				value[EASY_TAG_CHANNEL]  = itCam->channel_;
				value[EASY_TAG_NAME]	 = itCam->name_;
				value[EASY_TAG_STATUS]	 = itCam->status_;
				value[EASY_TAG_SNAP_URL] = itCam->snapJpgPath_;
				(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].append(value);
			}
		}
		DeviceMap->Release(device_serial);////////////////////////////////--
	}
	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	//构造响应报文(HTTP头)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(!msg.empty()?httpOK:httpNotImplemented);
	if (!msg.empty())
		httpAck.AppendContentLengthHeader((UInt32)msg.length());

	//响应完成后断开连接
	httpAck.AppendConnectionCloseHeader();

	//Push MSG to OutputBuffer
	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (!msg.empty())
		pOutputStream->Put((char*)msg.data(), msg.length());

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSCameraListReq(const char* json)
{
	/*	
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	EasyDarwin::Protocol::EasyProtocol      req(json);
	string strDeviceSerial	=	req.GetBodyValue(EASY_TAG_SERIAL);

	if(strDeviceSerial.size()<=0)
		return QTSS_BadArgument;

	EasyDarwin::Protocol::EasyProtocolACK		rsp(MSG_SC_DEVICE_INFO_ACK);
	EasyJsonValue header,body;

	header[EASY_TAG_VERSION]		=	EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ]			=	req.GetHeaderValue(EASY_TAG_CSEQ);
	header[EASY_TAG_ERROR_NUM]		=	200;
	header[EASY_TAG_ERROR_STRING]	=	EasyDarwin::Protocol::EasyProtocol::GetErrorString(200);
	body[EASY_TAG_SERIAL]			=	strDeviceSerial;

	OSRefTableEx* DeviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = DeviceMap->Resolve(strDeviceSerial);////////////////////////////////++
	if(theDevRef==NULL)//不存在指定设备
	{
		return EASY_ERROR_DEVICE_NOT_FOUND;//交给错误处理中心处理
	}
	else//存在指定设备，则获取这个设备的摄像头信息
	{

		Json::Value *proot = rsp.GetRoot();
		strDevice *deviceInfo =  ((HTTPSession*)theDevRef->GetObjectPtr())->GetDeviceInfo();
		if(deviceInfo->eAppType==EASY_APP_TYPE_CAMERA)
		{
			body[EASY_TAG_SNAP_URL] = deviceInfo->snapJpgPath_;
		}
		else
		{
			EasyDevices *camerasInfo = &(deviceInfo->channels_);
			EasyDevicesIterator itCam;

			body[EASY_TAG_CHANNEL_COUNT] = ((HTTPSession*)theDevRef->GetObjectPtr())->GetDeviceInfo()->channelCount_;
			for(itCam=camerasInfo->begin();itCam!=camerasInfo->end();itCam++)
			{
				Json::Value value;
				value[EASY_TAG_CHANNEL]		=	itCam->channel_;
				value[EASY_TAG_NAME]		=	itCam->name_;
				value[EASY_TAG_STATUS]		=	itCam->status_;
				body[EASY_TAG_SNAP_URL]		=	itCam->snapJpgPath_;
				(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].append(value);
			}
		}
		DeviceMap->Release(strDeviceSerial);////////////////////////////////--
	}
	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	//构造响应报文(HTTP头)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(!msg.empty()?httpOK:httpNotImplemented);
	if (!msg.empty())
		httpAck.AppendContentLengthHeader((UInt32)msg.length());

	//响应完成后断开连接
	httpAck.AppendConnectionCloseHeader();

	//Push MSG to OutputBuffer
	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (!msg.empty())
		pOutputStream->Put((char*)msg.data(), msg.length());

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ProcessRequest()//处理请求
{
	//OSCharArrayDeleter charArrayPathDeleter(theRequestBody);//不要在这删除，因为可能执行多次，仅当对请求的处理完毕后再进行删除

	if(fRequestBody == NULL)//表示没有正确的解析请求，SetUpRequest环节没有解析出数据部分
		return QTSS_NoErr;

	//消息处理
	QTSS_Error theErr = QTSS_NoErr;

	EasyDarwin::Protocol::EasyProtocol protocol(fRequestBody);
	int nNetMsg = protocol.GetMessageType(),nRspMsg = MSG_SC_EXCEPTION;

	switch (nNetMsg)
	{
	case MSG_DS_REGISTER_REQ://处理设备上线消息,设备类型包括NVR、摄像头和智能主机
		theErr = ExecNetMsgDSRegisterReq(fRequestBody);
		nRspMsg = MSG_SD_REGISTER_ACK;
		break;
	case MSG_CS_GET_STREAM_REQ://客户端的开始流请求
		theErr = ExecNetMsgCSGetStreamReq(fRequestBody);
		nRspMsg = MSG_SC_GET_STREAM_ACK;
		break;
	case MSG_DS_PUSH_STREAM_ACK://设备的开始流回应
		theErr = ExecNetMsgDSPushStreamAck(fRequestBody);
		nRspMsg = MSG_DS_PUSH_STREAM_ACK;//注意，这里实际上是不应该再回应的
		break;
	case MSG_CS_FREE_STREAM_REQ://客户端的停止直播请求
		theErr = ExecNetMsgCSFreeStreamReq(fRequestBody);
		nRspMsg = MSG_SC_FREE_STREAM_ACK;
		break;
	case MSG_DS_STREAM_STOP_ACK://设备对EasyCMS的停止推流回应
		theErr = ExecNetMsgDSStreamStopAck(fRequestBody);
		nRspMsg = MSG_DS_STREAM_STOP_ACK;//注意，这里实际上是不需要在进行回应的
		break;
	case MSG_CS_DEVICE_LIST_REQ://设备列表请求
		theErr = ExecNetMsgCSDeviceListReq(fRequestBody);//add
		nRspMsg=MSG_SC_DEVICE_LIST_ACK;
		break;
	case MSG_CS_DEVICE_INFO_REQ://摄像头列表请求,设备的具体信息
		theErr = ExecNetMsgCSCameraListReq(fRequestBody);//add
		nRspMsg = MSG_SC_DEVICE_INFO_ACK;
		break;
	case MSG_DS_POST_SNAP_REQ://设备快照上传
		theErr = ExecNetMsgDSPostSnapReq(fRequestBody);
		nRspMsg = MSG_SD_POST_SNAP_ACK;
		break;
	default:
		theErr = ExecNetMsgErrorReqHandler(httpNotImplemented);
		break;
	}

	//如果不想进入错误自动处理则一定要返回QTSS_NoErr
	if(theErr != QTSS_NoErr)//无论是正确回应还是等待返回都是QTSS_NoErr，出现错误，对错误进行统一回应
	{
		EasyDarwin::Protocol::EasyProtocol req(fRequestBody);
		EasyDarwin::Protocol::EasyProtocolACK rsp(nRspMsg);
		EasyJsonValue header;
		header[EASY_TAG_VERSION]	=	EASY_PROTOCOL_VERSION;
		header[EASY_TAG_CSEQ]		=	req.GetHeaderValue(EASY_TAG_CSEQ);

		switch(theErr)
		{
		case QTSS_BadArgument:
			header[EASY_TAG_ERROR_NUM]		= 400;
			header[EASY_TAG_ERROR_STRING]	= EasyDarwin::Protocol::EasyProtocol::GetErrorString(400);
			break;
		case httpUnAuthorized:
			header[EASY_TAG_ERROR_NUM]		= 401;
			header[EASY_TAG_ERROR_STRING]	= EasyDarwin::Protocol::EasyProtocol::GetErrorString(401);
			break;
		case QTSS_AttrNameExists:
			header[EASY_TAG_ERROR_NUM]		= 409;
			header[EASY_TAG_ERROR_STRING]	= EasyDarwin::Protocol::EasyProtocol::GetErrorString(409);
			break;
		case EASY_ERROR_DEVICE_NOT_FOUND:
			header[EASY_TAG_ERROR_NUM]		= 603;
			header[EASY_TAG_ERROR_STRING]	= EasyDarwin::Protocol::EasyProtocol::GetErrorString(603);
			break;
		case EASY_ERROR_SERVICE_NOT_FOUND:
			header[EASY_TAG_ERROR_NUM]		= 605;
			header[EASY_TAG_ERROR_STRING]	= EasyDarwin::Protocol::EasyProtocol::GetErrorString(605);
			break;
		case httpRequestTimeout:
			header[EASY_TAG_ERROR_NUM]		= 408;
			header[EASY_TAG_ERROR_STRING]	= EasyDarwin::Protocol::EasyProtocol::GetErrorString(408);
			break;
		case EASY_ERROR_SERVER_INTERNAL_ERROR:
			header[EASY_TAG_ERROR_NUM]		= 500;
			header[EASY_TAG_ERROR_STRING]	= EasyDarwin::Protocol::EasyProtocol::GetErrorString(500);
			break;
		case EASY_ERROR_SERVER_NOT_IMPLEMENTED:
			header[EASY_TAG_ERROR_NUM]		= 501;
			header[EASY_TAG_ERROR_STRING]	= EasyDarwin::Protocol::EasyProtocol::GetErrorString(501);
			break;
		default:
			header[EASY_TAG_ERROR_NUM]		= 400;
			header[EASY_TAG_ERROR_STRING]	= EasyDarwin::Protocol::EasyProtocol::GetErrorString(400);
			break;
		}

		rsp.SetHead(header);
		string msg = rsp.GetMsg();
		//构造响应报文(HTTP Header)
		HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
		httpAck.CreateResponseHeader(!msg.empty()?httpOK:httpNotImplemented);
		if (!msg.empty())
			httpAck.AppendContentLengthHeader((UInt32)msg.length());

		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

		HTTPResponseStream *pOutputStream = GetOutputStream();
		pOutputStream->Put(respHeader);

		//将相应报文添加到HTTP输出缓冲区中
		if (!msg.empty()) 
			pOutputStream->Put((char*)msg.data(), msg.length());
	}
	return QTSS_NoErr;
}