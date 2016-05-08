/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef EASY_PROTOCOL_H
#define	EASY_PROTOCOL_H


#include <EasyProtocolBase.h>
#include <map>
#include <vector>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <list>
#include <set>
using namespace std;


namespace EasyDarwin { namespace Protocol
{

class Easy_API EasyDevice//摄像头信息类
{
public:
	EasyDevice();
	EasyDevice(string serial, string name);
	EasyDevice(string serial, string name, string status);
	~EasyDevice(){}

public:
	string serial_;
	string name_;
	string status_;	//online/offline
	string password_;
	string tag_;
	string channel_;
};

typedef vector<EasyDevice> EasyDevices;		//摄像头数组
typedef EasyDevices::iterator EasyDevicesIterator;

typedef boost::variant<int, float, string> value_t;
typedef map<string, value_t> EasyJsonValue;	//key为string,value可以是int、float、string的一种
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
}EasyNVRMessage;//NVR消息

typedef vector<EasyNVRMessage> EasyNVRMessageQueue;//NVR消息数组


class EasyJsonValueVisitor : public boost::static_visitor<string>
{
public:
	string operator()(int value) const { return boost::lexical_cast<string>(value); }
	string operator()(float value) const { return boost::lexical_cast<string>(value); }
	string operator()(string value) const { return value; }
};

class Easy_API EasyNVR : public EasyDevice//NVR类型
{
public:
	EasyNVR();
	EasyNVR(string serial, string name, string password, string tag, EasyDevices &channel);
	~EasyNVR(){}

public:	
	EasyDevices channels_;                                                           
	EasyObject object_;
};

typedef map<string, EasyNVR> EasyNVRs;//维护所有的NVR

// MSG_DS_REGISTER_REQ
class Easy_API EasyMsgDSRegisterREQ : public EasyProtocol	//封装NVR的注册请求
{
public:
	EasyMsgDSRegisterREQ(EasyDarwinTerminalType terminalType, EasyDarwinAppType appType, EasyNVR &nvr, size_t cseq = 1);
	EasyMsgDSRegisterREQ(const char* msg);
	virtual ~EasyMsgDSRegisterREQ(){}

public:
	EasyNVR& GetNVR(){ return nvr_; }

private:
	EasyNVR nvr_;
};

// MSG_SD_REGISTER_ACK
class Easy_API EasyMsgSDRegisterACK : public EasyProtocol	//封装NVR的注册回应
{
public:
	EasyMsgSDRegisterACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgSDRegisterACK(const char* msg);
	virtual ~EasyMsgSDRegisterACK(){}
};

// MSG_SD_PUSH_STREAM_REQ
class EasyMsgSDPushStreamREQ : public EasyProtocol
{
public:
	EasyMsgSDPushStreamREQ(EasyJsonValue &body, size_t cseq);
	EasyMsgSDPushStreamREQ(const char* msg);
	~EasyMsgSDPushStreamREQ(){}
};

// MSG_DS_PUSH_STREAM_ACK
class EasyMsgDSPushSteamACK : public EasyProtocol
{
public:
	EasyMsgDSPushSteamACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgDSPushSteamACK(const char* msg);
	~EasyMsgDSPushSteamACK(){}
};

// MSG_SD_STREAM_STOP_REQ
class EasyMsgSDStopStreamREQ : public EasyProtocol
{
public:
	EasyMsgSDStopStreamREQ(EasyJsonValue &body, size_t cseq = 1);
	EasyMsgSDStopStreamREQ(const char* msg);
	~EasyMsgSDStopStreamREQ(){}
};

// MSG_DS_STREAM_STOP_ACK
class EasyMsgDSStopStreamACK : public EasyProtocol
{
public:
	EasyMsgDSStopStreamACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgDSStopStreamACK(const char* msg);
	~EasyMsgDSStopStreamACK(){}
};

// MSG_SC_DEVICE_LIST_ACK
class EasyMsgSCDeviceListACK : public EasyProtocol
{
public:
	EasyMsgSCDeviceListACK(EasyDevices &devices, size_t cseq = 1, size_t error = 200);
	EasyMsgSCDeviceListACK(const char* msg);
	~EasyMsgSCDeviceListACK(){}

	EasyDevices& GetDevices() { return devices_; }

private:
	EasyDevices devices_;
};

// MSG_SC_DEVICE_INFO_ACK
class EasyMsgSCDeviceInfoACK : public EasyProtocol
{
public:
	EasyMsgSCDeviceInfoACK(EasyDevices &cameras, string devcei_serial, size_t cseq = 1, size_t error = 200);
	EasyMsgSCDeviceInfoACK(const char* msg);
	~EasyMsgSCDeviceInfoACK() {}

