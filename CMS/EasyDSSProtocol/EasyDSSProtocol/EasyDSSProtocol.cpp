/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#include <EasyDSSProtocol.h>
#include <string.h>
#include <stdio.h>
#include <EasyDSSUtil.h>

namespace EasyDSS { namespace Protocol
{

EasyDarwinRegisterReq::EasyDarwinRegisterReq()
: EasyDSSProtocol(MSG_DEV_CMS_REGISTER_REQ)
{

}

EasyDarwinRegisterReq::EasyDarwinRegisterReq(const char* msg)
: EasyDSSProtocol(msg, MSG_DEV_CMS_REGISTER_REQ)
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
: EasyDSSProtocol(MSG_DEV_CMS_REGISTER_RSP)
{

}

EasyDarwinRegisterRsp::EasyDarwinRegisterRsp(const char* msg)
: EasyDSSProtocol(msg, MSG_DEV_CMS_REGISTER_RSP)
{

}

EasyDarwinDeviceListRsp::EasyDarwinDeviceListRsp()
: EasyDSSProtocol(MSG_CLI_CMS_DEVICE_LIST_RSP)
{
}

EasyDarwinDeviceListRsp::EasyDarwinDeviceListRsp(const char* msg)
: EasyDSSProtocol(msg, MSG_CLI_CMS_DEVICE_LIST_RSP)
{
}

bool EasyDarwinDeviceListRsp::AddDevice(EasyDarwinDevice &device)
{
	AVSXmlUtil device_;
	device_.SetStringValue("DeviceSerial", device.DeviceSerial.c_str());
	device_.SetStringValue("DeviceName", device.DeviceName.c_str());
	device_.SetStringValue("DeviceSnap", device.DeviceSnap.c_str());
	return devices.AddArray("", device_);
}

int EasyDarwinDeviceListRsp::StartGetDevice()
{
	device_list.clear();
    AVSXmlUtil xml;
	if (!xml.Read(json))
	{
		printf("AVSXmlUtil read xml errror\n");
	}

	AVSXmlObject obj = xml.GetChild("EasyDarwin");
	if (obj == NULL)
	{
		printf("not found EasyDarwin\n");
		return 0;
	}

	AVSXmlUtil easydarwin(obj);

	obj = easydarwin.GetChild("Body");
	if (obj == NULL)
	{
		printf("not found Header\n");
		return 0;
	}
	AVSXmlUtil body_(obj);
	
	AVSXmlUtil device_ = body_.GetChild("Devices");

	device_.GetAllChild("", "", device_list);
	
    return device_list.size();   
}

bool EasyDarwinDeviceListRsp::GetNextDevice(EasyDarwinDevice &device)
{
	 if(device_list.empty())
    {
        return false;
    }
    
    AVSXmlUtil device_ = device_list.front();
    
	device_.GetValueAsString("DeviceSerial", device.DeviceSerial);    
	device_.GetValueAsString("DeviceName", device.DeviceName);
	device_.GetValueAsString("DeviceSnap", device.DeviceSnap);
	device_list.pop_front();   

    return true;
}

EasyDarwinDeviceSnapUpdateReq::EasyDarwinDeviceSnapUpdateReq()
: EasyDSSProtocol(MSG_DEV_CMS_SNAP_UPDATE_REQ)
{
}

EasyDarwinDeviceSnapUpdateReq::EasyDarwinDeviceSnapUpdateReq(const char *msg)
: EasyDSSProtocol(msg, MSG_DEV_CMS_SNAP_UPDATE_REQ)
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
: EasyDSSProtocol(MSG_DEV_CMS_SNAP_UPDATE_RSP)
{
}

EasyDarwinDeviceSnapUpdateRsp::EasyDarwinDeviceSnapUpdateRsp(const char *msg)
: EasyDSSProtocol(msg, MSG_DEV_CMS_SNAP_UPDATE_RSP)
{
}


}}//namespace

