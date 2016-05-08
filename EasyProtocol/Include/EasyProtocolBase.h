/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyProtocolBase.h
 * Author: wellsen
 *
 * Created on 2014年11月15日, 下午5:21
 */

#ifndef EASY_PROTOCOL_BASE_H
#define	EASY_PROTOCOL_BASE_H

#include <EasyProtocolDef.h>
#include <json/json.h>
#include <boost/lexical_cast.hpp>

namespace EasyDarwin { namespace Protocol
{

class EasyDarwinUserInfo;
class EasyDarwinDeviceInfo;
/*!
\brief EasyDarwin Protocol base class
\ingroup EasyProtocolBase
*/
class EASYDARWIN_API EasyProtocol
{
public:
	EasyProtocol(int iMsgType);
	EasyProtocol(const std::string msg, int iMsgType = -1);
	virtual ~EasyProtocol();

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
	template <class type>
	bool SetHeaderValue(const char* tag, type value)
	{
		root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_HEADER][tag] = boost::lexical_cast<std::string>(value);
		return true;
	}
	
	template <class type>
	bool SetBodyValue(const char* tag, type value)
	{
		root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY][tag] = boost::lexical_cast<std::string>(value);;
		return true;
	}

	std::string GetHeaderValue(const char* tag);
	std::string GetBodyValue(const char* tag);

	Json::Value * GetRoot(){return &root;}
public:
	static std::string GetErrorString(int error);
	
    static std::string GetMsgTypeString(int type);
    static int GetMsgType(std::string sMessageType);
    
	//enum EasyDarwinDeviceStatus
    static std::string GetDeviceStatusString(int status);
    static int GetDeviceStatus(std::string sStatus);
        
	//enum EasyDarwinPortType
    static int GetPortType(std::string sPortType);
    static std::string GetPortTypeString(int iPortType);

	//enum EasyDarwinProtocolType
	static int GetProtocolType(std::string sProtocolType);
	static std::string GetProtocolString(int iProtocolType);
	
	//enum EasyDarwinEncodeType
	static int GetMediaEncodeType(std::string sMediaEncode);
	static std::string GetMediaEncodeTypeString(int iMediaEncodeType);

	//enum EasyDarwinTerminalType
	static int GetTerminalType(std::string sTerminalType);
	static std::string GetTerminalTypeString(int iTerminalType);

    //common json tag define
protected:  
	Json::Value root;
	//Json::Value header;
	//Json::Value body;
	//Json::Value devices;
	//Json::Value device_list;

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

	Json::Reader reader;
	Json::StyledWriter writer;//or StyledWriter FastWriter
	
};

}}//namespace
#endif	/* EASY_PROTOCOL_BASE_H */