/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
	File:       HTTPSession.cpp
	Contains:   EasyCMS HTTPSession
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
static const int sHeaderSize = 2048;
static const int sIPSize = 20;
static const int sPortSize = 6;

#define	_WIDTHBYTES(c)		((c+31)/32*4)	// c = width * bpp
#define	SNAP_CAPTURE_TIME	30
#define SNAP_IMAGE_WIDTH	320
#define	SNAP_IMAGE_HEIGHT	180
#define	SNAP_SIZE			SNAP_IMAGE_WIDTH * SNAP_IMAGE_HEIGHT * 3 + 58

HTTPSession::HTTPSession()
	: HTTPSessionInterface(),
	fRequest(NULL),
	fReadMutex(),
	fCurrentModule(0),
	fState(kReadingFirstRequest)
{
	this->SetTaskName("HTTPSession");

	//All EasyCameraSession/EasyNVRSession/EasyHTTPSession
	QTSServerInterface::GetServer()->AlterCurrentHTTPSessionCount(1);

	fModuleState.curModule = NULL;
	fModuleState.curTask = this;
	fModuleState.curRole = 0;
	fModuleState.globalLockRequested = false;

	memset(&decodeParam, 0x00, sizeof(DECODE_PARAM_T));

	decodeParam.gopTally = SNAP_CAPTURE_TIME;

	decodeParam.imageData = new char[SNAP_SIZE];
	memset(decodeParam.imageData, 0, SNAP_SIZE);

	OSRefTableEx* sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
	sessionMap->Register(fSessionID, this);

	qtss_printf("Create HTTPSession:%s\n", fSessionID);
}

HTTPSession::~HTTPSession()
{
	if (decodeParam.imageData)
	{
		delete[]decodeParam.imageData;
		decodeParam.imageData = NULL;
	}

	fLiveSession = false;
	this->CleanupRequest();

	QTSServerInterface::GetServer()->AlterCurrentHTTPSessionCount(-1);

	OSRefTableEx* sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
	sessionMap->UnRegister(fSessionID);

	qtss_printf("Release HTTPSession:%s\n", fSessionID);

	if (fRequestBody)
	{
		delete[]fRequestBody;
		fRequestBody = NULL;
	}
}

SInt64 HTTPSession::Run()
{
	EventFlags events = this->GetEvents();
	QTSS_Error err = QTSS_NoErr;

	OSThreadDataSetter theSetter(&fModuleState, NULL);

	if (events & kKillEvent)
		fLiveSession = false;

	//if (events & kUpdateEvent)
	//{
	//	OSMutexLocker locker(&fQueueMutex);

	//	if (fEventQueue.GetLength() > 0)
	//	{
	//		SendMsgEvent* theEvent = static_cast<SendMsgEvent*>(fEventQueue.DeQueue()->GetEnclosingObject());
	//		string msg = theEvent->GetHTTPMessage();
	//		HTTPType httpType = theEvent->GetHTTPType();
	//		//构造响应报文(HTTP Header)
	//		HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpType);

	//		if (httpAck.CreateResponseHeader(!msg.empty() ? httpOK : httpNotImplemented))
	//		{
	//			if (!msg.empty())
	//				httpAck.AppendContentLengthHeader(static_cast<UInt32>(msg.length()));

	//			char respHeader[sHeaderSize] = { 0 };
	//			StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	//			strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

	//			HTTPResponseStream* pOutputStream = GetOutputStream();
	//			pOutputStream->Put(respHeader);

	//			//将相应报文添加到HTTP输出缓冲区中
	//			if (!msg.empty())
	//				pOutputStream->Put(const_cast<char*>(msg.data()), msg.length());
	//		}
	//		delete theEvent;
	//	}
	//	fState = kSendingResponse;
	//}

	if (events & kTimeoutEvent)
	{
		char msgStr[512];
		qtss_snprintf(msgStr, sizeof(msgStr), "Timeout HTTPSession，Device_serial[%s]\n", fDevice.serial_.c_str());
		QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);
		fLiveSession = false;
		this->Signal(kKillEvent);
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
					Assert(!this->IsLiveSession());
					break;
				}
				fState = kHaveCompleteMessage;
			}
		case kHaveCompleteMessage:
			{
				Assert(fInputStream.GetRequestBuffer());

				Assert(fRequest == NULL);
				fRequest = NEW HTTPRequest(&QTSServerInterface::GetServerHeader(), fInputStream.GetRequestBuffer());

				fReadMutex.Lock();
				fSessionMutex.Lock();

				fOutputStream.ResetBytesWritten();

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
				fTimeoutTask.RefreshTimeout();

				QTSS_Error theErr = SetupRequest();

				if (theErr == QTSS_WouldBlock)
				{
					this->ForceSameThread();
					fInputSocketP->RequestEvent(EV_RE);
					// We are holding mutexes, so we need to force
					// the same thread to be used for next Run()
					return 0;
				}

				if (theErr != QTSS_NoErr)
				{
					ExecNetMsgErrorReqHandler(httpBadRequest);
				}

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
				ProcessRequest();

				if (fOutputStream.GetBytesWritten() > 0)
				{
					delete[] fRequestBody;
					fRequestBody = NULL;
					fState = kSendingResponse;
					break;
				}

				delete[] fRequestBody;
				fRequestBody = NULL;
				fState = kCleaningUp;
				break;
			}

		case kProcessingRequest:
			{
				if (fOutputStream.GetBytesWritten() == 0)
				{
					ExecNetMsgErrorReqHandler(httpInternalServerError);
					fState = kSendingResponse;
					break;
				}

				fState = kSendingResponse;
			}
		case kSendingResponse:
			{
				Assert(fRequest != NULL);

				err = fOutputStream.Flush();

				if (err == EAGAIN)
				{
					// If we get this error, we are currently flow-controlled and should
					// wait for the socket to become writeable again
					fSocket.RequestEvent(EV_WR);
					this->ForceSameThread();
					// We are holding mutexes, so we need to force
					// the same thread to be used for next Run()
					return 0;
				}
				if (err != QTSS_NoErr)
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

				this->CleanupRequest();

				{
					OSMutexLocker locker(&fQueueMutex);
					if (fEventQueue.GetLength() > 0)
					{
						return 0;
					}
				}

				fState = kReadingRequest;
			}
		}
	}

	this->CleanupRequest();

	if (fObjectHolders == 0)
		return -1;

	return 0;
}

