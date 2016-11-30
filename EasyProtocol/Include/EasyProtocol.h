/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
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
using namespace std;

namespace EasyDarwin { namespace Protocol
{

class Easy_API EasyDevice//����ͷ��Ϣ��
{
public:
	EasyDevice();
	EasyDevice(const string& serial, const string& name);
	EasyDevice(const string& serial, const string& name, const string& status);
	~EasyDevice(){}

public:
	string serial_;
	string name_;
	string status_;	//online/offline
	string password_;
	string tag_;
	string channel_;
	string snapJpgPath_;//���µĿ���·��
};

//typedef vector<EasyDevice> EasyDevices;		//����ͷ����
//typedef EasyDevices::iterator EasyDevicesIterator;

typedef map<string,EasyDevice> EasyDevices;		//����ͷ����Ϊmap.������ҡ�keyΪchannel_��valueΪEasyDevice
typedef EasyDevices::iterator EasyDevicesIterator;


typedef boost::variant<int, float, string> value_t;
typedef map<string, value_t> EasyJsonValue;	//keyΪstring,value������int��float��string��һ��
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
} EasyNVRMessage;//NVR��Ϣ

typedef vector<EasyNVRMessage> EasyNVRMessageQueue;//NVR��Ϣ����


class EasyJsonValueVisitor : public boost::static_visitor<string>
{
public:
	string operator()(int value) const { return boost::lexical_cast<string>(value); }
	string operator()(float value) const { return boost::lexical_cast<string>(value); }
	string operator()(string value) const { return value; }
};

class Easy_API EasyNVR : public EasyDevice//NVR����
{
public:
	EasyNVR();
	EasyNVR(const string& serial, const string& name, const string& password, const string& tag, EasyDevices &channel);
	~EasyNVR(){}

public:	
	EasyDevices channels_;                                                           
	EasyObject object_;
};

// MSG_DS_REGISTER_REQ
class Easy_API EasyMsgDSRegisterREQ : public EasyProtocol	//��װNVR��ע������
{
public:
	EasyMsgDSRegisterREQ(EasyDarwinTerminalType terminalType, EasyDarwinAppType appType, EasyNVR &nvr, size_t cseq = 1);
	EasyMsgDSRegisterREQ(const string& msg);
	virtual ~EasyMsgDSRegisterREQ() {}

public:
	EasyNVR& GetNVR() { return nvr_; }

private:
	EasyNVR nvr_;
};

// MSG_SD_REGISTER_ACK
class Easy_API EasyMsgSDRegisterACK : public EasyProtocol	//��װNVR��ע���Ӧ
{
public:
	EasyMsgSDRegisterACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgSDRegisterACK(const string& msg);
	virtual ~EasyMsgSDRegisterACK() {}
};

// MSG_SD_PUSH_STREAM_REQ
class EasyMsgSDPushStreamREQ : public EasyProtocol
{
public:
	EasyMsgSDPushStreamREQ(EasyJsonValue &body, size_t cseq);
	EasyMsgSDPushStreamREQ(const string& msg);
	virtual ~EasyMsgSDPushStreamREQ() {}
};

// MSG_DS_PUSH_STREAM_ACK
class EasyMsgDSPushSteamACK : public EasyProtocol
{
public:
	EasyMsgDSPushSteamACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgDSPushSteamACK(const string& msg);
	virtual ~EasyMsgDSPushSteamACK() {}
};

// MSG_SD_STREAM_STOP_REQ
class EasyMsgSDStopStreamREQ : public EasyProtocol
{
public:
	EasyMsgSDStopStreamREQ(EasyJsonValue &body, size_t cseq = 1);
	EasyMsgSDStopStreamREQ(const string& msg);
	virtual ~EasyMsgSDStopStreamREQ() {}
};

// MSG_DS_STREAM_STOP_ACK
class EasyMsgDSStopStreamACK : public EasyProtocol
{
public:
	EasyMsgDSStopStreamACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgDSStopStreamACK(const string& msg);
	virtual ~EasyMsgDSStopStreamACK() {}
};

// MSG_SC_DEVICE_LIST_ACK
class EasyMsgSCDeviceListACK : public EasyProtocol
{
public:
	EasyMsgSCDeviceListACK(EasyDevices &devices, size_t cseq = 1, size_t error = 200);
	EasyMsgSCDeviceListACK(const string& msg);
	virtual ~EasyMsgSCDeviceListACK() {}

	EasyDevices& GetDevices() { return devices_; }

private:
	EasyDevices devices_;
};

// MSG_SC_DEVICE_INFO_ACK
class EasyMsgSCDeviceInfoACK : public EasyProtocol
{
public:
	EasyMsgSCDeviceInfoACK(EasyDevices &cameras, const string& devcei_serial, size_t cseq = 1, size_t error = 200);
	EasyMsgSCDeviceInfoACK(const string& msg);
	~EasyMsgSCDeviceInfoACK() {}

