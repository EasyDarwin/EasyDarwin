/*
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       HTTPSession.cpp
    Contains:   ʵ�ֶԷ���Ԫÿһ��Session�Ự�����籨�Ĵ���
*/

#include "HTTPSession.h"
#include "QTSServerInterface.h"
#include "OSMemory.h"
#include "EasyUtil.h"

#include "OSArrayObjectDeleter.h"

#if __FreeBSD__ || __hpux__	
    #include <unistd.h>
#endif

#include <errno.h>

#if __solaris__ || __linux__ || __sgi__	|| __hpux__
    #include <crypt.h>
#endif

static StrPtrLen	sServiceStr("EasyDariwn_CMS");

using namespace std;

HTTPSession::HTTPSession( )
: HTTPSessionInterface(),
  fRequest(NULL),
  fReadMutex(),
  fCurrentModule(0),
  fState(kReadingFirstRequest)
{
    this->SetTaskName("CMSSession");
    
	//��ȫ�ַ��������Session������һ��
    QTSServerInterface::GetServer()->AlterCurrentServiceSessionCount(1);

    // Setup the QTSS param block, as none of these fields will change through the course of this session.
    fRoleParams.rtspRequestParams.inRTSPSession = this;
    fRoleParams.rtspRequestParams.inRTSPRequest = NULL;
    fRoleParams.rtspRequestParams.inClientSession = NULL;
    
    fModuleState.curModule = NULL;
    fModuleState.curTask = this;
    fModuleState.curRole = 0;
    fModuleState.globalLockRequested = false;

	fDeviceSnap = NEW char[EASYDARWIN_MAX_URL_LENGTH];
	fDeviceSnap[0] = '\0';

	qtss_printf("create session:%s\n", fSessionID);
}

HTTPSession::~HTTPSession()
{
	char remoteAddress[20] = {0};
	StrPtrLen theIPAddressStr(remoteAddress,sizeof(remoteAddress));
	QTSS_GetValue(this, qtssRTSPSesRemoteAddrStr, 0, (void*)theIPAddressStr.Ptr, &theIPAddressStr.Len);

	char msgStr[2048] = { 0 };
	qtss_snprintf(msgStr, sizeof(msgStr), "session offline from ip[%s]",remoteAddress);
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

	if (fDeviceSnap != NULL)
        delete [] fDeviceSnap; 

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
    QTSSModule* theModule = NULL;
    UInt32 numModules = 0;
    // Some callbacks look for this struct in the thread object
    OSThreadDataSetter theSetter(&fModuleState, NULL);
        
    //��ʱ�¼�����Kill�¼��������ͷ����̣����� & ����-1
    if (events & Task::kKillEvent)
        fLiveSession = false;

	if(events & Task::kTimeoutEvent)
	{
		//�ͻ���Session��ʱ����ʱ������ 
		char msgStr[512];
		qtss_snprintf(msgStr, sizeof(msgStr), "session timeout,release session\n");
		QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);
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
                Assert( fInputStream.GetRequestBuffer() );
                
                Assert(fRequest == NULL);
				//���ݾ��������Ĺ���HTTPRequest������
				fRequest = NEW HTTPRequest(&sServiceStr, fInputStream.GetRequestBuffer());

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
				if(theErr == QTSS_WouldBlock)
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
	HTTPRequest httpAck(&sServiceStr);
	httpAck.CreateResponseHeader(contentXML->Len?httpOK:httpNotImplemented);
	if (contentXML->Len)
		httpAck.AppendContentLengthHeader(contentXML->Len);

	if(connectionClose)
		httpAck.AppendConnectionCloseHeader();

	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteResponseHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);
	
	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (contentXML->Len > 0) 
		pOutputStream->Put(contentXML->Ptr, contentXML->Len);

	if (pOutputStream->GetBytesWritten() != 0)
	{
		pOutputStream->Flush();
	}

	//����HTTPSession�����ü���һ
	if(fObjectHolders && decrement)
		DecrementObjectHolderCount();

	if(connectionClose)
		this->Signal(Task::kKillEvent);

	return QTSS_NoErr;
}

