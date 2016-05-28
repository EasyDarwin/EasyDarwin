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

// EasyCMS IP
static char*            sEasyCMS_IP		= NULL;
static char*            sDefaultEasyCMS_IP_Addr = "127.0.0.1";
// EasyCMS Port
static UInt16			sEasyCMSPort			= 10000;
static UInt16			sDefaultEasyCMSPort		= 10000;
// EasyCamera Serial
static char*            sEasy_Serial			= NULL;
static char*            sDefaultEasy_Serial		= "CAM00000001";
// EasyCamera Name
static char*            sEasy_Name				= NULL;
static char*            sDefaultEasy_Name		= "CAM001";
// EasyCamera Secret key
static char*            sEasy_Key				= NULL;
static char*            sDefaultEasy_Key		= "123456";
// EasyCamera tag name
static char*			sEasy_Tag				= NULL;
static char*			sDefaultEasy_Tag		= "CAMTag001";
// EasyCMS Keep-Alive Interval
static UInt32			sKeepAliveInterval		= 30;
static UInt32			sDefKeepAliveInterval	= 30;


// 初始化读取配置文件中各项配置
void EasyCMSSession::Initialize(QTSS_ModulePrefsObject cmsModulePrefs)
{
	delete [] sEasyCMS_IP;
    sEasyCMS_IP = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "easycms_ip", sDefaultEasyCMS_IP_Addr);

	QTSSModuleUtils::GetAttribute(cmsModulePrefs, "easycms_port", qtssAttrDataTypeUInt16, &sEasyCMSPort, &sDefaultEasyCMSPort, sizeof(sEasyCMSPort));

	delete [] sEasy_Serial;
    sEasy_Serial = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "device_serial", sDefaultEasy_Serial);
	
	delete [] sEasy_Name;
    sEasy_Name = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "device_name", sDefaultEasy_Name);
	
	delete [] sEasy_Key;
    sEasy_Key = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "device_key", sDefaultEasy_Key);

	delete[] sEasy_Tag;
	sEasy_Tag = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "device_tag", sDefaultEasy_Tag);

	QTSSModuleUtils::GetAttribute(cmsModulePrefs, "keep_alive_interval", qtssAttrDataTypeUInt32, &sKeepAliveInterval, &sDefKeepAliveInterval, sizeof(sKeepAliveInterval));
}

EasyCMSSession::EasyCMSSession()
:	Task(),
	fSocket(NEW TCPClientSocket(Socket::kNonBlockingSocketType)),    
	fTimeoutTask(this, sKeepAliveInterval * 1000),
	fInputStream(fSocket),
	fOutputStream(fSocket, &fTimeoutTask),
	fSessionStatus(kSessionOffline),
	fState(kIdle),
	fRequest(NULL),
	fReadMutex(),
	fMutex(),
	fContentBufferOffset(0),
	fContentBuffer(NULL),
	fSnapReq(NULL),
	fSendMessageCount(0)
{
	this->SetTaskName("EasyCMSSession");

	UInt32 inAddr = SocketUtils::ConvertStringToAddr(sEasyCMS_IP);

	if (inAddr)
	{
		fSocket->Set(inAddr, sEasyCMSPort);
	}
	else
	{
		//connect default EasyCMS server
		fSocket->Set(SocketUtils::ConvertStringToAddr("121.40.50.44"), sEasyCMSPort);
	}

	fTimeoutTask.RefreshTimeout();

}