QTSS_Error HTTPSession::SendHTTPPacket(StrPtrLen* contentXML, Bool16 connectionClose, Bool16 decrement)
{
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

	if (httpAck.CreateResponseHeader(httpOK))
	{
		if (contentXML->Len)
			httpAck.AppendContentLengthHeader(contentXML->Len);

		if (connectionClose)
			httpAck.AppendConnectionCloseHeader();

		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		string sendString(ackPtr->Ptr, ackPtr->Len);
		sendString.append(contentXML->Ptr, contentXML->Len);

		UInt32 theLengthSent = 0;
		UInt32 amtInBuffer = sendString.size();
		do
		{
			QTSS_Error theErr = fOutputSocketP->Send(sendString.c_str(), amtInBuffer, &theLengthSent);

			if (theErr != QTSS_NoErr && theErr != EAGAIN)
			{
				return theErr;
			}

			if (theLengthSent == amtInBuffer)
			{
				// We were able to send all the data in the buffer. Great. Flush it.
				return QTSS_NoErr;
			}
			// Not all the data was sent, so report an EAGAIN
			sendString.erase(0, theLengthSent);
			amtInBuffer = sendString.size();
			theLengthSent = 0;

		} while (amtInBuffer > 0);
	}

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::SetupRequest()
{
	QTSS_Error theErr = fRequest->Parse();
	if (theErr != QTSS_NoErr)
		return QTSS_BadArgument;

	if (fRequest->GetRequestPath() != NULL)
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
			if (path.size() == 2)
			{
				if (path[0] == "api" && path[1] == "getdevicelist")
				{
					return ExecNetMsgCSGetDeviceListReqRESTful(fRequest->GetQueryString());
				}
				if (path[0] == "api" && path[1] == "getdeviceinfo")
				{
					return ExecNetMsgCSGetCameraListReqRESTful(fRequest->GetQueryString());
				}
				if (path[0] == "api" && path[1] == "getdevicestream")
				{
					return ExecNetMsgCSGetStreamReqRESTful(fRequest->GetQueryString());
				}
                if (path[0] == "api" && path[1] == "ptzcontrol")
                {
                    return ExecNetMsgCSPTZControlReqRESTful(fRequest->GetQueryString());
                }
				if (path[0] == "api" && path[1] == "presetcontrol")
				{
					return ExecNetMsgCSPresetControlReqRESTful(fRequest->GetQueryString());
				}
			}

			EasyMsgExceptionACK rsp;
			string msg = rsp.GetMsg();
			StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
			this->SendHTTPPacket(&theValue, false, false);

			return QTSS_NoErr;
		}
	}

	//READ json Content

	//1、get json content length
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
	UInt32 theLen = sizeof(theRequestBody);
	theErr = QTSS_GetValue(this, EasyHTTPSesContentBody, 0, &theRequestBody, &theLen);

	if (theErr != QTSS_NoErr)
	{
		// First time we've been here for this request. Create a buffer for the content body and
		// shove it in the request.
		theRequestBody = NEW char[content_length + 1];
		memset(theRequestBody, 0, content_length + 1);
		theLen = sizeof(theRequestBody);
		theErr = QTSS_SetValue(this, EasyHTTPSesContentBody, 0, &theRequestBody, theLen);// SetValue creates an internal copy.
		Assert(theErr == QTSS_NoErr);

		// Also store the offset in the buffer
		theLen = sizeof(theBufferOffset);
		theErr = QTSS_SetValue(this, EasyHTTPSesContentBodyOffset, 0, &theBufferOffset, theLen);
		Assert(theErr == QTSS_NoErr);
	}

	theLen = sizeof(theBufferOffset);
	QTSS_GetValue(this, EasyHTTPSesContentBodyOffset, 0, &theBufferOffset, &theLen);

	// We have our buffer and offset. Read the data.
	//theErr = QTSS_Read(this, theRequestBody + theBufferOffset, content_length - theBufferOffset, &theLen);
	theErr = fInputStream.Read(theRequestBody + theBufferOffset, content_length - theBufferOffset, &theLen);
	Assert(theErr != QTSS_BadArgument);

	if ((theErr != QTSS_NoErr) && (theErr != EAGAIN))
	{
		OSCharArrayDeleter charArrayPathDeleter(theRequestBody);

		// NEED TO RETURN HTTP ERROR RESPONSE
		return QTSS_RequestFailed;
	}
	/*
	if (theErr == QTSS_RequestFailed)
	{
		OSCharArrayDeleter charArrayPathDeleter(theRequestBody);

		// NEED TO RETURN HTTP ERROR RESPONSE
		return QTSS_RequestFailed;
	}
	*/

	qtss_printf("HTTPSession read content-length:%d (%d/%d) \n", theLen, theBufferOffset + theLen, content_length);
	if ((theErr == QTSS_WouldBlock) || (theLen < (content_length - theBufferOffset)))
	{
		//
		// Update our offset in the buffer
		theBufferOffset += theLen;
		(void)QTSS_SetValue(this, EasyHTTPSesContentBodyOffset, 0, &theBufferOffset, sizeof(theBufferOffset));
		// The entire content body hasn't arrived yet. Request a read event and wait for it.

		Assert(theErr == QTSS_NoErr);
		return QTSS_WouldBlock;
	}

	// get complete HTTPHeader+JSONContent
	fRequestBody = theRequestBody;
	Assert(theErr == QTSS_NoErr);

	////TODO:://
	//if (theBufferOffset < sHeaderSize)
	//qtss_printf("Recv message: %s\n", fRequestBody);

	UInt32 offset = 0;
	(void)QTSS_SetValue(this, EasyHTTPSesContentBodyOffset, 0, &offset, sizeof(offset));
	char* content = NULL;
	(void)QTSS_SetValue(this, EasyHTTPSesContentBody, 0, &content, 0);

	return theErr;
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
		UInt32 maxConnections = static_cast<UInt32>(maxConns) + buffer;
		if (theServer->GetNumServiceSessions() > maxConnections)
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