/*
	Content���Ķ�ȡ�����
	ͬ�����б��Ĵ�������ظ�����
*/
QTSS_Error HTTPSession::SetupRequest()
{
    //����������
    QTSS_Error theErr = fRequest->Parse();
    if (theErr != QTSS_NoErr)
        return QTSS_BadArgument;


	if(::strcmp(fRequest->GetRequestPath(),"getdevicelist") == 0)
	{
		ExecNetMsgGetDeviceListReq();
		return 0;
	}

    QTSS_RTSPStatusCode statusCode = qtssSuccessOK;
    char *body = NULL;
    UInt32 bodySizeBytes = 0;

	//��ȡ����Content json���ݲ���

	//1����ȡjson���ֳ���
	StrPtrLen* lengthPtr = fRequest->GetHeaderValue(httpContentLengthHeader);

	StringParser theContentLenParser(lengthPtr);
    theContentLenParser.ConsumeWhitespace();
    UInt32 content_length = theContentLenParser.ConsumeInteger(NULL);
       
	qtss_printf("HTTPSession read content-length:%d \n", content_length);

    if (content_length <= 0) return QTSS_BadArgument;

	   //
    // Check for the existence of 2 attributes in the request: a pointer to our buffer for
    // the request body, and the current offset in that buffer. If these attributes exist,
    // then we've already been here for this request. If they don't exist, add them.
    UInt32 theBufferOffset = 0;
    char* theRequestBody = NULL;
	 UInt32 theLen = 0;
    theLen = sizeof(theRequestBody);
    theErr = QTSS_GetValue(this, qtssEasySesContentBody, 0, &theRequestBody, &theLen);

    if (theErr != QTSS_NoErr)
    {
        // First time we've been here for this request. Create a buffer for the content body and
        // shove it in the request.
        theRequestBody = NEW char[content_length + 1];
        memset(theRequestBody,0,content_length + 1);
        theLen = sizeof(theRequestBody);
        theErr = QTSS_SetValue(this, qtssEasySesContentBody, 0, &theRequestBody, theLen);// SetValue creates an internal copy.
        Assert(theErr == QTSS_NoErr);
        
        // Also store the offset in the buffer
        theLen = sizeof(theBufferOffset);
        theErr = QTSS_SetValue(this, qtssEasySesContentBodyOffset, 0, &theBufferOffset, theLen);
        Assert(theErr == QTSS_NoErr);
    }
    
    theLen = sizeof(theBufferOffset);
    theErr = QTSS_GetValue(this, qtssEasySesContentBodyOffset, 0, &theBufferOffset, &theLen);

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
    if ((theErr == QTSS_WouldBlock) || (theLen < ( content_length - theBufferOffset)))
    {
		//
        // Update our offset in the buffer
        theBufferOffset += theLen;
        (void)QTSS_SetValue(this, qtssEasySesContentBodyOffset, 0, &theBufferOffset, sizeof(theBufferOffset));
        // The entire content body hasn't arrived yet. Request a read event and wait for it.
       
        Assert(theErr == QTSS_NoErr);
        return QTSS_WouldBlock;
    }

    Assert(theErr == QTSS_NoErr);
    
	OSCharArrayDeleter charArrayPathDeleter(theRequestBody);

	//���Ĵ������������
	EasyDarwin::Protocol::EasyProtocol protocol(theRequestBody);
	int nNetMsg = protocol.GetMessageType();

	switch (nNetMsg)
	{
		case MSG_DEV_CMS_REGISTER_REQ://�����豸������Ϣ
			ExecNetMsgDevRegisterReq(theRequestBody);
			break;
		case MSG_NGX_CMS_NEED_STREAM_REQ:
			ExecNetMsgNgxStreamReq(theRequestBody);
			break;
		case MSG_CMS_DEV_STREAM_START_ACK:
			break;
		case MSG_CMS_DEV_STREAM_STOP_ACK:
			break;
		case MSG_CLI_CMS_DEVICE_LIST_REQ:
			break;
		case MSG_DEV_CMS_SNAP_UPDATE_REQ:
			ExecNetMsgSnapUpdateReq(theRequestBody);
		default:
			ExecNetMsgDefaultReqHandler(theRequestBody);
			break;
	}
	
	UInt32 offset = 0;
	(void)QTSS_SetValue(this, qtssEasySesContentBodyOffset, 0, &offset, sizeof(offset));
	char* content = NULL;
	(void)QTSS_SetValue(this, qtssEasySesContentBody, 0, &content, 0);

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

//
//MSG_DEV_CMS_REGISTER_REQ��Ϣ����
//
QTSS_Error HTTPSession::ExecNetMsgDevRegisterReq(const char* json)
{	
	QTSS_Error theErr = QTSS_NoErr;		

	EasyDarwin::Protocol::EasyDarwinRegisterReq req(json);
	do
	{
		if(fAuthenticated)
		{
			break;
		}

		//��ȡ�豸SN���к�
		std::string strDeviceSN = req.GetSerialNumber();
		std::string strDevicePWD = req.GetAuthCode();

		//����Դ�����û�н���Response�ظ���ʵ������Ҫ���д����
		if(strDeviceSN.length()<=0) return QTSS_BadArgument;

		//TODO:���豸SN�����������֤
		/*	˵��:��Դ��Ŀ�����豸�����кź����������֤���û����Ը����Լ�������,
			��������н���ȡ�����豸���кź��������û����豸���ݿ����жԱ���֤,
			������ȷ���������¼�����������󷵻�ʧ�ܣ��Ͽ����ӣ�
		*/
		printf("MSG_DEV_CMS_REGISTER_REQ(%s:%s)\n", strDeviceSN.c_str(), strDevicePWD.c_str());
		//if(myAuthenticFail)
		//{
		//	theErr = httpUnAuthorized;
		//	break;
		//}

		//���豸�б�ά��
		qtss_sprintf(fSerial,"%s", strDeviceSN.c_str());
		//����Session����
		fSessionType = qtssDeviceSession;
		QTSS_SetValue(this, qtssEasySesSerial, 0, fSerial, strlen(fSerial));
		//Device OSRef
		fDevSerialPtr.Set( fSerial, ::strlen(fSerial));
		fDevRef.Set( fDevSerialPtr, this);
		//ȫ��ά���豸�б�
		OS_Error theErr = QTSServerInterface::GetServer()->GetDeviceSessionMap()->Register(GetRef());
		if(theErr == OS_NoErr)
		{
			//��֤��Ȩ��ʶ,��ǰSession�Ͳ���Ҫ�ٽ�����֤������
			fAuthenticated = true;
		}
		else
		{
			//���߳�ͻ
			theErr =  QTSS_AttrNameExists;
			break;
		}
	}while(0);

	if(theErr != QTSS_NoErr) return theErr;

	//����MSG_DEV_CMS_REGISTER_RSP��Ӧ������HTTP Content����
	EasyDarwin::Protocol::EasyDarwinRegisterAck rsp;
	rsp.SetHeaderValue(EASYDARWIN_TAG_VERSION, EASYDARWIN_PROTOCOL_VERSION);
	rsp.SetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(EASYDARWIN_TERMINAL_TYPE_CAMERA).c_str());
	rsp.SetHeaderValue(EASYDARWIN_TAG_CSEQ, req.GetHeaderValue(EASYDARWIN_TAG_CSEQ).c_str());	
	rsp.SetHeaderValue(EASYDARWIN_TAG_SESSION_ID, fSessionID);
	rsp.SetHeaderValue(EASYDARWIN_TAG_ERROR_NUM, "200");//EASYDARWIN_ERROR_SUCCESS_OK
	rsp.SetHeaderValue(EASYDARWIN_TAG_ERROR_STRING, EasyProtocol::GetErrorString(EASYDARWIN_ERROR_SUCCESS_OK).c_str());
	string msg = rsp.GetMsg();

	StrPtrLen jsonRSP((char*)msg.c_str());

	//������Ӧ����(HTTP Header)
	HTTPRequest httpAck(&sServiceStr);
	httpAck.CreateResponseHeader(jsonRSP.Len?httpOK:httpNotImplemented);
	if (jsonRSP.Len)
		httpAck.AppendContentLengthHeader(jsonRSP.Len);

	//�ж��Ƿ���Ҫ�رյ�ǰSession����
	//if(connectionClose)
	//	httpAck.AppendConnectionCloseHeader();

	//Push HTTP Header to OutputBuffer
	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteResponseHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);
	
	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	
	//Push HTTP Content to OutputBuffer
	if (jsonRSP.Len > 0) 
		pOutputStream->Put(jsonRSP.Ptr, jsonRSP.Len);
	
	return QTSS_NoErr;
}

