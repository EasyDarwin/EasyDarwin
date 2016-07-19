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
#include <OSMemory.h>
#include <SocketUtils.h>
#include <OSArrayObjectDeleter.h>
#include <QTSSModuleUtils.h>

// EasyCMS IP
static char*            sEasyCMS_IP = NULL;
static char*            sDefaultEasyCMS_IP_Addr = "www.easydss.com";
// EasyCMS Port
static UInt16			sEasyCMSPort = 10000;
static UInt16			sDefaultEasyCMSPort = 10000;
// EasyCamera Serial
static char*            sEasy_Serial = NULL;
static char*            sDefaultEasy_Serial = "CAM00000001";
// EasyCamera Name
static char*            sEasy_Name = NULL;
static char*            sDefaultEasy_Name = "CAM001";
// EasyCamera Secret key
static char*            sEasy_Key = NULL;
static char*            sDefaultEasy_Key = "123456";
// EasyCamera tag name
static char*			sEasy_Tag = NULL;
static char*			sDefaultEasy_Tag = "CAMTag001";
// EasyCMS Keep-Alive Interval
static UInt32			sKeepAliveInterval = 30;
static UInt32			sDefKeepAliveInterval = 30;
// EasyCMS Upload snap Interval
static UInt32			sUploadSnapInterval = 180;
static UInt32			sDefaultUploadSnapInterval = 180;

// 初始化读取配置文件中各项配置
void EasyCMSSession::Initialize(QTSS_ModulePrefsObject cmsModulePrefs)
{
	delete[] sEasyCMS_IP;
	sEasyCMS_IP = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "easycms_ip", sDefaultEasyCMS_IP_Addr);

	QTSSModuleUtils::GetAttribute(cmsModulePrefs, "easycms_port", qtssAttrDataTypeUInt16, &sEasyCMSPort, &sDefaultEasyCMSPort, sizeof(sEasyCMSPort));

	delete[] sEasy_Serial;
	sEasy_Serial = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "device_serial", sDefaultEasy_Serial);

	delete[] sEasy_Name;
	sEasy_Name = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "device_name", sDefaultEasy_Name);

	delete[] sEasy_Key;
	sEasy_Key = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "device_key", sDefaultEasy_Key);

	delete[] sEasy_Tag;
	sEasy_Tag = QTSSModuleUtils::GetStringAttribute(cmsModulePrefs, "device_tag", sDefaultEasy_Tag);

	QTSSModuleUtils::GetAttribute(cmsModulePrefs, "keep_alive_interval", qtssAttrDataTypeUInt32, &sKeepAliveInterval, &sDefKeepAliveInterval, sizeof(sKeepAliveInterval));

	QTSSModuleUtils::GetAttribute(cmsModulePrefs, "upload_snap_interval", qtssAttrDataTypeUInt32, &sUploadSnapInterval, &sDefaultUploadSnapInterval, sizeof(sUploadSnapInterval));

}

EasyCMSSession::EasyCMSSession()
	: Task(),
	fSocket(NEW TCPClientSocket(Socket::kNonBlockingSocketType)),
	fTimeoutTask(NULL),
	fInputStream(new HTTPRequestStream(fSocket)),
	fOutputStream(NULL),
	fSessionStatus(kSessionOffline),
	fState(kIdle),
	fRequest(NULL),
	fReadMutex(),
	fMutex(),
	fContentBufferOffset(0),
	fContentBuffer(NULL),
	fNoneACKMsgCount(0),
	fCSeq(1)
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
		fSocket->Set(SocketUtils::ConvertStringToAddr("www.easydss.com"), sEasyCMSPort);
	}

	fTimeoutTask = new TimeoutTask(this, sKeepAliveInterval * 1000);
	fOutputStream = new HTTPResponseStream(fSocket, fTimeoutTask);
	fTimeoutTask->RefreshTimeout();

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

	if (fTimeoutTask)
	{
		delete fTimeoutTask;
		fTimeoutTask = NULL;
	}

	if (fInputStream)
	{
		delete fInputStream;
		fInputStream = NULL;
	}

	if (fOutputStream)
	{
		delete fOutputStream;
		fOutputStream = NULL;
	}

}

