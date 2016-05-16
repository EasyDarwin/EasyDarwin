/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       HTTPSessionInterface.cpp
    Contains:   Implementation of HTTPSessionInterface object.
*/

#include "atomic.h"
#include "HTTPSessionInterface.h"
#include "QTSServerInterface.h"
#include "OSMemory.h"
#include <errno.h>
#include "EasyUtil.h"



#if DEBUG
	#define RTSP_SESSION_INTERFACE_DEBUGGING 1
#else
    #define RTSP_SESSION_INTERFACE_DEBUGGING 0
#endif

unsigned int            HTTPSessionInterface::sSessionIndexCounter = kFirstCMSSessionID;

QTSSAttrInfoDict::AttrInfo  HTTPSessionInterface::sAttributes[] = 
{   
	/*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0 */ { "EasyHTTPSesIndex",            NULL,          qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 1 */ { "EasyHTTPSesLocalAddr",       SetupParams,    qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
    /* 2 */ { "EasyHTTPSesLocalAddrStr",    SetupParams,    qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
    /* 3 */ { "EasyHTTPSesLocalDNS",        SetupParams,    qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
    /* 4 */ { "EasyHTTPSesRemoteAddr",      SetupParams,    qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
    /* 5 */ { "EasyHTTPSesRemoteAddrStr",   SetupParams,    qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
    /* 6 */ { "EasyHTTPSesEventCntxt",      NULL,           qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 7 */ { "EasyHTTPSesType",            NULL,           qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite },
    /* 8 */ { "EasyHTTPSesSerial",			NULL,           qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite },
    
    /* 9 */ { "qtssRTSPSesLastUserName",    NULL,           qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe  },
    /* 10 */{ "qtssRTSPSesLastUserPassword",NULL,           qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe  },
    /* 11 */{ "EasyHTTPSessionID",			NULL,			qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe  },
    
    /* 12 */{ "EasyHTTPSesLocalPort",       SetupParams,    qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
    /* 13 */{ "EasyHTTPSesRemotePort",      SetupParams,    qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
    
	/* 14 */{ "EasyHTTPSesContentBody",		NULL,           qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite },
	/* 15 */{ "EasyHTTPSesContentBodyOffset",NULL,          qtssAttrDataTypeUInt32,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite }
};


void    HTTPSessionInterface::Initialize()
{
    for (UInt32 x = 0; x < EasyHTTPSesNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kHTTPSessionDictIndex)->
            SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr, sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);
}

HTTPSessionInterface::HTTPSessionInterface()
	: QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kHTTPSessionDictIndex)),
	Task(),
	fTimeoutTask(NULL, QTSServerInterface::GetServer()->GetPrefs()->GetSessionTimeoutInSecs() * 1000),
	fInputStream(&fSocket),
	fOutputStream(&fSocket, &fTimeoutTask),
	fSessionMutex(),
	fSocket(NULL, Socket::kNonBlockingSocketType),
	fOutputSocketP(&fSocket),
	fInputSocketP(&fSocket),
	fSessionType(EasyHTTPSession),
	fTerminalType(0),
	fLiveSession(true),
	fObjectHolders(0),
	fRequestBodyLen(-1),
	fAuthenticated(false),
	fRequestBody(NULL),//add
	fCSeq(1)//add
{
    fTimeoutTask.SetTask(this);
    fSocket.SetTask(this);

	//fSerial[0] = 0;
	//::memset(fSerial, 0, EASY_MAX_SERIAL_LENGTH);
	//fDevSerialPtr.Set( fSerial, ::strlen(fSerial));
	//fDevRef.Set( fDevSerialPtr, this);

    fSessionIndex = (UInt32)atomic_add(&sSessionIndexCounter, 1);
    this->SetVal(EasyHTTPSesIndex, &fSessionIndex, sizeof(fSessionIndex));

    this->SetVal(EasyHTTPSesEventCntxt, &fOutputSocketP, sizeof(fOutputSocketP));
    this->SetVal(EasyHTTPSesType, &fSessionType, sizeof(fSessionType));
	//this->SetEmptyVal(EasyHTTPSesSerial, &fSerial[0], EASY_MAX_SERIAL_LENGTH);

	qtss_sprintf(fSessionID, "%s", EasyUtil::GetUUID().c_str());
	this->SetValue(EasyHTTPSessionID, 0, fSessionID, ::strlen(fSessionID), QTSSDictionary::kDontObeyReadOnly);
    
    fInputStream.ShowMSG(QTSServerInterface::GetServer()->GetPrefs()->GetMSGDebugPrintfs());
    fOutputStream.ShowMSG(QTSServerInterface::GetServer()->GetPrefs()->GetMSGDebugPrintfs());

	//add
	fInfo.cWaitingState=0;	//初始为处理第一次请求状态
	fInfo.uWaitingTime=0;	//初始为不用等待回应
	/*
		HTTPSessionInterface * p=this;
		int iSize=sizeof(QTSSDictionary)+sizeof(Task)+sizeof(fUserNameBuf)+sizeof(fUserPasswordBuf)+sizeof(fSessionID)+sizeof(fLastSMSSessionID)
			+sizeof(fDevSerial)+sizeof(fStreamReqCount)+sizeof(fNVROperatorMutex)+sizeof(fStreamReqCountMutex)+sizeof(fCond)+sizeof(fNVRMessageQueue)
			+sizeof(fTimeoutTask)+sizeof(fInputStream)+sizeof(fOutputStream)+sizeof(fSessionMutex)+sizeof(fSocket)+sizeof(fOutputSocketP)+sizeof(fInputSocketP)
			+sizeof(fSessionType)+sizeof(fLiveSession)+sizeof(fObjectHolders)+sizeof(fSessionIndex)+sizeof(fLocalAddr)+sizeof(fRemoteAddr)+sizeof(fAuthenticated)
			+sizeof(sSessionIndexCounter)+sizeof(sAttributes)+sizeof(fDevice)+sizeof(fRequestBody)+sizeof(fMutexCSeq)+sizeof(fCSeq)+sizeof(fMsgMap)+sizeof(fin);
		iSize=sizeof(HTTPSessionInterface);
	*/
}

HTTPSessionInterface::~HTTPSessionInterface()
{
    // If the input socket is != output socket, the input socket was created dynamically
    if (fInputSocketP != fOutputSocketP) 
        delete fInputSocketP;
	
	char remoteAddress[20] = {0};
	StrPtrLen theIPAddressStr(remoteAddress,sizeof(remoteAddress));
	QTSS_GetValue(this, EasyHTTPSesRemoteAddrStr, 0, (void*)theIPAddressStr.Ptr, &theIPAddressStr.Len);
	char msgStr[2048] = { 0 };
	
	//客户端连接断开时，进行自动停止推流处理，放到对fSessionType类型判断里面更好
	AutoStopStreamJudge();
	//客户端连接断开时，进行自动停止推流处理
	switch(fSessionType)
	{
	case EasyCameraSession:
		this->UnRegDevSession();
		this->ReleaseMsgMap();
		qtss_snprintf(msgStr, sizeof(msgStr), "EasyCameraSession offline from ip[%s], device_serial[%s]",remoteAddress, fDevice.serial_.c_str());
		break;
	case EasyNVRSession:
		qtss_snprintf(msgStr, sizeof(msgStr), "EasyNVRSession offline from ip[%s]",remoteAddress);
		break;
	case EasyHTTPSession:
		qtss_snprintf(msgStr, sizeof(msgStr), "EasyHTTPSession offline from ip[%s]",remoteAddress);
		break;
	default:
		qtss_snprintf(msgStr, sizeof(msgStr), "Unknown session offline from ip[%s]",remoteAddress);
		break;
	}
	QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);
}

void HTTPSessionInterface::DecrementObjectHolderCount()
{

//#if __Win32__
//maybe don't need this special case but for now on Win32 we do it the old way since the killEvent code hasn't been verified on Windows.
    this->Signal(Task::kReadEvent);//have the object wakeup in case it can go away.
    atomic_sub(&fObjectHolders, 1);
//#else
//    if (0 == atomic_sub(&fObjectHolders, 1))
//        this->Signal(Task::kKillEvent);
//#endif

}

QTSS_Error HTTPSessionInterface::Write(void* inBuffer, UInt32 inLength,
                                            UInt32* outLenWritten, UInt32 inFlags)
{
    UInt32 sendType = HTTPResponseStream::kDontBuffer;
    if ((inFlags & qtssWriteFlagsBufferData) != 0)
        sendType = HTTPResponseStream::kAlwaysBuffer;
    
    iovec theVec[2];
    theVec[1].iov_base = (char*)inBuffer;
    theVec[1].iov_len = inLength;
    return fOutputStream.WriteV(theVec, 2, inLength, outLenWritten, sendType);
}

QTSS_Error HTTPSessionInterface::WriteV(iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength, UInt32* outLenWritten)
{
    return fOutputStream.WriteV(inVec, inNumVectors, inTotalLength, outLenWritten, HTTPResponseStream::kDontBuffer);
}

QTSS_Error HTTPSessionInterface::Read(void* ioBuffer, UInt32 inLength, UInt32* outLenRead)
{
    //
    // Don't let callers of this function accidently creep past the end of the
    // request body.  If the request body size isn't known, fRequestBodyLen will be -1
    
    if (fRequestBodyLen == 0)
        return QTSS_NoMoreData;
        
    if ((fRequestBodyLen > 0) && ((SInt32)inLength > fRequestBodyLen))
        inLength = fRequestBodyLen;
    
    UInt32 theLenRead = 0;
    QTSS_Error theErr = fInputStream.Read(ioBuffer, inLength, &theLenRead);
    
    if (fRequestBodyLen >= 0)
        fRequestBodyLen -= theLenRead;

    if (outLenRead != NULL)
        *outLenRead = theLenRead;
    
    return theErr;
}

QTSS_Error HTTPSessionInterface::RequestEvent(QTSS_EventType inEventMask)
{
    if (inEventMask & QTSS_ReadableEvent)
        fInputSocketP->RequestEvent(EV_RE);
    if (inEventMask & QTSS_WriteableEvent)
        fOutputSocketP->RequestEvent(EV_WR);
        
    return QTSS_NoErr;
}

/*
    take the TCP socket away from a RTSP session that's
    waiting to be snarfed.
    
*/

void    HTTPSessionInterface::SnarfInputSocket( HTTPSessionInterface* fromRTSPSession )
{
    Assert( fromRTSPSession != NULL );
    Assert( fromRTSPSession->fOutputSocketP != NULL );
    
    fInputStream.SnarfRetreat( fromRTSPSession->fInputStream );

    if (fInputSocketP == fOutputSocketP)
        fInputSocketP = NEW TCPSocket( this, Socket::kNonBlockingSocketType );
    else
        fInputSocketP->Cleanup();   // if this is a socket replacing an old socket, we need
                                    // to make sure the file descriptor gets closed
    fInputSocketP->SnarfSocket( fromRTSPSession->fSocket );
    
    // fInputStream, meet your new input socket
    fInputStream.AttachToSocket( fInputSocketP );
}


void* HTTPSessionInterface::SetupParams(QTSSDictionary* inSession, UInt32* /*outLen*/)
{
    HTTPSessionInterface* theSession = (HTTPSessionInterface*)inSession;
 
    theSession->fLocalAddr = theSession->fSocket.GetLocalAddr();
    theSession->fRemoteAddr = theSession->fSocket.GetRemoteAddr();
    
    theSession->fLocalPort = theSession->fSocket.GetLocalPort();
    theSession->fRemotePort = theSession->fSocket.GetRemotePort();
    
    StrPtrLen* theLocalAddrStr = theSession->fSocket.GetLocalAddrStr();
    StrPtrLen* theLocalDNSStr = theSession->fSocket.GetLocalDNSStr();
    StrPtrLen* theRemoteAddrStr = theSession->fSocket.GetRemoteAddrStr();
    if (theLocalAddrStr == NULL || theLocalDNSStr == NULL || theRemoteAddrStr == NULL)
    {    //the socket is bad most likely values are all 0. If the socket had an error we shouldn't even be here.
         //theLocalDNSStr is set to localAddr if it is unavailable, so it should be present at this point as well.
         Assert(0);   //for debugging
         return NULL; //nothing to set
    }
    theSession->SetVal(EasyHTTPSesLocalAddr, &theSession->fLocalAddr, sizeof(theSession->fLocalAddr));
    theSession->SetVal(EasyHTTPSesLocalAddrStr, theLocalAddrStr->Ptr, theLocalAddrStr->Len);
    theSession->SetVal(EasyHTTPSesLocalDNS, theLocalDNSStr->Ptr, theLocalDNSStr->Len);
    theSession->SetVal(EasyHTTPSesRemoteAddr, &theSession->fRemoteAddr, sizeof(theSession->fRemoteAddr));
    theSession->SetVal(EasyHTTPSesRemoteAddrStr, theRemoteAddrStr->Ptr, theRemoteAddrStr->Len);
    
    theSession->SetVal(EasyHTTPSesLocalPort, &theSession->fLocalPort, sizeof(theSession->fLocalPort));
    theSession->SetVal(EasyHTTPSesRemotePort, &theSession->fRemotePort, sizeof(theSession->fRemotePort));
    return NULL;
}



QTSS_Error HTTPSessionInterface::SendHTTPPacket(StrPtrLen* contentXML, Bool16 connectionClose, Bool16 decrement)
{
	return QTSS_NoErr;
}

QTSS_Error HTTPSessionInterface::RegDevSession(const char* serial, UInt32 serailLen)
{
	//if((::strlen(serial) == 0) || (serailLen == 0))
	//	return QTSS_ValueNotFound;
	//fSessionType = EasyCameraSession;
	//QTSS_SetValue(this, EasyHTTPSesSerial, 0, serial, serailLen);

	//fDevSerialPtr.Set( fSerial, serailLen);
	//fDevRef.Set( fDevSerialPtr, this);
	//OS_Error theErr = QTSServerInterface::GetServer()->GetDeviceSessionMap()->Register(GetRef());
	////printf("[line:%d]HTTPSessionInterface::RegDevSession theErr = %d\n",__LINE__, theErr);
	//if(theErr == OS_NoErr)
	//	fAuthenticated = true;
	//return theErr;
	return QTSS_Unimplemented;
}

QTSS_Error HTTPSessionInterface::UpdateDevSnap(const char* inSnapTime, const char* inSnapJpg)
{
	if(!fAuthenticated) return QTSS_NoErr;
	return QTSS_NoErr;
}

void HTTPSessionInterface::UnRegDevSession()
{
	if (fAuthenticated)
	{
		QTSServerInterface::GetServer()->GetDeviceMap()->UnRegister(fDevice.serial_);//add
		//在redis上删除设备
		QTSServerInterface::GetServer()->RedisDelDevName(fDevice.serial_.c_str());
	}
}

QTSS_Error HTTPSessionInterface::UpdateDevRedis()
{
	return QTSS_NoErr;
}

//add
void HTTPSessionInterface::InsertToSet(const string &strCameraSerial,void * pObject)//加入到set中
{
	OSMutexLocker MutexLocker(&fMutexSet);
	DevMapItera it=fDevmap.find(strCameraSerial);
	if(it==fDevmap.end())//表示这是对当前设备这个摄像头的第一次直播
	{
		DevSet setTemp;
		setTemp.insert(pObject);
		fDevmap[strCameraSerial]=setTemp;
	}
	else//之间已经创建了
	{
		DevSet *setTemp=&(it->second);//获取当前摄像头直播列表
		setTemp->insert(pObject);
	}
}
bool HTTPSessionInterface::EraseInSet(const string &strCameraSerial,void *pObject)//删除元素，并判断是否为空，为空返回true,失败返回false
{
	OSMutexLocker MutexLocker(&fMutexSet);
	DevMapItera it=fDevmap.find(strCameraSerial);
	if(it==fDevmap.end())
	{
		return false;
	}
	else
	{
		DevSet *setTemp=&(it->second);//获取当前摄像头直播列表
		setTemp->erase(pObject);
		return setTemp->empty();
	}
}
void HTTPSessionInterface::AutoStopStreamJudge()
{
	OSMutexLocker MutexLocker(&fMutexSet);
	CliStreamMapItera it;
	stStreamInfo stTemp;
	OSRefTableEx* DeviceMap=QTSServerInterface::GetServer()->GetDeviceMap();
	OSRefTableEx::OSRefEx* theDevRef;
	for(it=fClientStreamMap.begin();it!=fClientStreamMap.end();it++)
	{
		stTemp=it->second;
		theDevRef=DeviceMap->Resolve(stTemp.strDeviceSerial);////////////////////////////////++
		if(theDevRef==NULL)
			continue;
		//走到这说明存在指定设备
		HTTPSessionInterface *pDevSession=(HTTPSessionInterface *)theDevRef->GetObjectPtr();//获得当前设备会话
		if(pDevSession->EraseInSet(stTemp.strCameraSerial,this))//当前摄像头的拉流客户端为空，则向设备发出停止推流请求
		{
			EasyDarwin::Protocol::EasyProtocolACK		reqreq(MSG_SD_STREAM_STOP_REQ);
			EasyJsonValue headerheader,bodybody;

			char chTemp[16]={0};
			UInt32 uDevCseq=pDevSession->GetCSeq();
			sprintf(chTemp,"%d",uDevCseq);
			headerheader["CSeq"]	=string(chTemp);//注意这个地方不能直接将UINT32->int,因为会造成数据失真
			headerheader[EASY_TAG_VERSION]=		EASY_PROTOCOL_VERSION;

			bodybody["DeviceSerial"]	=	stTemp.strDeviceSerial;
			bodybody["CameraSerial"]	=	stTemp.strCameraSerial;
			bodybody["StreamID"]		=   stTemp.strStreamID;
			bodybody["Protocol"]		=	stTemp.strProtocol;

			reqreq.SetHead(headerheader);
			reqreq.SetBody(bodybody);

			string buffer = reqreq.GetMsg();
			QTSS_SendHTTPPacket(pDevSession,(char*)buffer.c_str(),buffer.size(),false,false);
		}
		DeviceMap->Release(stTemp.strDeviceSerial);
	}
}