EasyCMSSession::~EasyCMSSession()
{
	if (fSocket)
	{
		delete fSocket;
		fSocket = NULL;
	}
	if (fRequest)
	{
		delete fRequest;
		fRequest = NULL;
	}
	if (fContentBuffer)
	{
		delete[] fContentBuffer;
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
	//OSMutexLocker locker(&fMutex);

	OS_Error theErr = OS_NoErr;
	EventFlags events = this->GetEvents();

	while(1)
	{
		switch (fState)
		{
			case kIdle:
				{
					if(!IsConnected())
					{
						// TCPSocket未连接的情况,首先进行登录连接
						DSRegister();
					}
					else
					{
						// TCPSocket已连接的情况下先区分具体事件类型
						if(events & Task::kStartEvent)
						{
							// 已连接，但状态为离线,重新进行上线动作
							if(kSessionOffline == fSessionStatus)
								DSRegister();
						}

						if(events & Task::kReadEvent)
						{
							// 已连接，有新消息需要读取(数据或者断开)
							fState = kReadingMessage;
						}

						if(events & Task::kTimeoutEvent)
						{
							// 已连接，保活时间到需要发送保活报文
							DSRegister();
							fTimeoutTask.RefreshTimeout();
						}

						if(events & Task::kUpdateEvent)
						{
							//已连接，发送快照更新
							DSPostSnap();
						}
					}
					
					// 如果有消息需要发送则进入发送流程
					if (fOutputStream.GetBytesWritten() > 0)
					{
						fState = kSendingMessage;
					}

					// 
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
						this->ResetClientSocket();

						// make zero
						fSendMessageCount = 0;

						return 0;
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

					QTSS_Error theErr = ProcessMessage();

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
					if (fSendMessageCount >= 3)
					{
						fState = kIdle;
						fSendMessageCount = 0;

						return 0;
					}

					theErr = fOutputStream.Flush();
                
					if (theErr == EAGAIN || theErr == EINPROGRESS)
					{
						qtss_printf("EasyCMSSession::Run fOutputStream.Flush() theErr:%d \n", theErr);
						fSocket->GetSocket()->SetTask(this);
						fSocket->GetSocket()->RequestEvent(fSocket->GetEventMask());
						this->ForceSameThread();
						return 0;
					}
					else if (theErr != QTSS_NoErr)
					{
						// Any other error means that the client has disconnected, right?
						Assert(!this->IsConnected());
						ResetClientSocket();
						// make zero
						fSendMessageCount = 0;

						return 0;
					}
            
					fState = kCleaningUp;
					break;
				}
			case kCleaningUp:
				{
					// Cleaning up consists of making sure we've read all the incoming Request Body
					// data off of the socket
					////if (this->GetRemainingReqBodyLen() > 0)
					////{
					////	err = this->DumpRequestData();
              
					////	if (err == EAGAIN)
					////	{
					////		fInputSocketP->RequestEvent(EV_RE);
					////		this->ForceSameThread();    // We are holding mutexes, so we need to force
					////									// the same thread to be used for next Run()
					////		return 0;
					////	}
					////}

					// clean up and wait for next run
					this->CleanupRequest();
					fState = kIdle;
					
					if(IsConnected())
					{
						fSocket->GetSocket()->SetTask(this);
						fSocket->GetSocket()->RequestEvent(EV_RE | EV_WR);
					}
					return 0;
				}
		}
	}
    return 0;
}

QTSS_Error EasyCMSSession::ProcessMessage()
{
	if(NULL == fRequest) return QTSS_BadArgument;

    QTSS_Error theErr = fRequest->Parse();
    if (theErr != QTSS_NoErr) 
		return QTSS_BadArgument;

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
		case  MSG_SD_REGISTER_ACK:
			{	
				EasyMsgSDRegisterACK ack(fContentBuffer);

				// make zero
				fSendMessageCount = 0;

				qtss_printf("session id = %s\n", ack.GetBodyValue(EASY_TAG_SESSION_ID).c_str());
				qtss_printf("device serial = %s\n", ack.GetBodyValue(EASY_TAG_SERIAL).c_str());
			}
			break;
		case MSG_SD_POST_SNAP_ACK:
			{
				// make zero
				fSendMessageCount = 0;
			}
			break;
		case MSG_SD_PUSH_STREAM_REQ:
			{
				EasyMsgSDPushStreamREQ	startStreamReq(fContentBuffer);

				string serial = startStreamReq.GetBodyValue(EASY_TAG_SERIAL);
				string ip = startStreamReq.GetBodyValue(EASY_TAG_SERVER_IP);
				string port = startStreamReq.GetBodyValue(EASY_TAG_SERVER_PORT);
				string protocol = startStreamReq.GetBodyValue(EASY_TAG_PROTOCOL);
				string channel = startStreamReq.GetBodyValue(EASY_TAG_CHANNEL);
				string streamID = startStreamReq.GetBodyValue(EASY_TAG_STREAM_ID);
				string reserve = startStreamReq.GetBodyValue(EASY_TAG_RESERVE);

				qtss_printf("Serial = %s\n", serial.c_str());
				qtss_printf("Server_IP = %s\n", ip.c_str());
				qtss_printf("Server_Port = %s\n", port.c_str());

				//TODO::这里需要对传入的Serial/StreamID/Channel做一下容错处理
				if (serial.empty() || ip.empty() || port.empty())
				{
					return QTSS_ValueNotFound;
				}
				
				QTSS_RoleParams params;

				params.startStreaParams.inIP = ip.c_str();
				params.startStreaParams.inPort = atoi(port.c_str());
				params.startStreaParams.inSerial = serial.c_str();
				params.startStreaParams.inProtocol = protocol.c_str();
				params.startStreaParams.inChannel = channel.c_str();
				params.startStreaParams.inStreamID = streamID.c_str();

				QTSS_Error errCode = QTSS_NoErr;
				UInt32 fCurrentModule = 0;
				UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kStartStreamRole);
				for (; fCurrentModule < numModules; ++fCurrentModule)
				{
					QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kStartStreamRole, fCurrentModule);
					errCode = theModule->CallDispatch(Easy_StartStream_Role, &params);
				}
				fCurrentModule = 0;

				EasyJsonValue body;
				body[EASY_TAG_SERIAL] = params.startStreaParams.inSerial;
				body[EASY_TAG_CHANNEL] = params.startStreaParams.inChannel;
				body[EASY_TAG_PROTOCOL] = params.startStreaParams.inProtocol;
				body[EASY_TAG_SERVER_IP] = params.startStreaParams.inIP;
				body[EASY_TAG_SERVER_PORT] = params.startStreaParams.inPort;
				body[EASY_TAG_RESERVE] = reserve;

				EasyMsgDSPushSteamACK rsp(body, startStreamReq.GetMsgCSeq(), GetStatusNo(errCode));

				string msg = rsp.GetMsg();
				StrPtrLen jsonContent((char*)msg.data());
				HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

				if(httpAck.CreateResponseHeader())
				{
					if (jsonContent.Len)
						httpAck.AppendContentLengthHeader(jsonContent.Len);

					//Push msg to OutputBuffer
					char respHeader[2048] = { 0 };
					StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
					strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);
		
					fOutputStream.Put(respHeader);
					if (jsonContent.Len > 0) 
						fOutputStream.Put(jsonContent.Ptr, jsonContent.Len);
				}
			}
			break;
		case MSG_SD_STREAM_STOP_REQ:
			{
				EasyMsgSDStopStreamREQ	stopStreamReq(fContentBuffer);

				QTSS_RoleParams params;

				string serial = stopStreamReq.GetBodyValue(EASY_TAG_SERIAL);
				params.stopStreamParams.inSerial = serial.c_str();
				string protocol = stopStreamReq.GetBodyValue(EASY_TAG_PROTOCOL);
				params.stopStreamParams.inProtocol = protocol.c_str();
				string channel = stopStreamReq.GetBodyValue(EASY_TAG_CHANNEL);
				params.stopStreamParams.inChannel = channel.c_str();

				QTSS_Error	errCode = QTSS_NoErr;
				UInt32 fCurrentModule = 0;
				UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kStopStreamRole);
				for (; fCurrentModule < numModules; ++fCurrentModule)
				{
					QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kStopStreamRole, fCurrentModule);
					errCode = theModule->CallDispatch(Easy_StopStream_Role, &params);
				}
				fCurrentModule = 0;

				EasyJsonValue body;
				body[EASY_TAG_SERIAL] = params.stopStreamParams.inSerial;
				body[EASY_TAG_CHANNEL] = params.stopStreamParams.inChannel;
				body[EASY_TAG_PROTOCOL] = params.stopStreamParams.inProtocol;				

				EasyMsgDSStopStreamACK rsp(body, stopStreamReq.GetMsgCSeq(), GetStatusNo(errCode));
				string msg = rsp.GetMsg();

				//回应
				StrPtrLen jsonContent((char*)msg.data());
				HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

				if(httpAck.CreateResponseHeader())
				{
					if (jsonContent.Len)
						httpAck.AppendContentLengthHeader(jsonContent.Len);

					//Push msg to OutputBuffer
					char respHeader[2048] = { 0 };
					StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
					strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);
		
					fOutputStream.Put(respHeader);
					if (jsonContent.Len > 0) 
						fOutputStream.Put(jsonContent.Ptr, jsonContent.Len);
				}
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

