/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
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
class Easy_API EasyProtocol
{
public:
	EasyProtocol(int iMsgType);
	EasyProtocol(const std::string& msg, int iMsgType = -1);
	virtual ~EasyProtocol();

	struct MsgType
	{
		int value;
		char str[512];
	};

public:
	void Read(const std::string& msg, int iMsgType = -1);

	std::string GetMsg();
	bool GetMsg(char *dest, int size);

	void SetMsgType(int type);
	int GetMsgType() const;
	std::string GetMsgTypeStr() const;
	int GetMessageType();

	int GetAppType();
	int GetTerminalType();

	int GetMsgCSeq();

	std::string GetSerialNumber();
	void Reset();

public:
	template <class type>
	bool SetHeaderValue(const char* tag, type value)
	{
		root[EASY_TAG_ROOT][EASY_TAG_HEADER][tag] = boost::lexical_cast<std::string>(value);
		return true;
	}

	template <class type>
	bool SetBodyValue(const char* tag, type value)
	{
		root[EASY_TAG_ROOT][EASY_TAG_BODY][tag] = boost::lexical_cast<std::string>(value);
		return true;
	}

	std::string GetHeaderValue(const char* tag);
	std::string GetBodyValue(const char* tag);

	Json::Value* GetRoot() { return &root; }
public:
	static std::string GetErrorString(int error);

	static std::string GetMsgTypeString(int type);
	static int GetMsgType(const std::string& sMessageType);

	//enum EasyDarwinDeviceStatus
	static std::string GetDeviceStatusString(int status);
	static int GetDeviceStatus(const std::string& sStatus);

	//enum EasyDarwinPortType
	static int GetPortType(const std::string& sPortType);
	static std::string GetPortTypeString(int iPortType);

	//enum EasyDarwinProtocolType
	static int GetProtocolType(const std::string& sProtocolType);
	static std::string GetProtocolString(int iProtocolType);

	//enum EasyDarwinEncodeType
	static int GetMediaEncodeType(const std::string& sMediaEncode);
	static std::string GetMediaEncodeTypeString(int iMediaEncodeType);

	//enum EasyDarwinTerminalType
	static int GetTerminalType(const std::string& sTerminalType);
	static std::string GetTerminalTypeString(int iTerminalType);

	//enum EasyDarwinAppType
	static int GetAppType(const std::string& sAppType);
	static std::string GetAppTypeString(int iAppType);

	//enum EasyDarwinSnapType
	static int GetSnapType(const std::string& sSnapType);
	static std::string GetSnapTypeString(int iSnapType);

	//enum EasyDarwinPTZActionType
	static int GetPTZActionType(const std::string& sPTZActionType);
	static std::string GetPTZActionTypeString(int iPTZActionType);

	//enum EasyDarwinPTZCMDType
	static int GetPTZCMDType(const std::string& sPTZCMDType);
	static std::string GetPTZCMDTypeString(int iPTZCMDType);

	//enum EasyDarwinPresetCMDType
	static int GetPresetCMDType(const std::string& sPresetCMDType);
	static std::string GetPresetCMDTypeString(int iPresetCMDType);

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
	static MsgType AppTypeMap[];
	static MsgType LiveTypeMap[];
	static MsgType SnapTypeMap[];
	static MsgType PTZActionTypeMap[];
	static MsgType PTZCMDTypeMap[];
	static MsgType PresetCMDTypeMap[];

	Json::Reader reader;
	Json::StyledWriter writer;//or StyledWriter FastWriter

};

}
}//namespace
#endif	/* EASY_PROTOCOL_BASE_H */