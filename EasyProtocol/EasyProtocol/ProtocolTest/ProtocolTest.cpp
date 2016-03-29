/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   ProtocolTest.cpp
 * Author: wellsen
 * 
 * Created on 2014年11月15日, 下午8:20
 */

#include <sstream>

#include "ProtocolTest.h"

#include <iostream>
#include <stdio.h>
using namespace std;

using namespace EasyDarwin::Protocol;

ProtocolTest::ProtocolTest()
{
}


ProtocolTest::~ProtocolTest()
{
}

void ProtocolTest::PrintMsg(const char* msg)
{
    cout << msg << endl;
}

//void ProtocolTest::TestCreateSessionAck()
//{
//    //use namepace 
//    EasyDarwin::Protocol::EasyDSSCreateSessionAck msgpack;
//    cout << "****************test CreateSessionAck pack*****************" << endl;
//    msgpack.Pack(1000, "10000100010001", "www.easydss.org", 554, EASYDARWIN_ERROR_SUCCESS_OK);    
//    PrintMsg(msgpack.GetMsg().c_str());  
//    
//    cout << "****************test CreateSessionAck parse*****************" << endl;
//    EasyDSSCreateSessionAck msgparse(msgpack.GetMsg().c_str());
//    cout << "Version: " << msgparse.GetVersion() <<endl;
//    cout << "CSeq: " << msgparse.GetCSeq() << endl;
//    cout << "SessionID: " << msgparse.GetSessionID() << endl;
//    cout << "ServerAddress: " << msgparse.GetRedirectServerAddress() << endl;
//    cout << "ServerPort: " << msgparse.GetRedirectServerPort() << endl;
//    cout << "ErrorNum: " << msgparse.GetErrorNum() << endl;
//    cout << "ErrorString: " << msgparse.GetErrorString() << endl;
//}
//
//void ProtocolTest::TestCreateSessionReq()
//{
//    EasyDSSCreateSessionReq msg;
//    cout << "****************test CreateSessionReq pack*****************" << endl;
//    msg.Pack(100, EASYDARWIN_APP_TYPE_PC, EASYDARWIN_TERMINAL_TYPE_USER);    
//    
//    string buffer = msg.GetMsg();
//    
//    PrintMsg(buffer.c_str());  
//
//    cout << "****************test CreateSessionReq parse*****************" << endl;
//    EasyDSSCreateSessionAck msgparse(buffer.c_str());
//    //some tag not exist, will report a log
//    cout << "Version: " << msgparse.GetVersion() <<endl;
//    cout << "TerminalType: " << msgparse.GetTerminalType() << endl;
//    cout << "CSeq: " << msgparse.GetCSeq() << endl;
//    cout << "SessionID: " << msgparse.GetSessionID() << endl;
//    cout << "ServerAddress: " << msgparse.GetRedirectServerAddress() << endl;
//    cout << "ServerPort: " << msgparse.GetRedirectServerPort() << endl;
//    cout << "ErrorNum: " << msgparse.GetErrorNum() << endl;
//    cout << "ErrorString: " << msgparse.GetErrorString() << endl;    
//}

void ProtocolTest::TestRegisterReq()
{
	EasyDevices channels;

	for(int i = 0; i < 5; i++)
	{		
		char name[20];
		sprintf(name, "camera00%i", i + 1);
		EasyDevice camera(name, name, "online");
		channels.push_back(camera);
	}

	EasyNVR nvr("nvr001", "nvr001_", "123456", channels);
	

	EasyDarwinRegisterReq req(nvr);


	string msg = req.GetMsg();

	PrintMsg(msg.c_str());

	
	EasyDarwinRegisterReq parse(msg.c_str());

	
	cout << "header.version = " << parse.GetHeaderValue(EASYDARWIN_TAG_VERSION) << endl;
	cout << "header.TerminialType = " << parse.GetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE) << endl;
	cout << "header.MessageType = " << parse.GetMessageType() << ", " << EasyProtocol::GetMsgTypeString(parse.GetMessageType()) << endl;
	
	cout << "body.SerialNumber = " << parse.GetNVR().serial_ << endl;
	cout << "body.CameraCount = " << parse.GetNVR().channels_.size() << endl;
	cout << "body.AuthCode = " << parse.GetNVR().password_ << endl;
	cout << "body.camera1 = " << parse.GetNVR().channels_[0].name_ << endl;
	//////

}

