/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
	File:       HTTPSession.cpp
	Contains:   ʵ�ֶԷ���Ԫÿһ��Session�Ự�����籨�Ĵ���
*/

#undef COMMON_UTILITIES_LIB
#include "HTTPSession.h"
#include "QTSServerInterface.h"
#include "OSArrayObjectDeleter.h"
#include "OSMemory.h"
#include "QTSSMemoryDeleter.h"
#include "QueryParamList.h"

#include "EasyProtocolDef.h"
#include "EasyProtocol.h"
#include "EasyUtil.h"

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
static StrPtrLen	sGetRTSPPushSessions("api/getrtsppushsessions");

#define	QUERY_STREAM_NAME		"name"
#define QUERY_STREAM_URL		"url"
#define QUERY_STREAM_CMD		"cmd"
#define QUERY_STREAM_TIMEOUT	"timeout"
#define QUERY_STREAM_CMD_START	"start"
#define QUERY_STREAM_CMD_STOP	"stop"

HTTPSession::HTTPSession()
	: HTTPSessionInterface(),
	fRequest(NULL),
	fReadMutex(),
	fCurrentModule(0),
	fState(kReadingFirstRequest)
{
	this->SetTaskName("HTTPSession");

	//��ȫ�ַ��������Session������һ��
	QTSServerInterface::GetServer()->AlterCurrentRTSPSessionCount(1);

	// Setup the QTSS param block, as none of these fields will change through the course of this session.
	fRoleParams.rtspRequestParams.inRTSPSession = this;
	fRoleParams.rtspRequestParams.inRTSPRequest = NULL;
	fRoleParams.rtspRequestParams.inClientSession = NULL;

	fModuleState.curModule = NULL;
	fModuleState.curTask = this;
	fModuleState.curRole = 0;
	fModuleState.globalLockRequested = false;
}

HTTPSession::~HTTPSession()
{
	char remoteAddress[20] = { 0 };
	StrPtrLen theIPAddressStr(remoteAddress, sizeof(remoteAddress));
	QTSS_GetValue(this, easyHTTPSesRemoteAddrStr, 0, (void*)theIPAddressStr.Ptr, &theIPAddressStr.Len);

	char msgStr[2048] = { 0 };
	qtss_snprintf(msgStr, sizeof(msgStr), "HTTPSession offline from ip[%s]", remoteAddress);
	QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);
	// Invoke the session closing modules
	QTSS_RoleParams theParams;
	theParams.rtspSessionClosingParams.inRTSPSession = this;

	//�Ự�Ͽ�ʱ������ģ�����һЩֹͣ�Ĺ���
	for (UInt32 x = 0; x < QTSServerInterface::GetNumModulesInRole(QTSSModule::kRTSPSessionClosingRole); x++)
		(void)QTSServerInterface::GetModule(QTSSModule::kRTSPSessionClosingRole, x)->CallDispatch(QTSS_RTSPSessionClosing_Role, &theParams);

	fLiveSession = false; //used in Clean up request to remove the RTP session.
	this->CleanupRequest();// Make sure that all our objects are deleted
	//if (fSessionType == qtssServiceSession)
	//    QTSServerInterface::GetServer()->AlterCurrentServiceSessionCount(-1);

}