void EasyCMSSession::cleanupRequest()
{
	if (fRequest != NULL)
	{
		// NULL out any references to the current request
		delete fRequest;
		fRequest = NULL;
	}

	//清空发送缓冲区
	fOutputStream->ResetBytesWritten();
}

SInt64 EasyCMSSession::Run()
{
	//OSMutexLocker locker(&fMutex);

	OS_Error theErr = OS_NoErr;
	EventFlags events = this->GetEvents();

	while (true)
	{
		switch (fState)
		{
		case kIdle:
			{
				if (!isConnected())
				{
					// TCPSocket未连接的情况,首先进行登录连接
					if (doDSRegister() == QTSS_NoErr && sUploadSnapInterval >= sKeepAliveInterval)
					{
						doDSPostSnap();
					}
				}
				else
				{
					printf("EasyCamera EasyCMSSession ackcount -- %d \n", fNoneACKMsgCount);
					if (fNoneACKMsgCount > 3)
					{
						printf("ackmsgcount > 3 \n");
						this->resetClientSocket();

						return 0;
					}

					// TCPSocket已连接的情况下先区分具体事件类型
					if (events & Task::kStartEvent)
					{
						// 已连接，但状态为离线,重新进行上线动作
						if (kSessionOffline == fSessionStatus)
						{
							if (doDSRegister() == QTSS_NoErr && sUploadSnapInterval >= sKeepAliveInterval)
							{
								doDSPostSnap();
							}
						}

					}

					if (events & Task::kReadEvent)
					{
						// 已连接，有新消息需要读取(数据或者断开)
						fState = kReadingMessage;
					}

					if (events & Task::kTimeoutEvent)
					{
						// 已连接，保活时间到需要发送保活报文
						if (sUploadSnapInterval >= sKeepAliveInterval && fCSeq % (sUploadSnapInterval / sKeepAliveInterval) == 1)
						{
							printf("post snap \n");
							doDSPostSnap();
						}
						else
						{
							printf("register camera \n");
							doDSRegister();
						}

						fTimeoutTask->RefreshTimeout();
					}

					if (events & Task::kUpdateEvent && sUploadSnapInterval >= sKeepAliveInterval)
					{
						//已连接，发送快照更新
						doDSPostSnap();
					}
				}

				// 如果有消息需要发送则进入发送流程
				if (fOutputStream->GetBytesWritten() > 0)
				{
					fState = kSendingMessage;
				}

				// 
				if (kIdle == fState)
				{
					return 0;
				}

				break;
			}
		case kReadingMessage:
			{
				// 网络请求报文存储在fInputStream中
				if ((theErr = fInputStream->ReadRequest()) == QTSS_NoErr)
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
					printf("reading message theErr == %d \n", theErr);
					this->resetClientSocket();

					return 0;
				}

				// 网络请求超过了缓冲区，返回Bad Request
				if ((theErr == E2BIG) || (theErr == QTSS_BadArgument))
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
				Assert(fInputStream->GetRequestBuffer());
				Assert(fRequest == NULL);

				// 根据具体请求报文构造HTTPRequest请求类
				fRequest = NEW HTTPRequest(&QTSServerInterface::GetServerHeader(), fInputStream->GetRequestBuffer());

				// 清空发送缓冲区
				fOutputStream->ResetBytesWritten();

				Assert(theErr == QTSS_RequestArrived);

				QTSS_Error err = processMessage();

				if (err == QTSS_WouldBlock)
				{
					fSocket->GetSocket()->SetTask(this);
					fSocket->GetSocket()->RequestEvent(EV_RE);

					return 0;
				}

				// 每一步都检测响应报文是否已完成，完成则直接进行回复响应
				if (fOutputStream->GetBytesWritten() > 0)
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
				theErr = fOutputStream->Flush();

				if (theErr == EAGAIN || theErr == EINPROGRESS)
				{
					//qtss_printf("EasyCMSSession::Run fOutputStream.Flush() theErr:%d \n", theErr);
					fSocket->GetSocket()->SetTask(this);
					fSocket->GetSocket()->RequestEvent(fSocket->GetEventMask());
					this->ForceSameThread();
					return 0;
				}
				else if (theErr != QTSS_NoErr)
				{
					// Any other error means that the client has disconnected, right?
					printf("sending message != noerr errno %d \n", theErr);
					Assert(!this->isConnected());
					resetClientSocket();
					return 0;
				}

				++fNoneACKMsgCount;

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
				cleanupRequest();
				fState = kIdle;

				if (isConnected())
				{
					fSocket->GetSocket()->SetTask(this);
					fSocket->GetSocket()->RequestEvent(EV_RE | EV_WR);
				}
				return 0;
			}

		}
	}
}

