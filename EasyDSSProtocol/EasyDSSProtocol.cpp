/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#include <EasyDSSProtocol.h>
#include <string.h>
#include <stdio.h>
#include <EasyDSSUtil.h>

namespace EasyDSS { namespace Protocol
{

EasyDarwinRegisterReq::EasyDarwinRegisterReq()
: EasyDSSProtocol(MSG_DEV_CMS_REGISTER_REQ)
{

}

EasyDarwinRegisterReq::EasyDarwinRegisterReq(const char* msg)
: EasyDSSProtocol(msg, MSG_DEV_CMS_REGISTER_REQ)
{

}

std::string EasyDarwinRegisterReq::GetSerialNumber()
{
	return GetBodyValue("SerialNumber");
}

std::string EasyDarwinRegisterReq::GetAuthCode()
{
	return GetBodyValue("AuthCode");
}

EasyDarwinRegisterRsp::EasyDarwinRegisterRsp()
: EasyDSSProtocol(MSG_DEV_CMS_REGISTER_RSP)
{

}

EasyDarwinRegisterRsp::EasyDarwinRegisterRsp(const char* msg)
: EasyDSSProtocol(msg, MSG_DEV_CMS_REGISTER_RSP)
{

}

}}//namespace

