/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyRelayModule.cpp
    Contains:   RTSP On Demand Relay Module
*/

#include "QTSServerInterface.h"
#include "EasyRelayModule.h"
#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"
#include "QTSSMemoryDeleter.h"
#include "QueryParamList.h"
#include "OSRef.h"
#include "StringParser.h"
#include "EasyRelaySession.h"



#ifndef __Win32__
    #include <unistd.h>
	#include <netdb.h>

#else

#include "tinystr.h"
#include "tinyxml.h"
#ifdef _DEBUG
#pragma comment(lib, "tinyxml_d.lib")
#else
#pragma comment(lib, "tinyxml_r.lib")
#endif

#endif

// STATIC DATA
static OSRefTable* sRelaySessionMap = NULL;
static QTSS_PrefsObject sServerPrefs = NULL;
static QTSS_ServerObject sServer = NULL;
static QTSS_ModulePrefsObject       sPrefs = NULL;

static StrPtrLen    sRelaySuffix("EasyRelayModule");

#define MAX_PATH_LENGTH			128
static char*            sLocal_IP_Addr = NULL;
static char*            sDefaultLocal_IP_Addr = "127.0.0.1";

#define QUERY_STREAM_NAME	"name"
#define QUERY_STREAM_CHANNEL "channel"
#define QUERY_STREAM_URL	"url"
#define QUERY_STREAM_CMD	"cmd"
#define QUERY_STREAM_CMD_START "start"
#define QUERY_STREAM_CMD_STOP "stop"

// FUNCTION PROTOTYPES
static QTSS_Error EasyRelayModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs();

static QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error GetChannelInfoById(char *inChannelId, char *outUrl, int urlsize, char *outUsername, int usernameSize, char *outPassword, int passwordSize, char *outPushAddr, int pushAddrSize);
static int VSRM_GetPath(char *filepath, unsigned int size);
int		tinyxml_WriteChannelList();

// FUNCTION IMPLEMENTATIONS
QTSS_Error EasyRelayModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, EasyRelayModuleDispatch);
}

QTSS_Error  EasyRelayModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
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
   }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RTSPPreProcessor_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);    
    
    // Tell the server our name!
    static char* sModuleName = "EasyRelayModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
	sRelaySessionMap = NEW OSRefTable();

    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

    // Report to the server that this module handles DESCRIBE
	static QTSS_RTSPMethod sSupportedMethods[] = { qtssOptionsMethod, qtssDescribeMethod};
	QTSSModuleUtils::SetupSupportedMethods(inParams->inServer, sSupportedMethods, 2);

    RereadPrefs();
    
   return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
	delete [] sLocal_IP_Addr;
    sLocal_IP_Addr = QTSSModuleUtils::GetStringAttribute(sPrefs, "local_ip_address", sDefaultLocal_IP_Addr);

	return QTSS_NoErr;
}

QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams)
{
	OSMutexLocker locker (sRelaySessionMap->GetMutex());
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
             
    return QTSS_NoErr;
}
QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams)
{
	//解析命令
    char* theFullPathStr = NULL;
    QTSS_Error theErr = QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqFileName, 0, &theFullPathStr);
    Assert(theErr == QTSS_NoErr);
    QTSSCharArrayDeleter theFullPathStrDeleter(theFullPathStr);
        
    if (theErr != QTSS_NoErr)
        return NULL;

    StrPtrLen theFullPath(theFullPathStr);

    if (theFullPath.Len != sRelaySuffix.Len )
	return NULL;

	StrPtrLen endOfPath2(&theFullPath.Ptr[theFullPath.Len -  sRelaySuffix.Len], sRelaySuffix.Len);
    if (!endOfPath2.Equal(sRelaySuffix))
    {   
        return NULL;
    }

	//解析查询字符串
    char* theQueryStr = NULL;
    theErr = QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqQueryString, 0, &theQueryStr);
    Assert(theErr == QTSS_NoErr);
    QTSSCharArrayDeleter theQueryStringDeleter(theQueryStr);
        
    if (theErr != QTSS_NoErr)
        return NULL;

    StrPtrLen theQueryString(theQueryStr);

	QueryParamList parList(theQueryStr);

	const char* sName = parList.DoFindCGIValueForParam(QUERY_STREAM_NAME);
	const char* sChannel = parList.DoFindCGIValueForParam(QUERY_STREAM_CHANNEL);
	if(sName == NULL && sChannel == NULL)
	{
		return NULL;
	}
	char szChannelURL[256] = {0,};
	char szUsername[32] = {0,};
	char szPassword[32] = {0,};

	const char* sURL = NULL;
	char sPushServerAddr[256] = {0,};

	if (NULL != sChannel)
	{
		//find channel info
		
		GetChannelInfoById( (char*)sChannel, szChannelURL, sizeof(szChannelURL), szUsername, sizeof(szUsername), szPassword, sizeof(szPassword), sPushServerAddr, sizeof(sPushServerAddr));

		if ( (int)strlen(szChannelURL) < 1 )	return NULL;	//not found the channel

		//sURL = "rtsp://192.168.1.186:8557";
		sURL = szChannelURL;
	}
	else
	{
		sURL = parList.DoFindCGIValueForParam(QUERY_STREAM_URL);
	}
	//if(sURL == NULL) return NULL;

	const char* sCMD = parList.DoFindCGIValueForParam(QUERY_STREAM_CMD);

	bool bStop = false;
	if(sCMD)
	{
		if(::strcmp(sCMD,QUERY_STREAM_CMD_STOP) == 0)
			bStop = true;
	}

	StrPtrLen streamName(NULL!=sName?(char*)sName:(char*)sChannel);
	//从接口获取信息结构体
	EasyRelaySession* session = NULL;
	//首先查找Map里面是否已经有了对应的流
	OSRef* sessionRef = sRelaySessionMap->Resolve(&streamName);
	if(sessionRef != NULL)
	{
		session = (EasyRelaySession*)sessionRef->GetObject();
	}
	else
	{
		if(bStop) return NULL;

		if(sURL == NULL) return NULL;

		session = NEW EasyRelaySession((char*)sURL, EasyRelaySession::kRTSPTCPClientType, NULL!=sName?(char*)sName:(char*)sChannel, sPushServerAddr);

		QTSS_Error theErr = session->RelaySessionStart();

		if(theErr == QTSS_NoErr)
		{
			OS_Error theErr = sRelaySessionMap->Register(session->GetRef());
			Assert(theErr == QTSS_NoErr);
		}
		else
		{
			session->Signal(Task::kKillEvent);
			return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientNotFound, 0); 
		}

		//增加一次对RelaySession的无效引用，后面会统一释放
		OSRef* debug = sRelaySessionMap->Resolve(&streamName);
		Assert(debug == session->GetRef());
	}

	sRelaySessionMap->Release(session->GetRef());

	if(bStop)
	{
		sRelaySessionMap->UnRegister(session->GetRef());
		session->Signal(Task::kKillEvent);
		return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssSuccessOK, 0); 
	}

	QTSS_RTSPStatusCode statusCode = qtssRedirectPermMoved;
	QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqStatusCode, 0, &statusCode, sizeof(statusCode));

	// Get the ip addr out of the prefs dictionary
	UInt16 thePort = 554;
	UInt32 theLen = sizeof(UInt16);
	theErr = QTSServerInterface::GetServer()->GetPrefs()->GetValue(qtssPrefsRTSPPorts, 0, &thePort, &theLen);
	Assert(theErr == QTSS_NoErr);   

	//构造本地URL
	char url[QTSS_MAX_URL_LENGTH] = { 0 };

	qtss_sprintf(url,"rtsp://%s:%d/%s.sdp", sLocal_IP_Addr, thePort, NULL!=sName?(char*)sName:(char*)sChannel);
	StrPtrLen locationRedirect(url);

	Bool16 sFalse = false;
	(void)QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));
	QTSS_AppendRTSPHeader(inParams->inRTSPRequest, qtssLocationHeader, locationRedirect.Ptr, locationRedirect.Len);	
	return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssRedirectPermMoved, 0);
}