void EasyCMSSession::stopPushStream() const
{
	QTSS_RoleParams params;

	UInt32 fCurrentModule = 0;
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kStopStreamRole);
	for (; fCurrentModule < numModules; ++fCurrentModule)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kStopStreamRole, fCurrentModule);
		QTSS_Error errCode = theModule->CallDispatch(Easy_StopStream_Role, &params);
	}
}

QTSS_Error EasyCMSSession::processMessage()
{
	if (NULL == fRequest) return QTSS_BadArgument;

	QTSS_Error theErr = fRequest->Parse();
	if (theErr != QTSS_NoErr)
		return QTSS_BadArgument;

	//获取具体Content json数据部分
	StrPtrLen* lengthPtr = fRequest->GetHeaderValue(httpContentLengthHeader);
	StringParser theContentLenParser(lengthPtr);
	theContentLenParser.ConsumeWhitespace();
	UInt32 content_length = theContentLenParser.ConsumeInteger(NULL);

	//printf("EasyCMSSession::ProcessMessage Parse ContentLength:%d \n", content_length);

	if (content_length)
	{
		//qtss_printf("EasyCMSSession::ProcessMessage read content-length:%lu \n", content_length);
		// 检查content的fContentBuffer和fContentBufferOffset是否有值存在,如果存在，说明我们已经开始
		// 进行content请求处理,如果不存在,我们需要创建并初始化fContentBuffer和fContentBufferOffset
		if (fContentBuffer == NULL)
		{
			fContentBuffer = NEW char[content_length + 1];
			memset(fContentBuffer, 0, content_length + 1);
			fContentBufferOffset = 0;
		}

		UInt32 theLen = 0;
		// 读取HTTP Content报文数据
		theErr = fInputStream->Read(fContentBuffer + fContentBufferOffset, content_length - fContentBufferOffset, &theLen);
		Assert(theErr != QTSS_BadArgument);

		//printf("EasyCMSSession::ProcessMessage Read Length:%d (%d/%d) \n", theLen, fContentBufferOffset+theLen, content_length);

		if (theErr == QTSS_RequestFailed)
		{
			OSCharArrayDeleter charArrayPathDeleter(fContentBuffer);
			fContentBufferOffset = 0;
			fContentBuffer = NULL;

			return QTSS_RequestFailed;
		}

		//printf("EasyCMSSession::ProcessMessage() Add Len:%lu \n", theLen);

		if ((theErr == QTSS_WouldBlock) || (theLen < (content_length - fContentBufferOffset)))
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

		//printf("EasyCMSSession::ProcessMessage Get Complete Msg:\n%s", fContentBuffer);

		fNoneACKMsgCount = 0;

		EasyProtocol protocol(fContentBuffer);
		int nNetMsg = protocol.GetMessageType();
		switch (nNetMsg)
		{
		case MSG_SD_REGISTER_ACK:
			{
				EasyMsgSDRegisterACK ack(fContentBuffer);

				qtss_printf("session id = %s\n", ack.GetBodyValue(EASY_TAG_SESSION_ID).c_str());
				qtss_printf("device serial = %s\n", ack.GetBodyValue(EASY_TAG_SERIAL).c_str());
			}
			break;
		case MSG_SD_POST_SNAP_ACK:
			{
				;
			}
			break;
		case MSG_SD_PUSH_STREAM_REQ:
			theErr = processStartStreamReq();
			break;
		case MSG_SD_STREAM_STOP_REQ:
			theErr = processStopStreamReq();
			break;
		case MSG_SD_CONTROL_PTZ_REQ:
			theErr = processControlPTZReq();
			break;
		case MSG_SD_CONTROL_PRESET_REQ:
			theErr = processControlPresetReq();
			break;
		case MSG_SD_CONTROL_TALKBACK_REQ:
			theErr = processControlTalkbackReq();
			break;
		default:
			break;
		}
	}

	// 重置fContentBuffer和fContentBufferOffset值
	fContentBufferOffset = 0;
	fContentBuffer = NULL;

	return theErr;
}

