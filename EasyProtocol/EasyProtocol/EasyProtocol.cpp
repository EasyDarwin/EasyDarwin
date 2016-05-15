/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#include <EasyProtocol.h>
#include <string.h>
#include <stdio.h>
#include <EasyUtil.h>

namespace EasyDarwin { namespace Protocol
{

EasyDevice::EasyDevice()
{
	snapJpgPath_.clear();
}

EasyDevice::EasyDevice(std::string channel, std::string name)
: serial_(channel)
, name_(name)
{

}

EasyDevice::EasyDevice(std::string channel, std::string name, std::string status)
: serial_(channel)
, name_(name)
, status_(status)
{
}


EasyNVR::EasyNVR()
{
}

EasyNVR::EasyNVR(std::string serial, std::string name, std::string password, string tag, EasyDevices &channel)
: channels_(channel)
{
	serial_ = serial;	
	name_ = name;
	password_ = password;
	tag_ = tag;
}

// MSG_DS_REGISTER_REQ消息报文构造
EasyMsgDSRegisterREQ::EasyMsgDSRegisterREQ(EasyDarwinTerminalType terminalType, EasyDarwinAppType appType, EasyNVR &nvr, size_t cseq)
: EasyProtocol(MSG_DS_REGISTER_REQ)
, nvr_(nvr)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_APP_TYPE, EasyProtocol::GetAppTypeString(appType));
	SetHeaderValue(EASY_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(terminalType));

	SetBodyValue(EASY_TAG_SERIAL, nvr.serial_);
	SetBodyValue(EASY_TAG_NAME, nvr.name_);
	SetBodyValue(EASY_TAG_TAG, nvr.tag_);
	SetBodyValue(EASY_TAG_TOKEN, nvr.password_);
	if(appType == EASY_APP_TYPE_NVR)
	{
		SetBodyValue(EASY_TAG_CHANNEL_COUNT, nvr.channels_.size());
		for(EasyDevices::iterator it = nvr.channels_.begin(); it != nvr.channels_.end(); it++)
		{
			Json::Value value;
			value[EASY_TAG_CHANNEL] = it->serial_;
			value[EASY_TAG_NAME] = it->name_;		
			value[EASY_TAG_STATUS] = it->status_;
			root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].append(value);		
		}
	}
}

// MSG_DS_REGISTER_REQ消息报文解析
EasyMsgDSRegisterREQ::EasyMsgDSRegisterREQ(const char* msg)
: EasyProtocol(msg, MSG_DS_REGISTER_REQ)
{
	nvr_.serial_ = GetBodyValue(EASY_TAG_SERIAL);
	nvr_.name_ = GetBodyValue(EASY_TAG_NAME);
	nvr_.tag_ = GetBodyValue(EASY_TAG_TAG);
	nvr_.password_ = GetBodyValue(EASY_TAG_TOKEN);
	
	nvr_.channels_.clear();

	int size = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CAMERAS].size();  

	for(int i = 0; i < size; i++)  
	{  
		Json::Value &json_camera = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CAMERAS][i];  
		EasyDevice camera;
		camera.name_ = json_camera[EASY_TAG_CAMERA_NAME].asString();
		camera.serial_ = json_camera[EASY_TAG_CAMERASERIAL].asString();		
		camera.status_ = json_camera[EASY_TAG_STATUS].asString();	
		nvr_.channels_.push_back(camera);
	}  
}

// MSG_SD_REGISTER_ACK消息构造
EasyMsgSDRegisterACK::EasyMsgSDRegisterACK(EasyJsonValue &body, size_t cseq, size_t error)
: EasyProtocol(MSG_SD_REGISTER_ACK)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_ERROR_NUM, error);
	SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

// MSG_SD_REGISTER_ACK消息解析
EasyMsgSDRegisterACK::EasyMsgSDRegisterACK(const char* msg)
: EasyProtocol(msg, MSG_SD_REGISTER_ACK)
{
}

// MSG_SD_PUSH_STREAM_REQ消息构造
EasyMsgSDPushStreamREQ::EasyMsgSDPushStreamREQ(EasyJsonValue &body, size_t cseq)
: EasyProtocol(MSG_SD_PUSH_STREAM_REQ)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);

	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