/*!
	\brief �¼���HTTPSession Task���д��������Ϊ���籨�Ĵ����¼�
	\param
	\return ������ɷ���0,�Ͽ�Session����-1
	\ingroup
	\see
*/
SInt64 HTTPSession::Run()
{
	//��ȡ�¼�����
	EventFlags events = this->GetEvents();
	QTSS_Error err = QTSS_NoErr;
	// Some callbacks look for this struct in the thread object
	OSThreadDataSetter theSetter(&fModuleState, NULL);

	//��ʱ�¼�����Kill�¼��������ͷ����̣����� & ����-1
	if (events & Task::kKillEvent)
		fLiveSession = false;

	if (events & Task::kTimeoutEvent)
	{
		// Session��ʱ,�ͷ�Session 
		return -1;
	}

	//�����¼���������
	while (this->IsLiveSession())
	{
		//���Ĵ�����״̬������ʽ�����Է����δ���ͬһ����Ϣ
		switch (fState)
		{
		case kReadingFirstRequest://�״ζ�Socket���ж�ȡ
			{
				if ((err = fInputStream.ReadRequest()) == QTSS_NoErr)
				{
					//���RequestStream����QTSS_NoErr���ͱ�ʾ�Ѿ���ȡ��Ŀǰ���������������
					//���������ܹ���һ�����屨�ģ���Ҫ�����ȴ���ȡ...
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
		case kReadingRequest://��ȡ������
			{
				//��ȡ�����Ѿ��ڴ���һ�����İ�ʱ�������������籨�ĵĶ�ȡ�ʹ���
				OSMutexLocker readMutexLocker(&fReadMutex);

				//���������Ĵ洢��fInputStream��
				if ((err = fInputStream.ReadRequest()) == QTSS_NoErr)
				{
					//���RequestStream����QTSS_NoErr���ͱ�ʾ�Ѿ���ȡ��Ŀǰ���������������
					//���������ܹ���һ�����屨�ģ���Ҫ�����ȴ���ȡ...
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
		case kHaveCompleteMessage://��ȡ��������������
			{
				Assert(fInputStream.GetRequestBuffer());

				Assert(fRequest == NULL);
				//���ݾ��������Ĺ���HTTPRequest������
				fRequest = NEW HTTPRequest(&QTSServerInterface::GetServerHeader(), fInputStream.GetRequestBuffer());

				//����������Ѿ���ȡ��һ��������Request����׼����������Ĵ���ֱ����Ӧ���ķ���
				//�ڴ˹����У���Session��Socket�������κ��������ݵĶ�/д��
				fReadMutex.Lock();
				fSessionMutex.Lock();

				//��շ��ͻ�����
				fOutputStream.ResetBytesWritten();

				//�������󳬹��˻�����������Bad Request
				if (err == E2BIG)
				{
					//����HTTP���ģ�������408
					//(void)QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientBadRequest, qtssMsgRequestTooLong);
					fState = kSendingResponse;
					break;
				}
				// Check for a corrupt base64 error, return an error
				if (err == QTSS_BadArgument)
				{
					//����HTTP���ģ�������408
					//(void)QTSSModuleUtils::SendErrorResponse(fRequest, qtssClientBadRequest, qtssMsgBadBase64);
					fState = kSendingResponse;
					break;
				}

				Assert(err == QTSS_RequestArrived);
				fState = kFilteringRequest;
			}

		case kFilteringRequest:
			{
				//ˢ��Session����ʱ��
				fTimeoutTask.RefreshTimeout();

				//�������Ľ��н���
				QTSS_Error theErr = SetupRequest();
				//��SetupRequest����δ��ȡ�����������籨�ģ���Ҫ���еȴ�
				if (theErr == QTSS_WouldBlock)
				{
					this->ForceSameThread();
					fInputSocketP->RequestEvent(EV_RE);
					// We are holding mutexes, so we need to force
					// the same thread to be used for next Run()
					return 0;//����0��ʾ���¼��Ž���֪ͨ������>0��ʾ�涨�¼������Run

				}

				//ÿһ���������Ӧ�����Ƿ�����ɣ������ֱ�ӽ��лظ���Ӧ
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
				//����Ԥ�������
				//TODO:���Ĵ������
				fState = kCleaningUp;
				break;
			}

		case kProcessingRequest:
			{
				if (fOutputStream.GetBytesWritten() == 0)
				{
					// ��Ӧ���Ļ�û���γ�
					//QTSSModuleUtils::SendErrorResponse(fRequest, qtssServerInternal, qtssMsgNoModuleForRequest);
					fState = kCleaningUp;
					break;
				}

				fState = kSendingResponse;
			}
		case kSendingResponse:
			{
				//��Ӧ���ķ��ͣ�ȷ����ȫ����
				Assert(fRequest != NULL);

				//������Ӧ����
				err = fOutputStream.Flush();

				if (err == EAGAIN)
				{
					// If we get this error, we are currently flow-controlled and should
					// wait for the socket to become writeable again
					//����յ�Socket EAGAIN������ô������Ҫ��Socket�ٴο�д��ʱ���ٵ��÷���
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

				//һ������Ķ�ȡ��������Ӧ�����������ȴ���һ�����籨�ģ�
				this->CleanupRequest();
				fState = kReadingRequest;
			}
		}
	}

	//���Sessionռ�õ�������Դ
	this->CleanupRequest();

	//Session������Ϊ0������-1��ϵͳ�Ὣ��Sessionɾ��
	if (fObjectHolders == 0)
		return -1;

	//��������ߵ����Sessionʵ���Ѿ���Ч�ˣ�Ӧ�ñ�ɾ������û�У���Ϊ���������ط�������Session����
	return 0;
}

/*
	����HTTP+json���ģ������Ƿ�رյ�ǰSession
	HTTP���ֹ��죬json�����ɺ�������
*/
QTSS_Error HTTPSession::SendHTTPPacket(StrPtrLen* contentXML, Bool16 connectionClose, Bool16 decrement)
{
	//������Ӧ����(HTTPͷ)
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

	//����HTTPSession�����ü���һ
	if (fObjectHolders && decrement)
		DecrementObjectHolderCount();

	if (connectionClose)
		this->Signal(Task::kKillEvent);

	return QTSS_NoErr;
}

/*
	����HTTP������
*/
QTSS_Error HTTPSession::SetupRequest()
{
	//����������
	QTSS_Error theErr = fRequest->Parse();
	if (theErr != QTSS_NoErr)
		return QTSS_BadArgument;


	if (fRequest->GetRequestPath())
	{
		StrPtrLen theFullPath(fRequest->GetRequestPath());

		if (theFullPath.Equal(sEasyHLSModule))
		{
			return ExecNetMsgEasyHLSModuleReq(fRequest->GetQueryString(), NULL);
		}

		if (theFullPath.Equal(sGetHLSSessions))
		{
			return ExecNetMsgGetHlsSessionsReq(fRequest->GetQueryString(), NULL);
		}

		if (theFullPath.Equal(sGetRTSPPushSessions))
		{
			return ExecNetMsgGetRTSPPushSessionsReq(fRequest->GetQueryString(), NULL);
		}
	}

	//��ȡ����Content json���ݲ���

	//1����ȡjson���ֳ���
	StrPtrLen* lengthPtr = fRequest->GetHeaderValue(httpContentLengthHeader);
	StringParser theContentLenParser(lengthPtr);
	theContentLenParser.ConsumeWhitespace();
	UInt32 content_length = theContentLenParser.ConsumeInteger(NULL);

	if (content_length)
	{
		qtss_printf("HTTPSession read content-length:%d \n", content_length);
		// Check for the existence of 2 attributes in the request: a pointer to our buffer for
		// the request body, and the current offset in that buffer. If these attributes exist,
		// then we've already been here for this request. If they don't exist, add them.
		UInt32 theBufferOffset = 0;
		char* theRequestBody = NULL;
		UInt32 theLen = sizeof(theRequestBody);
		theErr = QTSS_GetValue(this, easyHTTPSesContentBody, 0, &theRequestBody, &theLen);

		if (theErr != QTSS_NoErr)
		{
			// First time we've been here for this request. Create a buffer for the content body and
			// shove it in the request.
			theRequestBody = NEW char[content_length + 1];
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
		theErr = QTSS_GetValue(this, easyHTTPSesContentBodyOffset, 0, &theBufferOffset, &theLen);

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

		////���Ĵ������������
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
		char* content = NULL;
		(void)QTSS_SetValue(this, easyHTTPSesContentBody, 0, &content, 0);

	}

	qtss_printf("get complete http msg:%s QueryString:%s \n", fRequest->GetRequestPath(), fRequest->GetQueryString());

	return QTSS_NoErr;
}

void HTTPSession::CleanupRequest()
{
	if (fRequest != NULL)
	{
		// NULL out any references to the current request
		delete fRequest;
		fRequest = NULL;
		fRoleParams.rtspRequestParams.inRTSPRequest = NULL;
		fRoleParams.rtspRequestParams.inRTSPHeaders = NULL;
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
		UInt32 maxConnections = (UInt32)maxConns + buffer;
		if (theServer->GetNumRTSPSessions() > maxConnections)
		{
			overLimit = true;
		}
	}
	return overLimit;
}


QTSS_Error HTTPSession::DumpRequestData()
{
	char theDumpBuffer[QTSS_MAX_REQUEST_BUFFER_SIZE];

	QTSS_Error theErr = QTSS_NoErr;
	while (theErr == QTSS_NoErr)
		theErr = this->Read(theDumpBuffer, QTSS_MAX_REQUEST_BUFFER_SIZE, NULL);

	return theErr;
}

QTSS_Error HTTPSession::ExecNetMsgEasyHLSModuleReq(char* queryString, char* json)
{
	QTSS_Error theErr = QTSS_NoErr;
	bool bStop = false;

	char decQueryString[QTSS_MAX_URL_LENGTH] = { 0 };
	EasyUtil::Urldecode((unsigned char*)queryString, (unsigned char*)decQueryString);

	char* hlsURL = NEW char[QTSS_MAX_URL_LENGTH];
	hlsURL[0] = '\0';
	QTSSCharArrayDeleter theHLSURLDeleter(hlsURL);
	do {

		if (strlen(decQueryString) == 0)
			break;

		StrPtrLen theQueryString(decQueryString);

		QueryParamList parList(decQueryString);

		const char* sName = parList.DoFindCGIValueForParam(QUERY_STREAM_NAME);
		if (sName == NULL)
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
			if (sURL == NULL)
			{
				theErr = QTSS_Unimplemented;
				break;
			}

			//TODO::������Ҫ��URL���й���
			//


			if (sTIMEOUT)
				iTIMEOUT = ::atoi(sTIMEOUT);

			char msgStr[2048] = { 0 };
			qtss_snprintf(msgStr, sizeof(msgStr), "HTTPSession::ExecNetMsgEasyHLSModuleReq name=%s,url=%s,time=%d", sName, sURL, iTIMEOUT);
			QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);

			Easy_StartHLSession(sName, sURL, iTIMEOUT, hlsURL);
		}
	} while (0);

	//����MSG_SC_START_HLS_ACK��Ӧ����
	EasyMsgSCStartHLSACK ack;
	ack.SetHeaderValue(EASY_TAG_VERSION, "1.0");
	if (strlen(hlsURL))
		ack.SetStreamURL(hlsURL);

	string msg = ack.GetMsg();
	StrPtrLen msgJson((char*)msg.c_str());

	//������Ӧ����(HTTPͷ)
	HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
	httpAck.CreateResponseHeader(msgJson.Len ? httpOK : httpNotImplemented);
	if (msgJson.Len)
		httpAck.AppendContentLengthHeader(msgJson.Len);

	//��Ӧ��ɺ�Ͽ�����
	httpAck.AppendConnectionCloseHeader();

	//Push MSG to OutputBuffer
	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
	strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

	RTSPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (msgJson.Len > 0)
		pOutputStream->Put(msgJson.Ptr, msgJson.Len);

	return QTSS_NoErr;
}


QTSS_Error HTTPSession::ExecNetMsgGetHlsSessionsReq(char* queryString, char* json)
{
	QTSS_Error theErr = QTSS_NoErr;

	do {
		char* msgContent = (char*)Easy_GetHLSessions();

		StrPtrLen msgJson(msgContent);

		//������Ӧ����(HTTPͷ)
		HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
		httpAck.CreateResponseHeader(msgJson.Len ? httpOK : httpNotImplemented);
		if (msgJson.Len)
			httpAck.AppendContentLengthHeader(msgJson.Len);

		//��Ӧ��ɺ�Ͽ�����
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


QTSS_Error HTTPSession::ExecNetMsgGetRTSPPushSessionsReq(char* queryString, char* json)
{
	QTSS_Error theErr = QTSS_NoErr;

	do {
		// ��ȡ��ӦContent
		char* msgContent = (char*)Easy_GetRTSPPushSessions();

		StrPtrLen msgJson(msgContent);

		// ������Ӧ����(HTTPͷ)
		HTTPRequest httpAck(&QTSServerInterface::GetServerHeader(), httpResponseType);
		httpAck.CreateResponseHeader(msgJson.Len ? httpOK : httpNotImplemented);
		if (msgJson.Len)
			httpAck.AppendContentLengthHeader(msgJson.Len);

		// ��Ӧ��ɺ�Ͽ�����
		httpAck.AppendConnectionCloseHeader();

		// HTTP��ӦBody
		char respHeader[2048] = { 0 };
		StrPtrLen* ackPtr = httpAck.GetCompleteHTTPHeader();
		strncpy(respHeader, ackPtr->Ptr, ackPtr->Len);

		// HTTP��ӦContent
		RTSPResponseStream *pOutputStream = GetOutputStream();
		pOutputStream->Put(respHeader);
		if (msgJson.Len > 0)
			pOutputStream->Put(msgJson.Ptr, msgJson.Len);

		delete[] msgContent;
	} while (0);

	return theErr;
}