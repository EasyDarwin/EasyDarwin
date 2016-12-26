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
#include "OSArrayObjectDeleter.h"
#include "EasyUtil.h"
#include "QueryParamList.h"
#include "Format.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <unordered_set>

#if __FreeBSD__ || __hpux__	
#include <unistd.h>
#endif

#include <errno.h>

#if __solaris__ || __linux__ || __sgi__	|| __hpux__
#include <crypt.h>

typedef struct BITMAPFILEHEADER
{
	u_int16_t bfType;
	u_int32_t bfSize;
	u_int16_t bfReserved1;
	u_int16_t bfReserved2;
	u_int32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER
{
	u_int32_t biSize;
	u_int32_t biWidth;
	u_int32_t biHeight;
	u_int16_t biPlanes;
	u_int16_t biBitCount;
	u_int32_t biCompression;
	u_int32_t biSizeImage;
	u_int32_t biXPelsPerMeter;
	u_int32_t biYPelsPerMeter;
	u_int32_t biClrUsed;
	u_int32_t biClrImportant;
} BITMAPINFODEADER;
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
	: HTTPSessionInterface()
	, fRequest(nullptr)
	, fReadMutex()
	, fSendMutex()
	//, fCurrentModule(0)
	, fState(kReadingFirstRequest)
{
	this->SetTaskName("HTTPSession");

	//All EasyCameraSession/EasyNVRSession/EasyHTTPSession
	QTSServerInterface::GetServer()->AlterCurrentHTTPSessionCount(1);

	fModuleState.curModule = nullptr;
	fModuleState.curTask = this;
	fModuleState.curRole = 0;
	fModuleState.globalLockRequested = false;

	decodeParam.gopTally = SNAP_CAPTURE_TIME;

	decodeParam.imageData = new char[SNAP_SIZE];

	OSRefTableEx* sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
	sessionMap->Register(fSessionID, this);

	//qtss_printf("Create HTTPSession:%s\n", fSessionID);
}

HTTPSession::~HTTPSession()
{
	if (GetSessionType() == EasyHTTPSession)
	{
		auto mutexMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMutex();
		auto deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMap();
		{
			OSMutexLocker lock(mutexMap);

			for (auto& ref : *deviceMap)
			{
				auto session = static_cast<HTTPSession*>(ref.second->GetObjectPtr());
				if (session->GetTalkbackSession() == fSessionID)
				{
					session->SetTalkbackSession("");
				}
			}
		}
	}

	if (decodeParam.imageData)
	{
		delete[]decodeParam.imageData;
		decodeParam.imageData = nullptr;
	}

	fLiveSession = false;
	this->cleanupRequest();

	QTSServerInterface::GetServer()->AlterCurrentHTTPSessionCount(-1);

	auto sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
	sessionMap->UnRegister(fSessionID);

	//qtss_printf("Release HTTPSession:%s\n", fSessionID);

	if (fRequestBody)
	{
		delete[]fRequestBody;
		fRequestBody = nullptr;
	}
}

SInt64 HTTPSession::Run()
{
	EventFlags events = this->GetEvents();
	QTSS_Error err = QTSS_NoErr;

	// Some callbacks look for this struct in the thread object
	OSThreadDataSetter theSetter(&fModuleState, nullptr);

	if (events & kKillEvent)
		fLiveSession = false;

	if (events & kTimeoutEvent)
	{
		string msgStr = Format("Timeout HTTPSession，Device_serial[%s]\n", device_->serial_);
		QTSServerInterface::LogError(qtssMessageVerbosity, const_cast<char *>(msgStr.c_str()));
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

				Assert(fRequest == nullptr);
				fRequest = new HTTPRequest(&QTSServerInterface::GetServerHeader(), fInputStream.GetRequestBuffer());

				fReadMutex.Lock();
				fSessionMutex.Lock();

				fOutputStream.ResetBytesWritten();

				if ((err == E2BIG) || (err == QTSS_BadArgument))
				{
					execNetMsgErrorReqHandler(httpBadRequest);
					fState = kSendingResponse;
					break;
				}

				Assert(err == QTSS_RequestArrived);
				fState = kFilteringRequest;
			}

		case kFilteringRequest:
			{
				fTimeoutTask.RefreshTimeout();

				if (fSessionType != EasyHTTPSession && !device_->serial_.empty())
				{
					QTSS_RoleParams theParams;
					theParams.DeviceInfoParams.inDevice = &device_;
					UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisSetDeviceRole);
					for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
					{
						QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisSetDeviceRole, currentModule);
						(void)theModule->CallDispatch(Easy_RedisSetDevice_Role, &theParams);
					}
				}

				QTSS_Error theErr = setupRequest();

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
					execNetMsgErrorReqHandler(httpBadRequest);
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
				processRequest();

				if (fOutputStream.GetBytesWritten() > 0)
				{
					delete[] fRequestBody;
					fRequestBody = nullptr;
					fState = kSendingResponse;
					break;
				}

				delete[] fRequestBody;
				fRequestBody = nullptr;
				fState = kCleaningUp;
				break;
			}

		case kProcessingRequest:
			{
				if (fOutputStream.GetBytesWritten() == 0)
				{
					execNetMsgErrorReqHandler(httpInternalServerError);
					fState = kSendingResponse;
					break;
				}

				fState = kSendingResponse;
			}
		case kSendingResponse:
			{
				Assert(fRequest != nullptr);

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
					err = this->dumpRequestData();

					if (err == EAGAIN)
					{
						fInputSocketP->RequestEvent(EV_RE);
						this->ForceSameThread();    // We are holding mutexes, so we need to force
						// the same thread to be used for next Run()
						return 0;
					}
				}

				this->cleanupRequest();

				fState = kReadingRequest;
			}
		default: break;
		}
	}

	this->cleanupRequest();

	if (fObjectHolders == 0)
		return -1;

	return 0;
}