// MSG_SD_PUSH_STREAM_REQ消息解析
EasyMsgSDPushStreamREQ::EasyMsgSDPushStreamREQ(const char *msg)
: EasyProtocol(msg, MSG_SD_PUSH_STREAM_REQ)
{
}

// MSG_DS_PUSH_STREAM_ACK消息构造
EasyMsgDSPushSteamACK::EasyMsgDSPushSteamACK(EasyJsonValue &body, size_t cseq, size_t error)
: EasyProtocol(MSG_DS_PUSH_STREAM_ACK)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_ERROR_NUM, error);
	SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

// MSG_DS_PUSH_STREAM_ACK消息解析
EasyMsgDSPushSteamACK::EasyMsgDSPushSteamACK(const char *msg)
: EasyProtocol(msg, MSG_DS_PUSH_STREAM_ACK)
{
}

// MSG_SD_STREAM_STOP_REQ消息构造
EasyMsgSDStopStreamREQ::EasyMsgSDStopStreamREQ(EasyJsonValue &body, size_t cseq)
:EasyProtocol(MSG_SD_STREAM_STOP_REQ)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	
	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

// MSG_SD_STREAM_STOP_REQ消息解析
EasyMsgSDStopStreamREQ::EasyMsgSDStopStreamREQ(const char *msg)
: EasyProtocol(msg, MSG_SD_STREAM_STOP_REQ)
{
}

// MSG_SD_STREAM_STOP_REQ消息构造
EasyMsgDSStopStreamACK::EasyMsgDSStopStreamACK(EasyJsonValue &body, size_t cseq, size_t error)
: EasyProtocol(MSG_SD_STREAM_STOP_REQ)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_ERROR_NUM, error);
	SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

// MSG_SD_STREAM_STOP_REQ消息解析
EasyMsgDSStopStreamACK::EasyMsgDSStopStreamACK(const char *msg)
: EasyProtocol(msg, MSG_SD_STREAM_STOP_REQ)
{
}

/*
EasyDarwinDeviceListAck::EasyDarwinDeviceListAck()
: EasyProtocol(MSG_CLI_CMS_DEVICE_LIST_ACK)
{
}

EasyDarwinDeviceListAck::EasyDarwinDeviceListAck(const char* msg)
: EasyProtocol(msg, MSG_CLI_CMS_DEVICE_LIST_ACK)
{
}

bool EasyDarwinDeviceListAck::AddDevice(EasyDarwinDevice &device)
{	
	Json::Value value;
	value["DeviceSerial"] = device.DeviceSerial;
	value["DeviceName"] = device.DeviceName;
	value["DeviceSnap"] = device.DeviceSnap;
	root[EASY_TAG_ROOT][EASY_TAG_BODY]["Devices"].append(value);
	return true;
}

int EasyDarwinDeviceListAck::StartGetDevice()
{
	devices.clear();	
	
	int size = root[EASY_TAG_ROOT][EASY_TAG_BODY]["Devices"].size();  

	for(int i = 0; i < size; i++)  
	{  
		Json::Value &json_device = root[EASY_TAG_ROOT][EASY_TAG_BODY]["Devices"][i];  
		EasyDarwinDevice device;
		device.DeviceName = json_device["DeviceName"].asString();
		device.DeviceSerial = json_device["DeviceSerial"].asString();    
		device.DeviceSnap = json_device["DeviceSnap"].asString();

		devices.push_back(device);
	}  
	
	return devices.size();	
}

bool EasyDarwinDeviceListAck::GetNextDevice(EasyDarwinDevice &device)
{
	if(devices.empty())
    {
        return false;
    }
    
    device = devices.front();
    devices.pop_front();   

    return true;
}

EasyDarwinDeviceSnapUpdateReq::EasyDarwinDeviceSnapUpdateReq()
: EasyProtocol(MSG_DEV_CMS_SNAP_UPDATE_REQ)
{
}

EasyDarwinDeviceSnapUpdateReq::EasyDarwinDeviceSnapUpdateReq(const char *msg)
: EasyProtocol(msg, MSG_DEV_CMS_SNAP_UPDATE_REQ)
{
}


void EasyDarwinDeviceSnapUpdateReq::SetImageData(const char* sImageBase64Data, size_t iBase64DataSize)
{
	//std::string data;
	//data.assign(sImageBase64Data, iBase64DataSize);
	SetBodyValue("Img", sImageBase64Data);
}

bool EasyDarwinDeviceSnapUpdateReq::GetImageData(std::string &sImageBase64Data)
{
	sImageBase64Data.clear();
	sImageBase64Data = GetBodyValue("Img");
	return !sImageBase64Data.empty();
}

EasyDarwinDeviceSnapUpdateAck::EasyDarwinDeviceSnapUpdateAck()
: EasyProtocol(MSG_DEV_CMS_SNAP_UPDATE_ACK)
{
}

EasyDarwinDeviceSnapUpdateAck::EasyDarwinDeviceSnapUpdateAck(const char *msg)
: EasyProtocol(msg, MSG_DEV_CMS_SNAP_UPDATE_ACK)
{
}
*/

