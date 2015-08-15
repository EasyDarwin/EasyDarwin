/*
Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
Github: https://github.com/EasyDarwin
WEChat: EasyDarwin
Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyDSSProtocolBase.cpp
 * Author: wellsen
 * 
 * Created on 2014年11月15日, 下午5:21
 */

#include <EasyDSSProtocolBase.h>
#include <string.h>
#include <stdio.h>

namespace EasyDSS { namespace Protocol
{

EasyDSSProtocol::MsgType EasyDSSProtocol::MsgTypeMap[] = {
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

EasyDSSProtocol::MsgType EasyDSSProtocol::ErrorMap[] = {
	EASYDSS_ERROR_SUCCESS_OK,						"Success OK",
	EASYDSS_ERROR_SUCCESS_CREATED,					"Success Created",
	EASYDSS_ERROR_SUCCESS_ACCEPTED,					"Success Accepted",
	EASYDSS_ERROR_SUCCESS_NO_CONTENT,				"Success No Content",
	EASYDSS_ERROR_SUCCESS_PARTIAL_CONTENT,			"Success Partial Content",
	EASYDSS_ERROR_REDIRECT_PERMANENT_MOVED,			"Redirect Permanent Moved",
	EASYDSS_ERROR_REDIRECT_TEMP_MOVED,				"Redirect Temp Moved",
	EASYDSS_ERROR_REDIRECT_SEE_OTHER,				"Redirect See Other",
	EASYDSS_ERROR_USE_PROXY,						"Use Proxy",
	EASYDSS_ERROR_CLIENT_BAD_REQUEST,				"Client Bad Request",
	EASYDSS_ERROR_CLIENT_UNAUTHORIZED,				"Client Unauthorized",
	EASYDSS_ERROR_PAYMENT_REQUIRED,					"Payment Required",
	EASYDSS_ERROR_CLIENT_FORBIDDEN,					"Client Forbidden",
	EASYDSS_ERROR_NOT_FOUND,						"Not Found",
	EASYDSS_ERROR_METHOD_NOT_ALLOWED,				"Method Not Allowed",
	EASYDSS_ERROR_PROXY_AUTHENTICATION_REQUIRED,	"Proxy Authentication Required",
	EASYDSS_ERROR_REQUEST_TIMEOUT,					"Request Timeout",
	EASYDSS_ERROR_CONFLICT,							"Conflict",
	EASYDSS_ERROR_PRECONDITION_FAILED,				"Precondition Failed",
	EASYDSS_ERROR_UNSUPPORTED_MEDIA_TYPE,			"Unsupported Media Type",
	EASYDSS_ERROR_SERVER_INTERNAL_ERROR,			"Server Internal Error",
	EASYDSS_ERROR_SERVER_NOT_IMPLEMENTED,			"Server Not Implemented",
	EASYDSS_ERROR_SERVER_BAD_GATEWAY,				"Server Bad Gateway",
	EASYDSS_ERROR_SERVER_UNAVAILABLE,				"Server Unavailable",
	EASYDSS_ERROR_RTSP_VERSION_NOT_SUPPORTED,		"RTSP Version Not Supported",
	EASYDSS_ERROR_DEVICE_VERSION_TOO_OLD,			"Device Version Too Old",
	EASYDSS_ERROR_DEVICE_FAILURE,					"Device Failure",
	EASYDSS_ERROR_MEMCACHE_NOT_FOUND,				"Memcache Not Found",
	EASYDSS_ERROR_DATABASE_NOT_FOUND,				"Database Not Found",
	EASYDSS_ERROR_USER_NOT_FOUND,					"User Not Found",
	EASYDSS_ERROR_DEVICE_NOT_FOUND,					"Device Not Found",
	EASYDSS_ERROR_SESSION_NOT_FOUND,				"Session Not Found",
	EASYDSS_ERROR_SERVICE_NOT_FOUND,				"Service Not Found",
	EASYDSS_ERROR_PASSWORD_ERROR,					"Password Error",
	EASYDSS_ERROR_XML_PARSE_ERROR,					"XML Parse Error",
	EASYDSS_ERROR_PERMISSION_ERROR,					"Permission Error",
	EASYDSS_ERROR_LOCAL_SYSTEM_ERROR,               "Local System Error",
	EASYDSS_ERROR_PARAM_ERROR,                      "Param Error"
};

EasyDSSProtocol::MsgType EasyDSSProtocol::StatusMap[] = {
    EASYDSS_DEVICE_STATUS_OFFLINE,                  "DEVICE_STATUS_OFFLINE",
    EASYDSS_DEVICE_STATUS_ONLINE,                   "DEVICE_STATUS_ONLINE"
};

EasyDSSProtocol::MsgType EasyDSSProtocol::ProtocolTypeMap[] = {
	EASYDSS_PROTOCOL_TYPE_RTSP,						"RTSP",
	EASYDSS_PROTOCOL_TYPE_HLS,						"HLS"
};

EasyDSSProtocol::MsgType EasyDSSProtocol::MediaEncodeTypeMap[] = {
	EASYDSS_MEDIA_ENCODE_AUDIO_AAC,                 "AAC",
	EASYDSS_MEDIA_ENCODE_VIDEO_H264,                "H264"
};


EasyDSSProtocol::MsgType EasyDSSProtocol::TerminalTypeMap[] = {
	EASYDSS_TERMINAL_TYPE_CAMERA,						"Camera"
};

EasyDSSProtocol::EasyDSSProtocol(int iMsgType)
:fMsgType(iMsgType)
{	
}

EasyDSSProtocol::EasyDSSProtocol(const std::string msg, int iMsgType)
{	
	json = msg;
	
	if(iMsgType != -1)
    {
        fMsgType = iMsgType;
    }
    else
    {
        fMsgType = GetMessageType();
    }
}


EasyDSSProtocol::~EasyDSSProtocol()
{
}

void EasyDSSProtocol::Read(const std::string msg, int iMsgType)
{
	json = msg;

	if(iMsgType != -1)
	{
		fMsgType = iMsgType;
	}
	else
	{
		fMsgType = GetMessageType();
	}
}

void EasyDSSProtocol::Reset()
{
    root.Clear();	
    fMsgType = -1;
}


void EasyDSSProtocol::SetMsgType(int type)
{
    fMsgType = type;
}


int EasyDSSProtocol::GetMsgType()
{
    return fMsgType;
}

std::string EasyDSSProtocol::GetMsgTypeStr()
{
	return GetMsgTypeString(fMsgType);
}

std::string EasyDSSProtocol::GetMsgTypeString(int type)
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

int EasyDSSProtocol::GetMessageType()
{
	std::string sMessageType;
	AVSXmlUtil xml;
	if (!xml.Read(json))
	{
		printf("AVSXmlUtil read xml errror\n");
	}

	AVSXmlObject obj = xml.GetChild("EasyDarwin");
	if (obj == NULL)
	{
		printf("not found EasyDarwin\n");
		return -1;
	}

	AVSXmlUtil easydarwin(obj);

	obj = easydarwin.GetChild("Header");
	if (obj == NULL)
	{
		printf("not found Header\n");
		return -1;
	}
	AVSXmlUtil header_(obj);
	header_.GetValueAsString(EASYDSS_TAG_MESSAGE_TYPE, sMessageType);
	return GetMsgType(sMessageType);
}

std::string EasyDSSProtocol::GetErrorString(int error)
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

int EasyDSSProtocol::GetMsgType(std::string sMessageType)
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

std::string EasyDSSProtocol::GetDeviceStatusString(int status)
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

int EasyDSSProtocol::GetDeviceStatus(std::string sStatus)
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

int EasyDSSProtocol::GetProtocolType(std::string sProtocolType)
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

std::string EasyDSSProtocol::GetProtocolString(int iProtocolType)
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

int EasyDSSProtocol::GetMediaEncodeType(std::string sMediaEncode)
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

std::string EasyDSSProtocol::GetMediaEncodeTypeString(int iMediaEncodeType)
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

int EasyDSSProtocol::GetTerminalType(std::string sTerminalType)
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

std::string EasyDSSProtocol::GetTerminalTypeString(int iTerminalType)
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

std::string EasyDSSProtocol::GetMsg()
{
    std::string msg;
	

	header.SetStringValue(EASYDSS_TAG_MESSAGE_TYPE, GetMsgTypeString(fMsgType).c_str());
	
	root.AddChild(EASYDSS_TAG_ROOT, EASYDSS_TAG_HEADER, header);
	if(!body.IsEmpty())
	{
		if(!devices.IsEmpty())
		{
			body.Add("Devices", devices);
		}
		root.AddChild(EASYDSS_TAG_ROOT, EASYDSS_TAG_BODY, body);
	}
	root.Write(msg);
    return msg;
}

bool EasyDSSProtocol::GetMsg(char *dest, int size)
{
	std::string msg = GetMsg();
	if (msg.empty() || msg.size() > size)
	{
		return false;
	}
	strncpy(dest, msg.c_str(), size);
	
	return true;
}


bool EasyDSSProtocol::SetHeaderValue(const char* tag, const char* value)
{
	return header.SetStringValue(tag, value);
}


bool EasyDSSProtocol::SetBodyValue(const char* tag, const char* value)
{
	return body.SetStringValue(tag, value);
}

std::string EasyDSSProtocol::GetHeaderValue(const char* tag)
{
	std::string value;
		
	AVSXmlUtil xml;
	if (!xml.Read(json))
	{
		printf("AVSXmlUtil read xml errror\n");
	}

	AVSXmlObject obj = xml.GetChild("EasyDarwin");
	if (obj == NULL)
	{
		printf("not found EasyDarwin\n");
		return "";
	}

	AVSXmlUtil easydarwin(obj);

	obj = easydarwin.GetChild("Header");
	if (obj == NULL)
	{
		printf("not found Header\n");
		return "";
	}
	AVSXmlUtil header_(obj);
	header_.GetValueAsString(tag, value);
	return value;
}

std::string EasyDSSProtocol::GetBodyValue(const char* tag)
{
	std::string value;
	AVSXmlUtil xml;
	if (!xml.Read(json))
	{
		printf("AVSXmlUtil read xml errror\n");
	}

	AVSXmlObject obj = xml.GetChild("EasyDarwin");
	if (obj == NULL)
	{
		printf("not found EasyDarwin\n");
		return "";
	}

	AVSXmlUtil easydarwin(obj);

	obj = easydarwin.GetChild("Body");
	if (obj == NULL)
	{
		printf("not found Header\n");
		return "";
	}
	AVSXmlUtil body_(obj);
	body_.GetValueAsString(tag, value);	
	return value;
}

}}//namespace



