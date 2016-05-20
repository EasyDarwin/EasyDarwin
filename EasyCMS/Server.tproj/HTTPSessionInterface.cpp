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

unsigned int            HTTPSessionInterface::sSessionIndexCounter = kFirstHTTPSessionID;

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
    
    /* 9 */{ "EasyHTTPSessionID",			NULL,			qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe  },
    /* 10 */{ "EasyHTTPSesLocalPort",       SetupParams,    qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
    /* 11 */{ "EasyHTTPSesRemotePort",      SetupParams,    qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
    
	/* 12 */{ "EasyHTTPSesContentBody",		NULL,           qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite },
	/* 13 */{ "EasyHTTPSesContentBodyOffset",NULL,          qtssAttrDataTypeUInt32,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite }
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

QTSS_Error HTTPSessionInterface::UpdateDevSnap(const char* inSnapTime, const char* inSnapJpg)
{
	if(!fAuthenticated) return QTSS_NoErr;
	return QTSS_NoErr;
}

void HTTPSessionInterface::UnRegDevSession()
{
	if (fAuthenticated)
	{
		QTSServerInterface::GetServer()->GetDeviceSessionMap()->UnRegister(fDevice.serial_);//add
		//在redis上删除设备
		QTSServerInterface::GetServer()->RedisDelDevName(fDevice.serial_.c_str());
	}
}

QTSS_Error HTTPSessionInterface::UpdateDevRedis()
{
	return QTSS_NoErr;
}