// transfer error code for http status code
size_t EasyCMSSession::GetStatusNo(QTSS_Error inError)
{
	size_t error(404);

	switch (inError)
	{
	case QTSS_NoErr:
		error = 200;
		break;
	case QTSS_RequestFailed:
		error = 404;
		break;
	case QTSS_Unimplemented:
		error = 405;
		break;
	case QTSS_RequestArrived:
		error = 500;
		break;
	case QTSS_OutOfState:
		break;
	case QTSS_WrongVersion:
		break;
	case QTSS_NotAModule:
	case QTSS_IllegalService:
		error = 605;
		break;
	case QTSS_BadIndex:
	case QTSS_ValueNotFound:
		error = 603;
		break;
	case QTSS_BadArgument:
		error = 400;
		break;
	case QTSS_ReadOnly:
		break;
	case QTSS_NotPreemptiveSafe:
		break;
	case QTSS_NotEnoughSpace:
		break;
	case QTSS_WouldBlock:
		break;
	case QTSS_NotConnected:
		break;
	case QTSS_FileNotFound:
		break;
	case QTSS_NoMoreData:
		break;
	case QTSS_AttrDoesntExist:
		break;
	case QTSS_AttrNameExists:
		break;
	case QTSS_InstanceAttrsNotAllowed:
		break;
	default:
		error = 404;
		break;
	}

	return error;
}