void ProtocolTest::TestRegisterRsp()
{
	EasyJsonValue body;
	body["DeviceSerial"] = "000000000001";
	body["SessionID"] = "a939cd77c7cb4af6b2bd8f2090562b91";
	EasyDarwinRegisterRSP rsp(body, 1, 200);

	string msg = rsp.GetMsg();

	PrintMsg(msg.c_str());

	
	EasyDarwinRegisterRSP rsp_parse(msg.c_str());

	cout << "session id = " << rsp_parse.GetBodyValue("SessionID") << endl;
	cout << "device serial = " << rsp_parse.GetBodyValue("DeviceSerial") << endl;

}

void ProtocolTest::TestDeviceStreamReq()
{
	EasyJsonValue body;
	body["DeviceSerial"] = "000000000001";
	body["CameraSerial"] = "CameraSerial_01";
	body["StreamID"] = 2;
	body["Protocol"] = "RTSP";
	body["DssIP"] = "112.42.1.2";
	body["DssPort"] = 554;
	
	EasyDarwinDeviceStreamReq req(body, 1);
	

	string msg = req.GetMsg();

	PrintMsg(msg.c_str());

	EasyDarwinDeviceStreamReq rsp_parse(msg.c_str());
	cout << "DssIP = " << rsp_parse.GetBodyValue("DssIP") << endl;
	cout << "CameraSerial = " << rsp_parse.GetBodyValue("CameraSerial") << endl;
	cout << "StreamID = " << rsp_parse.GetBodyValue("StreamID") << endl;
	cout << "DssPort = " << rsp_parse.GetBodyValue("DssPort") << endl;
}

void ProtocolTest::TestDeviceListRsp()
{
	
	EasyDevices devices;
	char n[10];
	for(int i = 0; i < 5; i++)
	{
		sprintf(n, "%02d", i + 1);
		EasyDevice device;
		device.name_ = string("device") + n;
		device.serial_ = string("serial") + n;
		
		devices.push_back(device);
	}

	EasyDarwinDeviceListRsp req(devices);
	
	

	string msg = req.GetMsg();

	PrintMsg(msg.c_str());

	EasyDarwinDeviceListRsp parse(msg.c_str());
	
	devices.clear();

	devices = parse.GetDevices();
		
	cout << "body.device_count = " << parse.GetBodyValue("DeviceCount") << endl;

	for (EasyDevices::iterator it = devices.begin(); it != devices.end(); it++)
	{
		cout << it->name_ << "," << it->serial_ << endl;
	}
}

void ProtocolTest::TestCameraListRsp()
{
	
	EasyDevices devices;
	char n[10];
	for(int i = 0; i < 5; i++)
	{
		sprintf(n, "%02d", i + 1);
		EasyDevice device;
		device.name_ = string("camera") + n;
		device.serial_ = string("serial") + n;
		
		devices.push_back(device);
	}

	EasyDarwinCameraListRsp req(devices, "nvr00001");
	
	

	string msg = req.GetMsg();

	PrintMsg(msg.c_str());

	EasyDarwinCameraListRsp parse(msg.c_str());
	
	devices.clear();

	devices = parse.GetCameras();
		
	cout << "body.device_count = " << parse.GetBodyValue("CameraCount") << ", " << devices.size()<< endl;

	for (EasyDevices::iterator it = devices.begin(); it != devices.end(); it++)
	{
		cout << it->name_ << "," << it->serial_ << endl;
	}
}

void ProtocolTest::TestClientStartStreamRsp()
{
	EasyJsonValue body;
	body["DeviceSerial"] = "000000000001";
	body["CameraSerial"] = "CameraSerial_01";
	body["URL"] = "112.42.1.2";
	body["Protocol"] = "RTSP";		
	body["StreamID"] = 2;
	EasyDarwinClientStartStreamRsp req(body, 1, 200);


	string msg = req.GetMsg();

	PrintMsg(msg.c_str());

	EasyDarwinDeviceStreamReq rsp_parse(msg.c_str());
	cout << "URL = " << rsp_parse.GetBodyValue("URL") << endl;
	cout << "DeviceSerial = " << rsp_parse.GetBodyValue("DeviceSerial") << endl;
	cout << "CameraSerial = " << rsp_parse.GetBodyValue("CameraSerial") << endl;
	cout << "StreamID = " << rsp_parse.GetBodyValue("StreamID") << endl;
	cout << "Protocol = " << rsp_parse.GetBodyValue("Protocol") << endl;
}

