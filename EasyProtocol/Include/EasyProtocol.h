/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef EASYDARWIN_PROTOCOL
#define	EASYDARWIN_PROTOCOL

#include <EasyProtocolBase.h>
#include <list>

namespace EasyDarwin { namespace Protocol
{

class EASYDARWIN_API EasyDarwinRegisterReq : public EasyProtocol
{
public:
	EasyDarwinRegisterReq();
	EasyDarwinRegisterReq(const char* msg);
	virtual ~EasyDarwinRegisterReq(){}

public:
	std::string GetSerialNumber();	
    std::string GetAuthCode();

};

class EASYDARWIN_API EasyDarwinRegisterAck : public EasyProtocol
{
public:
	EasyDarwinRegisterAck();
	EasyDarwinRegisterAck(const char* msg);
	virtual ~EasyDarwinRegisterAck(){}
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

class EasyDarwinHLSession
{
public:
	EasyDarwinHLSession(){}
	~EasyDarwinHLSession(){}

public:
	int index;
	std::string SessionName;
	std::string HlsUrl;
	std::string sourceUrl;
	int bitrate;
};

class EASYDARWIN_API EasyDarwinDeviceListAck : public EasyProtocol
{
public:
	EasyDarwinDeviceListAck();
	EasyDarwinDeviceListAck(const char* msg);
	virtual ~EasyDarwinDeviceListAck(){}

public:
	bool AddDevice(EasyDarwinDevice &device);
	int StartGetDevice();
	bool GetNextDevice(EasyDarwinDevice &device);

private:
	std::list<EasyDarwinDevice> devices;
};

class EASYDARWIN_API EasyDarwinDeviceSnapUpdateReq : public EasyProtocol
{
public:
	EasyDarwinDeviceSnapUpdateReq();
	EasyDarwinDeviceSnapUpdateReq(const char *msg);
	~EasyDarwinDeviceSnapUpdateReq(){}

public:
	void SetImageData(const char* sImageBase64Data, size_t iBase64DataSize);
	bool GetImageData(std::string &sImageBase64Data);
};

class EASYDARWIN_API EasyDarwinDeviceSnapUpdateAck : public EasyProtocol
{
public:
	EasyDarwinDeviceSnapUpdateAck();
	EasyDarwinDeviceSnapUpdateAck(const char *msg);
	~EasyDarwinDeviceSnapUpdateAck(){}
};

class EASYDARWIN_API EasyDarwinEasyHLSAck : public EasyProtocol
{
public:
	EasyDarwinEasyHLSAck();
	EasyDarwinEasyHLSAck(const char *msg);
	~EasyDarwinEasyHLSAck(){}

	void SetStreamName(const char* sName);
	void SetStreamURL(const char* sURL);
};


class EASYDARWIN_API EasyDarwinHLSessionListAck : public EasyProtocol
{
public:
	EasyDarwinHLSessionListAck();
	EasyDarwinHLSessionListAck(const char* msg);
	virtual ~EasyDarwinHLSessionListAck(){}

public:
	bool AddSession(EasyDarwinHLSession &session);
	//int StartGetDevice();
	//bool GetNextDevice(EasyDarwinDevice &device);

private:
	std::list<EasyDarwinHLSession> sessions;
};


}}//namespace
#endif