QTSS_Error HTTPSession::ExecNetMsgDSPostSnapReq(const char* json)
{
	if (!fAuthenticated) return httpUnAuthorized;

	EasyMsgDSPostSnapREQ parse(json);

	string image = parse.GetBodyValue(EASY_TAG_IMAGE);
	string channel = parse.GetBodyValue(EASY_TAG_CHANNEL);
	string device_serial = parse.GetBodyValue(EASY_TAG_SERIAL);
	string strType = parse.GetBodyValue(EASY_TAG_TYPE);
	string strTime = parse.GetBodyValue(EASY_TAG_TIME);
	string reserve = parse.GetBodyValue(EASY_TAG_RESERVE);

	if (channel.empty())
		channel = "0";

	if (strTime.empty())
	{
		strTime = EasyUtil::NowTime(EASY_TIME_FORMAT_YYYYMMDDHHMMSSEx);
	}
	else//Time Filter 2015-07-20 12:55:30->20150720125530
	{
		EasyUtil::DelChar(strTime, '-');
		EasyUtil::DelChar(strTime, ':');
		EasyUtil::DelChar(strTime, ' ');
	}

	if ((image.size() <= 0) || (device_serial.size() <= 0) || (strType.size() <= 0) || (strTime.size() <= 0))
		return QTSS_BadArgument;

	image = EasyUtil::Base64Decode(image.data(), image.size());

	char jpgDir[512] = { 0 };
	qtss_sprintf(jpgDir, "%s%s", QTSServerInterface::GetServer()->GetPrefs()->GetSnapLocalPath(), device_serial.c_str());
	OS::RecursiveMakeDir(jpgDir);

	char jpgPath[512] = { 0 };

	//local path
	qtss_sprintf(jpgPath, "%s/%s_%s_%s.%s", jpgDir, device_serial.c_str(), channel.c_str(), strTime.c_str(), EasyProtocol::GetSnapTypeString(EASY_SNAP_TYPE_JPEG).c_str());

	FILE* fSnap = ::fopen(jpgPath, "wb");
	if (fSnap == NULL)
	{
		//DWORD e=GetLastError();
		return QTSS_NoErr;
	}

	if (fSessionType == EasyCameraSession)
	{
		fwrite(image.data(), 1, image.size(), fSnap);
	}
	else if (fSessionType == EasyNVRSession)
	{
		char decQueryString[EASY_MAX_URL_LENGTH] = { 0 };
		EasyUtil::Urldecode((unsigned char*)reserve.c_str(), reinterpret_cast<unsigned char*>(decQueryString));

		QueryParamList parList(decQueryString);
		int width = EasyUtil::String2Int(parList.DoFindCGIValueForParam("width"));
		int height = EasyUtil::String2Int(parList.DoFindCGIValueForParam("height"));
		int codec = EasyUtil::String2Int(parList.DoFindCGIValueForParam("codec"));

		rawData2Image((char*)image.data(), image.size(), codec, width, height);
		fwrite(decodeParam.imageData, 1, decodeParam.imageSize, fSnap);
	}

	::fclose(fSnap);

	//web path
	char snapURL[512] = { 0 };
	qtss_sprintf(snapURL, "%s%s/%s_%s_%s.%s", QTSServerInterface::GetServer()->GetPrefs()->GetSnapWebPath(), device_serial.c_str(),
		device_serial.c_str(), channel.c_str(), strTime.c_str(), EasyProtocol::GetSnapTypeString(EASY_SNAP_TYPE_JPEG).c_str());
	fDevice.HoldSnapPath(snapURL, channel);

	EasyProtocolACK rsp(MSG_SD_POST_SNAP_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = parse.GetHeaderValue(EASY_TAG_CSEQ);
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	body[EASY_TAG_SERIAL] = device_serial;
	body[EASY_TAG_CHANNEL] = channel;

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgErrorReqHandler(HTTPStatusCode errCode)
{
	//HTTP Header
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

	if (httpAck.CreateResponseHeader(errCode))
	{
		char respHeader[sHeaderSize] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		HTTPResponseStream* pOutputStream = GetOutputStream();
		pOutputStream->Put(respHeader);
	}

	this->fLiveSession = false;

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

	//update info each time
	if (!fDevice.GetDevInfo(json))
	{
		return  QTSS_BadArgument;
	}

	while (!fAuthenticated)
	{
		//1.获取TerminalType和AppType,进行逻辑验证，不符合则返回400 httpBadRequest;
		int appType = regREQ.GetAppType();
		//int terminalType = regREQ.GetTerminalType();
		switch (appType)
		{
		case EASY_APP_TYPE_CAMERA:
			{
				fSessionType = EasyCameraSession;
				//fTerminalType = terminalType;
				break;
			}
		case EASY_APP_TYPE_NVR:
			{
				fSessionType = EasyNVRSession;
				//fTerminalType = terminalType;
				break;
			}
		default:
			{
				break;
			}
		}

		if (fSessionType >= EasyHTTPSession)
		{
			//设备注册既不是EasyCamera，也不是EasyNVR，返回错误
			theErr = QTSS_BadArgument;
			break;
		}

		//2.验证Serial和Token进行权限验证，不符合则返回401 httpUnAuthorized;
		string serial = regREQ.GetBodyValue(EASY_TAG_SERIAL);
		string token = regREQ.GetBodyValue(EASY_TAG_TOKEN);

		if (serial.empty())
		{
			theErr = QTSS_AttrDoesntExist;
			break;
		}

		//验证Serial和Token是否合法
		/*if (false)
		{
			theErr = QTSS_NotPreemptiveSafe;
			break;
		}*/

		OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
		OS_Error regErr = deviceMap->Register(fDevice.serial_, this);
		if (regErr == OS_NoErr)
		{
			//在redis上增加设备
			char msgStr[512];
			qtss_snprintf(msgStr, sizeof(msgStr), "Device register，Device_serial[%s]\n", fDevice.serial_.c_str());
			QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);

			QTSS_RoleParams theParams;
			theParams.StreamNameParams.inStreamName = const_cast<char *>(fDevice.serial_.c_str());
			UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisAddDevNameRole);
			for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
			{
				QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisAddDevNameRole, currentModule);
				(void)theModule->CallDispatch(Easy_RedisAddDevName_Role, &theParams);
			}
			fAuthenticated = true;
		}
		else
		{
			//设备冲突的时候将前一个设备给挤掉,因为断电、断网情况下连接是不会断开的，如果设备来电、网络通顺之后就会产生冲突，
			//一个连接的超时时90秒，要等到90秒之后设备才能正常注册上线。
			OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(fDevice.serial_);////////////////////////////////++
			if (theDevRef != NULL)//找到指定设备
			{
				OSRefReleaserEx releaser(deviceMap, fDevice.serial_);
				HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备会话
				pDevSession->Signal(Task::kKillEvent);//终止设备连接
				//QTSServerInterface::GetServer()->GetDeviceSessionMap()->Release(fDevice.serial_);////////////////////////////////--
			}
			//这一次仍然返回上线冲突，因为虽然给设备发送了Task::kKillEvent消息，但设备可能不会立即终止，否则就要循环等待是否已经终止！
			theErr = QTSS_AttrNameExists;;
		}
		break;
	}

	if (theErr != QTSS_NoErr)	return theErr;

	//走到这说明该设备成功注册或者心跳
	EasyProtocol req(json);
	EasyProtocolACK rsp(MSG_SD_REGISTER_ACK);
	EasyJsonValue header, body;
	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = req.GetHeaderValue(EASY_TAG_CSEQ);
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	body[EASY_TAG_SERIAL] = fDevice.serial_;

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSFreeStreamReq(const char* json)//客户端的停止直播请求
{
	//算法描述：查找指定设备，若设备存在，则向设备发出停止流请求
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/
	EasyProtocol req(json);
	//从serial/channel中解析出serial和channel
	string strStreamName = req.GetBodyValue(EASY_TAG_SERIAL);//流名称
	if (strStreamName.size() <= 0)
		return QTSS_BadArgument;

	int iPos = strStreamName.find('/');
	if (iPos == string::npos)
		return QTSS_BadArgument;

	string strDeviceSerial = strStreamName.substr(0, iPos);
	string strChannel = strStreamName.substr(iPos + 1, strStreamName.size() - iPos - 1);

	//string strDeviceSerial	=	req.GetBodyValue(EASY_TAG_SERIAL);//设备序列号
	//string strChannel	=	req.GetBodyValue(EASY_TAG_CHANNEL);//摄像头序列号
	string strReserve = req.GetBodyValue(EASY_TAG_RESERVE);//StreamID
	string strProtocol = req.GetBodyValue(EASY_TAG_PROTOCOL);//Protocol

	//为可选参数填充默认值
	if (strChannel.empty())
		strChannel = "0";
	if (strReserve.empty())
		strReserve = "1";

	if ((strDeviceSerial.size() <= 0) || (strProtocol.size() <= 0))//参数判断
		return QTSS_BadArgument;

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(strDeviceSerial);////////////////////////////////++
	if (theDevRef == NULL)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, strDeviceSerial);
	//走到这说明存在指定设备，则该设备发出停止推流请求
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	EasyProtocolACK reqreq(MSG_SD_STREAM_STOP_REQ);
	EasyJsonValue headerheader, bodybody;

	char chTemp[16] = { 0 };
	UInt32 uDevCseq = pDevSession->GetCSeq();
	sprintf(chTemp, "%d", uDevCseq);
	headerheader[EASY_TAG_CSEQ] = string(chTemp);//注意这个地方不能直接将UINT32->int,因为会造成数据失真
	headerheader[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;

	bodybody[EASY_TAG_SERIAL] = strDeviceSerial;
	bodybody[EASY_TAG_CHANNEL] = strChannel;
	bodybody[EASY_TAG_RESERVE] = strReserve;
	bodybody[EASY_TAG_PROTOCOL] = strProtocol;
	bodybody[EASY_TAG_FROM] = fSessionID;
	bodybody[EASY_TAG_TO] = pDevSession->GetValue(EasyHTTPSessionID)->GetAsCString();
	bodybody[EASY_TAG_VIA] = QTSServerInterface::GetServer()->GetCloudServiceNodeID();

	reqreq.SetHead(headerheader);
	reqreq.SetBody(bodybody);

	string buffer = reqreq.GetMsg();

	StrPtrLen theValue(const_cast<char*>(buffer.c_str()), buffer.size());
	pDevSession->SendHTTPPacket(&theValue, false, false);

	//直接对客户端（EasyDarWin)进行正确回应
	EasyProtocolACK rsp(MSG_SC_FREE_STREAM_ACK);
	EasyJsonValue header, body;
	header[EASY_TAG_CSEQ] = req.GetHeaderValue(EASY_TAG_CSEQ);
	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	body[EASY_TAG_SERIAL] = strDeviceSerial;
	body[EASY_TAG_CHANNEL] = strChannel;
	body[EASY_TAG_RESERVE] = strReserve;
	body[EASY_TAG_PROTOCOL] = strProtocol;

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	StrPtrLen theValueAck(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValueAck, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgDSStreamStopAck(const char* json)//设备的停止推流回应
{
	if (!fAuthenticated)//没有进行认证请求
		return httpUnAuthorized;

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSGetStreamReqRESTful(const char* queryString)//放到ProcessRequest所在的状态去处理，方便多次循环调用
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	if (queryString == NULL)
	{
		return QTSS_BadArgument;
	}

	char decQueryString[EASY_MAX_URL_LENGTH] = { 0 };
	EasyUtil::Urldecode((unsigned char*)queryString, reinterpret_cast<unsigned char*>(decQueryString));

	QueryParamList parList(decQueryString);
	const char* chSerial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号
	const char* chChannel = parList.DoFindCGIValueForParam(EASY_TAG_L_CHANNEL);//获取通道
	const char* chProtocol = parList.DoFindCGIValueForParam(EASY_TAG_L_PROTOCOL);//获取通道
	const char* chReserve = parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE);//获取通道

	//为可选参数填充默认值
	if (chChannel == NULL)
		chChannel = "0";
	if (chReserve == NULL)
		chReserve = "1";

	if (chSerial == NULL || chProtocol == NULL)
		return QTSS_BadArgument;

	string strCSeq = EasyUtil::Int2String(GetCSeq());
	string strURL;//直播地址

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(chSerial);
	if (theDevRef == NULL)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, chSerial);
	//走到这说明存在指定设备
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	string strDssIP, strDssPort;
	char chDssIP[sIPSize] = { 0 };
	char chDssPort[sPortSize] = { 0 };

	QTSS_RoleParams theParams;
	theParams.GetAssociatedDarwinParams.inSerial = const_cast<char*>(chSerial);
	theParams.GetAssociatedDarwinParams.inChannel = const_cast<char*>(chChannel);
	theParams.GetAssociatedDarwinParams.outDssIP = chDssIP;
	theParams.GetAssociatedDarwinParams.outDssPort = chDssPort;

	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisGetEasyDarwinRole);
	for (UInt32 currentModule = 0; currentModule < numModules; ++currentModule)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisGetEasyDarwinRole, currentModule);
		(void)theModule->CallDispatch(Easy_RedisGetEasyDarwin_Role, &theParams);
	}
	if (chDssIP[0] != 0)//是否存在关联的EasyDarWin转发服务器test,应该用Redis上的数据，因为推流是不可靠的，而EasyDarWin上的数据是可靠的
	{
		strDssIP = chDssIP;
		strDssPort = chDssPort;
		//合成直播的RTSP地址，后续有可能根据请求流的协议不同而生成不同的直播地址，如RTMP、HLS等
		string strSessionID;
		char chSessionID[128] = { 0 };

		QTSS_RoleParams theParamsGetStream;
		theParamsGetStream.GenStreamIDParams.outStreanID = chSessionID;
		theParamsGetStream.GenStreamIDParams.inTimeoutMil = SessionIDTimeout;

		numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisGenStreamIDRole);
		for (UInt32 currentModule = 0; currentModule < numModules; ++currentModule)
		{
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisGenStreamIDRole, currentModule);
			(void)theModule->CallDispatch(Easy_RedisGenStreamID_Role, &theParamsGetStream);
		}

		if (chSessionID[0] == 0)//sessionID在redis上的存储失败
		{
			return EASY_ERROR_SERVER_INTERNAL_ERROR;
		}
		strSessionID = chSessionID;
		strURL = string("rtsp://").append(strDssIP).append(":").append(strDssPort).append("/")
			.append(chSerial).append("/")
			.append(chChannel).append(".sdp")
			.append("?token=").append(strSessionID);
	}
	else
	{
		//不存在关联的EasyDarWin
		QTSS_RoleParams theParamsRedis;
		theParamsRedis.GetBestDarwinParams.outDssIP = chDssIP;
		theParamsRedis.GetBestDarwinParams.outDssPort = chDssPort;

		numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisGetBestEasyDarwinRole);
		for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
		{
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisGetBestEasyDarwinRole, currentModule);
			(void)theModule->CallDispatch(Easy_RedisGetBestEasyDarwin_Role, &theParamsRedis);
		}

		if (chDssIP[0] == 0)//不存在DarWin
		{
			return EASY_ERROR_SERVICE_NOT_FOUND;
		}
		//向指定设备发送开始流请求

		strDssIP = chDssIP;
		strDssPort = chDssPort;
		EasyProtocolACK reqreq(MSG_SD_PUSH_STREAM_REQ);
		EasyJsonValue headerheader, bodybody;

		char chTemp[16] = { 0 };
		UInt32 uDevCseq = pDevSession->GetCSeq();
		sprintf(chTemp, "%d", uDevCseq);
		headerheader[EASY_TAG_CSEQ] = string(chTemp);//注意这个地方不能直接将UINT32->int,因为会造成数据失真
		headerheader[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;

		string strSessionID;
		char chSessionID[128] = { 0 };

		//QTSS_RoleParams theParams;
		theParams.GenStreamIDParams.outStreanID = chSessionID;
		theParams.GenStreamIDParams.inTimeoutMil = SessionIDTimeout;

		numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisGenStreamIDRole);
		for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
		{
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisGenStreamIDRole, currentModule);
			(void)theModule->CallDispatch(Easy_RedisGenStreamID_Role, &theParams);
		}
		if (chSessionID[0] == 0)//sessionID再redis上的存储失败
		{
			return EASY_ERROR_SERVER_INTERNAL_ERROR;
		}

		strSessionID = chSessionID;
		bodybody[EASY_TAG_STREAM_ID] = strSessionID;
		bodybody[EASY_TAG_SERVER_IP] = strDssIP;
		bodybody[EASY_TAG_SERVER_PORT] = strDssPort;
		bodybody[EASY_TAG_SERIAL] = chSerial;
		bodybody[EASY_TAG_CHANNEL] = chChannel;
		bodybody[EASY_TAG_PROTOCOL] = chProtocol;
		bodybody[EASY_TAG_RESERVE] = chReserve;
		bodybody[EASY_TAG_FROM] = fSessionID;
		bodybody[EASY_TAG_TO] = pDevSession->GetValue(EasyHTTPSessionID)->GetAsCString();
		bodybody[EASY_TAG_VIA] = QTSServerInterface::GetServer()->GetCloudServiceNodeID();

		reqreq.SetHead(headerheader);
		reqreq.SetBody(bodybody);

		string buffer = reqreq.GetMsg();
		StrPtrLen theValue(const_cast<char*>(buffer.c_str()), buffer.size());
		pDevSession->SendHTTPPacket(&theValue, false, false);
		fTimeoutTask.SetTimeout(3 * 1000);
		fTimeoutTask.RefreshTimeout();

		return QTSS_NoErr;
	}

	//走到这说明对客户端的正确回应,因为错误回应直接返回。
	EasyProtocolACK rsp(MSG_SC_GET_STREAM_ACK);
	EasyJsonValue header, body;
	body[EASY_TAG_URL] = strURL;
	body[EASY_TAG_SERIAL] = chSerial;
	body[EASY_TAG_CHANNEL] = chChannel;
	body[EASY_TAG_PROTOCOL] = chProtocol;//如果当前已经推流，则返回请求的，否则返回实际推流类型
	body[EASY_TAG_RESERVE] = chReserve;//如果当前已经推流，则返回请求的，否则返回实际推流类型

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = strCSeq;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgDSPushStreamAck(const char* json)//设备的开始流回应
{
	if (!fAuthenticated)//没有进行认证请求
		return httpUnAuthorized;

	//对于设备的推流回应是不需要在进行回应的，直接解析找到对应的客户端Session，赋值即可	
	EasyProtocol req(json);

	string strDeviceSerial = req.GetBodyValue(EASY_TAG_SERIAL);//设备序列号
	string strChannel = req.GetBodyValue(EASY_TAG_CHANNEL);//摄像头序列号
	string strProtocol = req.GetBodyValue(EASY_TAG_PROTOCOL);//协议,终端仅支持RTSP推送
	string strReserve = req.GetBodyValue(EASY_TAG_RESERVE);//流类型
	string strDssIP = req.GetBodyValue(EASY_TAG_SERVER_IP);//设备实际推流地址
	string strDssPort = req.GetBodyValue(EASY_TAG_SERVER_PORT);//和端口
	string strFrom = req.GetBodyValue(EASY_TAG_FROM);
	string strTo = req.GetBodyValue(EASY_TAG_TO);
	string strVia = req.GetBodyValue(EASY_TAG_VIA);

	string strCSeq = req.GetHeaderValue(EASY_TAG_CSEQ);//这个是关键字
	string strStateCode = req.GetHeaderValue(EASY_TAG_ERROR_NUM);//状态码

	if (strChannel.empty())
		strChannel = "0";
	if (strReserve.empty())
		strReserve = "1";

	OSRefTableEx* sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
	OSRefTableEx::OSRefEx* sessionRef = sessionMap->Resolve(strTo);
	if (sessionRef == NULL)
		return EASY_ERROR_SESSION_NOT_FOUND;

	OSRefReleaserEx releaser(sessionMap, strTo);
	HTTPSession* httpSession = static_cast<HTTPSession *>(sessionRef->GetObjectPtr());

	if (httpSession->IsLiveSession())
	{
		//合成直播地址

		string strSessionID;
		char chSessionID[128] = { 0 };

		QTSS_RoleParams theParams;
		theParams.GenStreamIDParams.outStreanID = chSessionID;
		theParams.GenStreamIDParams.inTimeoutMil = SessionIDTimeout;

		UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisGenStreamIDRole);
		for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
		{
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisGenStreamIDRole, currentModule);
			(void)theModule->CallDispatch(Easy_RedisGenStreamID_Role, &theParams);
		}
		if (chSessionID[0] == 0)//sessionID在redis上的存储失败
		{
			return EASY_ERROR_SERVER_INTERNAL_ERROR;
		}
		strSessionID = chSessionID;
		string strURL = string("rtsp://")
			.append(strDssIP).append(":").append(strDssPort).append("/")
			.append(strDeviceSerial).append("/")
			.append(strChannel).append(".sdp")
			.append("?token=").append(strSessionID);

		//走到这说明对客户端的正确回应,因为错误回应直接返回。
		EasyProtocolACK rsp(MSG_SC_GET_STREAM_ACK);
		EasyJsonValue header, body;
		body[EASY_TAG_URL] = strURL;
		body[EASY_TAG_SERIAL] = strDeviceSerial;
		body[EASY_TAG_CHANNEL] = strChannel;
		body[EASY_TAG_PROTOCOL] = strProtocol;//如果当前已经推流，则返回请求的，否则返回实际推流类型
		body[EASY_TAG_RESERVE] = strReserve;//如果当前已经推流，则返回请求的，否则返回实际推流类型

		header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
		header[EASY_TAG_CSEQ] = strCSeq;
		header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
		header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

		rsp.SetHead(header);
		rsp.SetBody(body);
		string msg = rsp.GetMsg();

		StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
		httpSession->SendHTTPPacket(&theValue, true, false);
	}

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSGetDeviceListReqRESTful(const char* queryString)//客户端获得设备列表
{
	/*
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/
	char decQueryString[EASY_MAX_URL_LENGTH] = { 0 };
	if (queryString != NULL)
	{
		EasyUtil::Urldecode((unsigned char*)queryString, reinterpret_cast<unsigned char*>(decQueryString));
	}
	QueryParamList parList(decQueryString);
	const char* chAppType = parList.DoFindCGIValueForParam(EASY_TAG_APP_TYPE);//APPType
	const char* chTerminalType = parList.DoFindCGIValueForParam(EASY_TAG_TERMINAL_TYPE);//TerminalType

	EasyProtocolACK rsp(MSG_SC_DEVICE_LIST_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);


	OSMutex* mutexMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMutex();
	OSHashMap* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMap();
	OSRefIt itRef;
	Json::Value* proot = rsp.GetRoot();

	{
		OSMutexLocker lock(mutexMap);
		int iDevNum = 0;

		for (itRef = deviceMap->begin(); itRef != deviceMap->end(); ++itRef)
		{
			strDevice* deviceInfo = static_cast<HTTPSession*>(itRef->second->GetObjectPtr())->GetDeviceInfo();
			if (chAppType != NULL)// AppType fileter
			{
				if (EasyProtocol::GetAppTypeString(deviceInfo->eAppType) != string(chAppType))
					continue;
			}
			if (chTerminalType != NULL)// TerminateType fileter
			{
				if (EasyProtocol::GetTerminalTypeString(deviceInfo->eDeviceType) != string(chTerminalType))
					continue;
			}

			iDevNum++;

			Json::Value value;
			value[EASY_TAG_SERIAL] = deviceInfo->serial_;//这个地方引起了崩溃,deviceMap里有数据，但是deviceInfo里面数据都是空
			value[EASY_TAG_NAME] = deviceInfo->name_;
			value[EASY_TAG_TAG] = deviceInfo->tag_;
			value[EASY_TAG_APP_TYPE] = EasyProtocol::GetAppTypeString(deviceInfo->eAppType);
			value[EASY_TAG_TERMINAL_TYPE] = EasyProtocol::GetTerminalTypeString(deviceInfo->eDeviceType);
			//如果设备是EasyCamera,则返回设备快照信息
			if (deviceInfo->eAppType == EASY_APP_TYPE_CAMERA)
			{
				value[EASY_TAG_SNAP_URL] = deviceInfo->snapJpgPath_;
			}
			(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES].append(value);
		}
		body[EASY_TAG_DEVICE_COUNT] = iDevNum;
	}

	rsp.SetHead(header);
	rsp.SetBody(body);

	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);
	
	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSDeviceListReq(const char* json)//客户端获得设备列表
{
	/*
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/
	EasyProtocol req(json);

	EasyProtocolACK rsp(MSG_SC_DEVICE_LIST_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = req.GetHeaderValue(EASY_TAG_CSEQ);
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSMutex* mutexMap = deviceMap->GetMutex();
	OSHashMap* deviceHashMap = deviceMap->GetMap();
	OSRefIt itRef;
	Json::Value* proot = rsp.GetRoot();

	{
		OSMutexLocker lock(mutexMap);
		body[EASY_TAG_DEVICE_COUNT] = deviceMap->GetEleNumInMap();
		for (itRef = deviceHashMap->begin(); itRef != deviceHashMap->end(); ++itRef)
		{
			Json::Value value;
			strDevice* deviceInfo = static_cast<HTTPSession*>(itRef->second->GetObjectPtr())->GetDeviceInfo();
			value[EASY_TAG_SERIAL] = deviceInfo->serial_;
			value[EASY_TAG_NAME] = deviceInfo->name_;
			value[EASY_TAG_TAG] = deviceInfo->tag_;
			value[EASY_TAG_APP_TYPE] = EasyProtocol::GetAppTypeString(deviceInfo->eAppType);
			value[EASY_TAG_TERMINAL_TYPE] = EasyProtocol::GetTerminalTypeString(deviceInfo->eDeviceType);
			//如果设备是EasyCamera,则返回设备快照信息
			if (deviceInfo->eAppType == EASY_APP_TYPE_CAMERA)
			{
				value[EASY_TAG_SNAP_URL] = deviceInfo->snapJpgPath_;
			}
			(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES].append(value);
		}
	}

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSGetCameraListReqRESTful(const char* queryString)
{
	/*
		if(!fAuthenticated)//没有进行认证请求
		return httpUnAuthorized;
	*/
	if (queryString == NULL)
	{
		return QTSS_BadArgument;
	}

	char decQueryString[EASY_MAX_URL_LENGTH] = { 0 };
	EasyUtil::Urldecode((unsigned char*)queryString, reinterpret_cast<unsigned char*>(decQueryString));

	QueryParamList parList(decQueryString);
	const char* device_serial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号

	if (device_serial == NULL)
		return QTSS_BadArgument;

	EasyProtocolACK rsp(MSG_SC_DEVICE_INFO_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;

	body[EASY_TAG_SERIAL] = device_serial;

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(device_serial);////////////////////////////////++
	if (theDevRef == NULL)//不存在指定设备
	{
		header[EASY_TAG_ERROR_NUM] = EASY_ERROR_DEVICE_NOT_FOUND;
		header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_DEVICE_NOT_FOUND);
	}
	else//存在指定设备，则获取这个设备的摄像头信息
	{
		OSRefReleaserEx releaser(deviceMap, device_serial);

		header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
		header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

		Json::Value* proot = rsp.GetRoot();
		strDevice* deviceInfo = static_cast<HTTPSession*>(theDevRef->GetObjectPtr())->GetDeviceInfo();
		if (deviceInfo->eAppType == EASY_APP_TYPE_CAMERA)
		{
			body[EASY_TAG_SNAP_URL] = deviceInfo->snapJpgPath_;
		}
		else
		{
			EasyDevicesIterator itCam;
			body[EASY_TAG_CHANNEL_COUNT] = static_cast<HTTPSession*>(theDevRef->GetObjectPtr())->GetDeviceInfo()->channelCount_;
			for (itCam = deviceInfo->channels_.begin(); itCam != deviceInfo->channels_.end(); ++itCam)
			{
				Json::Value value;
				value[EASY_TAG_CHANNEL] = itCam->second.channel_;
				value[EASY_TAG_NAME] = itCam->second.name_;
				value[EASY_TAG_STATUS] = itCam->second.status_;
				value[EASY_TAG_SNAP_URL] = itCam->second.snapJpgPath_;
				(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].append(value);
			}
		}

	}
	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSCameraListReq(const char* json)
{
	/*
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/
	EasyProtocol req(json);
	string strDeviceSerial = req.GetBodyValue(EASY_TAG_SERIAL);

	if (strDeviceSerial.size() <= 0)
		return QTSS_BadArgument;

	EasyProtocolACK rsp(MSG_SC_DEVICE_INFO_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = req.GetHeaderValue(EASY_TAG_CSEQ);
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);
	body[EASY_TAG_SERIAL] = strDeviceSerial;

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(strDeviceSerial);////////////////////////////////++
	if (theDevRef == NULL)//不存在指定设备
	{
		return EASY_ERROR_DEVICE_NOT_FOUND;//交给错误处理中心处理
	}
	//存在指定设备，则获取这个设备的摄像头信息
	OSRefReleaserEx releaser(deviceMap, strDeviceSerial);

	Json::Value* proot = rsp.GetRoot();
	strDevice* deviceInfo = static_cast<HTTPSession*>(theDevRef->GetObjectPtr())->GetDeviceInfo();
	if (deviceInfo->eAppType == EASY_APP_TYPE_CAMERA)
	{
		body[EASY_TAG_SNAP_URL] = deviceInfo->snapJpgPath_;
	}
	else
	{
		EasyDevices *camerasInfo = &(deviceInfo->channels_);
		EasyDevicesIterator itCam;

		body[EASY_TAG_CHANNEL_COUNT] = static_cast<HTTPSession*>(theDevRef->GetObjectPtr())->GetDeviceInfo()->channelCount_;
		for (itCam = camerasInfo->begin(); itCam != camerasInfo->end(); ++itCam)
		{
			Json::Value value;
			value[EASY_TAG_CHANNEL] = itCam->second.channel_;
			value[EASY_TAG_NAME] = itCam->second.name_;
			value[EASY_TAG_STATUS] = itCam->second.status_;
			body[EASY_TAG_SNAP_URL] = itCam->second.snapJpgPath_;
			(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].append(value);
		}
	}
	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ProcessRequest()//处理请求
{
	//OSCharArrayDeleter charArrayPathDeleter(theRequestBody);//不要在这删除，因为可能执行多次，仅当对请求的处理完毕后再进行删除

	if (fRequestBody == NULL)//表示没有正确的解析请求，SetUpRequest环节没有解析出数据部分
		return QTSS_NoErr;

	//消息处理
	QTSS_Error theErr;
	EasyProtocol protocol(fRequestBody);
	int nNetMsg = protocol.GetMessageType(), nRspMsg = MSG_SC_EXCEPTION;

	switch (nNetMsg)
	{
	case MSG_DS_REGISTER_REQ://处理设备上线消息,设备类型包括NVR、摄像头和智能主机
		theErr = ExecNetMsgDSRegisterReq(fRequestBody);
		nRspMsg = MSG_SD_REGISTER_ACK;
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
		nRspMsg = MSG_SC_DEVICE_LIST_ACK;
		break;
	case MSG_CS_DEVICE_INFO_REQ://摄像头列表请求,设备的具体信息
		theErr = ExecNetMsgCSCameraListReq(fRequestBody);//add
		nRspMsg = MSG_SC_DEVICE_INFO_ACK;
		break;
	case MSG_DS_POST_SNAP_REQ://设备快照上传
		theErr = ExecNetMsgDSPostSnapReq(fRequestBody);
		nRspMsg = MSG_SD_POST_SNAP_ACK;
		break;
    case MSG_DS_CONTROL_PTZ_ACK:
        theErr = ExecNetMsgDSPTZControlAck(fRequestBody);
        nRspMsg = MSG_DS_CONTROL_PTZ_ACK;
        break;
	case MSG_DS_CONTROL_PRESET_ACK:
		theErr = ExecNetMsgDSPresetControlAck(fRequestBody);
		nRspMsg = MSG_DS_CONTROL_PRESET_ACK;
		break;
	default:
		theErr = ExecNetMsgErrorReqHandler(httpNotImplemented);
		break;
	}

	//如果不想进入错误自动处理则一定要返回QTSS_NoErr
	if (theErr != QTSS_NoErr)//无论是正确回应还是等待返回都是QTSS_NoErr，出现错误，对错误进行统一回应
	{
		EasyProtocol req(fRequestBody);
		EasyProtocolACK rsp(nRspMsg);
		EasyJsonValue header;
		header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
		header[EASY_TAG_CSEQ] = req.GetHeaderValue(EASY_TAG_CSEQ);

		switch (theErr)
		{
		case QTSS_BadArgument:
			header[EASY_TAG_ERROR_NUM] = EASY_ERROR_CLIENT_BAD_REQUEST;
			header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_CLIENT_BAD_REQUEST);
			break;
		case httpUnAuthorized:
			header[EASY_TAG_ERROR_NUM] = EASY_ERROR_CLIENT_UNAUTHORIZED;
			header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_CLIENT_UNAUTHORIZED);
			break;
		case QTSS_AttrNameExists:
			header[EASY_TAG_ERROR_NUM] = EASY_ERROR_CONFLICT;
			header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_CONFLICT);
			break;
		case EASY_ERROR_DEVICE_NOT_FOUND:
			header[EASY_TAG_ERROR_NUM] = EASY_ERROR_DEVICE_NOT_FOUND;
			header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_DEVICE_NOT_FOUND);
			break;
		case EASY_ERROR_SERVICE_NOT_FOUND:
			header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SERVICE_NOT_FOUND;
			header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SERVICE_NOT_FOUND);
			break;
		case httpRequestTimeout:
			header[EASY_TAG_ERROR_NUM] = EASY_ERROR_REQUEST_TIMEOUT;
			header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_REQUEST_TIMEOUT);
			break;
		case EASY_ERROR_SERVER_INTERNAL_ERROR:
			header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SERVER_INTERNAL_ERROR;
			header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SERVER_INTERNAL_ERROR);
			break;
		case EASY_ERROR_SERVER_NOT_IMPLEMENTED:
			header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SERVER_NOT_IMPLEMENTED;
			header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SERVER_NOT_IMPLEMENTED);
			break;
		default:
			header[EASY_TAG_ERROR_NUM] = EASY_ERROR_CLIENT_BAD_REQUEST;
			header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_CLIENT_BAD_REQUEST);
			break;
		}

		rsp.SetHead(header);
		string msg = rsp.GetMsg();
		StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
		this->SendHTTPPacket(&theValue, false, false);
	}
	return theErr;
}

int	HTTPSession::yuv2BMPImage(unsigned int width, unsigned int height, char* yuvpbuf, unsigned int* rgbsize, unsigned char* rgbdata)
{
#ifndef __linux__
    int nBpp = 24;
	int dwW, dwH, dwWB;

	dwW = width;
	dwH = height;
	dwWB = _WIDTHBYTES(dwW * nBpp);

	// SaveFile to BMP
	BITMAPFILEHEADER bfh = { 0, };
	bfh.bfType = 0x4D42;
	bfh.bfSize = 0;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if (nBpp == 16)
	{
		bfh.bfOffBits += sizeof(RGBQUAD) * 3;
	}
	else
	{
		bfh.bfOffBits += sizeof(RGBQUAD) * 1;
	}

	DWORD dwWriteLength = sizeof(BITMAPFILEHEADER);
	int rgbOffset = 0;
	memcpy(rgbdata + rgbOffset, (PVOID)&bfh, dwWriteLength);
	rgbOffset += dwWriteLength;

	BITMAPINFOHEADER	bih = { 0, };
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = dwW;
	bih.biHeight = -(INT)dwH;
	bih.biPlanes = 1;
	bih.biBitCount = nBpp;
	bih.biCompression = (nBpp == 16) ? BI_BITFIELDS : BI_RGB;
	bih.biSizeImage = dwWB * height;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	dwWriteLength = sizeof(BITMAPINFOHEADER);

	memcpy(rgbdata + rgbOffset, (PVOID)&bih, dwWriteLength);
	rgbOffset += dwWriteLength;

	if (nBpp == 24)
	{
		DWORD rgbQuad = 0;
		dwWriteLength = sizeof(rgbQuad);
		memcpy(rgbdata + rgbOffset, (PVOID)&rgbQuad, dwWriteLength);
		rgbOffset += dwWriteLength;
	}

	dwWriteLength = dwWB * height;
	memcpy(rgbdata + rgbOffset, (PVOID)yuvpbuf, dwWriteLength);
	rgbOffset += dwWriteLength;

	if (NULL != rgbsize)	*rgbsize = rgbOffset;
#endif
	return 0;
}

QTSS_Error HTTPSession::rawData2Image(char* rawBuf, int bufSize, int codec, int width, int height)
{
	QTSS_Error	theErr = QTSS_NoErr;
#ifndef __linux__
	decodeParam.codec = codec;
	decodeParam.width = width;
	decodeParam.height = height;
	decoderHelper.SetVideoDecoderParam(width, height, codec, 3);

	int yuvdata_size = width * height * 3;
	char* yuvdata = new char[yuvdata_size + 1];
	memset(yuvdata, 0x00, yuvdata_size + 1);

	int snapHeight = height >= SNAP_IMAGE_HEIGHT ? SNAP_IMAGE_HEIGHT : height;
	int snapWidth = height >= SNAP_IMAGE_HEIGHT ? SNAP_IMAGE_WIDTH : height * 16 / 9;

	if (decoderHelper.DecodeVideo(rawBuf, bufSize, yuvdata, snapWidth, snapHeight) != 0)
	{
		theErr = QTSS_RequestFailed;
	}
	else
	{
		memset(decodeParam.imageData, 0, SNAP_SIZE);
		yuv2BMPImage(snapWidth, snapHeight, (char*)yuvdata, &decodeParam.imageSize, (unsigned char*)decodeParam.imageData);
	}

	if (NULL != yuvdata)
	{
		delete[] yuvdata;
		yuvdata = NULL;
	}

#endif
	return theErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSPTZControlReqRESTful(const char* queryString)
{
    /*//暂时注释掉，实际上是需要认证的
    if(!fAuthenticated)//没有进行认证请求
    return httpUnAuthorized;
    */

    if (queryString == NULL)
    {
        return QTSS_BadArgument;
    }

    char decQueryString[EASY_MAX_URL_LENGTH] = { 0 };
    EasyUtil::Urldecode((unsigned char*)queryString, reinterpret_cast<unsigned char*>(decQueryString));

    QueryParamList parList(decQueryString);
    const char* chSerial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号
    const char* chChannel = parList.DoFindCGIValueForParam(EASY_TAG_L_CHANNEL);//获取通道
    const char* chProtocol = parList.DoFindCGIValueForParam(EASY_TAG_L_PROTOCOL);//获取通道
    const char* chReserve(parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE));//获取通道
    const char* chActionType = parList.DoFindCGIValueForParam(EASY_TAG_L_ACTION_TYPE);
    const char* chCmd = parList.DoFindCGIValueForParam(EASY_TAG_L_CMD);
    const char* chSpeed = parList.DoFindCGIValueForParam(EASY_TAG_L_SPEED);

    if (chSerial == NULL || chProtocol == NULL || chActionType == NULL || chCmd == NULL)
        return QTSS_BadArgument;

    //为可选参数填充默认值
    if (chChannel == NULL)
        chChannel = "0";
    if (chReserve == NULL)
        chReserve = "1";

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(chSerial);
	if (theDevRef == NULL)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, chSerial);
	//走到这说明存在指定设备
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	EasyProtocolACK reqreq(MSG_SD_CONTROL_PTZ_REQ);
	EasyJsonValue headerheader, bodybody;

	char chTemp[16] = { 0 };
	UInt32 uDevCseq = pDevSession->GetCSeq();
	sprintf(chTemp, "%d", uDevCseq);
	headerheader[EASY_TAG_CSEQ] = string(chTemp);//注意这个地方不能直接将UINT32->int,因为会造成数据失真
	headerheader[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;

	string strProtocol(chProtocol);
	string strActionType(chActionType);
	string strCmd(chCmd);
	boost::to_upper(strProtocol);
	boost::to_upper(strActionType);
	boost::to_upper(strCmd);

	bodybody[EASY_TAG_SERIAL] = chSerial;
	bodybody[EASY_TAG_CHANNEL] = chChannel;
	bodybody[EASY_TAG_PROTOCOL] = strProtocol;
	bodybody[EASY_TAG_RESERVE] = chReserve;
	bodybody[EASY_TAG_ACTION_TYPE] = strActionType;
	bodybody[EASY_TAG_CMD] = strCmd;
	bodybody[EASY_TAG_SPEED] = chSpeed;
	bodybody[EASY_TAG_FROM] = fSessionID;
	bodybody[EASY_TAG_TO] = pDevSession->GetValue(EasyHTTPSessionID)->GetAsCString();
	bodybody[EASY_TAG_VIA] = QTSServerInterface::GetServer()->GetCloudServiceNodeID();

	reqreq.SetHead(headerheader);
	reqreq.SetBody(bodybody);

	string buffer = reqreq.GetMsg();
	StrPtrLen theValue(const_cast<char*>(buffer.c_str()), buffer.size());
	pDevSession->SendHTTPPacket(&theValue, false, false);

	EasyProtocolACK rsp(MSG_SC_PTZ_CONTROL_ACK);
	EasyJsonValue header, body;
	body[EASY_TAG_SERIAL] = chSerial;
	body[EASY_TAG_CHANNEL] = chChannel;
	body[EASY_TAG_PROTOCOL] = strProtocol;
	body[EASY_TAG_RESERVE] = chReserve;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = string(chTemp);;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();
	StrPtrLen theValueAck(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValueAck, false, false);
	
    return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgDSPTZControlAck(const char* json)
{
  //  if (!fAuthenticated)//没有进行认证请求
  //      return httpUnAuthorized;

  //  //对于设备的推流回应是不需要在进行回应的，直接解析找到对应的客户端Session，赋值即可	
  //  EasyProtocol req(json);

  //  string strDeviceSerial = req.GetBodyValue(EASY_TAG_SERIAL);//设备序列号
  //  string strChannel = req.GetBodyValue(EASY_TAG_CHANNEL);//摄像头序列号
  //  string strProtocol = req.GetBodyValue(EASY_TAG_PROTOCOL);//协议,终端仅支持RTSP推送
  //  string strReserve = req.GetBodyValue(EASY_TAG_RESERVE);//流类型
  //  string strFrom = req.GetBodyValue(EASY_TAG_FROM);
  //  string strTo = req.GetBodyValue(EASY_TAG_TO);
  //  string strVia = req.GetBodyValue(EASY_TAG_VIA);

  //  string strCSeq = req.GetHeaderValue(EASY_TAG_CSEQ);//这个是关键字
  //  string strStateCode = req.GetHeaderValue(EASY_TAG_ERROR_NUM);//状态码

  //  if (strChannel.empty())
  //      strChannel = "0";
  //  if (strReserve.empty())
  //      strReserve = "1";

  //  OSRefTableEx* sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
  //  OSRefTableEx::OSRefEx* sessionRef = sessionMap->Resolve(strTo);
  //  if (sessionRef == NULL)
  //      return EASY_ERROR_SESSION_NOT_FOUND;

  //  OSRefReleaserEx releaser(sessionMap, strTo);
  //  HTTPSession* httpSession = static_cast<HTTPSession *>(sessionRef->GetObjectPtr());

  //  if (httpSession->IsLiveSession())
  //  {
  //      //走到这说明对客户端的正确回应,因为错误回应直接返回。
  //      EasyProtocolACK rsp(MSG_SC_PTZ_CONTROL_ACK);
  //      EasyJsonValue header, body;
  //      body[EASY_TAG_SERIAL] = strDeviceSerial;
  //      body[EASY_TAG_CHANNEL] = strChannel;
  //      body[EASY_TAG_PROTOCOL] = strProtocol;//如果当前已经推流，则返回请求的，否则返回实际推流类型
  //      body[EASY_TAG_RESERVE] = strReserve;//如果当前已经推流，则返回请求的，否则返回实际推流类型

  //      header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
  //      header[EASY_TAG_CSEQ] = strCSeq;
  //      header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
  //      header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

  //      rsp.SetHead(header);
  //      rsp.SetBody(body);
  //      string msg = rsp.GetMsg();
		//httpSession->ProcessEvent(msg, httpResponseType);
  //  }

    return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgCSPresetControlReqRESTful(const char* queryString)
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	if (queryString == NULL)
	{
		return QTSS_BadArgument;
	}

	char decQueryString[EASY_MAX_URL_LENGTH] = { 0 };
	EasyUtil::Urldecode((unsigned char*)queryString, reinterpret_cast<unsigned char*>(decQueryString));

	QueryParamList parList(decQueryString);
	const char* chSerial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号
	const char* chChannel = parList.DoFindCGIValueForParam(EASY_TAG_L_CHANNEL);//获取通道
	const char* chProtocol = parList.DoFindCGIValueForParam(EASY_TAG_L_PROTOCOL);//获取通道
	const char* chReserve = parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE);//获取通道
	const char* chCmd = parList.DoFindCGIValueForParam(EASY_TAG_L_CMD);
	const char* chPreset = parList.DoFindCGIValueForParam(EASY_TAG_L_PRESET);

	if (chSerial == NULL || chProtocol == NULL || chCmd == NULL)
		return QTSS_BadArgument;

	//为可选参数填充默认值
	if (chChannel == NULL)
		chChannel = "0";
	if (chReserve == NULL)
		chReserve = "1";

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(chSerial);
	if (theDevRef == NULL)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, chSerial);
	//走到这说明存在指定设备
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	EasyProtocolACK reqreq(MSG_SD_CONTROL_PRESET_REQ);
	EasyJsonValue headerheader, bodybody;

	char chTemp[16] = { 0 };
	UInt32 uDevCseq = pDevSession->GetCSeq();
	sprintf(chTemp, "%d", uDevCseq);
	headerheader[EASY_TAG_CSEQ] = string(chTemp);//注意这个地方不能直接将UINT32->int,因为会造成数据失真
	headerheader[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;

	bodybody[EASY_TAG_SERIAL] = chSerial;
	bodybody[EASY_TAG_CHANNEL] = chChannel;
	bodybody[EASY_TAG_PROTOCOL] = chProtocol;
	bodybody[EASY_TAG_RESERVE] = chReserve;
	bodybody[EASY_TAG_CMD] = chCmd;
	bodybody[EASY_TAG_PRESET] = chPreset;
	bodybody[EASY_TAG_FROM] = fSessionID;
	bodybody[EASY_TAG_TO] = pDevSession->GetValue(EasyHTTPSessionID)->GetAsCString();
	bodybody[EASY_TAG_VIA] = QTSServerInterface::GetServer()->GetCloudServiceNodeID();

	reqreq.SetHead(headerheader);
	reqreq.SetBody(bodybody);

	string buffer = reqreq.GetMsg();
	StrPtrLen theValue(const_cast<char*>(buffer.c_str()), buffer.size());
	pDevSession->SendHTTPPacket(&theValue, false, false);
	fTimeoutTask.SetTimeout(3 * 1000);
	fTimeoutTask.RefreshTimeout();

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgDSPresetControlAck(const char* json)
{
	if (!fAuthenticated)//没有进行认证请求
		return httpUnAuthorized;

	//对于设备的推流回应是不需要在进行回应的，直接解析找到对应的客户端Session，赋值即可	
	EasyProtocol req(json);

	string strDeviceSerial = req.GetBodyValue(EASY_TAG_SERIAL);//设备序列号
	string strChannel = req.GetBodyValue(EASY_TAG_CHANNEL);//摄像头序列号
	string strProtocol = req.GetBodyValue(EASY_TAG_PROTOCOL);//协议,终端仅支持RTSP推送
	string strReserve = req.GetBodyValue(EASY_TAG_RESERVE);//流类型
	string strFrom = req.GetBodyValue(EASY_TAG_FROM);
	string strTo = req.GetBodyValue(EASY_TAG_TO);
	string strVia = req.GetBodyValue(EASY_TAG_VIA);

	string strCSeq = req.GetHeaderValue(EASY_TAG_CSEQ);//这个是关键字
	string strStateCode = req.GetHeaderValue(EASY_TAG_ERROR_NUM);//状态码

	if (strChannel.empty())
		strChannel = "0";
	if (strReserve.empty())
		strReserve = "1";

	OSRefTableEx* sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
	OSRefTableEx::OSRefEx* sessionRef = sessionMap->Resolve(strTo);
	if (sessionRef == NULL)
		return EASY_ERROR_SESSION_NOT_FOUND;

	OSRefReleaserEx releaser(sessionMap, strTo);
	HTTPSession* httpSession = static_cast<HTTPSession *>(sessionRef->GetObjectPtr());

	if (httpSession->IsLiveSession())
	{
		//走到这说明对客户端的正确回应,因为错误回应直接返回。
		EasyProtocolACK rsp(MSG_SC_PRESET_CONTROL_ACK);
		EasyJsonValue header, body;
		body[EASY_TAG_SERIAL] = strDeviceSerial;
		body[EASY_TAG_CHANNEL] = strChannel;
		body[EASY_TAG_PROTOCOL] = strProtocol;//如果当前已经推流，则返回请求的，否则返回实际推流类型
		body[EASY_TAG_RESERVE] = strReserve;//如果当前已经推流，则返回请求的，否则返回实际推流类型

		header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
		header[EASY_TAG_CSEQ] = strCSeq;
		header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
		header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

		rsp.SetHead(header);
		rsp.SetBody(body);
		string msg = rsp.GetMsg();
		StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
		httpSession->SendHTTPPacket(&theValue, false, false);
	}

	return QTSS_NoErr;
}