QTSS_Error EasyCMSSession::processStartStreamReq() const
{
	EasyMsgSDPushStreamREQ	startStreamReq(fContentBuffer);

	string serial = startStreamReq.GetBodyValue(EASY_TAG_SERIAL);
	string ip = startStreamReq.GetBodyValue(EASY_TAG_SERVER_IP);
	string port = startStreamReq.GetBodyValue(EASY_TAG_SERVER_PORT);
	string protocol = startStreamReq.GetBodyValue(EASY_TAG_PROTOCOL);
	string channel = startStreamReq.GetBodyValue(EASY_TAG_CHANNEL);
	string streamID = startStreamReq.GetBodyValue(EASY_TAG_STREAM_ID);
	string reserve = startStreamReq.GetBodyValue(EASY_TAG_RESERVE);
	string from = startStreamReq.GetBodyValue(EASY_TAG_FROM);
	string to = startStreamReq.GetBodyValue(EASY_TAG_TO);
	string via = startStreamReq.GetBodyValue(EASY_TAG_VIA);

	qtss_printf("Serial = %s\n", serial.c_str());
	qtss_printf("Server_IP = %s\n", ip.c_str());
	qtss_printf("Server_Port = %s\n", port.c_str());

	//TODO::这里需要对传入的Serial/StreamID/Channel做一下容错处理
	if (serial.empty() || ip.empty() || port.empty())
	{
		return QTSS_ValueNotFound;
	}

	QTSS_RoleParams params;

	params.startStreamParams.inIP = ip.c_str();
	params.startStreamParams.inPort = atoi(port.c_str());
	params.startStreamParams.inSerial = serial.c_str();
	params.startStreamParams.inProtocol = protocol.c_str();
	params.startStreamParams.inChannel = channel.c_str();
	params.startStreamParams.inStreamID = streamID.c_str();

	QTSS_Error errCode = QTSS_NoErr;
	UInt32 fCurrentModule = 0;
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kStartStreamRole);
	for (; fCurrentModule < numModules; ++fCurrentModule)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kStartStreamRole, fCurrentModule);
		errCode = theModule->CallDispatch(Easy_StartStream_Role, &params);
	}

	EasyJsonValue body;
	body[EASY_TAG_SERIAL] = params.startStreamParams.inSerial;
	body[EASY_TAG_CHANNEL] = params.startStreamParams.inChannel;
	body[EASY_TAG_PROTOCOL] = params.startStreamParams.inProtocol;
	body[EASY_TAG_SERVER_IP] = params.startStreamParams.inIP;
	body[EASY_TAG_SERVER_PORT] = params.startStreamParams.inPort;
	body[EASY_TAG_RESERVE] = reserve;
	body[EASY_TAG_FROM] = to;
	body[EASY_TAG_TO] = from;
	body[EASY_TAG_VIA] = via;

	EasyMsgDSPushSteamACK rsp(body, startStreamReq.GetMsgCSeq(), getStatusNo(errCode));

	string msg = rsp.GetMsg();
	StrPtrLen jsonContent((char*)msg.data());
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

	if (httpAck.CreateResponseHeader())
	{
		if (jsonContent.Len)
			httpAck.AppendContentLengthHeader(jsonContent.Len);

		//Push msg to OutputBuffer
		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		fOutputStream->Put(respHeader);
		if (jsonContent.Len > 0)
			fOutputStream->Put(jsonContent.Ptr, jsonContent.Len);
	}

	return errCode;
}

