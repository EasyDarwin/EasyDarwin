/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/


#ifndef EASYDSS_PROTOCOL
#define	EASYDSS_PROTOCOL

#include <EasyDSSProtocolBase.h>

namespace EasyDSS { namespace Protocol
{

class EasyDarwinRegisterReq : public EasyDSSProtocol
{
public:
	EasyDarwinRegisterReq();
	EasyDarwinRegisterReq(const char* msg);
	virtual ~EasyDarwinRegisterReq(){}

public:
	std::string GetSerialNumber();	
    std::string GetAuthCode();

};

class EasyDarwinRegisterRsp : public EasyDSSProtocol
{
public:
	EasyDarwinRegisterRsp();
	EasyDarwinRegisterRsp(const char* msg);
	virtual ~EasyDarwinRegisterRsp(){}
};



}}//namespace
#endif

