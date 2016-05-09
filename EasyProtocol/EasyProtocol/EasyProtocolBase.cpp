/*
Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
Github: https://github.com/EasyDarwin
WEChat: EasyDarwin
Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyProtocolBase.cpp
 * Author: wellsen
 * 
 * Created on 2014年11月15日, 下午5:21
*/

#include <EasyProtocolBase.h>
#include <string.h>
#include <stdio.h>

namespace EasyDarwin { namespace Protocol
{

EasyProtocol::MsgType EasyProtocol::MsgTypeMap[] = {
	MSG_DS_REGISTER_REQ,							"MSG_DS_REGISTER_REQ",
	MSG_SD_REGISTER_ACK,							"MSG_SD_REGISTER_ACK",
	MSG_SD_PUSH_STREAM_REQ,							"MSG_SD_PUSH_STREAM_REQ",
	MSG_DS_PUSH_STREAM_ACK,							"MSG_DS_PUSH_STREAM_ACK",
	MSG_SD_STREAM_STOP_REQ,							"MSG_SD_STREAM_STOP_REQ",
	MSG_DS_STREAM_STOP_ACK,							"MSG_DS_STREAM_STOP_ACK",
	MSG_CS_DEVICE_LIST_REQ,							"MSG_CS_DEVICE_LIST_REQ",
	MSG_SC_DEVICE_LIST_ACK,							"MSG_SC_DEVICE_LIST_ACK",
	MSG_CS_CAMERA_LIST_REQ,							"MSG_CS_CAMERA_LIST_REQ",
	MSG_SC_CAMERA_LIST_ACK,							"MSG_SC_CAMERA_LIST_ACK",
	MSG_CS_GET_STREAM_REQ,							"MSG_CS_GET_STREAM_REQ",
	MSG_SC_GET_STREAM_ACK,							"MSG_SC_GET_STREAM_ACK",
	MSG_CS_FREE_STREAM_REQ,							"MSG_CS_FREE_STREAM_REQ",
	MSG_SC_FREE_STREAM_ACK,							"MSG_SC_FREE_STREAM_ACK",
	MSG_DS_POST_SNAP_REQ,							"MSG_DS_POST_SNAP_REQ",
	MSG_SD_POST_SNAP_ACK,							"MSG_SD_POST_SNAP_ACK",

	//保留
	MSG_SC_START_HLS_ACK,							"MSG_SC_START_HLS_ACK",
	MSG_SC_HLS_SESSION_LIST_ACK,					"MSG_SC_HLS_SESSION_LIST_ACK",
	MSG_SC_RTSP_PUSH_SESSION_LIST_ACK,				"MSG_SC_RTSP_PUSH_SESSION_LIST_ACK",
	//保留
	MSG_SC_EXCEPTION,								"MSG_SC_EXCEPTION",

};
EasyProtocol::MsgType EasyProtocol::ErrorMap[] = {
	EASY_ERROR_SUCCESS_OK,							"Success OK",
	EASY_ERROR_SUCCESS_CREATED,						"Success Created",
	EASY_ERROR_SUCCESS_ACCEPTED,					"Success Accepted",
	EASY_ERROR_SUCCESS_NO_CONTENT,					"Success No Content",
	EASY_ERROR_SUCCESS_PARTIAL_CONTENT,				"Success Partial Content",
	EASY_ERROR_REDIRECT_PERMANENT_MOVED,			"Redirect Permanent Moved",
	EASY_ERROR_REDIRECT_TEMP_MOVED,					"Redirect Temp Moved",
	EASY_ERROR_REDIRECT_SEE_OTHER,					"Redirect See Other",
	EASY_ERROR_USE_PROXY,							"Use Proxy",
	EASY_ERROR_CLIENT_BAD_REQUEST,					"Client Bad Request",
	EASY_ERROR_CLIENT_UNAUTHORIZED,					"Client Unauthorized",
	EASY_ERROR_PAYMENT_REQUIRED,					"Payment Required",
	EASY_ERROR_CLIENT_FORBIDDEN,					"Client Forbidden",
	EASY_ERROR_NOT_FOUND,							"Not Found",
	EASY_ERROR_METHOD_NOT_ALLOWED,					"Method Not Allowed",
	EASY_ERROR_PROXY_AUTHENTICATION_REQUIRED,		"Proxy Authentication Required",
	EASY_ERROR_REQUEST_TIMEOUT,						"Request Timeout",
	EASY_ERROR_CONFLICT,							"Conflict",
	EASY_ERROR_PRECONDITION_FAILED,					"Precondition Failed",
	EASY_ERROR_UNSUPPORTED_MEDIA_TYPE,				"Unsupported Media Type",
	EASY_ERROR_SERVER_INTERNAL_ERROR,				"Server Internal Error",
	EASY_ERROR_SERVER_NOT_IMPLEMENTED,				"Server Not Implemented",
	EASY_ERROR_SERVER_BAD_GATEWAY,					"Server Bad Gateway",
	EASY_ERROR_SERVER_UNAVAILABLE,					"Server Unavailable",
	EASY_ERROR_RTSP_VERSION_NOT_SUPPORTED,			"RTSP Version Not Supported",
	EASY_ERROR_DEVICE_VERSION_TOO_OLD,				"Device Version Too Old",
	EASY_ERROR_DEVICE_FAILURE,						"Device Failure",
	EASY_ERROR_MEMCACHE_NOT_FOUND,					"Memcache Not Found",
	EASY_ERROR_DATABASE_NOT_FOUND,					"Database Not Found",
	EASY_ERROR_USER_NOT_FOUND,						"User Not Found",
	EASY_ERROR_DEVICE_NOT_FOUND,					"Device Not Found",
	EASY_ERROR_SESSION_NOT_FOUND,					"Session Not Found",
	EASY_ERROR_SERVICE_NOT_FOUND,					"Service Not Found",
	EASY_ERROR_PASSWORD_ERROR,						"Password Error",
	EASY_ERROR_XML_PARSE_ERROR,						"XML Parse Error",
	EASY_ERROR_PERMISSION_ERROR,					"Permission Error",
	EASY_ERROR_LOCAL_SYSTEM_ERROR,					"Local System Error",
	EASY_ERROR_PARAM_ERROR,							"Param Error"
};

EasyProtocol::MsgType EasyProtocol::StatusMap[] = {
    EASY_DEVICE_STATUS_OFFLINE,						"DEVICE_STATUS_OFFLINE",
    EASY_DEVICE_STATUS_ONLINE,						"DEVICE_STATUS_ONLINE"
};

EasyProtocol::MsgType EasyProtocol::ProtocolTypeMap[] = {
	EASY_PROTOCOL_TYPE_RTSP,						"RTSP",
	EASY_PROTOCOL_TYPE_HLS,							"HLS"
};

EasyProtocol::MsgType EasyProtocol::MediaEncodeTypeMap[] = {
	EASY_MEDIA_ENCODE_AUDIO_AAC,					"AAC",
	EASY_MEDIA_ENCODE_VIDEO_H264,					"H264"
};

EasyProtocol::MsgType EasyProtocol::TerminalTypeMap[] = {
	EASY_TERMINAL_TYPE_ARM,							"ARM",	
	EASY_TERMINAL_TYPE_Android,						"ANDROID",
	EASY_TERMINAL_TYPE_IOS,							"IOS",
	EASY_TERMINAL_TYPE_WEB,							"WEB",
	EASY_TERMINAL_TYPE_PC,							"PC"

};

EasyProtocol::MsgType EasyProtocol::AppTypeMap[] = {
	EASY_APP_TYPE_CAMERA,							"EasyCamera",
	EASY_APP_TYPE_NVR,								"EasyNVR"
};

EasyProtocol::EasyProtocol(int iMsgType)
:fMsgType(iMsgType)
{	
	SetHeaderValue(EASY_TAG_VERSION, "1.0");
	root[EASY_TAG_ROOT][EASY_TAG_HEADER][EASY_TAG_MESSAGE_TYPE] = GetMsgTypeString(fMsgType);	
}

EasyProtocol::EasyProtocol(const std::string msg, int iMsgType)
{	
	reader.parse(msg, root);
	//json = msg;
	
	if(iMsgType != -1)
    {
        fMsgType = iMsgType;
    }
    else
    {
        fMsgType = GetMessageType();
    }
}


EasyProtocol::~EasyProtocol()
{
}

void EasyProtocol::Read(const std::string msg, int iMsgType)
{
	reader.parse(msg, root);

	if(iMsgType != -1)
	{
		fMsgType = iMsgType;
	}
	else
	{
		fMsgType = GetMessageType();
	}
}

void EasyProtocol::Reset()
{
	root.clear();	
    fMsgType = -1;
}


void EasyProtocol::SetMsgType(int type)
{
    fMsgType = type;
}


int EasyProtocol::GetMsgType()
{
    return fMsgType;
}

std::string EasyProtocol::GetMsgTypeStr()
{
	return GetMsgTypeString(fMsgType);
}

std::string EasyProtocol::GetMsgTypeString(int type)
{
	for (int i = 0; i < sizeof(MsgTypeMap) / sizeof(MsgType); i++)
	{
		if (type == MsgTypeMap[i].value)
		{
			return std::string(MsgTypeMap[i].str);
		}
	}

	return std::string();
}

int EasyProtocol::GetMessageType()
{
	std::string sMessageType = root[EASY_TAG_ROOT][EASY_TAG_HEADER][EASY_TAG_MESSAGE_TYPE].asString();	

	return GetMsgType(sMessageType);
}

std::string EasyProtocol::GetErrorString(int error)
{
	for (int i = 0; i < sizeof(ErrorMap) / sizeof(MsgType); i++)
	{
		if (error == ErrorMap[i].value)
		{
			return std::string(ErrorMap[i].str);
		}
	}

	return std::string();
}

int EasyProtocol::GetMsgType(std::string sMessageType)
{
    for (int i = 0; i < sizeof(MsgTypeMap) / sizeof(MsgType); i++)
	{
		if (sMessageType.compare(MsgTypeMap[i].str) == 0)
		{
			return MsgTypeMap[i].value;
		}
	}
    
    return -1;
}

std::string EasyProtocol::GetDeviceStatusString(int status)
{
    for (int i = 0; i < sizeof(StatusMap) / sizeof(MsgType); i++)
	{
		if (status == StatusMap[i].value)
		{
			return std::string(StatusMap[i].str);
		}
	}

	return std::string();
}

int EasyProtocol::GetDeviceStatus(std::string sStatus)
{
    for (int i = 0; i < sizeof(StatusMap) / sizeof(MsgType); i++)
	{
		if (sStatus.compare(StatusMap[i].str) == 0)
		{
			return StatusMap[i].value;
		}
	}
    
    return -1;
}

int EasyProtocol::GetProtocolType(std::string sProtocolType)
{
	for (int i = 0; i < sizeof(ProtocolTypeMap) / sizeof(MsgType); i++)
	{
		if (sProtocolType.compare(ProtocolTypeMap[i].str) == 0)
		{
			return ProtocolTypeMap[i].value;
		}
	}

	return -1;
}

std::string EasyProtocol::GetProtocolString(int iProtocolType)
{
	for (int i = 0; i < sizeof(ProtocolTypeMap) / sizeof(MsgType); i++)
	{
		if (iProtocolType == ProtocolTypeMap[i].value)
		{
			return std::string(ProtocolTypeMap[i].str);
		}
	}

	return std::string();
}

int EasyProtocol::GetMediaEncodeType(std::string sMediaEncode)
{
	for (int i = 0; i < sizeof(MediaEncodeTypeMap) / sizeof(MsgType); i++)
	{
		if (sMediaEncode.compare(MediaEncodeTypeMap[i].str) == 0)
		{
			return MediaEncodeTypeMap[i].value;
		}
	}

	return -1;
}

std::string EasyProtocol::GetMediaEncodeTypeString(int iMediaEncodeType)
{
	for (int i = 0; i < sizeof(MediaEncodeTypeMap) / sizeof(MsgType); i++)
	{
		if (iMediaEncodeType == MediaEncodeTypeMap[i].value)
		{
			return std::string(MediaEncodeTypeMap[i].str);
		}
	}

	return std::string();
}

int EasyProtocol::GetTerminalType(std::string sTerminalType)
{
	for (int i = 0; i < sizeof(TerminalTypeMap) / sizeof(MsgType); i++)
	{
		if (sTerminalType.compare(TerminalTypeMap[i].str) == 0)
		{
			return TerminalTypeMap[i].value;
		}
	}

	return -1;
}

std::string EasyProtocol::GetTerminalTypeString(int iTerminalType)
{
	for (int i = 0; i < sizeof(TerminalTypeMap) / sizeof(MsgType); i++)
	{
		if (iTerminalType == TerminalTypeMap[i].value)
		{
			return std::string(TerminalTypeMap[i].str);
		}
	}

	return std::string();
}


int EasyProtocol::GetAppType(std::string sAppType)
{
	for (int i = 0; i < sizeof(AppTypeMap) / sizeof(MsgType); i++)
	{
		if (sAppType.compare(AppTypeMap[i].str) == 0)
		{
			return AppTypeMap[i].value;
		}
	}

	return -1;
}


std::string EasyProtocol::GetAppTypeString(int iAppType)
{
	for (int i = 0; i < sizeof(AppTypeMap) / sizeof(MsgType); i++)
	{
		if (iAppType == AppTypeMap[i].value)
		{
			return std::string(AppTypeMap[i].str);
		}
	}

	return std::string();
}


std::string EasyProtocol::GetMsg()
{   		
    return writer.write(root);;
}

bool EasyProtocol::GetMsg(char *dest, int size)
{
	std::string msg = GetMsg();
	if (msg.empty() || msg.size() > size)
	{
		return false;
	}
	strncpy(dest, msg.c_str(), size);
	
	return true;
}

/*
bool EasyProtocol::SetHeaderValue(const char* tag, const char* value){
	
	root[EASY_TAG_ROOT][EASY_TAG_HEADER][tag] = value;
	return true;
}


bool EasyProtocol::SetBodyValue(const char* tag, const char* value){
	
	root[EASY_TAG_ROOT][EASY_TAG_BODY][tag] = value;
	return true;
}*/

std::string EasyProtocol::GetHeaderValue(const char* tag)
{
	return root[EASY_TAG_ROOT][EASY_TAG_HEADER][tag].asString();
}

std::string EasyProtocol::GetBodyValue(const char* tag)
{
	return root[EASY_TAG_ROOT][EASY_TAG_BODY][tag].asString();	
}

}}//namespace



