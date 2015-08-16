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

EasyDarwinRegisterRsp::EasyDarwinRegisterRsp()
: EasyProtocol(MSG_DEV_CMS_REGISTER_RSP)
{

}

EasyDarwinRegisterRsp::EasyDarwinRegisterRsp(const char* msg)
: EasyProtocol(msg, MSG_DEV_CMS_REGISTER_RSP)
{

}

EasyDarwinDeviceListRsp::EasyDarwinDeviceListRsp()
: EasyProtocol(MSG_CLI_CMS_DEVICE_LIST_RSP)
{
}

EasyDarwinDeviceListRsp::EasyDarwinDeviceListRsp(const char* msg)
: EasyProtocol(msg, MSG_CLI_CMS_DEVICE_LIST_RSP)
{
}

bool EasyDarwinDeviceListRsp::AddDevice(EasyDarwinDevice &device)
{	
	Json::Value value;
	value["DeviceSerial"] = device.DeviceSerial;
	value["DeviceName"] = device.DeviceName;
	value["DeviceSnap"] = device.DeviceSnap;
	root[EASYDSS_TAG_ROOT][EASYDSS_TAG_BODY]["Devices"].append(value);
	return true;
}

int EasyDarwinDeviceListRsp::StartGetDevice()
{
	devices.clear();	
	
	int size = root[EASYDSS_TAG_ROOT][EASYDSS_TAG_BODY]["Devices"].size();  

	for(int i = 0; i < size; i++)  
	{  
		Json::Value &json_device = root[EASYDSS_TAG_ROOT][EASYDSS_TAG_BODY]["Devices"][i];  
		EasyDarwinDevice device;
		device.DeviceName = json_device["DeviceName"].asString();
		device.DeviceSerial = json_device["DeviceSerial"].asString();    
		device.DeviceSnap = json_device["DeviceSnap"].asString();

		devices.push_back(device);
	}  
	
	return devices.size();	
}

bool EasyDarwinDeviceListRsp::GetNextDevice(EasyDarwinDevice &device)
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

EasyDarwinDeviceSnapUpdateRsp::EasyDarwinDeviceSnapUpdateRsp()
: EasyProtocol(MSG_DEV_CMS_SNAP_UPDATE_RSP)
{
}

EasyDarwinDeviceSnapUpdateRsp::EasyDarwinDeviceSnapUpdateRsp(const char *msg)
: EasyProtocol(msg, MSG_DEV_CMS_SNAP_UPDATE_RSP)
{
}


}}//namespace

