/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyCMSSession.cpp
    Contains:   Implementation of object defined in EasyCMSSession.h. 
*/
#include "EasyCMSSession.h"
#include "EasyUtil.h"

EasyCMSSession::EasyCMSSession()
:	Task(),
	fSocket(NEW TCPClientSocket(Socket::kNonBlockingSocketType)),    
	fTimeoutTask(this, 30 * 1000),
	fInputStream(fSocket),
	fOutputStream(fSocket, &fTimeoutTask),
	fState(kIdle),//状态机的初始状态
	fRequest(NULL),
	fReadMutex(),
	fMutex(),
	fContentBufferOffset(0),
	fContentBuffer(NULL),
	fStreamName(NULL),
	fLiveSession(true)
{
	this->SetTaskName("EasyCMSSession");
	fTimeoutTask.RefreshTimeout();

}

EasyCMSSession::~EasyCMSSession()
{
	delete[] fStreamName;

	if(fSocket)
	{
		delete fSocket;
		fSocket = NULL;
	}

	if(fRequest)
	{
		delete fRequest;
		fRequest = NULL;
	}

	if(fContentBuffer)
	{
		delete [] fContentBuffer;
		fContentBuffer = NULL;
	}
}

void EasyCMSSession::CleanupRequest()
{
    if (fRequest != NULL)
    {
        // NULL out any references to the current request
        delete fRequest;
        fRequest = NULL;
    }

	//清空发送缓冲区
	fOutputStream.ResetBytesWritten();
}

SInt64 EasyCMSSession::Run()
{	
	OSMutexLocker locker(&fMutex);

	OS_Error theErr = OS_NoErr;
	EventFlags events = this->GetEvents();

    if ((events & Task::kTimeoutEvent) || (events & Task::kKillEvent))
        fLiveSession = false;

	while(fLiveSession)
	{
		switch (fState)
		{
			case kIdle:
				{
					//根据事件类型执行不同的动作
					if(events & Task::kStartEvent)
					{
						switch(fEasyMsgType)
						{
						case MSG_CS_FREE_STREAM_REQ:
							{
								CSFreeStream();
								break;
							}
						default:
							break;
						}
					}

					if(events & Task::kReadEvent)
					{
						// 已连接，有新消息需要读取(数据或者断开)
						fState = kReadingMessage;
					}

					// 如果有消息需要发送则进入发送流程
					if (fOutputStream.GetBytesWritten() > 0)
					{
						fState = kSendingMessage;
					}

					//当前状态没有进行数据发送，则等待下一次事件触发
					if(kIdle == fState)
					{
						return 0;
					}
					break;
				}

			case kReadingMessage:
				{
					// 网络请求报文存储在fInputStream中
					if ((theErr = fInputStream.ReadRequest()) == QTSS_NoErr)
					{
						//如果RequestStream返回QTSS_NoErr，就表示已经读取了目前所到达的网络数据
						//但！还不能构成一个整体报文Header部分，还要继续等待读取...
						fSocket->GetSocket()->SetTask(this);
						fSocket->GetSocket()->RequestEvent(EV_RE);

						fState = kIdle;
						return 0;
					}
                
					if ((theErr != QTSS_RequestArrived) && (theErr != E2BIG) && (theErr != QTSS_BadArgument))
					{
						//Any other error implies that the input connection has gone away.
						// We should only kill the whole session if we aren't doing HTTP.
						// (If we are doing HTTP, the POST connection can go away)
						Assert(theErr > 0);
						// If we've gotten here, this must be an HTTP session with
						// a dead input connection. If that's the case, we should
						// clean up immediately so as to not have an open socket
						// needlessly lingering around, taking up space.
						Assert(!fSocket->GetSocket()->IsConnected());
						//this->ResetClientSocket();
						//return 0;
						//读取数据失败，直接进入析构
					}
					// 网络请求超过了缓冲区，返回Bad Request
					if ( (theErr == E2BIG) || (theErr == QTSS_BadArgument) )
					{
						//返回HTTP报文，错误码408
						//(void)QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientBadRequest, qtssMsgBadBase64);
						fState = kCleaningUp;
					}
					else
					{
						fState = kProcessingMessage;
					}
					break;
				}
			case kProcessingMessage:
				{
					// 处理网络报文
					Assert( fInputStream.GetRequestBuffer() );
					Assert(fRequest == NULL);

					// 根据具体请求报文构造HTTPRequest请求类
					fRequest = NEW HTTPRequest(&QTSServerInterface::GetServerHeader(), fInputStream.GetRequestBuffer());
                
					// 清空发送缓冲区
					fOutputStream.ResetBytesWritten();

					Assert(theErr == QTSS_RequestArrived);

					// 处理收到的具体报文
					ProcessMessage();

					// 每一步都检测响应报文是否已完成，完成则直接进行回复响应
					if (fOutputStream.GetBytesWritten() > 0)
					{
						fState = kSendingMessage;
					}
					else
					{
						fState = kCleaningUp;
					}
					break;
				}
			case kSendingMessage:
				{
					//发送响应报文
					theErr = fOutputStream.Flush();

					if(theErr == 115)
					{
						fSocket->GetSocket()->SetTask(this);
						fSocket->GetSocket()->RequestEvent(EV_WR);
						this->ForceSameThread();
						return 0;
					}

					if (theErr == EAGAIN || theErr == EINPROGRESS)
					{
						// If we get this error, we are currently flow-controlled and should
						// wait for the socket to become writeable again
						// 如果收到Socket EAGAIN错误，那么我们需要等Socket再次可写的时候再调用发送
						fSocket->GetSocket()->SetTask(this);
						fSocket->GetSocket()->RequestEvent(fSocket->GetEventMask());
						this->ForceSameThread();
						// We are holding mutexes, so we need to force
						// the same thread to be used for next Run()
						return 0;
					}
					else if (theErr != QTSS_NoErr)
					{
						// Any other error means that the client has disconnected, right?
						Assert(!this->IsConnected());
						//向服务器发送数据失败，直接进入析构
						//ResetClientSocket();
						return -1;
					}
            
					fState = kCleaningUp;
					break;
				}
			case kCleaningUp:
				{
					// 一次请求的读取、处理、响应过程完整，等待下一次网络报文！
					this->CleanupRequest();
					fState = kIdle;
					if(IsConnected())
					{
						fSocket->GetSocket()->SetTask(this);
						fSocket->GetSocket()->RequestEvent(EV_RE | EV_WR);//网络事件监听
					}
					return 0;
				}
		}
	}
    return -1;
}