QTSS_Error HTTPSession::SendHTTPPacket(StrPtrLen* contentXML, bool connectionClose, bool decrement)
{
	OSMutexLocker lock(&fSendMutex);

	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

	if (httpAck.CreateResponseHeader(httpOK))
	{
		if (contentXML->Len)
			httpAck.AppendContentLengthHeader(contentXML->Len);

		if (connectionClose)
			httpAck.AppendConnectionCloseHeader();

		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		string sendString(ackPtr->Ptr, ackPtr->Len);
		if (contentXML->Len > 0)
			sendString.append(contentXML->Ptr, contentXML->Len);

		UInt32 theLengthSent = 0;
		UInt32 amtInBuffer = sendString.size();
		do
		{
			auto str = fOutputSocketP->GetRemoteAddrStr();
			string remote(str->Ptr);
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

QTSS_Error HTTPSession::setupRequest()
{
	QTSS_Error theErr = fRequest->Parse();
	if (theErr != QTSS_NoErr)
		return QTSS_BadArgument;

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
				if (path[0] == "api" && path[1] == EASY_PROTOCOL_VERSION && path[2] == "getdevicelist")
				{
					return execNetMsgCSGetDeviceListReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == EASY_PROTOCOL_VERSION && path[2] == "getdeviceinfo")
				{
					return execNetMsgCSGetCameraListReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == EASY_PROTOCOL_VERSION && path[2] == "startdevicestream")
				{
					return execNetMsgCSStartStreamReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == EASY_PROTOCOL_VERSION && path[2] == "stopdevicestream")
				{
					return execNetMsgCSStopStreamReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == EASY_PROTOCOL_VERSION && path[2] == "ptzcontrol")
				{
					return execNetMsgCSPTZControlReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == EASY_PROTOCOL_VERSION && path[2] == "presetcontrol")
				{
					return execNetMsgCSPresetControlReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == EASY_PROTOCOL_VERSION && path[2] == "getbaseconfig")
				{
					return execNetMsgCSGetBaseConfigReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == EASY_PROTOCOL_VERSION && path[2] == "setbaseconfig")
				{
					return execNetMsgCSSetBaseConfigReqRESTful(fRequest->GetQueryString());
				}

				if (path[0] == "api" && path[1] == EASY_PROTOCOL_VERSION && path[2] == "restart")
				{
					return execNetMsgCSRestartReqRESTful(fRequest->GetQueryString());
				}
			}

			execNetMsgCSGetUsagesReqRESTful(nullptr);

			return QTSS_NoErr;
		}
	}

	//READ json Content

	//1、get json content length
	StrPtrLen* lengthPtr = fRequest->GetHeaderValue(httpContentLengthHeader);

	StringParser theContentLenParser(lengthPtr);
	theContentLenParser.ConsumeWhitespace();
	UInt32 content_length = theContentLenParser.ConsumeInteger(nullptr);

	//qtss_printf("HTTPSession read content-length:%d \n", content_length);

	if (content_length <= 0)
	{
		return QTSS_BadArgument;
	}

	// Check for the existence of 2 attributes in the request: a pointer to our buffer for
	// the request body, and the current offset in that buffer. If these attributes exist,
	// then we've already been here for this request. If they don't exist, add them.
	UInt32 theBufferOffset = 0;
	char* theRequestBody = nullptr;
	UInt32 theLen = sizeof(theRequestBody);
	theErr = QTSS_GetValue(this, EasyHTTPSesContentBody, 0, &theRequestBody, &theLen);

	if (theErr != QTSS_NoErr)
	{
		// First time we've been here for this request. Create a buffer for the content body and
		// shove it in the request.
		theRequestBody = new char[content_length + 1];
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

	//qtss_printf("HTTPSession read content-length:%d (%d/%d) \n", theLen, theBufferOffset + theLen, content_length);
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

	//if (theBufferOffset < sHeaderSize)
	//qtss_printf("Recv message: %s\n", fRequestBody);

	UInt32 offset = 0;
	(void)QTSS_SetValue(this, EasyHTTPSesContentBodyOffset, 0, &offset, sizeof(offset));
	char* content = nullptr;
	(void)QTSS_SetValue(this, EasyHTTPSesContentBody, 0, &content, 0);

	return theErr;
}

void HTTPSession::cleanupRequest()
{
	if (fRequest != nullptr)
	{
		// nullptr out any references to the current request
		delete fRequest;
		fRequest = nullptr;
	}

	fSessionMutex.Unlock();
	fReadMutex.Unlock();

	// Clear out our last value for request body length before moving onto the next request
	this->SetRequestBodyLength(-1);
}

bool HTTPSession::overMaxConnections(UInt32 buffer)
{
	QTSServerInterface* theServer = QTSServerInterface::GetServer();
	SInt32 maxConns = theServer->GetPrefs()->GetMaxConnections();
	bool overLimit = false;

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

QTSS_Error HTTPSession::dumpRequestData()
{
	char theDumpBuffer[EASY_REQUEST_BUFFER_SIZE_LEN];

	QTSS_Error theErr = QTSS_NoErr;
	while (theErr == QTSS_NoErr)
		theErr = this->Read(theDumpBuffer, EASY_REQUEST_BUFFER_SIZE_LEN, nullptr);

	return theErr;
}

QTSS_Error HTTPSession::execNetMsgDSPostSnapReq(const char* json)
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
		channel = "1";

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

	string jpgDir = string(QTSServerInterface::GetServer()->GetPrefs()->GetSnapLocalPath()).append(device_serial);
	OS::RecursiveMakeDir(const_cast<char*>(jpgDir.c_str()));
	string jpgPath = Format("%s/%s_%s_%s.%s", jpgDir, device_serial, channel, strTime, EasyProtocol::GetSnapTypeString(EASY_SNAP_TYPE_JPEG));

	FILE* fSnap = ::fopen(jpgPath.c_str(), "wb");
	if (fSnap == nullptr)
	{
		//DWORD e=GetLastError();
		return QTSS_NoErr;
	}

	auto picType = EasyProtocol::GetSnapType(strType);

	if (picType == EASY_SNAP_TYPE_JPEG)
	{
		fwrite(image.data(), 1, image.size(), fSnap);
	}
	else if (picType == EASY_SNAP_TYPE_IDR)
	{
		string decQueryString = EasyUtil::Urldecode(reserve);

		QueryParamList parList(const_cast<char *>(decQueryString.c_str()));
		int width = EasyUtil::String2Int(parList.DoFindCGIValueForParam("width"));
		int height = EasyUtil::String2Int(parList.DoFindCGIValueForParam("height"));
		int codec = EasyUtil::String2Int(parList.DoFindCGIValueForParam("codec"));

		rawData2Image(const_cast<char*>(image.data()), image.size(), codec, width, height);
		fwrite(decodeParam.imageData, 1, decodeParam.imageSize, fSnap);
	}

	::fclose(fSnap);

	//web path

	string snapURL = Format("%s%s/%s_%s_%s.%s", string(QTSServerInterface::GetServer()->GetPrefs()->GetSnapWebPath()), device_serial,
		device_serial, channel, strTime, EasyProtocol::GetSnapTypeString(EASY_SNAP_TYPE_JPEG));
	device_->HoldSnapPath(snapURL, channel);

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

QTSS_Error HTTPSession::execNetMsgErrorReqHandler(HTTPStatusCode errCode)
{
	//HTTP Header
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);

	if (httpAck.CreateResponseHeader(errCode))
	{
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		UInt32 theLengthSent = 0;
		QTSS_Error theErr = fOutputSocketP->Send(ackPtr->Ptr, ackPtr->Len, &theLengthSent);
		if (theErr != QTSS_NoErr && theErr != EAGAIN)
		{
			return theErr;
		}
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
QTSS_Error HTTPSession::execNetMsgDSRegisterReq(const char* json)
{
	QTSS_Error theErr = QTSS_NoErr;
	EasyMsgDSRegisterREQ regREQ(json);

	//update info each time
	if (!device_->GetDevInfo(json))
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
			break;
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

		auto deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
		auto regErr = deviceMap->Register(device_->serial_, this);
		if (regErr == OS_NoErr)
		{
			//在redis上增加设备
			auto msgStr = Format("Device register，Device_serial[%s]\n", device_->serial_);
			QTSServerInterface::LogError(qtssMessageVerbosity, const_cast<char*>(msgStr.c_str()));

			QTSS_RoleParams theParams;
			theParams.DeviceInfoParams.inDevice = &device_;
			UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisSetDeviceRole);
			for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
			{
				QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisSetDeviceRole, currentModule);
				(void)theModule->CallDispatch(Easy_RedisSetDevice_Role, &theParams);
			}
			fAuthenticated = true;
		}
		else
		{
			//设备冲突的时候将前一个设备给挤掉,因为断电、断网情况下连接是不会断开的，如果设备来电、网络通顺之后就会产生冲突，
			//一个连接的超时时90秒，要等到90秒之后设备才能正常注册上线。
			OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(device_->serial_);
			if (theDevRef != nullptr)//找到指定设备
			{
				OSRefReleaserEx releaser(deviceMap, device_->serial_);
				HTTPSession* pDevSession = static_cast<HTTPSession*>(theDevRef->GetObjectPtr());//获得当前设备会话
				pDevSession->Signal(Task::kKillEvent);//终止设备连接
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

	body[EASY_TAG_SERIAL] = device_->serial_;

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSFreeStreamReq(const char* json)//客户端的停止直播请求
{
	//算法描述：查找指定设备，若设备存在，则向设备发出停止流请求
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/
	EasyProtocol req(json);

	string strDeviceSerial = req.GetBodyValue(EASY_TAG_SERIAL);
	string strChannel = req.GetBodyValue(EASY_TAG_CHANNEL);

	if (strDeviceSerial.size() <= 0)
	{
		return QTSS_BadArgument;
	}

	//string strDeviceSerial	=	req.GetBodyValue(EASY_TAG_SERIAL);//设备序列号
	//string strChannel	=	req.GetBodyValue(EASY_TAG_CHANNEL);//摄像头序列号
	string strReserve = req.GetBodyValue(EASY_TAG_RESERVE);//StreamID
	string strProtocol = req.GetBodyValue(EASY_TAG_PROTOCOL);//Protocol

	//为可选参数填充默认值
	if (strChannel.empty())
		strChannel = "1";
	if (strReserve.empty())
		strReserve = "1";

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(strDeviceSerial);////////////////////////////////++
	if (theDevRef == nullptr)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, strDeviceSerial);
	//走到这说明存在指定设备，则该设备发出停止推流请求
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	EasyProtocolACK reqreq(MSG_SD_STREAM_STOP_REQ);
	EasyJsonValue headerheader, bodybody;

	headerheader[EASY_TAG_CSEQ] = EasyUtil::ToString(pDevSession->GetCSeq());//注意这个地方不能直接将UINT32->int,因为会造成数据失真
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

QTSS_Error HTTPSession::execNetMsgDSStreamStopAck(const char* json) const
{
	if (!fAuthenticated)
	{
		return httpUnAuthorized;
	}

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSStartStreamReqRESTful(const char* queryString)//放到ProcessRequest所在的状态去处理，方便多次循环调用
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	if (queryString == nullptr)
	{
		return QTSS_BadArgument;
	}

	string decQueryString = EasyUtil::Urldecode(queryString);

	QueryParamList parList(const_cast<char *>(decQueryString.c_str()));
	const char* chSerial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号
	const char* chChannel = parList.DoFindCGIValueForParam(EASY_TAG_L_CHANNEL);//获取通道
	const char* chReserve = parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE);

	//为可选参数填充默认值
	if (!isRightChannel(chChannel))
		chChannel = "1";
	if (chReserve == nullptr)
		chReserve = "1";

	if (chSerial == nullptr)
		return QTSS_BadArgument;

	string strCSeq = EasyUtil::ToString(GetCSeq());
	string service;

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(chSerial);
	if (theDevRef == nullptr)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, chSerial);
	//走到这说明存在指定设备
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	string strDssIP, strHttpPort, strDssPort;
	char chDssIP[sIPSize] = { 0 };
	char chDssPort[sPortSize] = { 0 };
	char chHTTPPort[sPortSize] = { 0 };

	QTSS_RoleParams theParams;
	theParams.GetAssociatedDarwinParams.inSerial = const_cast<char*>(chSerial);
	theParams.GetAssociatedDarwinParams.inChannel = const_cast<char*>(chChannel);
	theParams.GetAssociatedDarwinParams.outDssIP = chDssIP;
	theParams.GetAssociatedDarwinParams.outHTTPPort = chHTTPPort;
	theParams.GetAssociatedDarwinParams.outDssPort = chDssPort;
	theParams.GetAssociatedDarwinParams.isOn = false;

	UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisGetEasyDarwinRole);
	for (UInt32 currentModule = 0; currentModule < numModules; ++currentModule)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisGetEasyDarwinRole, currentModule);
		(void)theModule->CallDispatch(Easy_RedisGetEasyDarwin_Role, &theParams);
	}

	int errorNo = EASY_ERROR_SUCCESS_OK;

	if (chDssIP[0] != 0)//是否存在关联的EasyDarWin转发服务器test,应该用Redis上的数据，因为推流是不可靠的，而EasyDarWin上的数据是可靠的
	{
		strDssIP = chDssIP;
		strHttpPort = chHTTPPort;
		strDssPort = chDssPort;

		service = string("IP=") + strDssIP + ";Port=" + strHttpPort + ";Type=EasyDarwin";

		if (!theParams.GetAssociatedDarwinParams.isOn)
		{
			EasyProtocolACK reqreq(MSG_SD_PUSH_STREAM_REQ);
			EasyJsonValue headerheader, bodybody;

			headerheader[EASY_TAG_CSEQ] = EasyUtil::ToString(pDevSession->GetCSeq());
			headerheader[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;

			bodybody[EASY_TAG_SERVER_IP] = strDssIP;
			bodybody[EASY_TAG_SERVER_PORT] = strDssPort;
			bodybody[EASY_TAG_SERIAL] = chSerial;
			bodybody[EASY_TAG_CHANNEL] = chChannel;
			bodybody[EASY_TAG_RESERVE] = chReserve;
			bodybody[EASY_TAG_FROM] = fSessionID;
			bodybody[EASY_TAG_TO] = pDevSession->GetValue(EasyHTTPSessionID)->GetAsCString();
			bodybody[EASY_TAG_VIA] = QTSServerInterface::GetServer()->GetCloudServiceNodeID();

			darwinHttpPort_ = strHttpPort;

			reqreq.SetHead(headerheader);
			reqreq.SetBody(bodybody);

			string buffer = reqreq.GetMsg();
			StrPtrLen theValue(const_cast<char*>(buffer.c_str()), buffer.size());
			pDevSession->SendHTTPPacket(&theValue, false, false);
			fTimeoutTask.SetTimeout(3 * 1000);
			fTimeoutTask.RefreshTimeout();

			return QTSS_NoErr;
		}
	}
	else
	{
		errorNo = EASY_ERROR_SERVER_UNAVAILABLE;
	}

	//走到这说明对客户端的正确回应,因为错误回应直接返回。
	EasyProtocolACK rsp(MSG_SC_START_STREAM_ACK);
	EasyJsonValue header, body;
	body[EASY_TAG_SERVICE] = service;
	body[EASY_TAG_SERIAL] = chSerial;
	body[EASY_TAG_CHANNEL] = chChannel;
	body[EASY_TAG_RESERVE] = chReserve;//如果当前已经推流，则返回请求的，否则返回实际推流类型

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = strCSeq;
	header[EASY_TAG_ERROR_NUM] = errorNo;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(errorNo);

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}


