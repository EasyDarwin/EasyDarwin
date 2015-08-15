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

class EASYDSS_API EasyDarwinRegisterReq : public EasyDSSProtocol
{
public:
	EasyDarwinRegisterReq();
	EasyDarwinRegisterReq(const char* msg);
	virtual ~EasyDarwinRegisterReq(){}

public:
	std::string GetSerialNumber();	
    std::string GetAuthCode();

};

class EASYDSS_API EasyDarwinRegisterRsp : public EasyDSSProtocol
{
public:
	EasyDarwinRegisterRsp();
	EasyDarwinRegisterRsp(const char* msg);
	virtual ~EasyDarwinRegisterRsp(){}
};

class EasyDarwinDevice
{
public:
	EasyDarwinDevice(){}
	~EasyDarwinDevice(){}

public:
	std::string DeviceSerial;
	std::string DeviceName;
	std::string DeviceSnap;
};

class EASYDSS_API EasyDarwinDeviceListRsp : public EasyDSSProtocol
{
public:
	EasyDarwinDeviceListRsp();
	EasyDarwinDeviceListRsp(const char* msg);
	virtual ~EasyDarwinDeviceListRsp(){}

public:
	bool AddDevice(EasyDarwinDevice &device);
	int StartGetDevice();
	bool GetNextDevice(EasyDarwinDevice &device);

};

class EASYDSS_API EasyDarwinDeviceSnapUpdateReq : public EasyDSSProtocol
{
public:
	EasyDarwinDeviceSnapUpdateReq();
	EasyDarwinDeviceSnapUpdateReq(const char *msg);
	~EasyDarwinDeviceSnapUpdateReq(){}

public:
	void SetImageData(const char* sImageBase64Data, size_t iBase64DataSize);
	bool GetImageData(std::string &sImageBase64Data);
};

class EASYDSS_API EasyDarwinDeviceSnapUpdateRsp : public EasyDSSProtocol
{
public:
	EasyDarwinDeviceSnapUpdateRsp();
	EasyDarwinDeviceSnapUpdateRsp(const char *msg);
	~EasyDarwinDeviceSnapUpdateRsp(){}
};

}}//namespace
#endif