EasyMsgSCStartHLSACK::EasyMsgSCStartHLSACK()
: EasyProtocol(MSG_SC_START_HLS_ACK)
{
}

EasyMsgSCStartHLSACK::EasyMsgSCStartHLSACK(const char *msg)
: EasyProtocol(msg, MSG_SC_START_HLS_ACK)
{
}

void EasyMsgSCStartHLSACK::SetStreamName(const char* sName)
{
	SetBodyValue(EASY_TAG_L_NAME, sName);
}

void EasyMsgSCStartHLSACK::SetStreamURL(const char* sURL)
{
	SetBodyValue(EASY_TAG_L_URL, sURL);
}

EasyMsgSCHLSessionListACK::EasyMsgSCHLSessionListACK()
: EasyProtocol(MSG_SC_HLS_SESSION_LIST_ACK)
{
}

EasyMsgSCHLSessionListACK::EasyMsgSCHLSessionListACK(const char* msg)
: EasyProtocol(msg, MSG_SC_HLS_SESSION_LIST_ACK)
{
}

bool EasyMsgSCHLSessionListACK::AddSession(EasyDarwinHLSession &session)
{	
	Json::Value value;
	value[EASY_TAG_L_INDEX] = session.index;
	value[EASY_TAG_L_NAME] = session.SessionName;
	value[EASY_TAG_L_URL] = session.HlsUrl;
	value[EASY_TAG_L_SOURCE] = session.sourceUrl;
	value[EASY_TAG_BITRATE] = session.bitrate;
	root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_SESSIONS].append(value);
	return true;
}


EasyMsgSCDeviceListACK::EasyMsgSCDeviceListACK(EasyDevices & devices, size_t cseq, size_t error)
: EasyProtocol(MSG_SC_DEVICE_LIST_ACK)
, devices_(devices)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_ERROR_NUM, error);
	SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

	SetBodyValue(EASY_TAG_DEVICE_COUNT, devices.size());
	for (EasyDevices::iterator it = devices.begin(); it != devices.end(); it++)
	{
		Json::Value value;
		value[EASY_TAG_DEVICESERIAL] = it->serial_;
		value[EASY_TAG_DEVICE_NAME] = it->name_;
		value[EASY_TAG_DEVICE_TAG] = it->tag_;
		//value["Status"] = it->status_;
		root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES].append(value);
	}
}

EasyMsgSCDeviceListACK::EasyMsgSCDeviceListACK(const char * msg)
: EasyProtocol(msg, MSG_SC_DEVICE_LIST_ACK)
{
	devices_.clear();
	int size = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES].size();

	for (int i = 0; i < size; i++)
	{
		Json::Value &json_ = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES][i];
		EasyDevice device;
		device.name_ = json_[EASY_TAG_DEVICE_NAME].asString();
		device.serial_ = json_[EASY_TAG_DEVICESERIAL].asString();
		device.tag_ = json_[EASY_TAG_DEVICE_TAG].asString();
		//device.status_ = json_["Status"].asString();
		devices_.push_back(device);
	}
}


