/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSOnDemandRelayModule.cpp
    Contains:   RTSP On Demand Relay Module
*/
#include "QTSSOnDemandRelayModule.h"
#include "QTSSModuleUtils.h"
#include "ReflectorSession.h"
#include "OSArrayObjectDeleter.h"
#include "QTSSMemoryDeleter.h"
#include "OSRef.h"
#include "Socket.h"
#include "SocketUtils.h"
#include "StringParser.h"

#include "RTPSessionOutput.h"
#include "SDPSourceInfo.h"
#include "SDPUtils.h"
#include "QueryParamList.h"

#include "RTSPRelaySession.h"
#include "ParseDevice.h"

#ifndef __Win32__
    #include <unistd.h>
	#include <netdb.h>
#endif

// ATTRIBUTES
static QTSS_AttributeID         sOutputAttr                 =   qtssIllegalAttrID;
static QTSS_AttributeID         sSessionAttr                =   qtssIllegalAttrID;
static QTSS_AttributeID         sStreamCookieAttr           =   qtssIllegalAttrID;
static QTSS_AttributeID         sRequestBodyAttr            =   qtssIllegalAttrID;
static QTSS_AttributeID         sBufferOffsetAttr           =   qtssIllegalAttrID;
static QTSS_AttributeID         sExpectedDigitFilenameErr   =   qtssIllegalAttrID;
static QTSS_AttributeID         sReflectorBadTrackIDErr     =   qtssIllegalAttrID;
static QTSS_AttributeID         sClientBroadcastSessionAttr =   qtssIllegalAttrID;
static QTSS_AttributeID         sRTSPBroadcastSessionAttr   =   qtssIllegalAttrID;

// STATIC DATA
static OSRefTable*      sSessionMap = NULL;
static OSRefTable*		sClientSessionMap = NULL;

// 设备映射表
CParseDevice*			parseDevice = NULL;

static const StrPtrLen  kCacheControlHeader("no-cache");
static QTSS_PrefsObject sServerPrefs = NULL;
static QTSS_ServerObject sServer = NULL;
static QTSS_ModulePrefsObject sPrefs = NULL;
static Bool16 sFalse = false;

static UInt32 theReadInterval = 50;
static UInt32 sockRcvBuf = 32768;
static Float32 speed = 2;
static char* packetPlayHeader = NULL;
static UInt32 overbufferwindowInK = 0;
static Bool16 sendOptions = false; 
static UInt32 rtcpInterval = 5000;

static StrPtrLen    sSDPNotValidMessage("Announced SDP is not a valid SDP");
static StrPtrLen    sSDPTimeNotValidMessage("SDP time is not valid or movie not available at this time.");
static StrPtrLen    sTheNowRangeHeader("npt=now-");

static char*	sDevicePrefs     = NULL;
static char*	sDefaultDevicePrefs = "./devices.xml";

// FUNCTION PROTOTYPES
static QTSS_Error QTSSOnDemandRelayModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);

static QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams);
ReflectorSession* FindOrCreateProxySession(StrPtrLen* inPath, QTSS_StandardRTSP_Params* inParams, StrPtrLen* inData = NULL, Bool16 *foundSessionPtr = NULL);
static QTSS_Error DoSetup(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error DoPlay(QTSS_StandardRTSP_Params* inParams, ReflectorSession* inSession);
static QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams);
static void RemoveOutput(ReflectorOutput* inOutput, ReflectorSession* inSession, Bool16 killClients);

static ReflectorSession* SetupProxySession(QTSS_StandardRTSP_Params* inParams, RTSPRelaySession* session);
static QTSS_Error RereadPrefs();
static QTSS_Error ProcessRelayRTPData(QTSS_RelayingData_Params* inParams);

inline void KeepSession(QTSS_RTSPRequestObject theRequest,Bool16 keep)
{
    (void)QTSS_SetValue(theRequest, qtssRTSPReqRespKeepAlive, 0, &keep, sizeof(keep));
}

// FUNCTION IMPLEMENTATIONS
QTSS_Error QTSSOnDemandRelayModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSOnDemandRelayModuleDispatch);
}

QTSS_Error  QTSSOnDemandRelayModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPPreProcessor_Role:
            return ProcessRTSPRequest(&inParams->rtspRequestParams);
        case QTSS_RTSPRelayingData_Role:
			return ProcessRelayRTPData(&inParams->rtspRelayingDataParams);
        case QTSS_ClientSessionClosing_Role:
            return DestroySession(&inParams->clientSessionClosingParams);
   }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RTSPPreProcessor_Role);
    (void)QTSS_AddRole(QTSS_ClientSessionClosing_Role);
    (void)QTSS_AddRole(QTSS_RTSPRelayingData_Role); // call me with interleaved RTP streams on the RTSP session
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);    
    
    // Add an RTP session attribute for tracking ReflectorSession objects
    static char*        sOutputName         = "QTSSOnDemandRelayModuleOutput";
    static char*        sSessionName        = "QTSSOnDemandRelayModuleSession";
    static char*        sStreamCookieName   = "QTSSOnDemandRelayModuleStreamCookie";
    static char*        sRequestBufferName  = "QTSSOnDemandRelayModuleRequestBuffer";
    static char*        sRequestBufferLenName= "QTSSOnDemandRelayModuleRequestBufferLen";
    static char*        sBroadcasterSessionName= "QTSSOnDemandRelayModuleBroadcasterSession";

    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sOutputName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sOutputName, &sOutputAttr);

    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sSessionName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sSessionName, &sSessionAttr);

    (void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sStreamCookieName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssRTPStreamObjectType, sStreamCookieName, &sStreamCookieAttr);

    (void)QTSS_AddStaticAttribute(qtssRTSPRequestObjectType, sRequestBufferName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssRTSPRequestObjectType, sRequestBufferName, &sRequestBodyAttr);

    (void)QTSS_AddStaticAttribute(qtssRTSPRequestObjectType, sRequestBufferLenName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPRequestObjectType, sRequestBufferLenName, &sBufferOffsetAttr);
    
    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sBroadcasterSessionName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sBroadcasterSessionName, &sClientBroadcastSessionAttr);
     
     // keep the same attribute name for the RTSPSessionObject as used int he ClientSessionObject
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sBroadcasterSessionName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sBroadcasterSessionName, &sRTSPBroadcastSessionAttr);

    // Reflector session needs to setup some parameters too.
    ReflectorStream::Register();
    // RTPSessionOutput needs to do the same
    RTPSessionOutput::Register();

    // Tell the server our name!
    static char* sModuleName = "QTSSOnDemandRelayModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}


QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
    sSessionMap = NEW OSRefTable();
	sClientSessionMap = NEW OSRefTable();

    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

    // Call helper class initializers
    //ReflectorStream::Initialize(sPrefs);
    //ReflectorSession::Initialize();
    
    // Report to the server that this module handles DESCRIBE, SETUP, PLAY, PAUSE, and TEARDOWN
	static QTSS_RTSPMethod sSupportedMethods[] = { qtssDescribeMethod, qtssSetupMethod, qtssTeardownMethod, qtssPlayMethod };
	QTSSModuleUtils::SetupSupportedMethods(inParams->inServer, sSupportedMethods, 4);

    RereadPrefs();
    
   return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
	delete [] sDevicePrefs;
    
    sDevicePrefs = QTSSModuleUtils::GetStringAttribute(sPrefs, "device_prefs_file", sDefaultDevicePrefs);

	parseDevice = NEW CParseDevice();
	if (success != parseDevice->Init())
	{
		qtss_printf("parseDevice Init fail\n");
		return QTSS_Unimplemented;
	}

	if (success != parseDevice->LoadDeviceXml(sDevicePrefs))
	{
		qtss_printf("parseDevice LoadDeviceXml %s fail\n", sDevicePrefs);
	}

	return QTSS_NoErr;
}

QTSS_Error ProcessRelayRTPData(QTSS_RelayingData_Params* inParams)
{
    ReflectorSession* theSession = NULL;
	RTSPRelaySession* relaySes = (RTSPRelaySession*)inParams->inRTSPSession;
	theSession = (ReflectorSession*)relaySes->GetReflectorSession();

	if (theSession == NULL) 
        return QTSS_NoErr;

    SourceInfo* theSoureInfo = theSession->GetSourceInfo(); 
    Assert(theSoureInfo != NULL);
    if (theSoureInfo == NULL)
        return QTSS_NoErr;

    UInt32  numStreams = theSession->GetNumStreams();

    char*   packetData= inParams->inPacketData;
    UInt8   packetChannel = inParams->inChannel;	
    UInt16  packetDataLen = inParams->inPacketLen;
    char*   rtpPacket = &packetData[0];
    
    {
        UInt32 inIndex = packetChannel / 2;
        ReflectorStream* theStream = NULL;
        if (inIndex < numStreams) 
        { 
			theStream = theSession->GetStreamByIndex(inIndex);

            SourceInfo::StreamInfo* theStreamInfo =theStream->GetStreamInfo();  
            UInt16 serverReceivePort =theStreamInfo->fPort;         

            Bool16 isRTCP =false;
            if (theStream != NULL)
            {   if (packetChannel & 1)
                {   serverReceivePort ++;
                    isRTCP = true;
                }

				//qtss_printf("%d",inIndex);
                theStream->PushPacket(rtpPacket,packetDataLen, isRTCP);
            }
        }
    }
	return QTSS_NoErr;
}

QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams)
{
    OSMutexLocker locker (sSessionMap->GetMutex()); //operating on sOutputAttr

    QTSS_RTSPMethod* theMethod = NULL;

	UInt32 theLen = 0;
    if ((QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqMethod, 0,
            (void**)&theMethod, &theLen) != QTSS_NoErr) || (theLen != sizeof(QTSS_RTSPMethod)))
    {
        Assert(0);
        return QTSS_RequestFailed;
    }

    if (*theMethod == qtssDescribeMethod)
        return DoDescribe(inParams);
    if (*theMethod == qtssSetupMethod)
        return DoSetup(inParams);
        
    RTPSessionOutput** theOutput = NULL;
    QTSS_Error theErr = QTSS_GetValuePtr(inParams->inClientSession, sOutputAttr, 0, (void**)&theOutput, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(RTPSessionOutput*))) // a broadcaster push session
    {   if (*theMethod == qtssPlayMethod || *theMethod == qtssRecordMethod)
            return DoPlay(inParams, NULL);
        else
            return QTSS_RequestFailed;
    }

    switch (*theMethod)
    {
        case qtssPlayMethod:
            return DoPlay(inParams, (*theOutput)->GetReflectorSession());
        case qtssTeardownMethod:
            // Tell the server that this session should be killed, and send a TEARDOWN response
            (void)QTSS_Teardown(inParams->inClientSession);
            (void)QTSS_SendStandardRTSPResponse(inParams->inRTSPRequest, inParams->inClientSession, 0);
            break;
        case qtssPauseMethod:
            (void)QTSS_Pause(inParams->inClientSession);
            (void)QTSS_SendStandardRTSPResponse(inParams->inRTSPRequest, inParams->inClientSession, 0);
            break;
        default:
            break;
    }           
    return QTSS_NoErr;
}

ReflectorSession* SetupProxySession(QTSS_StandardRTSP_Params* inParams, RTSPRelaySession* session)
{
	if(session == NULL) return NULL;

	UInt32 sdpLen = 0;
	char* relaySDP = NULL;
	relaySDP = session->GetSDPInfo()->GetLocalSDP(&sdpLen);;

	if(relaySDP == NULL) return NULL;
	StrPtrLen inData(relaySDP);
	return FindOrCreateProxySession(session->GetRef()->GetString(), inParams, &inData,0);  
}


void DoDescribeAddRequiredSDPLines2(QTSS_StandardRTSP_Params* inParams, ReflectorSession* theSession, QTSS_TimeVal modDate,  ResizeableStringFormatter *editedSDP, StrPtrLen* theSDPPtr)
{
    SDPContainer checkedSDPContainer;
    checkedSDPContainer.SetSDPBuffer( theSDPPtr );  
    if (!checkedSDPContainer.HasReqLines())
    {
        if (!checkedSDPContainer.HasLineType('v'))
        { // add v line
            editedSDP->Put("v=0\r\n");
        }
        
        if (!checkedSDPContainer.HasLineType('s'))
        { // add s line
           char* theSDPName = NULL;            
            (void)QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqFilePath, 0, &theSDPName);
            QTSSCharArrayDeleter thePathStrDeleter(theSDPName);
            editedSDP->Put("s=");
            editedSDP->Put(theSDPName);
            editedSDP->PutEOL();
        }
        
       if (!checkedSDPContainer.HasLineType('t'))
       { // add t line
            editedSDP->Put("t=0 0\r\n");
       }

       if (!checkedSDPContainer.HasLineType('o'))
       { // add o line
            editedSDP->Put("o=broadcast_sdp ");
            char tempBuff[256]= "";               
            tempBuff[255] = 0;
            qtss_snprintf(tempBuff,sizeof(tempBuff) - 1, "%lu", (UInt64) theSession);
            editedSDP->Put(tempBuff);

            editedSDP->Put(" ");
            // modified date is in milliseconds.  Convert to NTP seconds as recommended by rfc 2327
            qtss_snprintf(tempBuff, sizeof(tempBuff) - 1, "%"_64BITARG_"d", (SInt64) (modDate/1000) + 2208988800LU);
            editedSDP->Put(tempBuff);

            editedSDP->Put(" IN IP4 ");
            UInt32 buffLen = sizeof(tempBuff) -1;
            (void)QTSS_GetValue(inParams->inClientSession, qtssCliSesHostName, 0, &tempBuff, &buffLen);
            editedSDP->Put(tempBuff, buffLen);

            editedSDP->PutEOL();
        }
    } 
    editedSDP->Put(*theSDPPtr);
}

QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams)
{
	char* theUriStr = NULL;
    QTSS_Error err = QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqFileName, 0, &theUriStr);
    Assert(err == QTSS_NoErr);
    if(err != QTSS_NoErr)
		return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientBadRequest, 0);
    QTSSCharArrayDeleter theUriStrDeleter(theUriStr);

	//从接口获取信息结构体
	//TODO:
	DeviceInfo* pDeviceInfo = parseDevice->GetDeviceInfoByIdName(theUriStr);

	if(pDeviceInfo == NULL)
	{
		//qtss_printf("ID:%s Not Found\n",theUriStr);
		return QTSS_RequestFailed;
	}

	//信息存在rtsp://59.46.115.84:8554/h264/ch1/sub/av_stream
	RTSPRelaySession* clientSes = NULL;
	//首先查找Map里面是否已经有了对应的流
	StrPtrLen streamName(theUriStr);
	OSRef* clientSesRef = sClientSessionMap->Resolve(&streamName);
	if(clientSesRef != NULL)
	{
		clientSes = (RTSPRelaySession*)clientSesRef->GetObject();
	}
	else
	{
		clientSes = NEW RTSPRelaySession(pDeviceInfo->m_szSourceUrl, RTSPRelaySession::kRTSPTCPClientType, 0, true, theUriStr);


		QTSS_Error theErr = clientSes->SendDescribe();

		//qtss_printf("QTSSOnDemandRelayModule DoDescribe OK\n");


			if(theErr == QTSS_NoErr)
			{
				OS_Error theErr = sClientSessionMap->Register(clientSes->GetRef());
				Assert(theErr == QTSS_NoErr);
			}
			else
			{
				clientSes->Signal(Task::kKillEvent);
				return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientNotFound, 0); 
			}

			//增加一次对RelaySession的无效引用，后面会统一释放
			OSRef* debug = sClientSessionMap->Resolve(&streamName);
			Assert(debug == clientSes->GetRef());
		}

	ReflectorSession* theSession = SetupProxySession(inParams, clientSes);
    
    if (theSession == NULL)
	{
		//sClientSessionMap->Release(clientSes->GetRef());
  //      return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssServerNotImplemented, 0);
	}

	QTSS_Error Err = QTSS_SetValue(inParams->inRTSPSession, sRTSPBroadcastSessionAttr, 0, &theSession, sizeof(theSession));
	Assert(Err == QTSS_NoErr);

	clientSes->SetReflectorSession(theSession);
	theSession->SetRTSPRelaySession((void*)clientSes);
	sClientSessionMap->Release(clientSes->GetRef());

    iovec theDescribeVec[3] = { {0 }};
    Assert(theSession->GetLocalSDP()->Ptr != NULL);

    StrPtrLen theFileData;
	QTSS_TimeVal outModDate = 0;
	QTSS_TimeVal inModDate = -1;

	theFileData.Ptr = theSession->GetLocalSDP()->Ptr;
	theFileData.Len = theSession->GetLocalSDP()->Len;

// -------------- process SDP to remove connection info and add track IDs, port info, and default c= line
    StrPtrLen theSDPData;
    SDPSourceInfo tempSDPSourceInfo(theFileData.Ptr, theFileData.Len); // will make a copy and delete in destructor
    theSDPData.Ptr = tempSDPSourceInfo.GetLocalSDP(&theSDPData.Len); // returns a new buffer with processed sdp
    OSCharArrayDeleter sdpDeleter(theSDPData.Ptr); // delete the temp sdp source info buffer returned by GetLocalSDP
    
    if (theSDPData.Len <= 0) // can't find it on disk or it failed to parse just use the one in the session.
    {
        theSDPData.Ptr = theSession->GetLocalSDP()->Ptr; // this sdp isn't ours it must not be deleted
        theSDPData.Len = theSession->GetLocalSDP()->Len;
    }

// ------------  Clean up missing required SDP lines
    ResizeableStringFormatter editedSDP(NULL,0);
    DoDescribeAddRequiredSDPLines2(inParams, theSession, outModDate, &editedSDP, &theSDPData);
    StrPtrLen editedSDPSPL(editedSDP.GetBufPtr(),editedSDP.GetBytesWritten());

// ------------ Check the headers
    SDPContainer checkedSDPContainer;
    checkedSDPContainer.SetSDPBuffer( &editedSDPSPL );  
    if (!checkedSDPContainer.IsSDPBufferValid())
    {  
        return QTSSModuleUtils::SendErrorResponseWithMessage(inParams->inRTSPRequest, qtssUnsupportedMediaType, &sSDPNotValidMessage);
    }