QTSS_Error EasyCMSSession::processStopStreamReq() const
{
	EasyMsgSDStopStreamREQ stopStreamReq(fContentBuffer);

	QTSS_RoleParams params;

	string serial = stopStreamReq.GetBodyValue(EASY_TAG_SERIAL);
	params.stopStreamParams.inSerial = serial.c_str();
	string protocol = stopStreamReq.GetBodyValue(EASY_TAG_PROTOCOL);
	params.stopStreamParams.inProtocol = protocol.c_str();
	string channel = stopStreamReq.GetBodyValue(EASY_TAG_CHANNEL);
	params.stopStreamParams.inChannel = channel.c_str();
	string from = stopStreamReq.GetBodyValue(EASY_TAG_FROM);
	string to = stopStreamReq.GetBodyValue(EASY_TAG_TO);
	string via = stopStreamReq.GetBodyValue(EASY_TAG_VIA);

	QTSS_Error errCode = QTSS_NoErr;
	UInt32 fCurrentModule = 0;
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kStopStreamRole);
	for (; fCurrentModule < numModules; ++fCurrentModule)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kStopStreamRole, fCurrentModule);
		errCode = theModule->CallDispatch(Easy_StopStream_Role, &params);
	}

	EasyJsonValue body;
	body[EASY_TAG_SERIAL] = params.stopStreamParams.inSerial;
	body[EASY_TAG_CHANNEL] = params.stopStreamParams.inChannel;
	body[EASY_TAG_PROTOCOL] = params.stopStreamParams.inProtocol;
	body[EASY_TAG_FROM] = to;
	body[EASY_TAG_TO] = from;
	body[EASY_TAG_VIA] = via;

	EasyMsgDSStopStreamACK rsp(body, stopStreamReq.GetMsgCSeq(), getStatusNo(errCode));
	string msg = rsp.GetMsg();

	//回应
	StrPtrLen jsonContent((char*)msg.data());
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

	if (httpAck.CreateResponseHeader())
	{
		if (jsonContent.Len)
			httpAck.AppendContentLengthHeader(jsonContent.Len);

		//Push msg to OutputBuffer
		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		fOutputStream->Put(respHeader);
		if (jsonContent.Len > 0)
			fOutputStream->Put(jsonContent.Ptr, jsonContent.Len);
	}

	return errCode;
}

// transfer error code for http status code
size_t EasyCMSSession::getStatusNo(QTSS_Error inError)
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
void EasyCMSSession::resetClientSocket()
{
	qtss_printf("EasyCMSSession::ResetClientSocket()\n");

	stopPushStream();

	cleanupRequest();

	if (fSocket)
	{
		fSocket->GetSocket()->Cleanup();
		delete fSocket;
		fSocket = NULL;
	}

	fSocket = NEW TCPClientSocket(Socket::kNonBlockingSocketType);
	UInt32 inAddr = SocketUtils::ConvertStringToAddr(sEasyCMS_IP);
	if (inAddr) fSocket->Set(inAddr, sEasyCMSPort);

	fInputStream->AttachToSocket(fSocket);
	fOutputStream->AttachToSocket(fSocket);
	fState = kIdle;

	fNoneACKMsgCount = 0;
}

QTSS_Error EasyCMSSession::doDSRegister()
{
	QTSS_Error theErr = QTSS_NoErr;

	QTSS_RoleParams params;
	params.cameraStateParams.outIsLogin = 0;

	UInt32 fCurrentModule = 0;
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kGetCameraStateRole);
	for (; fCurrentModule < numModules; fCurrentModule++)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kGetCameraStateRole, fCurrentModule);
		theErr = theModule->CallDispatch(Easy_GetCameraState_Role, &params);
	}

	if ((theErr == QTSS_NoErr) && params.cameraStateParams.outIsLogin)
	{
		EasyDevices channels;

		EasyNVR nvr(sEasy_Serial, sEasy_Name, sEasy_Key, sEasy_Tag, channels);

		EasyMsgDSRegisterREQ req(EASY_TERMINAL_TYPE_ARM, EASY_APP_TYPE_CAMERA, nvr, fCSeq++);
		string msg = req.GetMsg();

		StrPtrLen jsonContent((char*)msg.data());

		// 构造HTTP注册报文,提交给fOutputStream进行发送
		HTTPRequest httpReq(&QTSServerInterface::GetServerHeader(), httpRequestType);

		if (!httpReq.CreateRequestHeader()) return QTSS_Unimplemented;

		if (jsonContent.Len)
			httpReq.AppendContentLengthHeader(jsonContent.Len);

		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpReq.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		fOutputStream->Put(respHeader);
		if (jsonContent.Len > 0)
			fOutputStream->Put(jsonContent.Ptr, jsonContent.Len);
	}

	return theErr;
}