EasyMsgSCDeviceInfoACK::EasyMsgSCDeviceInfoACK(EasyDevices & cameras, string devcei_serial, size_t cseq, size_t error)
: EasyProtocol(MSG_SC_CAMERA_LIST_ACK)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_ERROR_NUM, error);
	SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

	SetBodyValue(EASY_TAG_DEVICESERIAL, devcei_serial);
	SetBodyValue(EASY_TAG_CAMERA_COUNT, cameras.size());
	for (EasyDevices::iterator it = cameras.begin(); it != cameras.end(); it++)
	{
		Json::Value value;
		value[EASY_TAG_CAMERASERIAL] = it->serial_;
		value[EASY_TAG_CAMERA_NAME] = it->name_;
        value[EASY_TAG_STATUS] = it->status_;
		root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CAMERAS].append(value);
	}
}

EasyMsgSCDeviceInfoACK::EasyMsgSCDeviceInfoACK(const char * msg)
: EasyProtocol(msg, MSG_SC_CAMERA_LIST_ACK)
{
	cameras_.clear();
	int size = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CAMERAS].size();

	for (int i = 0; i < size; i++)
	{
		Json::Value &json_ = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CAMERAS][i];
		EasyDevice camera;
		camera.name_ = json_[EASY_TAG_CAMERASERIAL].asString();
		camera.serial_ = json_[EASY_TAG_CAMERA_NAME].asString();
        camera.status_ = json_[EASY_TAG_STATUS].asString();
		cameras_.push_back(camera);
	}
}

EasyMsgSCGetStreamACK::EasyMsgSCGetStreamACK(EasyJsonValue &body, size_t cseq, size_t error)
: EasyProtocol(MSG_SC_GET_STREAM_ACK)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_ERROR_NUM, error);
	SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

	for (EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyMsgSCGetStreamACK::EasyMsgSCGetStreamACK(const char *msg)
: EasyProtocol(msg, MSG_SC_GET_STREAM_ACK)
{
}

EasyMsgSCFreeStreamACK::EasyMsgSCFreeStreamACK(EasyJsonValue & body, size_t cseq, size_t error)
: EasyProtocol(MSG_SC_FREE_STREAM_ACK)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_ERROR_NUM, error);
	SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

	for (EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyMsgSCFreeStreamACK::EasyMsgSCFreeStreamACK(const char * msg)
: EasyProtocol(msg, MSG_SC_FREE_STREAM_ACK)
{
}

EasyMsgDSPostSnapREQ::EasyMsgDSPostSnapREQ(EasyJsonValue & body, size_t cseq)
: EasyProtocol(MSG_DS_POST_SNAP_REQ)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);

	for (EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyMsgDSPostSnapREQ::EasyMsgDSPostSnapREQ(const char * msg)
: EasyProtocol(msg, MSG_DS_POST_SNAP_REQ)
{
}

EasyMsgSDPostSnapACK::EasyMsgSDPostSnapACK(EasyJsonValue & body, size_t cseq, size_t error)
: EasyProtocol(MSG_SD_POST_SNAP_ACK)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_ERROR_NUM, error);
	SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

	for (EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyMsgSDPostSnapACK::EasyMsgSDPostSnapACK(const char * msg)
: EasyProtocol(msg, MSG_SD_POST_SNAP_ACK)
{
}

EasyMsgExceptionACK::EasyMsgExceptionACK(size_t cseq, size_t error)
:EasyProtocol(MSG_SC_EXCEPTION)
{
	SetHeaderValue(EASY_TAG_CSEQ, cseq);
	SetHeaderValue(EASY_TAG_ERROR_NUM, error);
	SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));
}

EasyMsgSCRTSPPushSessionListACK::EasyMsgSCRTSPPushSessionListACK()
: EasyProtocol(MSG_SC_RTSP_PUSH_SESSION_LIST_ACK)
{
}

EasyMsgSCRTSPPushSessionListACK::EasyMsgSCRTSPPushSessionListACK(const char* msg)
: EasyProtocol(msg, MSG_SC_RTSP_PUSH_SESSION_LIST_ACK)
{
}
bool EasyMsgSCRTSPPushSessionListACK::AddSession(EasyDarwinRTSPSession &session)
{	
	Json::Value value;
	value[EASY_TAG_L_INDEX] = session.index;
	value[EASY_TAG_L_URL] = session.Url;
	value[EASY_TAG_L_NAME] = session.Name;
	root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_SESSIONS].append(value);
	return true;
}