	EasyDevices& GetCameras() { return channels_; }

private:
	EasyDevices channels_;
};

// MSG_SC_GET_STREAM_ACK
class EasyMsgSCGetStreamACK : public EasyProtocol
{
public:
	EasyMsgSCGetStreamACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgSCGetStreamACK(const string& msg);
	virtual ~EasyMsgSCGetStreamACK() {}
};

// MSG_CS_FREE_STREAM_REQ
class EasyMsgCSFreeStreamREQ : public EasyProtocol
{
public:
	EasyMsgCSFreeStreamREQ(EasyJsonValue &body, size_t cseq);
	EasyMsgCSFreeStreamREQ(const string& msg);
	virtual ~EasyMsgCSFreeStreamREQ() {}
};

// MSG_SC_FREE_STREAM_ACK
class EasyMsgSCFreeStreamACK : public EasyProtocol
{
public:
	EasyMsgSCFreeStreamACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgSCFreeStreamACK(const string& msg);
	virtual ~EasyMsgSCFreeStreamACK() {}
};

// MSG_DS_POST_SNAP_REQ
class EasyMsgDSPostSnapREQ : public EasyProtocol
{
public:
	EasyMsgDSPostSnapREQ(EasyJsonValue &body, size_t cseq = 1);
	EasyMsgDSPostSnapREQ(const string& msg);
	virtual ~EasyMsgDSPostSnapREQ() {}
};

// MSG_SD_POST_SNAP_ACK
class EasyMsgSDPostSnapACK : public EasyProtocol
{
public:
	EasyMsgSDPostSnapACK(EasyJsonValue &body, size_t cseq = 1, size_t error = 200);
	EasyMsgSDPostSnapACK(const string& msg);
	virtual ~EasyMsgSDPostSnapACK() {}
};

// MSG_CS_PTZ_CONTROL_REQ
class EasyMsgCSPTZControlREQ : public EasyProtocol
{
public:
    EasyMsgCSPTZControlREQ(EasyJsonValue& body, size_t cseq = 1);
    EasyMsgCSPTZControlREQ(const string& msg);
    virtual ~EasyMsgCSPTZControlREQ() {}
};

// MSG_SC_PTZ_CONTROL_ACK
class EasyMsgSCPTZControlACK : public EasyProtocol
{
public:
    EasyMsgSCPTZControlACK(EasyJsonValue& body, size_t cseq = 1, size_t error = 200);
    EasyMsgSCPTZControlACK(const string& msg);
    virtual ~EasyMsgSCPTZControlACK() {}
};

// MSG_SD_CONTROL_PTZ_REQ
class EasyMsgSDControlPTZREQ : public EasyProtocol
{
public:
    EasyMsgSDControlPTZREQ(EasyJsonValue& body, size_t cseq = 1);
    EasyMsgSDControlPTZREQ(const string& msg);
    virtual ~EasyMsgSDControlPTZREQ() {}
};

// MSG_DS_CONTROL_PTZ_ACK
class EasyMsgDSControlPTZACK : public EasyProtocol
{
public:
    EasyMsgDSControlPTZACK(EasyJsonValue& body, size_t cseq = 1, size_t error = 200);
    EasyMsgDSControlPTZACK(const string& msg);
    virtual ~EasyMsgDSControlPTZACK() {}
};

// MSG_CS_PRESET_CONTROL_REQ
class EasyMsgCSPresetControlREQ : public EasyProtocol
{
public:
	EasyMsgCSPresetControlREQ(EasyJsonValue& body, size_t cseq = 1);
	EasyMsgCSPresetControlREQ(const string& msg);
	virtual ~EasyMsgCSPresetControlREQ() {}
};

// MSG_SC_PRESET_CONTROL_ACK
class EasyMsgSCPresetControlACK : public EasyProtocol
{
public:
	EasyMsgSCPresetControlACK(EasyJsonValue& body, size_t cseq = 1, size_t error = 200);
	EasyMsgSCPresetControlACK(const string& msg);
	virtual ~EasyMsgSCPresetControlACK() {}
};

// MSG_SD_CONTROL_PRESET_REQ
class EasyMsgSDControlPresetREQ : public EasyProtocol
{
public:
	EasyMsgSDControlPresetREQ(EasyJsonValue& body, size_t cseq = 1);
	EasyMsgSDControlPresetREQ(const string& msg);
	virtual ~EasyMsgSDControlPresetREQ() {}
};

// MSG_DS_CONTROL_PRESET_ACK
class EasyMsgDSControlPresetACK : public EasyProtocol
{
public:
	EasyMsgDSControlPresetACK(EasyJsonValue& body, size_t cseq = 1, size_t error = 200);
	EasyMsgDSControlPresetACK(const string& msg);
	virtual ~EasyMsgDSControlPresetACK() {}
};

// MSG_CS_TALKBACK_CONTROL_REQ
class EasyMsgCSTalkbackControlREQ : public EasyProtocol
{
public:
	EasyMsgCSTalkbackControlREQ(EasyJsonValue& body, size_t cseq = 1);
	EasyMsgCSTalkbackControlREQ(const string& msg);
	virtual ~EasyMsgCSTalkbackControlREQ() {}
};

// MSG_SC_TALKBACK_CONTROL_ACK
class EasyMsgSCTalkbackControlACK : public EasyProtocol
{
public:
	EasyMsgSCTalkbackControlACK(EasyJsonValue& body, size_t cseq = 1, size_t error = 200);
	EasyMsgSCTalkbackControlACK(const string& msg);
	virtual ~EasyMsgSCTalkbackControlACK() {}
};

// MSG_SD_CONTROL_TALKBACK_REQ
class EasyMsgSDControlTalkbackREQ : public EasyProtocol
{
public:
	EasyMsgSDControlTalkbackREQ(EasyJsonValue& body, size_t cseq = 1);
	EasyMsgSDControlTalkbackREQ(const string& msg);
	virtual ~EasyMsgSDControlTalkbackREQ() {}
};

// MSG_DS_CONTROL_TALKBACK_ACK
class EasyMsgDSControlTalkbackACK : public EasyProtocol
{
public:
	EasyMsgDSControlTalkbackACK(EasyJsonValue& body, size_t cseq = 1, size_t error = 200);
	EasyMsgDSControlTalkbackACK(const string& msg);
	virtual ~EasyMsgDSControlTalkbackACK() {}
};

class EasyMsgExceptionACK : public EasyProtocol
{
public:
	EasyMsgExceptionACK(size_t cseq = 1, size_t error = 400);
	virtual ~EasyMsgExceptionACK() {}
};

class EasyDarwinHLSession
{
public:
	EasyDarwinHLSession(): index(0), bitrate(0)
	{}

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
	EasyDarwinRTSPSession(): index(0), numOutputs(0)
	{}