	EasyDevices& GetCameras() { return cameras_; }

private:
	EasyDevices cameras_;
};

// MSG_SC_GET_STREAM_ACK
class EasyMsgSCGetStreamACK : public EasyProtocol
{
public:
	EasyMsgSCGetStreamACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgSCGetStreamACK(const char* msg);
	~EasyMsgSCGetStreamACK(){}
};

// MSG_SC_FREE_STREAM_ACK
class EasyMsgSCFreeStreamACK : public EasyProtocol
{
public:
	EasyMsgSCFreeStreamACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgSCFreeStreamACK(const char* msg);
	~EasyMsgSCFreeStreamACK() {}
};

// MSG_DS_POST_SNAP_REQ
class EasyMsgDSPostSnapREQ : public EasyProtocol
{
public:
	EasyMsgDSPostSnapREQ(EasyJsonValue &body, size_t cseq = 1);
	EasyMsgDSPostSnapREQ(const char* msg);
	~EasyMsgDSPostSnapREQ() {}
};

// MSG_SD_POST_SNAP_ACK
class EasyMsgSDPostSnapACK : public EasyProtocol
{
public:
	EasyMsgSDPostSnapACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgSDPostSnapACK(const char* msg);
	~EasyMsgSDPostSnapACK() {}
};

class EasyMsgExceptionACK : public EasyProtocol
{
public:
	EasyMsgExceptionACK(size_t cseq = 1, size_t error = 400);
	~EasyMsgExceptionACK() {}
};


/*
class Easy_API EasyDarwinDeviceListAck : public EasyProtocol
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

class Easy_API EasyDarwinDeviceSnapUpdateReq : public EasyProtocol
{
public:
	EasyDarwinDeviceSnapUpdateReq();
	EasyDarwinDeviceSnapUpdateReq(const char *msg);
	~EasyDarwinDeviceSnapUpdateReq(){}

public:
	void SetImageData(const char* sImageBase64Data, size_t iBase64DataSize);
	bool GetImageData(string &sImageBase64Data);
};

class Easy_API EasyDarwinDeviceSnapUpdateAck : public EasyProtocol
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
	int numOutputs;
};


class Easy_API EasyMsgSCStartHLSACK : public EasyProtocol
{
public:
	EasyMsgSCStartHLSACK();
	EasyMsgSCStartHLSACK(const char *msg);
	~EasyMsgSCStartHLSACK(){}

	void SetStreamName(const char* sName);
	void SetStreamURL(const char* sURL);
};

class Easy_API EasyMsgSCHLSessionListACK : public EasyProtocol
{
public:
	EasyMsgSCHLSessionListACK();
	EasyMsgSCHLSessionListACK(const char* msg);
	virtual ~EasyMsgSCHLSessionListACK(){}

public:
	bool AddSession(EasyDarwinHLSession &session);
	//int StartGetDevice();
	//bool GetNextDevice(EasyDarwinDevice &device);

private:
	list<EasyDarwinHLSession> sessions;
};
class Easy_API EasyMsgSCRTSPPushSessionListACK : public EasyProtocol
{
public:
	EasyMsgSCRTSPPushSessionListACK();
	EasyMsgSCRTSPPushSessionListACK(const char* msg);
	virtual ~EasyMsgSCRTSPPushSessionListACK(){}

public:
	bool AddSession(EasyDarwinRTSPSession &session);
	//int StartGetDevice();
	//bool GetNextDevice(EasyDarwinDevice &device);

private:
	std::list<EasyDarwinRTSPSession> sessions;
};


class Easy_API EasyDarwinRecordListAck : public EasyProtocol
{
public:
	EasyDarwinRecordListAck();
	EasyDarwinRecordListAck(const char* msg);
	virtual ~EasyDarwinRecordListAck(){}

public:
	bool AddRecord(std::string record);

private:
	std::list<std::string> records;
};


//add,Unisiot，start
enum EasyDSSTerminalType//设备类型
{
	EASYDSS_TERMINAL_TYPE_CAMERA	= 0,//摄像机
	EASYDSS_TERMINAL_TYPE_NVR		= 1,//NVR
	EASYDSS_TERMINAL_TYPE_SMARTHOST = 2,//智能主机
	EASYDSS_TERMINAL_TYPE_NUM		= 3 //设备类型个数
};
enum EasyDSSAppType//设备类型
{
	EASYDSS_APP_TYPE_ARM_LINUX		= 0,//linux终端
	EASYDSS_APP_TYPE_ANDROID		= 1,//Andorid Cli
	EASYDSS_APP_TYPE_IOS			= 2,//IOS Cli
	EASYDSS_APP_TYPE_WEB			= 3,//Web Cli
	EASYDSS_APP_TYPE_PC				= 4,//PC Cli
	EASYDSS_APP_TYPE_NUM			= 5
};
typedef struct 
{
	string strDeviceSerial;//设备序列号
	string strCameraSerial;//摄像头序列号
	string strProtocol;//直播协议
	string strStreamID;//直播流类型
}stStreamInfo;
typedef set<void*> DevSet;//void*对应的是客户端对象指针，客户端开始直播时加入，停止直播时删除，客户端连接终止时删除
typedef map<string,DevSet> DevMap;//key为string表示当前设备下的某个摄像头，value为对这个摄像头进行拉流的客户端
typedef DevMap::iterator DevMapItera;

typedef map<string,stStreamInfo> CliStreamMap;//客户端存储所有的直播信息，key为设备序列号和摄像头序列号的组合，value为流类型和协议.用来判断客户端对哪些设备进行了直播，一个客户端可能同时对多个设备的多个摄像头同时进行直播
typedef CliStreamMap::iterator CliStreamMapItera;
class strDevice//设备类型对应的信息
{
public:
	bool GetDevInfo(const char *msg);//由JSON文本得到设备信息
public:
	string serial_;//设备序列号
	string name_;//设备名称
	string password_;//密码
	string tag_;//标签
	string channelCount_;//该设备包含的摄像头个数

	EasyDevices cameras_;//摄像头信息
	EasyDSSTerminalType eDeviceType;//设备类型
	EasyDSSAppType eAppType;//App类型
};

class Easy_API EasyDarwinRSP : public EasyProtocol//封装CMS的一般回应的JSON部分
{
public:
	EasyDarwinRSP(int iMsgType):EasyProtocol(iMsgType){}
	void SetHead(EasyJsonValue& header);//设置头部
	void SetBody(EasyJsonValue& body);//设置JSON
};
class Easy_API EasyDarwinRecordListRSP : public EasyDarwinRSP//封装录像列表回应
{
public:
	EasyDarwinRecordListRSP(int iMsgType):EasyDarwinRSP(iMsgType){}
	void AddRecord(std::string record);
};
//add,Unisiot，end
}}//namespace
#endif
