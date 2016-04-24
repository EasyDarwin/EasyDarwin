/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
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
}

EasyDevice::EasyDevice(std::string serial, std::string name)
: serial_(serial)
, name_(name)
{
}

EasyDevice::EasyDevice(std::string serial, std::string name, std::string status)
: serial_(serial)
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

EasyDarwinRegisterReq::EasyDarwinRegisterReq(EasyNVR &nvr, size_t cseq)
: EasyProtocol(MSG_DEV_CMS_REGISTER_REQ)
, nvr_(nvr)
{
	SetHeaderValue("CSeq", cseq);
	
	SetBodyValue("DeviceSerial", nvr.serial_);
	SetBodyValue("DeviceName", nvr.name_);
	SetBodyValue("DeviceTag", nvr.tag_);
	SetBodyValue("AuthCode", nvr.password_);
	SetBodyValue("CameraCount", nvr.channels_.size());
	for(EasyDevices::iterator it = nvr.channels_.begin(); it != nvr.channels_.end(); it++)
	{
		Json::Value value;
		value["CameraSerial"] = it->serial_;
		value["CameraName"] = it->name_;		
		value["Status"] = it->status_;
		root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Cameras"].append(value);		
	}
}

EasyDarwinRegisterReq::EasyDarwinRegisterReq(const char* msg)
: EasyProtocol(msg, MSG_DEV_CMS_REGISTER_REQ)
{
	nvr_.serial_ = GetBodyValue("DeviceSerial");
	nvr_.name_ = GetBodyValue("DeviceName");
	nvr_.tag_ = GetBodyValue("DeviceTag");
	nvr_.password_ = GetBodyValue("AuthCode");
	
	nvr_.channels_.clear();

	int size = root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Cameras"].size();  

	for(int i = 0; i < size; i++)  
	{  
		Json::Value &json_camera = root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Cameras"][i];  
		EasyDevice camera;
		camera.name_ = json_camera["CameraName"].asString();
		camera.serial_ = json_camera["CameraSerial"].asString();		
		camera.status_ = json_camera["Status"].asString();	
		nvr_.channels_.push_back(camera);
	}  
}