QTSS_Error HTTPSession::execNetMsgCSStopStreamReqRESTful(const char* queryString)//放到ProcessRequest所在的状态去处理，方便多次循环调用
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	if (queryString == nullptr)
	{
		return QTSS_BadArgument;
	}

	string decQueryString = EasyUtil::Urldecode(queryString);

	QueryParamList parList(const_cast<char*>(decQueryString.c_str()));
	const char* strDeviceSerial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号
	const char* strChannel = parList.DoFindCGIValueForParam(EASY_TAG_L_CHANNEL);//获取通道
	const char* strReserve = parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE);//

	//为可选参数填充默认值
	if (!isRightChannel(strChannel))
		strChannel = "1";
	if (strReserve == nullptr)
		strReserve = "1";

	if (strDeviceSerial == nullptr)
		return QTSS_BadArgument;

	string strCSeq = EasyUtil::ToString(GetCSeq());

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(strDeviceSerial);////////////////////////////////++
	if (theDevRef == nullptr)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, strDeviceSerial);
	//走到这说明存在指定设备，则该设备发出停止推流请求
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	EasyProtocolACK reqreq(MSG_SD_STREAM_STOP_REQ);
	EasyJsonValue headerheader, bodybody;

	headerheader[EASY_TAG_CSEQ] = EasyUtil::ToString(pDevSession->GetCSeq());//注意这个地方不能直接将UINT32->int,因为会造成数据失真
	headerheader[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;

	bodybody[EASY_TAG_SERIAL] = strDeviceSerial;
	bodybody[EASY_TAG_CHANNEL] = strChannel;
	bodybody[EASY_TAG_RESERVE] = strReserve;
	bodybody[EASY_TAG_PROTOCOL] = "";
	bodybody[EASY_TAG_FROM] = fSessionID;
	bodybody[EASY_TAG_TO] = pDevSession->GetValue(EasyHTTPSessionID)->GetAsCString();
	bodybody[EASY_TAG_VIA] = QTSServerInterface::GetServer()->GetCloudServiceNodeID();

	reqreq.SetHead(headerheader);
	reqreq.SetBody(bodybody);

	string buffer = reqreq.GetMsg();

	StrPtrLen theValue(const_cast<char*>(buffer.c_str()), buffer.size());
	pDevSession->SendHTTPPacket(&theValue, false, false);

	//直接对客户端（EasyDarWin)进行正确回应
	EasyProtocolACK rsp(MSG_SC_STOP_STREAM_ACK);
	EasyJsonValue header, body;
	header[EASY_TAG_CSEQ] = strCSeq;
	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	body[EASY_TAG_SERIAL] = strDeviceSerial;
	body[EASY_TAG_CHANNEL] = strChannel;
	body[EASY_TAG_RESERVE] = strReserve;

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();

	StrPtrLen theValueAck(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValueAck, false, false);

	return QTSS_NoErr;
}