// ------------ Put SDP header lines in correct order
    Float32 adjustMediaBandwidthPercent = 1.0;
    Bool16 adjustMediaBandwidth = false;

    SDPLineSorter sortedSDP(&checkedSDPContainer,adjustMediaBandwidthPercent);

// ------------ Write the SDP 
    UInt32 sessLen = sortedSDP.GetSessionHeaders()->Len;
    UInt32 mediaLen = sortedSDP.GetMediaHeaders()->Len;
    theDescribeVec[1].iov_base = sortedSDP.GetSessionHeaders()->Ptr;
    theDescribeVec[1].iov_len = sortedSDP.GetSessionHeaders()->Len;

    theDescribeVec[2].iov_base = sortedSDP.GetMediaHeaders()->Ptr;
    theDescribeVec[2].iov_len = sortedSDP.GetMediaHeaders()->Len;

    (void)QTSS_AppendRTSPHeader(inParams->inRTSPRequest, qtssCacheControlHeader,
                                kCacheControlHeader.Ptr, kCacheControlHeader.Len);
    QTSSModuleUtils::SendDescribeResponse(inParams->inRTSPRequest, inParams->inClientSession,
                                            &theDescribeVec[0], 3, sessLen + mediaLen ); 
    return QTSS_NoErr;
}

ReflectorSession* FindOrCreateProxySession(StrPtrLen* inPath, QTSS_StandardRTSP_Params* inParams, StrPtrLen* inData, Bool16 *foundSessionPtr)
{
    OSMutexLocker locker(sSessionMap->GetMutex());
    OSRef* theSessionRef = sSessionMap->Resolve(inPath);
    ReflectorSession* theSession = NULL;

	if( inData == NULL ) return NULL;
     
    if (theSessionRef == NULL)
    {
        StrPtrLen theFileData;
        StrPtrLen theFileDeleteData;
        
		theFileData = *inData;
        OSCharArrayDeleter fileDataDeleter(theFileDeleteData.Ptr); 
            
        if (theFileData.Len <= 0)
            return NULL;
            
        SDPSourceInfo* theInfo = NEW SDPSourceInfo(theFileData.Ptr, theFileData.Len); // will make a copy
            
        //if (!theInfo->IsReflectable())
        //{   delete theInfo;
        //    return NULL;
        //}

        UInt32 theSetupFlag = ReflectorSession::kMarkSetup;
        
        theSession = NEW ReflectorSession(inPath);
		if (theSession == NULL) return NULL;

		theSession->SetHasBufferedStreams(true);
        
        QTSS_Error theErr = theSession->SetupReflectorSession(theInfo, inParams, theSetupFlag);
        if (theErr != QTSS_NoErr)
        {  
			delete theSession;
            return NULL;
        }

        theErr = sSessionMap->Register(theSession->GetRef());
        Assert(theErr == QTSS_NoErr);

    }
    else
    {        
        theSession = (ReflectorSession*)theSessionRef->GetObject(); 
    }
            
    Assert(theSession != NULL);
    return theSession;
}