//
// 清除原有ClientSocket,NEW出新的ClientSocket
// 赋值Socket连接的服务器地址和端口
// 更新fInputSocket和fOutputSocket的fSocket对象
// 重置状态机状态
void EasyCMSSession::ResetClientSocket()
{
	qtss_printf("EasyCMSSession::ResetClientSocket()\n");

	CleanupRequest();

	fSocket->GetSocket()->Cleanup();
	if (fSocket) delete fSocket;

	fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
	UInt32 inAddr = SocketUtils::ConvertStringToAddr(sEasyCMS_IP);
	if (inAddr) fSocket->Set(inAddr, sEasyCMSPort);

	fInputStream.AttachToSocket(fSocket);
	fOutputStream.AttachToSocket(fSocket);
	fState = kIdle;
}


QTSS_Error EasyCMSSession::DSRegister()
{
	EasyDevices channels;

	EasyNVR nvr(sEasy_Serial, sEasy_Name, sEasy_Key, sEasy_Tag, channels);

	EasyMsgDSRegisterREQ req(EASY_TERMINAL_TYPE_ARM, EASY_APP_TYPE_CAMERA, nvr);
	string msg = req.GetMsg();

	StrPtrLen jsonContent((char*)msg.data());

	// 构造HTTP注册报文,提交给fOutputStream进行发送
	HTTPRequest httpReq(&QTSServerInterface::GetServerHeader(), httpRequestType);

	if (!httpReq.CreateRequestHeader()) return QTSS_Unimplemented;

	if (jsonContent.Len)
		httpReq.AppendContentLengthHeader(jsonContent.Len);

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpReq.GetCompleteHTTPHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);
		
	fOutputStream.Put(respHeader);
	if (jsonContent.Len > 0) 
		fOutputStream.Put(jsonContent.Ptr, jsonContent.Len);

	// count+1
	++fSendMessageCount;

	return QTSS_NoErr;
}

QTSS_Error EasyCMSSession::UpdateSnapCache(Easy_PostSnap_Params* inParams)
{
	if (fSnapReq == NULL)
	{
		char szTime[32] = { 0, };
		EasyJsonValue body;
		body[EASY_TAG_SERIAL] = sEasy_Serial;

		string type = EasyProtocol::GetSnapTypeString(inParams->snapType);

		body[EASY_TAG_TYPE] = type.c_str();
		body[EASY_TAG_TIME] = szTime;	
		body[EASY_TAG_IMAGE] = EasyUtil::Base64Encode((const char*)inParams->snapPtr, inParams->snapLen);
		
		fSnapReq = new EasyMsgDSPostSnapREQ(body, 1);
	}

	this->Signal(Task::kUpdateEvent);
	return QTSS_NoErr;
}

QTSS_Error EasyCMSSession::DSPostSnap()
{
	if (fSnapReq)
	{
		string msg = fSnapReq->GetMsg();

		//请求上传快照
		StrPtrLen jsonContent((char*)msg.data());
		HTTPRequest httpReq(&QTSServerInterface::GetServerHeader(), httpRequestType);

		if (httpReq.CreateRequestHeader())
		{
			if (jsonContent.Len)
				httpReq.AppendContentLengthHeader(jsonContent.Len);

			//Push msg to OutputBuffer
			char respHeader[2048] = { 0 };
			StrPtrLen* ackPtr = httpReq.GetCompleteHTTPHeader();
			strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);
			
			fOutputStream.Put(respHeader);
			if (jsonContent.Len > 0) 
				fOutputStream.Put(jsonContent.Ptr, jsonContent.Len);

			// count+1
			++fSendMessageCount;
		}

		delete fSnapReq;
		fSnapReq = NULL;
	}

	return QTSS_NoErr;
}