QTSS_Error EasyCMSSession::ProcessMessage()
{
	if(NULL == fRequest) return QTSS_BadArgument;

    //解析HTTPRequest报文
    QTSS_Error theErr = fRequest->Parse();
    if (theErr != QTSS_NoErr) return QTSS_BadArgument;

	//获取具体Content json数据部分
	StrPtrLen* lengthPtr = fRequest->GetHeaderValue(httpContentLengthHeader);
	StringParser theContentLenParser(lengthPtr);
    theContentLenParser.ConsumeWhitespace();
    UInt32 content_length = theContentLenParser.ConsumeInteger(NULL);

    if (content_length)
	{	
		qtss_printf("EasyCMSSession::ProcessMessage read content-length:%d \n", content_length);
		// 检查content的fContentBuffer和fContentBufferOffset是否有值存在,如果存在，说明我们已经开始
		// 进行content请求处理,如果不存在,我们需要创建并初始化fContentBuffer和fContentBufferOffset
		if (fContentBuffer == NULL)
		{
			fContentBuffer = NEW char[content_length + 1];
			memset(fContentBuffer,0,content_length + 1);
			fContentBufferOffset = 0;
		}
	    
		UInt32 theLen = 0;
		// 读取HTTP Content报文数据
		theErr = fInputStream.Read(fContentBuffer + fContentBufferOffset, content_length - fContentBufferOffset, &theLen);
		Assert(theErr != QTSS_BadArgument);

		if (theErr == QTSS_RequestFailed)
		{
			OSCharArrayDeleter charArrayPathDeleter(fContentBuffer);
			fContentBufferOffset = 0;
			fContentBuffer = NULL;

			return QTSS_RequestFailed;
		}
	    
		qtss_printf("EasyCMSSession::ProcessMessage() Add Len:%d \n", theLen);
		if ((theErr == QTSS_WouldBlock) || (theLen < ( content_length - fContentBufferOffset)))
		{
			//
			// Update our offset in the buffer
			fContentBufferOffset += theLen;
	       
			Assert(theErr == QTSS_NoErr);
			return QTSS_WouldBlock;
		}

		Assert(theErr == QTSS_NoErr);

	    // 处理完成报文后会自动进行Delete处理
		OSCharArrayDeleter charArrayPathDeleter(fContentBuffer);

		qtss_printf("EasyCMSSession::ProcessMessage() Get Complete Msg:\n%s", fContentBuffer);

		EasyProtocol protocol(fContentBuffer);
		int nNetMsg = protocol.GetMessageType();
		switch (nNetMsg)
		{
		case  MSG_SC_FREE_STREAM_ACK:
			{
				string strErrorNum		=	protocol.GetHeaderValue(EASY_TAG_ERROR_NUM);
				string strSerial		=	protocol.GetBodyValue(EASY_TAG_SERIAL);
				string strChannle		=	protocol.GetBodyValue(EASY_TAG_CHANNEL);
				
				qtss_printf("EasyCMS停止推流响应:%s,Serial=%s,Channel=%s",strErrorNum.c_str(),strSerial.c_str(),strChannle.c_str());
				fLiveSession=false;//进入析构
			}
			break;
		default:
			break;
		}
	}
	
	// 重置fContentBuffer和fContentBufferOffset值
	fContentBufferOffset = 0;
	fContentBuffer = NULL;

	return QTSS_NoErr;
}

