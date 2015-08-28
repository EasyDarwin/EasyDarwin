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
	MSG_DEV_CMS_REGISTER_REQ,						"MSG_DEV_CMS_REGISTER_REQ",
	MSG_DEV_CMS_REGISTER_RSP,						"MSG_DEV_CMS_REGISTER_RSP",
	MSG_CMS_DEV_STREAM_START_REQ,					"MSG_CMS_DEV_STREAM_START_REQ",
	MSG_CMS_DEV_STREAM_START_RSP,					"MSG_CMS_DEV_STREAM_START_RSP",
	MSG_CMS_DEV_STREAM_STOP_REQ,					"MSG_CMS_DEV_STREAM_STOP_REQ",
	MSG_CMS_DEV_STREAM_STOP_RSP,					"MSG_CMS_DEV_STREAM_STOP_RSP",
	MSG_NGX_CMS_NEED_STREAM_REQ,					"MSG_NGX_CMS_NEED_STREAM_REQ",
	MSG_NGX_CMS_NEED_STREAM_RSP,					"MSG_NGX_CMS_NEED_STREAM_RSP",
	MSG_CLI_CMS_DEVICE_LIST_REQ,					"MSG_CLI_CMS_DEVICE_LIST_REQ",
	MSG_CLI_CMS_DEVICE_LIST_RSP,					"MSG_CLI_CMS_DEVICE_LIST_RSP",
	MSG_DEV_CMS_SNAP_UPDATE_REQ,					"MSG_DEV_CMS_SNAP_UPDATE_REQ",
	MSG_DEV_CMS_SNAP_UPDATE_RSP,					"MSG_DEV_CMS_SNAP_UPDATE_RSP"
};

EasyProtocol::MsgType EasyProtocol::ErrorMap[] = {
	EASYDARWIN_ERROR_SUCCESS_OK,						"Success OK",
	EASYDARWIN_ERROR_SUCCESS_CREATED,					"Success Created",
	EASYDARWIN_ERROR_SUCCESS_ACCEPTED,					"Success Accepted",
	EASYDARWIN_ERROR_SUCCESS_NO_CONTENT,				"Success No Content",
	EASYDARWIN_ERROR_SUCCESS_PARTIAL_CONTENT,			"Success Partial Content",
	EASYDARWIN_ERROR_REDIRECT_PERMANENT_MOVED,			"Redirect Permanent Moved",
	EASYDARWIN_ERROR_REDIRECT_TEMP_MOVED,				"Redirect Temp Moved",
	EASYDARWIN_ERROR_REDIRECT_SEE_OTHER,				"Redirect See Other",
	EASYDARWIN_ERROR_USE_PROXY,							"Use Proxy",
	EASYDARWIN_ERROR_CLIENT_BAD_REQUEST,				"Client Bad Request",
	EASYDARWIN_ERROR_CLIENT_UNAUTHORIZED,				"Client Unauthorized",
	EASYDARWIN_ERROR_PAYMENT_REQUIRED,					"Payment Required",
	EASYDARWIN_ERROR_CLIENT_FORBIDDEN,					"Client Forbidden",
	EASYDARWIN_ERROR_NOT_FOUND,							"Not Found",
	EASYDARWIN_ERROR_METHOD_NOT_ALLOWED,				"Method Not Allowed",
	EASYDARWIN_ERROR_PROXY_AUTHENTICATION_REQUIRED,		"Proxy Authentication Required",
	EASYDARWIN_ERROR_REQUEST_TIMEOUT,					"Request Timeout",
	EASYDARWIN_ERROR_CONFLICT,							"Conflict",
	EASYDARWIN_ERROR_PRECONDITION_FAILED,				"Precondition Failed",
	EASYDARWIN_ERROR_UNSUPPORTED_MEDIA_TYPE,			"Unsupported Media Type",
	EASYDARWIN_ERROR_SERVER_INTERNAL_ERROR,				"Server Internal Error",
	EASYDARWIN_ERROR_SERVER_NOT_IMPLEMENTED,			"Server Not Implemented",
	EASYDARWIN_ERROR_SERVER_BAD_GATEWAY,				"Server Bad Gateway",
	EASYDARWIN_ERROR_SERVER_UNAVAILABLE,				"Server Unavailable",
	EASYDARWIN_ERROR_RTSP_VERSION_NOT_SUPPORTED,		"RTSP Version Not Supported",
	EASYDARWIN_ERROR_DEVICE_VERSION_TOO_OLD,			"Device Version Too Old",
	EASYDARWIN_ERROR_DEVICE_FAILURE,					"Device Failure",
	EASYDARWIN_ERROR_MEMCACHE_NOT_FOUND,				"Memcache Not Found",
	EASYDARWIN_ERROR_DATABASE_NOT_FOUND,				"Database Not Found",
	EASYDARWIN_ERROR_USER_NOT_FOUND,					"User Not Found",
	EASYDARWIN_ERROR_DEVICE_NOT_FOUND,					"Device Not Found",
	EASYDARWIN_ERROR_SESSION_NOT_FOUND,					"Session Not Found",
	EASYDARWIN_ERROR_SERVICE_NOT_FOUND,					"Service Not Found",
	EASYDARWIN_ERROR_PASSWORD_ERROR,					"Password Error",
	EASYDARWIN_ERROR_XML_PARSE_ERROR,					"XML Parse Error",
	EASYDARWIN_ERROR_PERMISSION_ERROR,					"Permission Error",
	EASYDARWIN_ERROR_LOCAL_SYSTEM_ERROR,				"Local System Error",
	EASYDARWIN_ERROR_PARAM_ERROR,						"Param Error"
};

EasyProtocol::MsgType EasyProtocol::StatusMap[] = {
    EASYDARWIN_DEVICE_STATUS_OFFLINE,					"DEVICE_STATUS_OFFLINE",
    EASYDARWIN_DEVICE_STATUS_ONLINE,					"DEVICE_STATUS_ONLINE"
};

EasyProtocol::MsgType EasyProtocol::ProtocolTypeMap[] = {
	EASYDARWIN_PROTOCOL_TYPE_RTSP,						"RTSP",
	EASYDARWIN_PROTOCOL_TYPE_HLS,						"HLS"
};

EasyProtocol::MsgType EasyProtocol::MediaEncodeTypeMap[] = {
	EASYDARWIN_MEDIA_ENCODE_AUDIO_AAC,					"AAC",
	EASYDARWIN_MEDIA_ENCODE_VIDEO_H264,					"H264"
};


EasyProtocol::MsgType EasyProtocol::TerminalTypeMap[] = {
	EASYDARWIN_TERMINAL_TYPE_CAMERA,					"Camera"
};

EasyProtocol::EasyProtocol(int iMsgType)
:fMsgType(iMsgType)
{	
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
	std::string sMessageType = root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_HEADER][EASYDARWIN_TAG_MESSAGE_TYPE].asString();	

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

std::string EasyProtocol::GetMsg()
{   	
	root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_HEADER][EASYDARWIN_TAG_MESSAGE_TYPE] = GetMsgTypeString(fMsgType);	
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


bool EasyProtocol::SetHeaderValue(const char* tag, const char* value){
	
	root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_HEADER][tag] = value;
	return true;
}


bool EasyProtocol::SetBodyValue(const char* tag, const char* value){
	
	root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY][tag] = value;
	return true;
}

std::string EasyProtocol::GetHeaderValue(const char* tag)
{
	return root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_HEADER][tag].asString();
}

std::string EasyProtocol::GetBodyValue(const char* tag)
{
	return root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY][tag].asString();	
}

}}//namespace