EasyMsgSCListRecordACK::EasyMsgSCListRecordACK()
: EasyProtocol(MSG_SC_LIST_RECORD_ACK)
{
}

EasyMsgSCListRecordACK::EasyMsgSCListRecordACK(const char *msg)
: EasyProtocol(msg, MSG_SC_LIST_RECORD_ACK)
{
}

bool EasyMsgSCListRecordACK::AddRecord(std::string record)
{
	Json::Value value;	
	value[EASY_TAG_L_URL] = record;	
	int pos = record.find_last_of('/');	
	value[EASY_TAG_L_TIME] = record.substr(pos - 14, 14); // /20151123114500/*.m3u8
	root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_RECORDS].append(value);
	return true;
}

//add,紫光，start
strDevice::strDevice()
{
	snapJpgPath_.clear();
}
bool strDevice::GetDevInfo(const char* json)//由JSON文本得到设备信息
{
	EasyProtocol proTemp(json);
	do 
	{
		string strTerminalType	=	proTemp.GetHeaderValue(EASY_TAG_TERMINAL_TYPE);//获取设备类型
		string strAppType		=	proTemp.GetHeaderValue(EASY_TAG_APP_TYPE);//获取App类型
		serial_					=	proTemp.GetBodyValue(EASY_TAG_SERIAL);//获取设备序列号

		if(strTerminalType.size()<=0||serial_.size()<=0||strAppType.size()<=0)
			break;
		
		eDeviceType=(EasyDarwinTerminalType)EasyProtocol::GetTerminalType(strTerminalType);
		if(eDeviceType==-1)
			break;
		eAppType=(EasyDarwinAppType)EasyProtocol::GetAppType(strAppType);
		if(eAppType==-1)
			break;

		name_			=	proTemp.GetBodyValue(EASY_TAG_NAME);//获取设备名称
		password_		=	proTemp.GetBodyValue(EASY_TAG_TOKEN);//设备认证码
		tag_			=	proTemp.GetBodyValue(EASY_TAG_TAG);//标签
		channelCount_	=	proTemp.GetBodyValue(EASY_TAG_CHANNEL_COUNT);//当前设备包含的摄像头数量

		if(eAppType==EASY_APP_TYPE_NVR)//如果设备类型是NVR，则需要获取摄像头信息
		{
			cameras_.clear();
			Json::Value *proot=proTemp.GetRoot();
			int size = (*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].size(); //数组大小 

			for(int i = 0; i < size; i++)  
			{  
				Json::Value &json_camera = (*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS][i];  
				EasyDevice camera;
				camera.name_ = json_camera[EASY_TAG_NAME].asString();
				camera.channel_ = json_camera[EASY_TAG_CHANNEL].asString();		
				camera.status_ = json_camera[EASY_TAG_STATUS].asString();	
				cameras_.push_back(camera);
			}  
		}
		return true;
	} while (0);
	//执行到这说明得到的设备信息是错误的
	return false;
}
void strDevice::HoldSnapPath(std::string strJpgPath,std::string strChannel)//保留快照的时间属性
{
	if(EASY_APP_TYPE_CAMERA==eDeviceType)//如果是摄像头类型，那么只保留一个路径
		snapJpgPath_=strJpgPath;
	else//否则就要保留到对应的摄像头属性里
	{
		EasyDevicesIterator it;
		for(it=cameras_.begin();it!=cameras_.end();it++)
		{
			if(it->channel_==strChannel)
				it->snapJpgPath_=strJpgPath;
		}
	}
}
void EasyDarwinRSP::SetHead(EasyJsonValue &header)
{
	for(EasyJsonValue::iterator it = header.begin(); it != header.end(); it++)
	{
		SetHeaderValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}
void EasyDarwinRSP::SetBody(EasyJsonValue &body)
{
	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}
void EasyDarwinRecordListRSP::AddRecord(std::string record)
{
	Json::Value value;	
	value[EASY_TAG_L_URL] = record;	
	int pos = record.find_last_of('/');	
	value[EASY_TAG_L_TIME] = record.substr(pos - 14, 14); // /20151123114500/*.m3u8
	root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_RECORDS].append(value);
}
//add,紫光，end
}
}//namespace