QTSS_Error EasyCMSSession::doDSPostSnap()
{
	QTSS_Error theErr = QTSS_NoErr;

	QTSS_RoleParams params;
	params.cameraSnapParams.outSnapLen = 0;

	UInt32 fCurrentModule = 0;
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kGetCameraSnapRole);
	for (; fCurrentModule < numModules; fCurrentModule++)
	{
		qtss_printf("EasyCameraSource::Run::kGetCameraSnapRole\n");
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kGetCameraSnapRole, fCurrentModule);
		theErr = theModule->CallDispatch(Easy_GetCameraSnap_Role, &params);
	}

	if ((theErr == QTSS_NoErr) && (params.cameraSnapParams.outSnapLen > 0))
	{
		char szTime[32] = { 0, };
		EasyJsonValue body;
		body[EASY_TAG_SERIAL] = sEasy_Serial;

		string type = EasyProtocol::GetSnapTypeString(params.cameraSnapParams.outSnapType);

		body[EASY_TAG_TYPE] = type.c_str();
		body[EASY_TAG_TIME] = szTime;
		body[EASY_TAG_IMAGE] = EasyUtil::Base64Encode(reinterpret_cast<const char*>(params.cameraSnapParams.outSnapPtr), params.cameraSnapParams.outSnapLen);

		EasyMsgDSPostSnapREQ req(body, fCSeq++);
		string msg = req.GetMsg();

		//请求上传快照
		StrPtrLen jsonContent(const_cast<char*>(msg.data()));
		HTTPRequest httpReq(&QTSServerInterface::GetServerHeader(), httpRequestType);

		if (httpReq.CreateRequestHeader())
		{
			if (jsonContent.Len)
				httpReq.AppendContentLengthHeader(jsonContent.Len);

			//Push msg to OutputBuffer
			char respHeader[2048] = { 0 };
			StrPtrLen* ackPtr = httpReq.GetCompleteHTTPHeader();
			strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

			fOutputStream->Put(respHeader);
			if (jsonContent.Len > 0)
				fOutputStream->Put(jsonContent.Ptr, jsonContent.Len);
		}
	}

	return theErr;
}

