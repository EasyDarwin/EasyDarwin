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

EasyDarwinRegisterReq::EasyDarwinRegisterReq()
: EasyProtocol(MSG_DEV_CMS_REGISTER_REQ)
{

}

EasyDarwinRegisterReq::EasyDarwinRegisterReq(const char* msg)
: EasyProtocol(msg, MSG_DEV_CMS_REGISTER_REQ)
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

EasyDarwinRegisterAck::EasyDarwinRegisterAck()
: EasyProtocol(MSG_DEV_CMS_REGISTER_ACK)
{

}

EasyDarwinRegisterAck::EasyDarwinRegisterAck(const char* msg)
: EasyProtocol(msg, MSG_DEV_CMS_REGISTER_ACK)
{

}

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
	root[EASYDARWIN_TAG_ROOT][EASYDARWIN_TAG_BODY]["Sessions"].append(value);
	return true;
}

}}//namespace