QTSS_Error HTTPSession::execNetMsgDSPushStreamAck(const char* json) const
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
		strChannel = "1";
	if (strReserve.empty())
		strReserve = "1";

	OSRefTableEx* sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
	OSRefTableEx::OSRefEx* sessionRef = sessionMap->Resolve(strTo);
	if (sessionRef == nullptr)
		return EASY_ERROR_SESSION_NOT_FOUND;

	OSRefReleaserEx releaser(sessionMap, strTo);
	HTTPSession* httpSession = static_cast<HTTPSession *>(sessionRef->GetObjectPtr());

	if (httpSession->IsLiveSession())
	{
		string service = string("IP=") + strDssIP + ";Port=" + httpSession->GetDarwinHTTPPort() + ";Type=EasyDarwin";

		//走到这说明对客户端的正确回应,因为错误回应直接返回。
		EasyProtocolACK rsp(MSG_SC_START_STREAM_ACK);
		EasyJsonValue header, body;
		body[EASY_TAG_SERVICE] = service;
		body[EASY_TAG_SERIAL] = strDeviceSerial;
		body[EASY_TAG_CHANNEL] = strChannel;
		body[EASY_TAG_RESERVE] = strReserve;

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

QTSS_Error HTTPSession::execNetMsgCSGetDeviceListReqRESTful(const char* queryString)//客户端获得设备列表
{

	//if (!fAuthenticated)//没有进行认证请求
	//	return httpUnAuthorized;

	std::string queryTemp;
	if (queryString != nullptr)
	{
		queryTemp = EasyUtil::Urldecode(queryString);
	}
	QueryParamList parList(const_cast<char *>(queryTemp.c_str()));
	const char* chAppType = parList.DoFindCGIValueForParam(EASY_TAG_APP_TYPE);//APPType
	const char* chTerminalType = parList.DoFindCGIValueForParam(EASY_TAG_TERMINAL_TYPE);//TerminalType

	EasyProtocolACK rsp(MSG_SC_DEVICE_LIST_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	unordered_set<string> terminalSet;
	if (chTerminalType != nullptr)
	{
		string terminalTemp(chTerminalType);

		if (boost::ends_with(terminalTemp, "|"))
		{
			boost::erase_tail(terminalTemp, 1);
		}
		boost::split(terminalSet, terminalTemp, boost::is_any_of("|"), boost::token_compress_on);
	}

	OSMutex* mutexMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMutex();
	OSHashMap* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMap();
	OSRefIt itRef;
	Json::Value* proot = rsp.GetRoot();

	{
		OSMutexLocker lock(mutexMap);
		int iDevNum = 0;

		for (itRef = deviceMap->begin(); itRef != deviceMap->end(); ++itRef)
		{
			auto deviceInfo = static_cast<HTTPSession*>(itRef->second->GetObjectPtr())->GetDeviceInfo();
			if (chAppType != nullptr)// AppType fileter
			{
				if (EasyProtocol::GetAppTypeString(deviceInfo->eAppType) != string(chAppType))
					continue;
			}
			if (chTerminalType != nullptr)// TerminateType fileter
			{
				if (terminalSet.find(EasyProtocol::GetTerminalTypeString(deviceInfo->eDeviceType)) == terminalSet.end())
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

QTSS_Error HTTPSession::execNetMsgCSDeviceListReq(const char* json)//客户端获得设备列表
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
			auto deviceInfo = static_cast<HTTPSession*>(itRef->second->GetObjectPtr())->GetDeviceInfo();
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

QTSS_Error HTTPSession::execNetMsgCSGetCameraListReqRESTful(const char* queryString)
{
	/*
		if(!fAuthenticated)//没有进行认证请求
		return httpUnAuthorized;
	*/
	if (queryString == nullptr)
	{
		return QTSS_BadArgument;
	}

	string decQueryString = EasyUtil::Urldecode(queryString);

	QueryParamList parList(const_cast<char *>(decQueryString.c_str()));

	const char* device_serial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号

	if (device_serial == nullptr)
		return QTSS_BadArgument;

	EasyProtocolACK rsp(MSG_SC_DEVICE_INFO_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;

	body[EASY_TAG_SERIAL] = device_serial;

	auto deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	auto theDevRef = deviceMap->Resolve(device_serial);
	if (theDevRef == nullptr)//不存在指定设备
	{
		header[EASY_TAG_ERROR_NUM] = EASY_ERROR_DEVICE_NOT_FOUND;
		header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_DEVICE_NOT_FOUND);
	}
	else//存在指定设备，则获取这个设备的摄像头信息
	{
		OSRefReleaserEx releaser(deviceMap, device_serial);

		header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
		header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

		auto proot = rsp.GetRoot();
		auto deviceInfo = static_cast<HTTPSession*>(theDevRef->GetObjectPtr())->GetDeviceInfo();
		if (deviceInfo->eAppType == EASY_APP_TYPE_CAMERA)
		{
			body[EASY_TAG_SNAP_URL] = deviceInfo->snapJpgPath_;
		}
		else
		{
			EasyDevicesIterator itCam;
			body[EASY_TAG_CHANNEL_COUNT] = deviceInfo->channelCount_;
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

QTSS_Error HTTPSession::execNetMsgCSCameraListReq(const char* json)
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
	if (theDevRef == nullptr)//不存在指定设备
	{
		return EASY_ERROR_DEVICE_NOT_FOUND;//交给错误处理中心处理
	}
	//存在指定设备，则获取这个设备的摄像头信息
	OSRefReleaserEx releaser(deviceMap, strDeviceSerial);

	Json::Value* proot = rsp.GetRoot();
	auto deviceInfo = static_cast<HTTPSession*>(theDevRef->GetObjectPtr())->GetDeviceInfo();
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

QTSS_Error HTTPSession::processRequest()//处理请求
{
	//OSCharArrayDeleter charArrayPathDeleter(theRequestBody);//不要在这删除，因为可能执行多次，仅当对请求的处理完毕后再进行删除

	if (fRequestBody == nullptr)//表示没有正确的解析请求，SetUpRequest环节没有解析出数据部分
		return QTSS_NoErr;

	//消息处理
	QTSS_Error theErr;
	EasyProtocol protocol(fRequestBody);
	int nNetMsg = protocol.GetMessageType(), nRspMsg = MSG_SC_EXCEPTION;

	switch (nNetMsg)
	{
	case MSG_DS_REGISTER_REQ://处理设备上线消息,设备类型包括NVR、摄像头和智能主机
		theErr = execNetMsgDSRegisterReq(fRequestBody);
		nRspMsg = MSG_SD_REGISTER_ACK;
		break;
	case MSG_DS_PUSH_STREAM_ACK://设备的开始流回应
		theErr = execNetMsgDSPushStreamAck(fRequestBody);
		nRspMsg = MSG_DS_PUSH_STREAM_ACK;//注意，这里实际上是不应该再回应的
		break;
	case MSG_CS_FREE_STREAM_REQ://客户端的停止直播请求
		theErr = execNetMsgCSFreeStreamReq(fRequestBody);
		nRspMsg = MSG_SC_FREE_STREAM_ACK;
		break;
	case MSG_DS_STREAM_STOP_ACK://设备对EasyCMS的停止推流回应
		theErr = execNetMsgDSStreamStopAck(fRequestBody);
		nRspMsg = MSG_DS_STREAM_STOP_ACK;//注意，这里实际上是不需要在进行回应的
		break;
	case MSG_CS_DEVICE_LIST_REQ://设备列表请求
		theErr = execNetMsgCSDeviceListReq(fRequestBody);//add
		nRspMsg = MSG_SC_DEVICE_LIST_ACK;
		break;
	case MSG_CS_DEVICE_INFO_REQ://摄像头列表请求,设备的具体信息
		theErr = execNetMsgCSCameraListReq(fRequestBody);//add
		nRspMsg = MSG_SC_DEVICE_INFO_ACK;
		break;
	case MSG_DS_POST_SNAP_REQ://设备快照上传
		theErr = execNetMsgDSPostSnapReq(fRequestBody);
		nRspMsg = MSG_SD_POST_SNAP_ACK;
		break;
	case MSG_DS_CONTROL_PTZ_ACK:
		theErr = execNetMsgDSPTZControlAck(fRequestBody);
		nRspMsg = MSG_DS_CONTROL_PTZ_ACK;
		break;
	case MSG_DS_CONTROL_PRESET_ACK:
		theErr = execNetMsgDSPresetControlAck(fRequestBody);
		nRspMsg = MSG_DS_CONTROL_PRESET_ACK;
		break;
	case MSG_CS_TALKBACK_CONTROL_REQ:
		theErr = execNetMsgCSTalkbackControlReq(fRequestBody);
		nRspMsg = MSG_SC_TALKBACK_CONTROL_ACK;
		break;
	case MSG_DS_CONTROL_TALKBACK_ACK:
		theErr = execNetMSGDSTalkbackControlAck(fRequestBody);
		nRspMsg = MSG_DS_CONTROL_TALKBACK_ACK;
		break;
	default:
		theErr = execNetMsgErrorReqHandler(httpNotImplemented);
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
	/*if (nBpp == 16)
	{
		bfh.bfOffBits += sizeof(RGBQUAD) * 3;
	}
	else
	{
		bfh.bfOffBits += sizeof(RGBQUAD) * 1;
	}*/

	auto dwWriteLength = sizeof(BITMAPFILEHEADER);
	int rgbOffset = 0;
	memcpy(rgbdata + rgbOffset, static_cast<void*>(&bfh), dwWriteLength);
	rgbOffset += dwWriteLength;

	BITMAPINFOHEADER	bih = { 0, };
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = dwW;
	bih.biHeight = -static_cast<int>(dwH);
	bih.biPlanes = 1;
	bih.biBitCount = nBpp;
	//bih.biCompression = (nBpp == 16) ? BI_BITFIELDS : BI_RGB;
	bih.biCompression = 0;
	bih.biSizeImage = dwWB * height;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	dwWriteLength = sizeof(BITMAPINFOHEADER);

	memcpy(rgbdata + rgbOffset, static_cast<void*>(&bih), dwWriteLength);
	rgbOffset += dwWriteLength;

	if (nBpp == 24)
	{
		unsigned long rgbQuad = 0;
		dwWriteLength = sizeof(rgbQuad);
		memcpy(rgbdata + rgbOffset, static_cast<void*>(&rgbQuad), dwWriteLength);
		rgbOffset += dwWriteLength;
	}

	dwWriteLength = dwWB * height;
	memcpy(rgbdata + rgbOffset, static_cast<void*>(yuvpbuf), dwWriteLength);
	rgbOffset += dwWriteLength;

	if (nullptr != rgbsize)	*rgbsize = rgbOffset;

	return 0;
}

QTSS_Error HTTPSession::rawData2Image(char* rawBuf, int bufSize, int codec, int width, int height)
{
	QTSS_Error theErr = QTSS_NoErr;

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
		yuv2BMPImage(snapWidth, snapHeight, static_cast<char*>(yuvdata), &decodeParam.imageSize, reinterpret_cast<unsigned char*>(decodeParam.imageData));
	}

	if (nullptr != yuvdata)
	{
		delete[] yuvdata;
	}

	return theErr;
}

QTSS_Error HTTPSession::execNetMsgCSPTZControlReqRESTful(const char* queryString)
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	if (queryString == nullptr)
	{
		return QTSS_BadArgument;
	}

	string decQueryString = EasyUtil::Urldecode(queryString);

	QueryParamList parList(const_cast<char *>(decQueryString.c_str()));

	const char* chSerial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号
	const char* chChannel = parList.DoFindCGIValueForParam(EASY_TAG_L_CHANNEL);//获取通道
	const char* chProtocol = parList.DoFindCGIValueForParam(EASY_TAG_L_PROTOCOL);//获取通道
	const char* chReserve(parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE));//获取通道
	const char* chActionType = parList.DoFindCGIValueForParam(EASY_TAG_L_ACTION_TYPE);
	const char* chCmd = parList.DoFindCGIValueForParam(EASY_TAG_L_CMD);
	const char* chSpeed = parList.DoFindCGIValueForParam(EASY_TAG_L_SPEED);

	if (chSerial == nullptr || chProtocol == nullptr || chActionType == nullptr || chCmd == nullptr)
		return QTSS_BadArgument;

	//为可选参数填充默认值
	if (!isRightChannel(chChannel))
		chChannel = "1";
	if (chReserve == nullptr)
		chReserve = "1";

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(chSerial);
	if (theDevRef == nullptr)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, chSerial);
	//走到这说明存在指定设备
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	EasyProtocolACK reqreq(MSG_SD_CONTROL_PTZ_REQ);
	EasyJsonValue headerheader, bodybody;

	string strCSEQ = EasyUtil::ToString(pDevSession->GetCSeq());
	headerheader[EASY_TAG_CSEQ] = strCSEQ;//注意这个地方不能直接将UINT32->int,因为会造成数据失真
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
	header[EASY_TAG_CSEQ] = strCSEQ;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();
	StrPtrLen theValueAck(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValueAck, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgDSPTZControlAck(const char* json)
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
	//      strChannel = "1";
	//  if (strReserve.empty())
	//      strReserve = "1";

	//  OSRefTableEx* sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
	//  OSRefTableEx::OSRefEx* sessionRef = sessionMap->Resolve(strTo);
	//  if (sessionRef == nullptr)
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

QTSS_Error HTTPSession::execNetMsgCSPresetControlReqRESTful(const char* queryString)
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	if (queryString == nullptr)
	{
		return QTSS_BadArgument;
	}

	string decQueryString = EasyUtil::Urldecode(queryString);

	QueryParamList parList(const_cast<char *>(decQueryString.c_str()));

	const char* chSerial = parList.DoFindCGIValueForParam(EASY_TAG_L_DEVICE);//获取设备序列号
	const char* chChannel = parList.DoFindCGIValueForParam(EASY_TAG_L_CHANNEL);//获取通道
	const char* chProtocol = parList.DoFindCGIValueForParam(EASY_TAG_L_PROTOCOL);//获取通道
	const char* chReserve = parList.DoFindCGIValueForParam(EASY_TAG_L_RESERVE);//获取通道
	const char* chCmd = parList.DoFindCGIValueForParam(EASY_TAG_L_CMD);
	const char* chPreset = parList.DoFindCGIValueForParam(EASY_TAG_L_PRESET);

	if (chSerial == nullptr || chProtocol == nullptr || chCmd == nullptr)
		return QTSS_BadArgument;

	//为可选参数填充默认值
	if (!isRightChannel(chChannel))
		chChannel = "1";
	if (chReserve == nullptr)
		chReserve = "1";

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(chSerial);
	if (theDevRef == nullptr)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, chSerial);
	//走到这说明存在指定设备
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	EasyProtocolACK reqreq(MSG_SD_CONTROL_PRESET_REQ);
	EasyJsonValue headerheader, bodybody;

	headerheader[EASY_TAG_CSEQ] = EasyUtil::ToString(pDevSession->GetCSeq());//注意这个地方不能直接将UINT32->int,因为会造成数据失真
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

QTSS_Error HTTPSession::execNetMsgDSPresetControlAck(const char* json) const
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
		strChannel = "1";
	if (strReserve.empty())
		strReserve = "1";

	OSRefTableEx* sessionMap = QTSServerInterface::GetServer()->GetHTTPSessionMap();
	OSRefTableEx::OSRefEx* sessionRef = sessionMap->Resolve(strTo);
	if (sessionRef == nullptr)
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

QTSS_Error HTTPSession::execNetMsgCSTalkbackControlReq(const char* json)
{
	//if (!fAuthenticated)//没有进行认证请求
	//	return httpUnAuthorized;

	EasyProtocol req(json);

	string strDeviceSerial = req.GetBodyValue(EASY_TAG_SERIAL);
	string strChannel = req.GetBodyValue(EASY_TAG_CHANNEL);
	string strProtocol = req.GetBodyValue(EASY_TAG_PROTOCOL);
	string strReserve = req.GetBodyValue(EASY_TAG_RESERVE);
	string strCmd = req.GetBodyValue(EASY_TAG_CMD);
	string strAudioType = req.GetBodyValue(EASY_TAG_AUDIO_TYPE);
	string strAudioData = req.GetBodyValue(EASY_TAG_AUDIO_DATA);
	string strPts = req.GetBodyValue(EASY_TAG_PTS);

	string strCSeq = req.GetHeaderValue(EASY_TAG_CSEQ);//这个是关键字

	if (strChannel.empty())
		strChannel = "1";
	if (strReserve.empty())
		strReserve = "1";

	OSRefTableEx* deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRefTableEx::OSRefEx* theDevRef = deviceMap->Resolve(strDeviceSerial);
	if (theDevRef == nullptr)//找不到指定设备
		return EASY_ERROR_DEVICE_NOT_FOUND;

	OSRefReleaserEx releaser(deviceMap, strDeviceSerial);
	//走到这说明存在指定设备
	HTTPSession* pDevSession = static_cast<HTTPSession *>(theDevRef->GetObjectPtr());//获得当前设备回话

	string errNo, errString;
	if (strCmd == "SENDDATA")
	{
		if (!pDevSession->GetTalkbackSession().empty() && pDevSession->GetTalkbackSession() == fSessionID)
		{
			EasyProtocolACK reqreq(MSG_SD_CONTROL_TALKBACK_REQ);
			EasyJsonValue headerheader, bodybody;

			headerheader[EASY_TAG_CSEQ] = EasyUtil::ToString(pDevSession->GetCSeq());//注意这个地方不能直接将UINT32->int,因为会造成数据失真
			headerheader[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;

			bodybody[EASY_TAG_SERIAL] = strDeviceSerial;
			bodybody[EASY_TAG_CHANNEL] = strChannel;
			bodybody[EASY_TAG_PROTOCOL] = strProtocol;
			bodybody[EASY_TAG_RESERVE] = strReserve;
			bodybody[EASY_TAG_CMD] = strCmd;
			bodybody[EASY_TAG_AUDIO_TYPE] = strAudioType;
			bodybody[EASY_TAG_AUDIO_DATA] = strAudioData;
			bodybody[EASY_TAG_PTS] = strPts;
			bodybody[EASY_TAG_FROM] = fSessionID;
			bodybody[EASY_TAG_TO] = pDevSession->GetValue(EasyHTTPSessionID)->GetAsCString();
			bodybody[EASY_TAG_VIA] = QTSServerInterface::GetServer()->GetCloudServiceNodeID();

			reqreq.SetHead(headerheader);
			reqreq.SetBody(bodybody);

			string buffer = reqreq.GetMsg();
			StrPtrLen theValue(const_cast<char*>(buffer.c_str()), buffer.size());
			pDevSession->SendHTTPPacket(&theValue, false, false);

			errNo = EASY_ERROR_SUCCESS_OK;
			errString = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);
		}
		else
		{
			errNo = EASY_ERROR_CLIENT_BAD_REQUEST;
			errString = EasyProtocol::GetErrorString(EASY_ERROR_CLIENT_BAD_REQUEST);
			goto ACK;
		}
	}
	else
	{
		if (strCmd == "START")
		{
			if (pDevSession->GetTalkbackSession().empty())
			{
				pDevSession->SetTalkbackSession(fSessionID);
				errNo = EASY_ERROR_SUCCESS_OK;
				errString = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);
			}
			else if (pDevSession->GetTalkbackSession() == fSessionID)
			{

			}
			else
			{
				errNo = EASY_ERROR_CLIENT_BAD_REQUEST;
				errString = EasyProtocol::GetErrorString(EASY_ERROR_CLIENT_BAD_REQUEST);
				goto ACK;
			}
		}
		else if (strCmd == "STOP")
		{
			if (pDevSession->GetTalkbackSession().empty() || pDevSession->GetTalkbackSession() != fSessionID)
			{
				errNo = EASY_ERROR_CLIENT_BAD_REQUEST;
				errString = EasyProtocol::GetErrorString(EASY_ERROR_CLIENT_BAD_REQUEST);
				goto ACK;
			}
			else
			{
				pDevSession->SetTalkbackSession("");
				errNo = EASY_ERROR_SUCCESS_OK;
				errString = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);
			}
		}

		EasyProtocolACK reqreq(MSG_SD_CONTROL_TALKBACK_REQ);
		EasyJsonValue headerheader, bodybody;

		headerheader[EASY_TAG_CSEQ] = EasyUtil::ToString(pDevSession->GetCSeq());//注意这个地方不能直接将UINT32->int,因为会造成数据失真
		headerheader[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;

		bodybody[EASY_TAG_SERIAL] = strDeviceSerial;
		bodybody[EASY_TAG_CHANNEL] = strChannel;
		bodybody[EASY_TAG_PROTOCOL] = strProtocol;
		bodybody[EASY_TAG_RESERVE] = strReserve;
		bodybody[EASY_TAG_CMD] = strCmd;
		bodybody[EASY_TAG_AUDIO_TYPE] = strAudioType;
		bodybody[EASY_TAG_AUDIO_DATA] = strAudioData;
		bodybody[EASY_TAG_PTS] = strPts;
		bodybody[EASY_TAG_FROM] = fSessionID;
		bodybody[EASY_TAG_TO] = pDevSession->GetValue(EasyHTTPSessionID)->GetAsCString();
		bodybody[EASY_TAG_VIA] = QTSServerInterface::GetServer()->GetCloudServiceNodeID();

		reqreq.SetHead(headerheader);
		reqreq.SetBody(bodybody);

		string buffer = reqreq.GetMsg();
		StrPtrLen theValue(const_cast<char*>(buffer.c_str()), buffer.size());
		pDevSession->SendHTTPPacket(&theValue, false, false);
	}

ACK:

	EasyProtocolACK rsp(MSG_SC_TALKBACK_CONTROL_ACK);
	EasyJsonValue header, body;
	body[EASY_TAG_SERIAL] = strDeviceSerial;
	body[EASY_TAG_CHANNEL] = strChannel;
	body[EASY_TAG_PROTOCOL] = strProtocol;
	body[EASY_TAG_RESERVE] = strReserve;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = EasyUtil::ToString(pDevSession->GetCSeq());
	header[EASY_TAG_ERROR_NUM] = errNo;
	header[EASY_TAG_ERROR_STRING] = errString;

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();
	StrPtrLen theValueAck(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValueAck, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMSGDSTalkbackControlAck(const char* json)
{
	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSGetBaseConfigReqRESTful(const char* queryString)
{
	//if (!fAuthenticated)//没有进行认证请求
	//	return httpUnAuthorized;

	EasyProtocolACK rsp(MSG_SC_SERVER_BASE_CONFIG_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	body[EASY_TAG_CONFIG_SERVICE_WAN_IP] = QTSServerInterface::GetServer()->GetPrefs()->GetServiceWANIP();
	body[EASY_TAG_CONFIG_SERVICE_LAN_PORT] = to_string(QTSServerInterface::GetServer()->GetPrefs()->GetServiceLANPort());
	body[EASY_TAG_CONFIG_SERVICE_WAN_PORT] = to_string(QTSServerInterface::GetServer()->GetPrefs()->GetServiceWANPort());
	body[EASY_TAG_CONFIG_SNAP_LOCAL_PATH] = QTSServerInterface::GetServer()->GetPrefs()->GetSnapLocalPath();
	body[EASY_TAG_CONFIG_SNAP_WEB_PATH] = QTSServerInterface::GetServer()->GetPrefs()->GetSnapWebPath();

	rsp.SetHead(header);
	rsp.SetBody(body);

	string msg = rsp.GetMsg();
	StrPtrLen theValue(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValue, false, false);

	return QTSS_NoErr;
}

QTSS_Error HTTPSession::execNetMsgCSSetBaseConfigReqRESTful(const char* queryString)
{
	//if (!fAuthenticated)//没有进行认证请求
	//	return httpUnAuthorized;

	string queryTemp;
	if (queryString)
	{
		queryTemp = EasyUtil::Urldecode(queryString);
	}
	QueryParamList parList(const_cast<char*>(queryTemp.c_str()));

	const char* chWanIP = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_SERVICE_WAN_IP);
	if (chWanIP)
	{
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), qtssPrefsServiceWANIPAddr, 0, (void*)chWanIP, strlen(chWanIP));
	}

	const char* chHTTPLanPort = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_SERVICE_LAN_PORT);
	if (chHTTPLanPort)
	{
		UInt16 uHTTPLanPort = stoi(chHTTPLanPort);
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), qtssPrefsServiceLANPort, 0, &uHTTPLanPort, sizeof(uHTTPLanPort));
	}

	const char*	chHTTPWanPort = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_SERVICE_WAN_PORT);
	if (chHTTPWanPort)
	{
		UInt16 uHTTPWanPort = stoi(chHTTPWanPort);
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), qtssPrefsServiceWANPort, 0, &uHTTPWanPort, sizeof(uHTTPWanPort));
	}

	const char* chSnapLocalPath = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_SNAP_LOCAL_PATH);
	if (chSnapLocalPath)
	{
		string snapLocalPath(chSnapLocalPath);
		if (snapLocalPath.back() != '\\')
		{
			snapLocalPath.push_back('\\');
		}
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), qtssPrefsSnapLocalPath, 0, (void*)snapLocalPath.c_str(), snapLocalPath.size());
	}

	const char* chSnapWebPath = parList.DoFindCGIValueForParam(EASY_TAG_CONFIG_SNAP_WEB_PATH);
	if (chSnapWebPath)
	{
		string snapWebPath(chSnapWebPath);
		if (snapWebPath.back() != '\/')
		{
			snapWebPath.push_back('\/');
		}
		(void)QTSS_SetValue(QTSServerInterface::GetServer()->GetPrefs(), qtssPrefsSnapWebPath, 0, (void*)snapWebPath.c_str(), snapWebPath.size());
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

QTSS_Error HTTPSession::execNetMsgCSRestartReqRESTful(const char* queryString)
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

#ifdef WIN32
	::ExitProcess(0);
#else
	exit(0);
#endif //WIN32
}