QTSS_Error EasyCMSSession::processControlPTZReq() const
{
	EasyMsgSDControlPTZREQ ctrlPTZReq(fContentBuffer);

	string serial = ctrlPTZReq.GetBodyValue(EASY_TAG_SERIAL);
	string protocol = ctrlPTZReq.GetBodyValue(EASY_TAG_PROTOCOL);
	string channel = ctrlPTZReq.GetBodyValue(EASY_TAG_CHANNEL);
	string reserve = ctrlPTZReq.GetBodyValue(EASY_TAG_RESERVE);
	string actionType = ctrlPTZReq.GetBodyValue(EASY_TAG_ACTION_TYPE);
	string command = ctrlPTZReq.GetBodyValue(EASY_TAG_CMD);
	string speed = ctrlPTZReq.GetBodyValue(EASY_TAG_SPEED);
	string from = ctrlPTZReq.GetBodyValue(EASY_TAG_FROM);
	string to = ctrlPTZReq.GetBodyValue(EASY_TAG_TO);
	string via = ctrlPTZReq.GetBodyValue(EASY_TAG_VIA);

	if (serial.empty() || channel.empty() || command.empty())
	{
		return QTSS_ValueNotFound;
	}

	QTSS_RoleParams params;
	params.cameraPTZParams.inActionType = EasyProtocol::GetPTZActionType(actionType);
	params.cameraPTZParams.inCommand = EasyProtocol::GetPTZCMDType(command);
	params.cameraPTZParams.inSpeed = EasyUtil::String2Int(speed);

	QTSS_Error errCode = QTSS_NoErr;
	UInt32 fCurrentModule = 0;
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kControlPTZRole);
	for (; fCurrentModule < numModules; ++fCurrentModule)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kControlPTZRole, fCurrentModule);
		errCode = theModule->CallDispatch(Easy_ControlPTZ_Role, &params);
	}

	EasyJsonValue body;
	body[EASY_TAG_SERIAL] = serial;
	body[EASY_TAG_CHANNEL] = channel;
	body[EASY_TAG_RESERVE] = reserve;
	body[EASY_TAG_PROTOCOL] = protocol;
	body[EASY_TAG_FROM] = to;
	body[EASY_TAG_TO] = from;
	body[EASY_TAG_VIA] = via;

	EasyMsgDSControlPTZACK rsp(body, ctrlPTZReq.GetMsgCSeq(), getStatusNo(errCode));

	string msg = rsp.GetMsg();
	StrPtrLen jsonContent(const_cast<char*>(msg.data()));
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

	if (httpAck.CreateResponseHeader())
	{
		if (jsonContent.Len)
			httpAck.AppendContentLengthHeader(jsonContent.Len);

		//Push msg to OutputBuffer
		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		fOutputStream->Put(respHeader);
		if (jsonContent.Len > 0)
			fOutputStream->Put(jsonContent.Ptr, jsonContent.Len);
	}

	return errCode;
}

QTSS_Error EasyCMSSession::processControlPresetReq() const
{
	EasyMsgSDControlPresetREQ ctrlPresetReq(fContentBuffer);

	string serial = ctrlPresetReq.GetBodyValue(EASY_TAG_SERIAL);
	string protocol = ctrlPresetReq.GetBodyValue(EASY_TAG_PROTOCOL);
	string channel = ctrlPresetReq.GetBodyValue(EASY_TAG_CHANNEL);
	string reserve = ctrlPresetReq.GetBodyValue(EASY_TAG_RESERVE);
	string command = ctrlPresetReq.GetBodyValue(EASY_TAG_CMD);
	string preset = ctrlPresetReq.GetBodyValue(EASY_TAG_PRESET);
	string from = ctrlPresetReq.GetBodyValue(EASY_TAG_FROM);
	string to = ctrlPresetReq.GetBodyValue(EASY_TAG_TO);
	string via = ctrlPresetReq.GetBodyValue(EASY_TAG_VIA);

	if (serial.empty() || channel.empty() || command.empty())
	{
		return QTSS_ValueNotFound;
	}

	QTSS_RoleParams params;
	params.cameraPresetParams.inCommand = EasyProtocol::GetPresetCMDType(command);
	params.cameraPresetParams.inPreset = EasyUtil::String2Int(preset);

	QTSS_Error errCode = QTSS_NoErr;
	UInt32 fCurrentModule = 0;
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kControlPresetRole);
	for (; fCurrentModule < numModules; ++fCurrentModule)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kControlPresetRole, fCurrentModule);
		errCode = theModule->CallDispatch(Easy_ControlPreset_Role, &params);
	}

	EasyJsonValue body;
	body[EASY_TAG_SERIAL] = serial;
	body[EASY_TAG_CHANNEL] = channel;
	body[EASY_TAG_RESERVE] = reserve;
	body[EASY_TAG_PROTOCOL] = protocol;
	body[EASY_TAG_FROM] = to;
	body[EASY_TAG_TO] = from;
	body[EASY_TAG_VIA] = via;

	EasyMsgDSControlPresetACK rsp(body, ctrlPresetReq.GetMsgCSeq(), getStatusNo(errCode));

	string msg = rsp.GetMsg();
	StrPtrLen jsonContent(const_cast<char*>(msg.data()));
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

	if (httpAck.CreateResponseHeader())
	{
		if (jsonContent.Len)
			httpAck.AppendContentLengthHeader(jsonContent.Len);

		//Push msg to OutputBuffer
		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		fOutputStream->Put(respHeader);
		if (jsonContent.Len > 0)
			fOutputStream->Put(jsonContent.Ptr, jsonContent.Len);
	}

	return errCode;
}