QTSS_Error EasyCMSSession::CSFreeStream()
{
	EasyDarwin::Protocol::EasyProtocolACK req(MSG_CS_FREE_STREAM_REQ);
	EasyJsonValue header,body;

	header[EASY_TAG_CSEQ]		=	1;
	header[EASY_TAG_VERSION]	=	EASY_PROTOCOL_VERSION;
	header[EASY_TAG_APP_TYPE]	=	EasyProtocol::GetAppTypeString(EASY_APP_TYPE_EASYDARWIN);

#ifdef __Win32__
	header[EASY_TAG_TERMINAL_TYPE]	=	EasyProtocol::GetTerminalTypeString(EASY_TERMINAL_TYPE_WINDOWS);
#elif defined __linux__
	header[EASY_TAG_TERMINAL_TYPE]	=	EasyProtocol::GetTerminalTypeString(EASY_TERMINAL_TYPE_LINUX);
#elif defined __MacOSX__
	header[EASY_TAG_TERMINAL_TYPE]	=	EasyProtocol::GetTerminalTypeString(EASY_TERMINAL_TYPE_IOS);
#endif


	body[EASY_TAG_SERIAL]		=	fStreamName;
	//body[EASY_TAG_CHANNEL]		=	fChannel;
	body[EASY_TAG_PROTOCOL]		=	EasyProtocol::GetProtocolString(EASY_PROTOCOL_TYPE_RTSP);
	body[EASY_TAG_RESERVE]		=	"1";

	req.SetHead(header);
	req.SetBody(body);

	string msg = req.GetMsg();

	StrPtrLen jsonContent((char*)msg.data());

	// 构造HTTP注册报文,提交给fOutputStream进行发送
	HTTPRequest httpReq(&QTSServerInterface::GetServerHeader(), httpRequestType);

	if(!httpReq.CreateRequestHeader()) return QTSS_Unimplemented;

	if (jsonContent.Len)
		httpReq.AppendContentLengthHeader(jsonContent.Len);

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpReq.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);

	fOutputStream.Put(respHeader);
	if (jsonContent.Len > 0) 
		fOutputStream.Put(jsonContent.Ptr, jsonContent.Len);

	return QTSS_NoErr;
}

QTSS_Error EasyCMSSession::FreeStream(const char * streamName)
{
	QTSS_Error theErr = QTSS_NoErr;

	do{
		//1.参数解析
		fEasyMsgType = MSG_CS_FREE_STREAM_REQ;

		if(strlen(streamName) == 0)
		{
			theErr = QTSS_BadArgument;
			break;
		}
		fStreamName = new char[strlen(streamName) + 1];
		strcpy(fStreamName,streamName);
		
		char chSerial[32] = {0};
		char * chPos = strstr(fStreamName,"/");
		if(chPos == NULL)
			return QTSS_BadArgument;

		memcpy(chSerial,fStreamName,chPos-fStreamName);
		//TODO::解析fStreamName出Serial/Channel.sdp
		//如果解析失败,返回QTSS_BadArgument

		//2.根据Serial查询到设备所在的EasyCMS信息
		char chCMSIP[20]={0};
		char chCMSPort[6] = {0};
		UInt16 uCMSPort;

		//if(!QTSServerInterface::GetServer()->RedisGetAssociatedCMS(chSerial,chCMSIP,uCMSPort))

		QTSS_RoleParams theParams;
		theParams.GetAssociatedCMSParams.outCMSIP = chCMSIP;
		theParams.GetAssociatedCMSParams.outCMSPort = chCMSPort;
		theParams.GetAssociatedCMSParams.inSerial = chSerial;
		UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kGetAssociatedCMSRole);
		for ( UInt32 currentModule=0;currentModule < numModules; currentModule++)
		{
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kGetAssociatedCMSRole, currentModule);
			(void)theModule->CallDispatch(Easy_RedisGetAssociatedCMS_Role, &theParams);
		}

		if(chCMSIP[0] == 0)
		{
			qtss_printf("获取关联CMS失败\n");
			return QTSS_ValueNotFound;
		}
		else
		{
			uCMSPort = atoi(chCMSPort);
			qtss_printf("获取关联CMS成功\n");
		}
		//TODO::
		//如果查询失败，返回QTSS_ValueNotFound


		//3.获取到EasyCMS参数进行Socket初始化
		UInt32 inAddr = SocketUtils::ConvertStringToAddr(chCMSIP);
		if(inAddr)
		{
			fSocket->Set(inAddr, uCMSPort);
		}
		else
		{
			theErr = QTSS_IllegalService;
			break;
		}

	}while(0);

	return theErr;
}