QTSS_Error DoSetup(QTSS_StandardRTSP_Params* inParams)
{
    ReflectorSession* theSession = NULL;
	UInt32 len = sizeof(theSession);
	QTSS_GetValue(inParams->inRTSPSession, sRTSPBroadcastSessionAttr, 0, &theSession, &len);

	if(theSession == NULL)
		return QTSS_RequestFailed;

    Bool16 foundSession = false;
    UInt32 theLen = 0;
    RTPSessionOutput** theOutput = NULL;
    QTSS_Error theErr = QTSS_GetValuePtr(inParams->inClientSession, sOutputAttr, 0, (void**)&theOutput, &theLen);
    if (theLen != sizeof(RTPSessionOutput*))
	{
        if (theErr != QTSS_NoErr)
        {            
            RTPSessionOutput* theNewOutput = NEW RTPSessionOutput(inParams->inClientSession, theSession, sServerPrefs, sStreamCookieAttr );
            theSession->AddOutput(theNewOutput,true);
            (void)QTSS_SetValue(inParams->inClientSession, sOutputAttr, 0, &theNewOutput, sizeof(theNewOutput));
        }
    }

    //unless there is a digit at the end of this path (representing trackID), don't
    //even bother with the request
    char* theDigitStr = NULL;
    (void)QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqFileDigit, 0, &theDigitStr);
    QTSSCharArrayDeleter theDigitStrDeleter(theDigitStr);
    if (theDigitStr == NULL)
    {
        return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientBadRequest,sExpectedDigitFilenameErr);
    }
    
    UInt32 theTrackID = ::strtol(theDigitStr, NULL, 10);
   
    // Get info about this trackID
    SourceInfo::StreamInfo* theStreamInfo = theSession->GetSourceInfo()->GetStreamInfoByTrackID(theTrackID);
    // If theStreamInfo is NULL, we don't have a legit track, so return an error
    if (theStreamInfo == NULL)
        return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientBadRequest,
                                                    sReflectorBadTrackIDErr);
                                                    
    StrPtrLen* thePayloadName = &theStreamInfo->fPayloadName;
    QTSS_RTPPayloadType thePayloadType = theStreamInfo->fPayloadType;

    StringParser parser(thePayloadName);
    
    parser.GetThru(NULL, '/');
    theStreamInfo->fTimeScale = parser.ConsumeInteger(NULL);
    if (theStreamInfo->fTimeScale == 0)
        theStreamInfo->fTimeScale = 90000;
    
    QTSS_RTPStreamObject newStream = NULL;
    {
        // Ok, this is completely crazy but I can't think of a better way to do this that's
        // safe so we'll do it this way for now. Because the ReflectorStreams use this session's
        // stream queue, we need to make sure that each ReflectorStream is not reflecting to this
        // session while we call QTSS_AddRTPStream. One brutal way to do this is to grab each
        // ReflectorStream's mutex, which will stop every reflector stream from running.
        
        for (UInt32 x = 0; x < theSession->GetNumStreams(); x++)
            theSession->GetStreamByIndex(x)->GetMutex()->Lock();
            
        theErr = QTSS_AddRTPStream(inParams->inClientSession, inParams->inRTSPRequest, &newStream, 0);

        for (UInt32 y = 0; y < theSession->GetNumStreams(); y++)
            theSession->GetStreamByIndex(y)->GetMutex()->Unlock();
            
        if (theErr != QTSS_NoErr)
            return theErr;
    }
    
    // Set up dictionary items for this stream
    theErr = QTSS_SetValue(newStream, qtssRTPStrPayloadName, 0, thePayloadName->Ptr, thePayloadName->Len);
    Assert(theErr == QTSS_NoErr);
    theErr = QTSS_SetValue(newStream, qtssRTPStrPayloadType, 0, &thePayloadType, sizeof(thePayloadType));
    Assert(theErr == QTSS_NoErr);
    theErr = QTSS_SetValue(newStream, qtssRTPStrTrackID, 0, &theTrackID, sizeof(theTrackID));
    Assert(theErr == QTSS_NoErr);
    theErr = QTSS_SetValue(newStream, qtssRTPStrTimescale, 0, &theStreamInfo->fTimeScale, sizeof(theStreamInfo->fTimeScale));
    Assert(theErr == QTSS_NoErr);

    // We only want to allow over buffering to dynamic rate clients   
    SInt32  canDynamicRate = -1;
    theLen = sizeof(canDynamicRate);
    (void) QTSS_GetValue(inParams->inRTSPRequest, qtssRTSPReqDynamicRateState, 0, (void*) &canDynamicRate, &theLen);
    if (canDynamicRate < 1) // -1 no rate field, 0 off
        (void)QTSS_SetValue(inParams->inClientSession, qtssCliSesOverBufferEnabled, 0, &sFalse, sizeof(sFalse));

    // Place the stream cookie in this stream for future reference
    void* theStreamCookie = theSession->GetStreamCookie(theTrackID);
    Assert(theStreamCookie != NULL);
    theErr = QTSS_SetValue(newStream, sStreamCookieAttr, 0, &theStreamCookie, sizeof(theStreamCookie));
    Assert(theErr == QTSS_NoErr);

    // Set the number of quality levels.
    static UInt32 sNumQualityLevels = ReflectorSession::kNumQualityLevels;
    theErr = QTSS_SetValue(newStream, qtssRTPStrNumQualityLevels, 0, &sNumQualityLevels, sizeof(sNumQualityLevels));
    Assert(theErr == QTSS_NoErr);
    
    //send the setup response
    (void)QTSS_AppendRTSPHeader(inParams->inRTSPRequest, qtssCacheControlHeader,
                                kCacheControlHeader.Ptr, kCacheControlHeader.Len);
    (void)QTSS_SendStandardRTSPResponse(inParams->inRTSPRequest, newStream, qtssSetupRespDontWriteSSRC);
    return QTSS_NoErr;
}