QTSS_Error GetChannelInfoById(char *inChannelId, char *outUrl, int urlSize, char *outUsername, int usernameSize, char *outPassword, int passwordSize, char *outPushAddr, int pushAddrSize)
{
	char channelConfig[MAX_PATH_LENGTH] = {0,};
	int ret = VSRM_GetPath(channelConfig, MAX_PATH_LENGTH);
	if (ret < 0)		return -2;

#ifdef _WIN32
	strcat(channelConfig, "\\");
#else
	strcat(channelConfig, "/");
#endif

	//tinyxml_WriteChannelList();

	strcat(channelConfig, "EasyNVR.xml");

	TiXmlDocument m_DocR;
	if (! m_DocR.LoadFile(channelConfig))
	{
		//_TRACE("设备列表不存在.\n");
		return -3;
	}

#define EASY_NVR_CONF			"CHANNEL_LIST"
#define EASY_NVR_CHANNEL_GROUP	"GROUP"
#define EASY_NVR_GROUP_NAME		"NAME"
#define EASY_NVR_GROUP_ADDR		"PUSH_ADDR"
#define EASY_NVR_CHANNEL		"CHANNEL"

	TiXmlHandle hDoc(&m_DocR);
	TiXmlHandle hRoot(0);

	unsigned int nGrpIdx = 0;
	TiXmlElement *pGroup = hDoc.FirstChild(EASY_NVR_CONF).FirstChild(EASY_NVR_CHANNEL_GROUP).ToElement();
	for (;pGroup;pGroup=pGroup->NextSiblingElement(EASY_NVR_CHANNEL_GROUP))
	{
		TiXmlElement *pE;

		pE = pGroup->FirstChildElement(EASY_NVR_GROUP_ADDR);
		if (pE && pE->GetText())
		{
			if (NULL != outPushAddr)	strcpy(outPushAddr, pE->GetText());		//推送地址
		}

		TiXmlElement *pChannel = pGroup->FirstChildElement(EASY_NVR_CHANNEL);
		for (;pChannel;pChannel=pChannel->NextSiblingElement(EASY_NVR_CHANNEL))
		{
			TiXmlElement *pE = NULL;

			pE = pChannel->FirstChildElement("ID");
			if (pE && pE->GetText())
			{
				if ( 0 != strcmp(pE->GetText(), inChannelId) )
				{
					continue;
				}
			}

			pE = pChannel->FirstChildElement("URL");
			if (pE && pE->GetText())
			{
				int len = (int)strlen(pE->GetText());
				if (NULL != outUrl)		memcpy(outUrl, pE->GetText(), len>urlSize?urlSize:len);
			}

			pE = pChannel->FirstChildElement("USERNAME");
			if (pE && pE->GetText())
			{
				int len = (int)strlen(pE->GetText());
				if (NULL != outUsername)		memcpy(outUsername, pE->GetText(), len>usernameSize?usernameSize:len);
			}

			pE = pChannel->FirstChildElement("PASSWORD");
			if (pE && pE->GetText())
			{
				int len = (int)strlen(pE->GetText());
				if (NULL != outPassword)		memcpy(outPassword, pE->GetText(), len>passwordSize?passwordSize:len);
			}
		}
		nGrpIdx ++;
	}

	return 0;
}




int VSRM_GetPath(char *filepath, unsigned int size)
{
#ifdef _WIN32
	int nSize = 0, i=0;
	char szPath[MAX_PATH] = {0,};
	GetModuleFileName(NULL, szPath, sizeof(szPath));
	nSize = (int)strlen(szPath);

	i=nSize;
	for (i=nSize; i>0; i--)
	{
		if (szPath[i]=='\\')
		{
			szPath[i] = '\0';
			break;
		}
		szPath[i] = '\0';
	}

	if (i < 1)
	{
		GetCurrentDirectory(MAX_PATH, szPath);
		nSize = (int)strlen(szPath);
		if ((unsigned int)nSize > size)
		{
			printf("VSRM_GetPath::  传入路径长度太小. 无法复制当前路径.");
		}
		else
		{
			strncpy(filepath, szPath, nSize);
		}
	}
	else
	{
		strncpy(filepath, szPath, nSize);
	}

	return 0;
#else
	char path[260] = {0,};
	int cnt = readlink("/proc/self/exe", path, sizeof(path));
	if (cnt < 0 || cnt>=sizeof(path))
	{
		printf("readlink error..\n");
		return -1;
	}
	
	int i=0;
	for (i=cnt; i>=0; --i)
	{
		if (path[i] == '/')
		{
			path[i+1] = '\0';
			break;
		}
	}

	int nSize = (int)strlen(path);
	if ((unsigned int)nSize > size)
	{
		_TRACE_ERR("VSRM_GetPath::  传入路径长度太小. 无法复制当前路径.");
	}
	else
	{
		strncpy(filepath, path, nSize);
		return 0;
	}
	return -1;
#endif
}