QTSS_Error EasyCMSSession::processControlTalkbackReq() const
{
	EasyMsgSDControlTalkbackREQ ctrlTalkbackReq(fContentBuffer);

	string serial = ctrlTalkbackReq.GetBodyValue(EASY_TAG_SERIAL);
	string protocol = ctrlTalkbackReq.GetBodyValue(EASY_TAG_PROTOCOL);
	string channel = ctrlTalkbackReq.GetBodyValue(EASY_TAG_CHANNEL);
	string reserve = ctrlTalkbackReq.GetBodyValue(EASY_TAG_RESERVE);
	string command = ctrlTalkbackReq.GetBodyValue(EASY_TAG_CMD);
	string audioType = ctrlTalkbackReq.GetBodyValue(EASY_TAG_AUDIO_TYPE);
	string audio = ctrlTalkbackReq.GetBodyValue(EASY_TAG_AUDIO_DATA);
	string pts = ctrlTalkbackReq.GetBodyValue(EASY_TAG_PTS);
	string from = ctrlTalkbackReq.GetBodyValue(EASY_TAG_FROM);
	string to = ctrlTalkbackReq.GetBodyValue(EASY_TAG_TO);
	string via = ctrlTalkbackReq.GetBodyValue(EASY_TAG_VIA);

	if (serial.empty() || channel.empty() || command.empty())
	{
		return QTSS_ValueNotFound;
	}

	QTSS_RoleParams params;
	params.cameraTalkbackParams.inCommand = EasyProtocol::GetTalkbackCMDType(command);
	params.cameraTalkbackParams.inType = EasyProtocol::GetTalkbackAudioType(audioType);

	string audioData;
	audioData.reserve(1600);
	if (audio.empty())
	{
		params.cameraTalkbackParams.inBuff = NULL;
		params.cameraTalkbackParams.inBuffLen = 0;
		params.cameraTalkbackParams.inPts = 0;
	}
	else
	{
		audioData.clear();
		audioData = EasyUtil::Base64Decode(audio);
		params.cameraTalkbackParams.inBuff = const_cast<char*>(audioData.data());
		params.cameraTalkbackParams.inBuffLen = audioData.size();
		params.cameraTalkbackParams.inPts = EasyUtil::String2Int(pts);
	}

	QTSS_Error errCode = QTSS_NoErr;
	UInt32 fCurrentModule = 0;
	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kControlTalkbackRole);
	for (; fCurrentModule < numModules; ++fCurrentModule)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kControlTalkbackRole, fCurrentModule);
		errCode = theModule->CallDispatch(Easy_ControlTalkback_Role, &params);
	}

	EasyJsonValue body;
	body[EASY_TAG_SERIAL] = serial;
	body[EASY_TAG_CHANNEL] = channel;
	body[EASY_TAG_RESERVE] = reserve;
	body[EASY_TAG_PROTOCOL] = protocol;
	body[EASY_TAG_FROM] = to;
	body[EASY_TAG_TO] = from;
	body[EASY_TAG_VIA] = via;

	EasyMsgDSControlTalkbackACK rsp(body, ctrlTalkbackReq.GetMsgCSeq(), getStatusNo(errCode));

	string msg = rsp.GetMsg();
	StrPtrLen jsonContent(const_cast<char*>(msg.data()));
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

	if (httpAck.CreateResponseHeader())
	{
		if (jsonContent.Len)
			httpAck.AppendContentLengthHeader(jsonContent.Len);

		//Push msg to OutputBuffer
		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		fOutputStream->Put(respHeader);
		if (jsonContent.Len > 0)
			fOutputStream->Put(jsonContent.Ptr, jsonContent.Len);
	}

	return errCode;
}