//
//MSG_NGX_CMS_NEED_STREAM_REQ��Ϣ����
//
QTSS_Error HTTPSession::ExecNetMsgNgxStreamReq(const char* json)
{
	QTSS_Error theErr = QTSS_NoErr;

	//MSG_NGX_CMS_NEED_STREAM_REQ�����豸ֱ����Ϣ����
	EasyDarwin::Protocol::EasyProtocol req(json);

	//��ȡ�豸SN���к�
	std::string strDeviceSN = req.GetBodyValue(EASYDARWIN_TAG_DEVICE_SERIAL);

	//����Դ�����û�н���Response�ظ���ʵ������Ҫ���д����
	if(strDeviceSN.length()<=0) return QTSS_BadArgument;

	printf("MSG_NGX_CMS_NEED_STREAM_REQ(%s)\n", strDeviceSN.c_str());

	OSRefTable* devMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	OSRef* theDevRef = NULL;

	do
	{
		//���豸�б�MAP�в����豸
		char strSerial[EASY_MAX_SERIAL_LENGTH] = { 0 };
		qtss_sprintf(strSerial,"%s",strDeviceSN.c_str());

		StrPtrLen devSerialPtr(strSerial);

		// Device RefCount++
		theDevRef = devMap->Resolve(&devSerialPtr);

		if (theDevRef == NULL)
		{
			//δ���ҵ��豸������404
			theErr = QTSS_ValueNotFound;
			break;
		}

		//����MSG_CMS_DEV_STREAM_START_REQ�����豸������Ϣ
		EasyDarwin::Protocol::EasyProtocol streamingREQ(MSG_CMS_DEV_STREAM_START_REQ);
		//Header
		streamingREQ.SetHeaderValue(EASYDARWIN_TAG_VERSION, EASYDARWIN_PROTOCOL_VERSION);
		streamingREQ.SetHeaderValue(EASYDARWIN_TAG_CSEQ, "1");	
		//Body
		streamingREQ.SetBodyValue(EASYDARWIN_TAG_STREAM_ID, EASYDARWIN_PROTOCOL_STREAM_MAIN);
		streamingREQ.SetBodyValue(EASYDARWIN_TAG_PROTOCOL, "RTSP");
		streamingREQ.SetBodyValue("DssIP","www.easydarwin.org");
		streamingREQ.SetBodyValue("DssPort","554");
	
		string buffer = streamingREQ.GetMsg();
		//����MSG_CMS_DEV_STREAM_START_REQ���ĵ��豸
		QTSS_SendHTTPPacket(theDevRef->GetObject(), (char*)buffer.c_str(), ::strlen(buffer.c_str()), false, false);

	}while(0);
	
	//Device RefCount--
	if(theDevRef)
		devMap->Release(theDevRef);

	//����MSG_NGX_CMS_NEED_STREAM_RSP����ֱ����Ϣ��Ӧ����
	EasyDarwin::Protocol::EasyProtocol rsp(MSG_NGX_CMS_NEED_STREAM_ACK);
	rsp.SetHeaderValue(EASYDARWIN_TAG_VERSION, EASYDARWIN_PROTOCOL_VERSION);
	rsp.SetHeaderValue(EASYDARWIN_TAG_ERROR_NUM, "200");//EASYDARWIN_ERROR_SUCCESS_OK
	rsp.SetHeaderValue(EASYDARWIN_TAG_ERROR_STRING, EasyProtocol::GetErrorString(EASYDARWIN_ERROR_SUCCESS_OK).c_str());
	rsp.SetBodyValue("DssIP","www.easydarwin.org");
	rsp.SetBodyValue("DssPort","554");

	string msg = rsp.GetMsg();

	StrPtrLen msgJson((char*)msg.c_str());

	//������Ӧ����(HTTPͷ)
	HTTPRequest httpAck(&sServiceStr);
	httpAck.CreateResponseHeader(msgJson.Len?httpOK:httpNotImplemented);
	if (msgJson.Len)
		httpAck.AppendContentLengthHeader(msgJson.Len);

	//��Ӧ��ɺ�Ͽ�����
	httpAck.AppendConnectionCloseHeader();

	//Push MSG to OutputBuffer
	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteResponseHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);
	
	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (msgJson.Len > 0) 
		pOutputStream->Put(msgJson.Ptr, msgJson.Len);
	
	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgDefaultReqHandler(const char* json)
{
	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgSnapUpdateReq(const char* json)
{
	QTSS_Error theErr = QTSS_NoErr;
	bool close = false;
	EasyDarwinDeviceSnapUpdateReq parse(json);
	
	string image;
	parse.GetImageData(image);

	const char* inSnapJpg = image.c_str();

	if(!fAuthenticated) return QTSS_NoErr;

	//�ȶ����ݽ���Base64����
	int base64DataLen = Base64decode_len(inSnapJpg);
	char* snapDecodedBuffer = (char*)malloc(base64DataLen);
	::memset(snapDecodedBuffer, 0, base64DataLen);
	//�����ݽ���Base64����
	int jpgLen = ::Base64decode(snapDecodedBuffer, inSnapJpg); 

	char jpgDir[512] = { 0 };
	qtss_sprintf(jpgDir,"%s%s", QTSServerInterface::GetServer()->GetPrefs()->GetSnapLocalPath() ,fSerial);
	OS::RecursiveMakeDir(jpgDir);

	char jpgPath[512] = { 0 };
	string jpgFileName = EasyUtil::GetUUID();
	qtss_sprintf(jpgPath,"%s/%s.jpg", jpgDir, jpgFileName.c_str());

	FILE* fSnap = ::fopen(jpgPath, "wb");
	fwrite(snapDecodedBuffer, 1, jpgLen, fSnap);
	::fclose(fSnap);
	free(snapDecodedBuffer);

	qtss_sprintf(fDeviceSnap, "%s%s/%s.jpg",QTSServerInterface::GetServer()->GetPrefs()->GetSnapWebPath(), fSerial, jpgFileName.c_str());
	return QTSS_NoErr;
}

QTSS_Error HTTPSession::ExecNetMsgGetDeviceListReq()
{
	QTSS_Error theErr = QTSS_NoErr;

	printf("Get Device List! \n");

	OSRefTable* devMap = QTSServerInterface::GetServer()->GetDeviceSessionMap();
	//if(devMap->GetNumRefsInTable() == 0 )
	//	return QTSS_NoErr;

	EasyDarwinDeviceListAck req;
	req.SetHeaderValue(EASYDARWIN_TAG_VERSION, "1.0");
	req.SetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(EASYDARWIN_TERMINAL_TYPE_CAMERA).c_str());
	req.SetHeaderValue(EASYDARWIN_TAG_CSEQ, "1");	
	char count[16] = { 0 };
	sprintf(count,"%d", devMap->GetNumRefsInTable());
	req.SetBodyValue("DeviceCount", count );

	OSRef* theDevRef = NULL;

	OSMutexLocker locker(devMap->GetMutex());
    for (OSRefHashTableIter theIter(devMap->GetHashTable()); !theIter.IsDone(); theIter.Next())
    {
        OSRef* theRef = theIter.GetCurrent();
        HTTPSession* theSession = (HTTPSession*)theRef->GetObject();

		EasyDarwinDevice device;
		device.DeviceName = string(theSession->GetDeviceSerial());
		device.DeviceSerial = string(theSession->GetDeviceSerial());
		device.DeviceSnap = string(theSession->GetDeviceSnap());
		req.AddDevice(device);
    }   

	string msg = req.GetMsg();

	printf(msg.c_str());

	StrPtrLen msgJson((char*)msg.c_str());

	//������Ӧ����(HTTPͷ)
	HTTPRequest httpAck(&sServiceStr);
	httpAck.CreateResponseHeader(msgJson.Len?httpOK:httpNotImplemented);
	if (msgJson.Len)
		httpAck.AppendContentLengthHeader(msgJson.Len);

	//֧�ֿ���
	StrPtrLen allowOrigin("*");
	httpAck.AppendResponseHeader(httpAccessControlAllowOriginHeader, &allowOrigin);

	//��Ӧ��ɺ�Ͽ�����
	httpAck.AppendConnectionCloseHeader();

	//Push MSG to OutputBuffer
	char respHeader[2048] = { 0 };
	StrPtrLen* ackPtr = httpAck.GetCompleteResponseHeader();
	strncpy(respHeader,ackPtr->Ptr, ackPtr->Len);
	
	HTTPResponseStream *pOutputStream = GetOutputStream();
	pOutputStream->Put(respHeader);
	if (msgJson.Len > 0) 
		pOutputStream->Put(msgJson.Ptr, msgJson.Len);
	
	return QTSS_NoErr;
}