void ProtocolTest::TestDeviceStreamRsp()
{
	EasyJsonValue body;
	body["DeviceSerial"] = "000000000001";
	body["CameraSerial"] = "CameraSerial_01";
	body["StreamID"] = 0;

	EasyDarwinDeviceStreamRsp rsp(body, 1, 200);

	string msg = rsp.GetMsg();

	PrintMsg(msg.c_str());

	EasyDarwinDeviceStreamRsp rsp_parse(msg.c_str());
	
	cout << "Error: " << rsp_parse.GetHeaderValue("ErrorNum")<< ", " << rsp_parse.GetHeaderValue("ErrorString") << endl;
	cout << "CameraSerial = " << rsp_parse.GetBodyValue("CameraSerial") << endl;
	cout << "DeviceSerial = " << rsp_parse.GetBodyValue("DeviceSerial") << endl;

}

void ProtocolTest::TestDeviceStreamStopReq()
{
	EasyJsonValue body;
	body["DeviceSerial"] = "000000000001";
	body["CameraSerial"] = "CameraSerial_01";
	body["StreamID"] = 1;	 

	EasyDarwinDeviceStreamStop req(body, 1);

	string msg = req.GetMsg();

	PrintMsg(msg.c_str());
	
	EasyDarwinDeviceStreamStop parse(msg.c_str());

	cout << "StreamID: " << parse.GetBodyValue("StreamID") << endl;
	cout << "CameraSerial = " << parse.GetBodyValue("CameraSerial") << endl;
	cout << "DeviceSerial = " << parse.GetBodyValue("DeviceSerial") << endl;

}

void ProtocolTest::TestDeviceStreamStopRsp()
{
	EasyJsonValue body;
	body["DeviceSerial"] = "000000000001";
	body["CameraSerial"] = "CameraSerial_01";
	body["StreamID"] = 0;

	EasyDarwinDeviceStreamStopRsp rsp(body, 1, 404);

	string msg = rsp.GetMsg();

	PrintMsg(msg.c_str());

	EasyDarwinDeviceStreamStopRsp rsp_parse(msg.c_str());
	
	cout << "Error: " << rsp_parse.GetHeaderValue("ErrorNum")<< ", " << rsp_parse.GetHeaderValue("ErrorString") << endl;
	cout << "CameraSerial = " << rsp_parse.GetBodyValue("CameraSerial") << endl;
	cout << "DeviceSerial = " << rsp_parse.GetBodyValue("DeviceSerial") << endl;
}

void ProtocolTest::TestDeviceSnapReq()
{
	EasyJsonValue body;
	body["DeviceSerial"] = "000000000001";
	body["CameraSerial"] = "CameraSerial_01";
	body["Type"] = "JPEG";
	body["Time"] = "2015-07-20 12:55:30";
	body["Image"] = "Base64Encode_Image_Data";
	EasyDarwinDeviceUpdateSnapReq req(body, 1);
	
	string msg = req.GetMsg();

	PrintMsg(msg.c_str());

	EasyDarwinDeviceUpdateSnapReq rsp_parse(msg.c_str());
	
	cout << "DeviceSerial = " << rsp_parse.GetBodyValue("DeviceSerial") << endl;
	cout << "CameraSerial = " << rsp_parse.GetBodyValue("CameraSerial") << endl;
	cout << "Type = " << rsp_parse.GetBodyValue("Type") << endl;
	cout << "Time = " << rsp_parse.GetBodyValue("Time") << endl;
	cout << "Image = " << rsp_parse.GetBodyValue("Image") << endl;
}

void ProtocolTest::TestDeviceSnapRsp()
{
	EasyJsonValue body;
	body["DeviceSerial"] = "000000000001";
	body["CameraSerial"] = "CameraSerial_01";

	EasyDarwinDeviceUpdateSnapRsp rsp(body, 1, 200);

	string msg = rsp.GetMsg();

	PrintMsg(msg.c_str());

	EasyDarwinDeviceUpdateSnapRsp rsp_parse(msg.c_str());

	cout << "Error: " << rsp_parse.GetHeaderValue("ErrorNum") << ", " << rsp_parse.GetHeaderValue("ErrorString") << endl;
	cout << "CameraSerial = " << rsp_parse.GetBodyValue("CameraSerial") << endl;
	cout << "DeviceSerial = " << rsp_parse.GetBodyValue("DeviceSerial") << endl;
}