QTSS_Error HTTPSession::execNetMsgCSGetUsagesReqRESTful(const char* queryString)
{
	/*//暂时注释掉，实际上是需要认证的
	if(!fAuthenticated)//没有进行认证请求
	return httpUnAuthorized;
	*/

	EasyProtocolACK rsp(MSG_SC_SERVER_USAGES_ACK);
	EasyJsonValue header, body;

	header[EASY_TAG_VERSION] = EASY_PROTOCOL_VERSION;
	header[EASY_TAG_CSEQ] = 1;
	header[EASY_TAG_ERROR_NUM] = EASY_ERROR_SUCCESS_OK;
	header[EASY_TAG_ERROR_STRING] = EasyProtocol::GetErrorString(EASY_ERROR_SUCCESS_OK);

	Json::Value* proot = rsp.GetRoot();

	{
		Json::Value value;
		value[EASY_TAG_HTTP_METHOD] = EASY_TAG_HTTP_GET;
		value[EASY_TAG_ACTION] = "GetDeviceList";
		value[EASY_TAG_PARAMETER] = "";
		value[EASY_TAG_EXAMPLE] = "http://ip:port/api/v1/getdevicelist";
		value[EASY_TAG_DESCRIPTION] = "";
		(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_API].append(value);
	}

	{
		Json::Value value;
		value[EASY_TAG_HTTP_METHOD] = EASY_TAG_HTTP_GET;
		value[EASY_TAG_ACTION] = "GetDeviceInfo";
		value[EASY_TAG_PARAMETER] = "device=[Serial]";
		value[EASY_TAG_EXAMPLE] = "http://ip:port/api/v1/getdeviceinfo?device=00100100001";
		value[EASY_TAG_DESCRIPTION] = "";
		(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_API].append(value);
	}

	{
		Json::Value value;
		value[EASY_TAG_HTTP_METHOD] = EASY_TAG_HTTP_GET;
		value[EASY_TAG_ACTION] = "StartDeviceStream";
		value[EASY_TAG_PARAMETER] = "device=[Serial]&channel=[Channel]&reserve=[Reserve]";
		value[EASY_TAG_EXAMPLE] = "http://ip:port/api/v1/startdevicestream?device=001002000001&channel=1&reserve=1";
		value[EASY_TAG_DESCRIPTION] = "";
		(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_API].append(value);
	}

	{
		Json::Value value;
		value[EASY_TAG_HTTP_METHOD] = EASY_TAG_HTTP_GET;
		value[EASY_TAG_ACTION] = "StopDeviceStream";
		value[EASY_TAG_PARAMETER] = "device=[Serial]&channel=[Channel]&reserve=[Reserve]";
		value[EASY_TAG_EXAMPLE] = "http://ip:port/api/v1/stopdevicestream?device=001002000001&channel=1&reserve=1";
		value[EASY_TAG_DESCRIPTION] = "";
		(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_API].append(value);
	}

	{
		Json::Value value;
		value[EASY_TAG_HTTP_METHOD] = EASY_TAG_HTTP_GET;
		value[EASY_TAG_ACTION] = "PTZControl";
		value[EASY_TAG_PARAMETER] = "device=[Serial]&channel=[Channel]&protocol=[ONVIF/SDK]&actiontype=[Continuous/Single]&command=[Stop/Up/Down/Left/Right/Zoomin/Zoomout/Focusin/Focusout/Aperturein/Apertureout]&speed=[Speed]&reserve=[Reserve]";
		value[EASY_TAG_EXAMPLE] = "http://ip:port/api/v1/ptzcontrol?device=001002000001&channel=1&protocol=onvif&actiontype=single&command=down&speed=5&reserve=1";
		value[EASY_TAG_DESCRIPTION] = "";
		(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_API].append(value);
	}

	{
		Json::Value value;
		value[EASY_TAG_HTTP_METHOD] = EASY_TAG_HTTP_GET;
		value[EASY_TAG_ACTION] = "PresetControl";
		value[EASY_TAG_PARAMETER] = "device=[Serial]&channel=[Channel]&protocol=[ONVIF/SDK]&preset=[Preset]&command=[Goto/Set/Remove]";
		value[EASY_TAG_EXAMPLE] = "http://ip:port/api/v1/presetcontrol?device=001001000058&channel=1&command=goto&preset=1&protocol=onvif";
		value[EASY_TAG_DESCRIPTION] = "";
		(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_API].append(value);
	}

	{
		Json::Value value;
		value[EASY_TAG_HTTP_METHOD] = EASY_TAG_HTTP_POST;
		value[EASY_TAG_ACTION] = "MSG_CS_TALKBACK_CONTROL_REQ";
		value[EASY_TAG_PARAMETER] = "";
		value[EASY_TAG_EXAMPLE] = "http://ip:port";
		value[EASY_TAG_DESCRIPTION] = "";
		(*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_API].append(value);
	}

	rsp.SetHead(header);
	rsp.SetBody(body);
	string msg = rsp.GetMsg();
	StrPtrLen theValueAck(const_cast<char*>(msg.c_str()), msg.size());
	this->SendHTTPPacket(&theValueAck, false, false);

	return QTSS_NoErr;
}

bool HTTPSession::isRightChannel(const char* channel) const
{
	if (channel == nullptr)
	{
		return false;
	}

	try
	{
		int channelNum = boost::lexical_cast<unsigned int>(channel);
		if (channelNum > 64)
		{
			return false;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}