int CheckFolder(char *folderName, int bCreate)
{
#ifdef _WIN32
	//检查文件夹是否存在
	WIN32_FIND_DATA wfd;
	HANDLE m_hFile = NULL;
	
	if (NULL == folderName)		return -1;
	
	m_hFile = FindFirstFile(folderName, &wfd);
	if (INVALID_HANDLE_VALUE == m_hFile)	//不存在
	{
		if (bCreate==0x01)
		{
			if (! CreateDirectory(folderName, NULL))	//创建文件夹失败
			{
				//TRACE(szLog, "创建文件夹失败: %s\n", folderName);
				return -1;
			}
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return 0;
	}
#else

	DIR *dir = opendir(folderName);
	if (NULL != dir)
	{
		closedir(dir);
		_TRACE("Path already exist.\n");
		return 0;
	}
	else
	{
		if (bCreate==0x01)
		{
			int err = mkdir(folderName, 0755);
			if (err == 0)
			{
				_TRACE("Path not exist. create success.\n");
			}
			else
			{
				_TRACE("Path not exist. create fail.%d\n", err);
				return -1;
			}
			return 0;
		}
		else
		{
			return -1;
		}
	}

	return 0;
#endif
	return -1;
}




int		tinyxml_WriteChannelList()
{
	char szFile[MAX_PATH_LENGTH] = {0,};
	int ret = VSRM_GetPath(szFile, MAX_PATH_LENGTH);
	if (ret < 0)		return -2;

#ifdef _WIN32
	strcat(szFile, "\\");
#else
	strcat(szFile, "/");
#endif
	strcat(szFile, "EasyNVR.xml");

	char szCharset[16] = {0,};

	strcpy(szCharset, "UTF-8");
	//if (_app_param.language==1001)	strcpy(szCharset, "gb2312");
	//else if (_app_param.language==1000)	strcpy(szCharset, "UTF-8");

	TiXmlDocument xmlDoc( szFile );
	TiXmlDeclaration Declaration( "1.0", szCharset, "yes" );
	xmlDoc.InsertEndChild( Declaration );

	TiXmlElement* pRootElm = NULL;
	pRootElm = new TiXmlElement( "CHANNEL_LIST" );


		TiXmlElement *pGroup = new TiXmlElement("GROUP");
		TiXmlElement *pGroupName = new TiXmlElement("NAME");
		TiXmlElement *pPushAddr = new TiXmlElement("PUSH_ADDR");

		TiXmlText *pName = new TiXmlText("group1");
		TiXmlText *pPushAddrText = new TiXmlText("192.168.1.7");

		pGroupName->InsertEndChild(*pName);
		pGroup->InsertEndChild(*pGroupName);

		pPushAddr->InsertEndChild(*pPushAddrText);
		pGroup->InsertEndChild(*pPushAddr);


			TiXmlElement *pDevice = new TiXmlElement("CHANNEL");
			{
				TiXmlElement *pChannelId = new TiXmlElement("ID");
				TiXmlText* pText = new TiXmlText("1");
				pChannelId->InsertEndChild(*pText);
				pDevice->InsertEndChild(*pChannelId);
				delete pText;
				delete pChannelId;
			}
			{
				TiXmlElement *pChannelId = new TiXmlElement("URL");
				TiXmlText* pText = new TiXmlText("rtsp://192.168.1.186:8557");
				pChannelId->InsertEndChild(*pText);
				pDevice->InsertEndChild(*pChannelId);
				delete pText;
				delete pChannelId;
			}

			pGroup->InsertEndChild(*pDevice);
			delete pDevice;


		pRootElm->InsertEndChild(*pGroup);
		delete pGroup;
		delete pGroupName;
		delete pName;
		delete pPushAddr;
		delete pPushAddrText;
	xmlDoc.InsertEndChild(*pRootElm) ;

	//xmlDoc.Print() ;
	if (xmlDoc.SaveFile())
	{
	}

	delete pRootElm;

	return 0;
}