Bool16 HaveRelayStreamBuffers(QTSS_StandardRTSP_Params* inParams, ReflectorSession* inSession)
{
    if (inSession == NULL || inParams == NULL)
        return false;
        
    UInt32 firstTimeStamp = 0;
    UInt16 firstSeqNum = 0;            
    ReflectorSender* theSender = NULL;
    ReflectorStream* theReflectorStream = NULL;
    QTSS_RTPStreamObject* theRef = NULL;
    UInt32 theStreamIndex = 0;
    UInt32 theLen = 0; 
    QTSS_Error theErr = QTSS_NoErr;
    Bool16 haveBufferedStreams = true; // set to false and return if we can't set the packets
    UInt32 y = 0;
    
    
    SInt64 packetArrivalTime = 0;

    //lock all streams
    for (y = 0; y < inSession->GetNumStreams(); y++)
        inSession->GetStreamByIndex(y)->GetMutex()->Lock();

           
    for (   theStreamIndex = 0;
            QTSS_GetValuePtr(inParams->inClientSession, qtssCliSesStreamObjects, theStreamIndex, (void**)&theRef, &theLen) == QTSS_NoErr;
            theStreamIndex++)
    {        
        theReflectorStream = inSession->GetStreamByIndex(theStreamIndex);

      //  if (!theReflectorStream->HasFirstRTCP())
      //      printf("theStreamIndex =%lu no rtcp\n", theStreamIndex);
        
      //  if (!theReflectorStream->HasFirstRTP())
      //      printf("theStreamIndex = %lu no rtp\n", theStreamIndex);
            
        if ((theReflectorStream == NULL) || (false == theReflectorStream->HasFirstRTP()) )
        {    
            haveBufferedStreams = false;
            //printf("1 breaking no buffered streams\n");
             break;
        }                
        
        theSender = theReflectorStream->GetRTPSender();                
        haveBufferedStreams =  theSender->GetFirstPacketInfo(&firstSeqNum, &firstTimeStamp, &packetArrivalTime);
        //printf("theStreamIndex= %lu haveBufferedStreams=%d, seqnum=%d, timestamp=%lu\n", theStreamIndex, haveBufferedStreams, firstSeqNum, firstTimeStamp);
       
       if (!haveBufferedStreams)
        {    
            //printf("2 breaking no buffered streams\n");
            break;
        }                        
        
        theErr = QTSS_SetValue(*theRef, qtssRTPStrFirstSeqNumber, 0, &firstSeqNum, sizeof(firstSeqNum));
        Assert(theErr == QTSS_NoErr);
        
        theErr = QTSS_SetValue(*theRef, qtssRTPStrFirstTimestamp, 0, &firstTimeStamp, sizeof(firstTimeStamp));
        Assert(theErr == QTSS_NoErr);
        
   
    }     
    //unlock all streams
    for (y = 0; y < inSession->GetNumStreams(); y++)
        inSession->GetStreamByIndex(y)->GetMutex()->Unlock();
            
    return haveBufferedStreams;
}

QTSS_Error DoPlay(QTSS_StandardRTSP_Params* inParams, ReflectorSession* inSession)
{
    QTSS_Error theErr = QTSS_NoErr;
    UInt32 flags = 0;
    UInt32 theLen = 0;
    
    if (inSession == NULL) // it is a broadcast session so store the broadcast session.
		return QTSS_RequestFailed;
    else
    {   
        UInt32 bitsPerSecond =  inSession->GetBitRate();
        (void)QTSS_SetValue(inParams->inClientSession, qtssCliSesMovieAverageBitRate, 0, &bitsPerSecond, sizeof(bitsPerSecond));
   
        QTSS_Error theErr = QTSS_Play(inParams->inClientSession, inParams->inRTSPRequest, qtssPlayFlagsAppendServerInfo);
        if (theErr != QTSS_NoErr)
            return theErr;
    }
    
	//OSRef* theRelaySessionRef = sClientSessionMap->Resolve(inSession->GetRef()->GetString());
	//if(theRelaySessionRef != NULL)
	//{
	//	RTSPRelaySession* relaySes = (RTSPRelaySession*)theRelaySessionRef->GetObject();
	//	//QTSS_Error err = relaySes->Start();
	//	sClientSessionMap->Release(theRelaySessionRef);
	//}
	//else
	//{
	//	return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssServerInternal, 0);
	//}

    (void)QTSS_SendStandardRTSPResponse(inParams->inRTSPRequest, inParams->inClientSession, flags);
    return QTSS_NoErr;
}
 
QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams)
{
    RTPSessionOutput**  theOutput = NULL;
    ReflectorOutput*    outputPtr = NULL;
    ReflectorSession*   theSession = NULL;

	OSMutexLocker locker (sSessionMap->GetMutex());
    
    //UInt32 theLen = sizeof(theSession);
    //QTSS_Error theErr = QTSS_GetValue(inParams->inClientSession, sClientBroadcastSessionAttr, 0, &theSession, &theLen);
    if (/*theSession != NULL && */false)
    {   
        //ReflectorSession*   deletedSession = NULL;
        //theErr = QTSS_SetValue(inParams->inClientSession, sClientBroadcastSessionAttr, 0, &deletedSession, sizeof(deletedSession));

        //SourceInfo* theSoureInfo = theSession->GetSourceInfo(); 
        //if (theSoureInfo == NULL)
        //    return QTSS_NoErr;
        //    
        //UInt32  numStreams = theSession->GetNumStreams();
        //SourceInfo::StreamInfo* theStreamInfo = NULL;
        //    
        //for (UInt32 index = 0; index < numStreams; index++)
        //{   theStreamInfo = theSoureInfo->GetStreamInfo(index);
        //    if (theStreamInfo != NULL)
        //        theStreamInfo->fSetupToReceive = false;
        //}
        //theSession->RemoveSessionFromOutput(inParams->inClientSession);
        //RemoveOutput(NULL, theSession, 1);
    }
    else
    {
        UInt32 theLen = 0;
		QTSS_Error theErr = QTSS_GetValuePtr(inParams->inClientSession, sOutputAttr, 0, (void**)&theOutput, &theLen);
        if ((theErr != QTSS_NoErr) || (theLen != sizeof(RTPSessionOutput*)) || (theOutput == NULL))
            return QTSS_RequestFailed;
        theSession = (*theOutput)->GetReflectorSession();
    
        if (theOutput != NULL)
            outputPtr = (ReflectorOutput*) *theOutput;
            
        if (outputPtr != NULL)
            RemoveOutput(outputPtr, theSession, false);
                
    }

    return QTSS_NoErr;
}

void RemoveOutput(ReflectorOutput* inOutput, ReflectorSession* inSession, Bool16 killClients)
{
    //This function removes the output from the ReflectorSession, then
    Assert(inSession);
    if (inSession != NULL)
	{
        if (inOutput != NULL)
        {
			inSession->RemoveOutput(inOutput,true);

            //qtss_printf("QTSSReflectorModule.cpp:RemoveOutput it is a client session\n");
        }
        else
        {   // it is a Broadcaster session
            //qtss_printf("QTSSReflectorModule.cpp:RemoveOutput it is a broadcaster session\n");
            //SourceInfo* theInfo = inSession->GetSourceInfo();         
            //Assert(theInfo);
            //
            //if (theInfo->IsRTSPControlled())
            //{   
            //    FileDeleter(inSession->GetSourcePath());
            //}
            //    
 
            //if (killClients || 1)
            //{    inSession->TearDownAllOutputs();
            //}
        }
    
        //qtss_printf("QTSSReflectorModule.cpp:RemoveOutput refcount =%lu\n", inSession->GetRef()->GetRefCount());

        //check if the ReflectorSession should be deleted
        //(it should if its ref count has dropped to 0)
        OSMutexLocker locker (sSessionMap->GetMutex());
        
        OSRef* theSessionRef = inSession->GetRef();
        if (theSessionRef != NULL) 
        {               
            if (theSessionRef->GetRefCount() == 0)
            { 
				RTSPRelaySession* proxySession = (RTSPRelaySession*)inSession->GetRTSPRelaySession();
				if(proxySession != NULL)
				{
					proxySession->SetReflectorSession(NULL);
					sClientSessionMap->UnRegister(proxySession->GetRef());
					proxySession->Signal(Task::kKillEvent);
				}

				qtss_printf("Delete Reflector Session \n");
				inSession->SetRTSPRelaySession(NULL);
				sSessionMap->UnRegister(theSessionRef);
				delete inSession;
            }
			//else if (theSessionRef->GetRefCount() == 1)
			//{  
			//	//qtss_printf("QTSSReflector.cpp:RemoveOutput Delete SESSION=%lu\n",(UInt32)inSession);
			//	RTSPRelaySession* proxySession = (RTSPRelaySession*)inSession->GetRelaySession();
			//	if(proxySession != NULL)
			//	{
			//		proxySession->SetReflectorSession(NULL);
			//		sClientSessionMap->UnRegister(proxySession->GetRef());
			//		proxySession->Signal(Task::kKillEvent);
			//	}

			//	inSession->SetRelaySession(NULL);
			//	sSessionMap->UnRegister(theSessionRef);
			//	delete inSession;
			//}
            else
            {
				qtss_printf("QTSSReflector.cpp:RemoveOutput Release SESSION=%lu RefCount=%d\n",(UInt64)inSession,theSessionRef->GetRefCount());
                sSessionMap->Release(theSessionRef); //  one of the sessions on the ref is ending just decrement the count
            }
        }
    }
    delete inOutput;
}