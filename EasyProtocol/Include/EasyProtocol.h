/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef EASYDARWIN_PROTOCOL
#define	EASYDARWIN_PROTOCOL

#include <EasyProtocolBase.h>
#include <map>
#include <vector>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <list>
using namespace std;

namespace EasyDarwin { namespace Protocol
{

class EASYDARWIN_API EasyDevice
{
public:
	EasyDevice();
	EasyDevice(string serial, string name);
	EasyDevice(string serial, string name, string status);
	~EasyDevice(){}

public:
	string serial_;
	string name_;
	string status_;//online/offline
	string password_;
	string tag_;
};

typedef vector<EasyDevice> EasyDevices;
typedef boost::variant<int, float, string> value_t;
typedef map<string, value_t> EasyJsonValue;
typedef void* EasyObject;


typedef struct
{
	string device_serial_;
	string camera_serial_;
	string stream_id_;
	EasyObject object_;
	unsigned long timestamp_;
	unsigned long timeout_;//second
	int message_type_;//see EasyDarwin_Message_Type_Define
}EasyNVRMessage;

typedef vector<EasyNVRMessage> EasyNVRMessageQueue;


class EasyJsonValueVisitor : public boost::static_visitor<string>
{
public:
	string operator()(int value) const { return boost::lexical_cast<string>(value); }
	string operator()(float value) const { return boost::lexical_cast<string>(value); }
	string operator()(string value) const { return value; }
};

class EASYDARWIN_API EasyNVR : public EasyDevice
{
public:
	EasyNVR();
	EasyNVR(string serial, string name, string password, string tag, EasyDevices &channel);
	~EasyNVR(){}

public:	
	EasyDevices channels_;
	EasyObject object_;
};

typedef map<string, EasyNVR> EasyNVRs;

class EASYDARWIN_API EasyDarwinRegisterReq : public EasyProtocol
{
public:
	EasyDarwinRegisterReq(EasyNVR &nvr, size_t cseq = 1);
	EasyDarwinRegisterReq(const char* msg);
	virtual ~EasyDarwinRegisterReq(){}

public:
	EasyNVR& GetNVR(){ return nvr_; }

private:
	EasyNVR nvr_;
};

class EASYDARWIN_API EasyDarwinRegisterRSP : public EasyProtocol
{
public:
	EasyDarwinRegisterRSP(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyDarwinRegisterRSP(const char* msg);
	virtual ~EasyDarwinRegisterRSP(){}
};

class EasyDarwinDeviceStreamReq : public EasyProtocol
{
public:
	EasyDarwinDeviceStreamReq(EasyJsonValue &body, size_t cseq);
	EasyDarwinDeviceStreamReq(const char* msg);
	~EasyDarwinDeviceStreamReq(){}
};

class EasyDarwinDeviceStreamRsp : public EasyProtocol
{
public:
	EasyDarwinDeviceStreamRsp(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyDarwinDeviceStreamRsp(const char* msg);
	~EasyDarwinDeviceStreamRsp(){}
};



class EasyDarwinDeviceStreamStop : public EasyProtocol
{
public:
	EasyDarwinDeviceStreamStop(EasyJsonValue &body, size_t cseq = 1);
	EasyDarwinDeviceStreamStop(const char* msg);
	~EasyDarwinDeviceStreamStop(){}
};

class EasyDarwinDeviceStreamStopRsp : public EasyProtocol
{
public:
	EasyDarwinDeviceStreamStopRsp(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyDarwinDeviceStreamStopRsp(const char* msg);
	~EasyDarwinDeviceStreamStopRsp(){}
};

//nvr list response
class EasyDarwinDeviceListRsp : public EasyProtocol
{
public:
	EasyDarwinDeviceListRsp(EasyDevices &devices, size_t cseq = 1, size_t error = 200);
	EasyDarwinDeviceListRsp(const char* msg);
	~EasyDarwinDeviceListRsp(){}

	EasyDevices& GetDevices() { return devices_; }

private:
	EasyDevices devices_;
};

//camera list response
class EasyDarwinCameraListRsp : public EasyProtocol
{
public:
	EasyDarwinCameraListRsp(EasyDevices &cameras, string devcei_serial, size_t cseq = 1, size_t error = 200);
	EasyDarwinCameraListRsp(const char* msg);
	~EasyDarwinCameraListRsp() {}

	EasyDevices& GetCameras() { return cameras_; }

private:
	EasyDevices cameras_;
};

class EasyDarwinClientStartStreamRsp : public EasyProtocol
{
public:
	EasyDarwinClientStartStreamRsp(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyDarwinClientStartStreamRsp(const char* msg);
	~EasyDarwinClientStartStreamRsp(){}
};

class EasyDarwinClientStopStreamRsp : public EasyProtocol
{
public:
	EasyDarwinClientStopStreamRsp(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyDarwinClientStopStreamRsp(const char* msg);
	~EasyDarwinClientStopStreamRsp() {}
};

class EasyDarwinDeviceUpdateSnapReq : public EasyProtocol
{
public:
	EasyDarwinDeviceUpdateSnapReq(EasyJsonValue &body, size_t cseq = 1);
	EasyDarwinDeviceUpdateSnapReq(const char* msg);
	~EasyDarwinDeviceUpdateSnapReq() {}
};

class EasyDarwinDeviceUpdateSnapRsp : public EasyProtocol
{
public:
	EasyDarwinDeviceUpdateSnapRsp(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyDarwinDeviceUpdateSnapRsp(const char* msg);
	~EasyDarwinDeviceUpdateSnapRsp() {}
};

class EasyDarwinExceptionRsp : public EasyProtocol
{
public:
	EasyDarwinExceptionRsp(size_t cseq = 1, size_t error = 400);
	~EasyDarwinExceptionRsp() {}
};


/*
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
	list<EasyDarwinDevice> devices;
};

class EASYDARWIN_API EasyDarwinDeviceSnapUpdateReq : public EasyProtocol
{
public:
	EasyDarwinDeviceSnapUpdateReq();
	EasyDarwinDeviceSnapUpdateReq(const char *msg);
	~EasyDarwinDeviceSnapUpdateReq(){}

public:
	void SetImageData(const char* sImageBase64Data, size_t iBase64DataSize);
	bool GetImageData(string &sImageBase64Data);
};

class EASYDARWIN_API EasyDarwinDeviceSnapUpdateAck : public EasyProtocol
{
public:
	EasyDarwinDeviceSnapUpdateAck();
	EasyDarwinDeviceSnapUpdateAck(const char *msg);
	~EasyDarwinDeviceSnapUpdateAck(){}
};
*/

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
class EasyDarwinRTSPSession
{
public:
	EasyDarwinRTSPSession(){}
	~EasyDarwinRTSPSession(){}

public:
	int index;
	std::string Url;
	std::string Name;
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
	list<EasyDarwinHLSession> sessions;
};

class EASYDARWIN_API EasyDarwinRTSPPushSessionListAck : public EasyProtocol
{
public:
	EasyDarwinRTSPPushSessionListAck();
	EasyDarwinRTSPPushSessionListAck(const char* msg);
	virtual ~EasyDarwinRTSPPushSessionListAck(){}

public:
	bool AddSession(EasyDarwinRTSPSession &session);
	//int StartGetDevice();
	//bool GetNextDevice(EasyDarwinDevice &device);

private:
	std::list<EasyDarwinRTSPSession> sessions;
};

}}//namespace
#endif