    ~EasyDarwinRTSPSession(){}

public:
	int index;
	std::string Url;
	std::string Name;
	int numOutputs;
};

// MSG_SC_START_HLS_ACK
class Easy_API EasyMsgSCStartHLSACK : public EasyProtocol
{
public:
	EasyMsgSCStartHLSACK();
	EasyMsgSCStartHLSACK(const string& msg);
	virtual ~EasyMsgSCStartHLSACK() {}

	void SetStreamName(const string& sName);
	void SetStreamURL(const string& sURL);
};

// MSG_SC_HLS_SESSION_LIST_ACK
class Easy_API EasyMsgSCHLSessionListACK : public EasyProtocol
{
public:
	EasyMsgSCHLSessionListACK();
	EasyMsgSCHLSessionListACK(const string& msg);
	virtual ~EasyMsgSCHLSessionListACK() {}

public:
	bool AddSession(EasyDarwinHLSession &session);
	//int StartGetDevice();
	//bool GetNextDevice(EasyDarwinDevice &device);

private:
	list<EasyDarwinHLSession> sessions;
};

// MSG_SC_RTSP_PUSH_SESSION_LIST_ACK
class Easy_API EasyMsgSCRTSPPushSessionListACK : public EasyProtocol
{
public:
	EasyMsgSCRTSPPushSessionListACK();
	EasyMsgSCRTSPPushSessionListACK(const string& msg);
	virtual ~EasyMsgSCRTSPPushSessionListACK() {}

public:
	bool AddSession(EasyDarwinRTSPSession& session);
	//int StartGetDevice();
	//bool GetNextDevice(EasyDarwinDevice &device);

private:
	std::list<EasyDarwinRTSPSession> sessions;
};

// MSG_SC_LIST_RECORD_ACK
class Easy_API EasyMsgSCListRecordACK : public EasyProtocol
{
public:
	EasyMsgSCListRecordACK();
	EasyMsgSCListRecordACK(const string& msg);
	virtual ~EasyMsgSCListRecordACK() {}

public:
	bool AddRecord(const string& record);

private:
	std::list<std::string> records;
};

class Easy_API EasyProtocolACK : public EasyProtocol
{
public:
	EasyProtocolACK(int iMsgType) :EasyProtocol(iMsgType) {}
	void SetHead(EasyJsonValue& header);//����ͷ��
	void SetBody(EasyJsonValue& body);//����JSON
};

class Easy_API EasyMsgSCRecordListACK : public EasyProtocolACK//��װ¼���б��Ӧ
{
public:
	EasyMsgSCRecordListACK(int iMsgType) :EasyProtocolACK(iMsgType) {}
	void AddRecord(const string& record);
};

//add,Unisiot��start
typedef struct 
{
	string strDeviceSerial;//�豸���к�
	string strCameraSerial;//����ͷ���к�
	string strProtocol;//ֱ��Э��
	string strStreamID;//ֱ��������
} stStreamInfo;

class strDevice//�豸���Ͷ�Ӧ����Ϣ
{
public:
	strDevice();
	bool GetDevInfo(const string& msg);//��JSON�ı��õ��豸��Ϣ
	void HoldSnapPath(const string& strJpgPath, const string& strChannel);//�������յ�·��
public:
	string serial_;//�豸���к�
	string name_;//�豸����
	string password_;//����
	string tag_;//��ǩ
	string channelCount_;//���豸����������ͷ����
	string snapJpgPath_;//���µĿ���·��


	EasyDevices channels_;//����ͷ��Ϣ
	EasyDarwinTerminalType eDeviceType;//�豸����
	EasyDarwinAppType eAppType;//App����
};
//add,Unisiot��end
}}//namespace
#endif