EasyDarwinRegisterRSP::EasyDarwinRegisterRSP(EasyJsonValue &body, size_t cseq, size_t error)
: EasyProtocol(MSG_DEV_CMS_REGISTER_RSP)
{
	SetHeaderValue("CSeq", cseq);
	SetHeaderValue("ErrorNum", error);
	SetHeaderValue("ErrorString", GetErrorString(error));

	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyDarwinRegisterRSP::EasyDarwinRegisterRSP(const char* msg)
: EasyProtocol(msg, MSG_DEV_CMS_REGISTER_RSP)
{
}

EasyDarwinDeviceStreamReq::EasyDarwinDeviceStreamReq(EasyJsonValue &body, size_t cseq)
: EasyProtocol(MSG_CMS_DEV_STREAM_START_REQ)
{
	SetHeaderValue("CSeq", cseq);

	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyDarwinDeviceStreamReq::EasyDarwinDeviceStreamReq(const char *msg)
: EasyProtocol(msg, MSG_CMS_DEV_STREAM_START_REQ)
{
}

EasyDarwinDeviceStreamRsp::EasyDarwinDeviceStreamRsp(EasyJsonValue &body, size_t cseq, size_t error)
: EasyProtocol(MSG_CMS_DEV_STREAM_START_RSP)
{
	SetHeaderValue("CSeq", cseq);
	SetHeaderValue("ErrorNum", error);
	SetHeaderValue("ErrorString", GetErrorString(error));

	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyDarwinDeviceStreamRsp::EasyDarwinDeviceStreamRsp(const char *msg)
: EasyProtocol(msg, MSG_CMS_DEV_STREAM_START_RSP)
{
}

EasyDarwinDeviceStreamStop::EasyDarwinDeviceStreamStop(EasyJsonValue &body, size_t cseq)
:EasyProtocol(MSG_CMS_DEV_STREAM_STOP_REQ)
{
	SetHeaderValue("CSeq", cseq);
	
	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyDarwinDeviceStreamStop::EasyDarwinDeviceStreamStop(const char *msg)
: EasyProtocol(msg, MSG_CMS_DEV_STREAM_STOP_REQ)
{
}

EasyDarwinDeviceStreamStopRsp::EasyDarwinDeviceStreamStopRsp(EasyJsonValue &body, size_t cseq, size_t error)
: EasyProtocol(MSG_CMS_DEV_STREAM_STOP_RSP)
{
	SetHeaderValue("CSeq", cseq);
	SetHeaderValue("ErrorNum", error);
	SetHeaderValue("ErrorString", GetErrorString(error));

	for(EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyDarwinDeviceStreamStopRsp::EasyDarwinDeviceStreamStopRsp(const char *msg)
: EasyProtocol(msg, MSG_CMS_DEV_STREAM_STOP_RSP)
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
	root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Devices"].append(value);
	return true;
}

int EasyDarwinDeviceListAck::StartGetDevice()
{
	devices.clear();	
	
	int size = root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Devices"].size();  

	for(int i = 0; i < size; i++)  
	{  
		Json::Value &json_device = root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Devices"][i];  
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
EasyDarwinEasyHLSAck::EasyDarwinEasyHLSAck()
: EasyProtocol(MSG_CLI_SMS_HLS_ACK)
{
}

EasyDarwinEasyHLSAck::EasyDarwinEasyHLSAck(const char *msg)
: EasyProtocol(msg, MSG_CLI_SMS_HLS_ACK)
{
}

void EasyDarwinEasyHLSAck::SetStreamName(const char* sName)
{
	SetBodyValue("name", sName);
}

void EasyDarwinEasyHLSAck::SetStreamURL(const char* sURL)
{
	SetBodyValue("url", sURL);
}

EasyDarwinHLSessionListAck::EasyDarwinHLSessionListAck()
: EasyProtocol(MSG_CLI_SMS_HLS_LIST_ACK)
{
}

EasyDarwinHLSessionListAck::EasyDarwinHLSessionListAck(const char* msg)
: EasyProtocol(msg, MSG_CLI_SMS_HLS_LIST_ACK)
{
}

bool EasyDarwinHLSessionListAck::AddSession(EasyDarwinHLSession &session)
{	
	Json::Value value;
	value["index"] = session.index;
	value["name"] = session.SessionName;
	value["url"] = session.HlsUrl;
	value["source"] = session.sourceUrl;
	value["Bitrate"] = session.bitrate;
	root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Sessions"].append(value);
	return true;
}

EasyDarwinDeviceListRsp::EasyDarwinDeviceListRsp(EasyDevices & devices, size_t cseq, size_t error)
: EasyProtocol(MSG_CLI_CMS_DEVICE_LIST_RSP)
, devices_(devices)
{
	SetHeaderValue("CSeq", cseq);
	SetHeaderValue("ErrorNum", error);
	SetHeaderValue("ErrorString", GetErrorString(error));

	SetBodyValue("DeviceCount", devices.size());
	for (EasyDevices::iterator it = devices.begin(); it != devices.end(); it++)
	{
		Json::Value value;
		value["DeviceSerial"] = it->serial_;
		value["DeviceName"] = it->name_;
		value["DeviceTag"] = it->tag_;
		//value["Status"] = it->status_;
		root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Devices"].append(value);
	}
}

EasyDarwinDeviceListRsp::EasyDarwinDeviceListRsp(const char * msg)
: EasyProtocol(msg, MSG_CLI_CMS_DEVICE_LIST_RSP)
{
	devices_.clear();
	int size = root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Devices"].size();

	for (int i = 0; i < size; i++)
	{
		Json::Value &json_ = root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Devices"][i];
		EasyDevice device;
		device.name_ = json_["DeviceSerial"].asString();
		device.serial_ = json_["DeviceName"].asString();
		device.tag_ = json_["DeviceTag"].asString();
		//device.status_ = json_["Status"].asString();
		devices_.push_back(device);
	}
}


EasyDarwinCameraListRsp::EasyDarwinCameraListRsp(EasyDevices & cameras, string devcei_serial, size_t cseq, size_t error)
: EasyProtocol(MSG_CLI_CMS_CAMERA_LIST_RSP)
{
	SetHeaderValue("CSeq", cseq);
	SetHeaderValue("ErrorNum", error);
	SetHeaderValue("ErrorString", GetErrorString(error));

	SetBodyValue("DeviceSerial", devcei_serial);
	SetBodyValue("CameraCount", cameras.size());
	for (EasyDevices::iterator it = cameras.begin(); it != cameras.end(); it++)
	{
		Json::Value value;
		value["CameraSerial"] = it->serial_;
		value["CameraName"] = it->name_;
        value["Status"] = it->status_;
		root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Cameras"].append(value);
	}
}

EasyDarwinCameraListRsp::EasyDarwinCameraListRsp(const char * msg)
: EasyProtocol(msg, MSG_CLI_CMS_CAMERA_LIST_RSP)
{
	cameras_.clear();
	int size = root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Cameras"].size();

	for (int i = 0; i < size; i++)
	{
		Json::Value &json_ = root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Cameras"][i];
		EasyDevice camera;
		camera.name_ = json_["CameraSerial"].asString();
		camera.serial_ = json_["CameraName"].asString();
        camera.status_ = json_["Status"].asString();
		cameras_.push_back(camera);
	}
}

EasyDarwinClientStartStreamRsp::EasyDarwinClientStartStreamRsp(EasyJsonValue &body, size_t cseq, size_t error)
: EasyProtocol(MSG_CLI_CMS_STREAM_START_RSP)
{
	SetHeaderValue("CSeq", cseq);
	SetHeaderValue("ErrorNum", error);
	SetHeaderValue("ErrorString", GetErrorString(error));

	for (EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyDarwinClientStartStreamRsp::EasyDarwinClientStartStreamRsp(const char *msg)
: EasyProtocol(msg, MSG_CLI_CMS_STREAM_START_RSP)
{
}

EasyDarwinClientStopStreamRsp::EasyDarwinClientStopStreamRsp(EasyJsonValue & body, size_t cseq, size_t error)
: EasyProtocol(MSG_CLI_CMS_STREAM_STOP_RSP)
{
	SetHeaderValue("CSeq", cseq);
	SetHeaderValue("ErrorNum", error);
	SetHeaderValue("ErrorString", GetErrorString(error));

	for (EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyDarwinClientStopStreamRsp::EasyDarwinClientStopStreamRsp(const char * msg)
: EasyProtocol(msg, MSG_CLI_CMS_STREAM_STOP_RSP)
{
}

EasyDarwinDeviceUpdateSnapReq::EasyDarwinDeviceUpdateSnapReq(EasyJsonValue & body, size_t cseq)
: EasyProtocol(MSG_DEV_CMS_UPDATE_SNAP_REQ)
{
	SetHeaderValue("CSeq", cseq);

	for (EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyDarwinDeviceUpdateSnapReq::EasyDarwinDeviceUpdateSnapReq(const char * msg)
: EasyProtocol(msg, MSG_DEV_CMS_UPDATE_SNAP_REQ)
{
}

EasyDarwinDeviceUpdateSnapRsp::EasyDarwinDeviceUpdateSnapRsp(EasyJsonValue & body, size_t cseq, size_t error)
: EasyProtocol(MSG_DEV_CMS_UPDATE_SNAP_RSP)
{
	SetHeaderValue("CSeq", cseq);
	SetHeaderValue("ErrorNum", error);
	SetHeaderValue("ErrorString", GetErrorString(error));

	for (EasyJsonValue::iterator it = body.begin(); it != body.end(); it++)
	{
		SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
	}
}

EasyDarwinDeviceUpdateSnapRsp::EasyDarwinDeviceUpdateSnapRsp(const char * msg)
: EasyProtocol(msg, MSG_DEV_CMS_UPDATE_SNAP_RSP)
{
}

EasyDarwinExceptionRsp::EasyDarwinExceptionRsp(size_t cseq, size_t error)
:EasyProtocol(MSG_CLI_CMS_EXCEPTION)
{
	SetHeaderValue("CSeq", cseq);
	SetHeaderValue("ErrorNum", error);
	SetHeaderValue("ErrorString", GetErrorString(error));
}

EasyDarwinRTSPPushSessionListAck::EasyDarwinRTSPPushSessionListAck()
: EasyProtocol(MSG_CLI_SMS_PUSH_SESSION_LIST_ACK)
{
}

EasyDarwinRTSPPushSessionListAck::EasyDarwinRTSPPushSessionListAck(const char* msg)
: EasyProtocol(msg, MSG_CLI_SMS_PUSH_SESSION_LIST_ACK)
{
}

bool EasyDarwinRTSPPushSessionListAck::AddSession(EasyDarwinRTSPSession &session)
{	
	Json::Value value;
	value["index"] = session.index;
	value["url"] = session.Url;
	value["name"] = session.Name;
	value["AudienceNum"] = session.numOutputs;
	root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Sessions"].append(value);
	return true;
}

}
}//namespace

