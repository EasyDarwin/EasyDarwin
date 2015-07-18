/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyDSSProtocolBase.h
 * Author: wellsen
 *
 * Created on 2014年11月15日, 下午5:21
 */

#ifndef EASYDSS_PROTOCOL_BASE_H
#define	EASYDSS_PROTOCOL_BASE_H

#include <EasyDSSProtocolDef.h>
#include <AVSXmlUtil.h>

namespace EasyDSS { namespace Protocol
{

class EasyDSSUserInfo;
class EasyDSSDeviceInfo;
/*!
\brief EasyDSS Protocol base class
\ingroup EasyDSSProtocolBase
*/
class EASYDSS_API EasyDSSProtocol
{
public:
	EasyDSSProtocol(int iMsgType);
	EasyDSSProtocol(const std::string msg, int iMsgType = -1);
	virtual ~EasyDSSProtocol();

	struct MsgType
	{
		int value;
		char str[512];
	};

public:
	void Read(const std::string msg, int iMsgType = -1);

	std::string GetMsg();
	bool GetMsg(char *dest, int size);
    
    void SetMsgType(int type);
    int GetMsgType();    
    std::string GetMsgTypeStr();
	int GetMessageType();

	std::string GetSerialNumber();
    void Reset();
    
public:
	bool SetHeaderValue(const char* tag, const char* value);
	bool SetBodyValue(const char* tag, const char* value);

	std::string GetHeaderValue(const char* tag);
	std::string GetBodyValue(const char* tag);

public:
	static std::string GetErrorString(int error);
	
    static std::string GetMsgTypeString(int type);
    static int GetMsgType(std::string sMessageType);
    
	//enum EasyDSSDeviceStatus
    static std::string GetDeviceStatusString(int status);
    static int GetDeviceStatus(std::string sStatus);
        
	//enum EasyDSSPortType
    static int GetPortType(std::string sPortType);
    static std::string GetPortTypeString(int iPortType);

	//enum EasyDSSProtocolType
	static int GetProtocolType(std::string sProtocolType);
	static std::string GetProtocolString(int iProtocolType);
	
	//enum EasyDSSEncodeType
	static int GetMediaEncodeType(std::string sMediaEncode);
	static std::string GetMediaEncodeTypeString(int iMediaEncodeType);

	//enum EasyDSSTerminalType
	static int GetTerminalType(std::string sTerminalType);
	static std::string GetTerminalTypeString(int iTerminalType);

    //common xml tag define
protected:
    AVSXmlUtil root;	
	AVSXmlUtil header;
	AVSXmlUtil body;

	std::string json;
private:	
    int fMsgType;
	static MsgType MsgTypeMap[];
	static MsgType ErrorMap[];
    static MsgType StatusMap[];
    static MsgType ServiceMap[];
	static MsgType ProtocolTypeMap[];
	static MsgType MediaEncodeTypeMap[];
	static MsgType DeviceTypeMap[];
	static MsgType TerminalTypeMap[];
	static MsgType LiveTypeMap[];
};

}}//namespace
#endif	/* EASYDSS_PROTOCOL_BASE_H */

