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
	EasyDarwinRegisterReq req;

	///we should move this to class EasyDarwinRegisterReq
	req.SetHeaderValue(EASYDARWIN_TAG_VERSION, "1.0");
	req.SetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(EASYDARWIN_TERMINAL_TYPE_CAMERA).c_str());
	req.SetHeaderValue(EASYDARWIN_TAG_CSEQ, "1");	
	
	req.SetBodyValue("SerialNumber", "SerialNumber_abcdefg");
	req.SetBodyValue("AuthCode", "AuthCode_pwd123");
	req.SetBodyValue("LiveStatus", "1");
	req.SetBodyValue("DssIP", "darwin.org");
	//////

	string msg = req.GetMsg();

	PrintMsg(msg.c_str());

	/*
	AVSXmlUtil xml;
	if (!xml.Read(msg))
	{
		printf("AVSXmlUtil read xml errror\n");
	}

	AVSXmlObject obj = xml.GetChild("EasyDarwin");
	if (obj == NULL)
	{
		printf("not found EasyDarwin\n");
		return;
	}

	AVSXmlUtil easydarwin(obj);

	obj = easydarwin.GetChild("Header");
	if (obj == NULL)
	{
		printf("not found Header\n");
		return;
	}
	AVSXmlUtil header(obj);
	string value;
	header.GetValueAsString("Version", value);
	printf("header.version = %s\n", value.c_str());
	header.GetValueAsString("TerminalType", value);
	printf("header.TerminalType = %s\n", value.c_str());
	AVSXmlUtil body = easydarwin.GetChild("Body");

	body.GetValueAsString("SerialNumber", value);
	printf("body.SerialNumber = %s\n", value.c_str());
	*/

	EasyDarwinRegisterReq parse(msg.c_str());

	///we should move this to class EasyDarwinRegisterReq
	cout << "header.version = " << parse.GetHeaderValue(EASYDARWIN_TAG_VERSION) << endl;
	cout << "header.TerminialType = " << parse.GetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE) << endl;
	cout << "header.MessageType = " << parse.GetMessageType() << ", " << EasyProtocol::GetMsgTypeString(parse.GetMessageType()) << endl;
	
	cout << "body.SerialNumber = " << parse.GetBodyValue("SerialNumber") << endl;
	cout << "body.DssIP = " << parse.GetBodyValue("DssIP") << endl;
	cout << "body.AuthCode = " << parse.GetBodyValue("AuthCode") << endl;
	//////

}

void ProtocolTest::TestDeviceListRsp()
{
	EasyDarwinDeviceListAck req;
	req.SetHeaderValue(EASYDARWIN_TAG_VERSION, "1.0");
	req.SetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(EASYDARWIN_TERMINAL_TYPE_CAMERA).c_str());
	req.SetHeaderValue(EASYDARWIN_TAG_CSEQ, "1");	
	req.SetBodyValue("DeviceCount", "5");
	char n[10];
	for(int i = 0; i < 5; i++)
	{
		sprintf(n, "%02d", i + 1);
		EasyDarwinDevice device;
		device.DeviceName = string("device") + n;
		device.DeviceSerial = string("serial") + n;
		device.DeviceSnap = device.DeviceName+"_snap.jpg";
		req.AddDevice(device);
	}

	string msg = req.GetMsg();

	PrintMsg(msg.c_str());

	EasyDarwinDeviceListAck parse(msg.c_str());
	
	cout << "header.version = " << parse.GetHeaderValue(EASYDARWIN_TAG_VERSION) << endl;
	int count = parse.StartGetDevice();
	cout << "body.device_count = " << parse.GetBodyValue("DeviceCount") << endl;
	EasyDarwinDevice device;
	
	while(parse.GetNextDevice(device))
	{
		cout << device.DeviceName << "," << device.DeviceSerial << ", " << device.DeviceSnap<<endl;
	}


}

void ProtocolTest::TestDeviceSnapReq()
{
	EasyDarwinDeviceSnapUpdateReq req;
	req.SetHeaderValue(EASYDARWIN_TAG_VERSION, "1.0");
	req.SetHeaderValue(EASYDARWIN_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(EASYDARWIN_TERMINAL_TYPE_CAMERA).c_str());
	req.SetHeaderValue(EASYDARWIN_TAG_CSEQ, "1");	

	req.SetBodyValue("Time", EasyUtil::TimeT2String(EASYDARWIN_TIME_FORMAT_YYYYMMDDHHMMSS, EasyUtil::NowTime()).c_str());
	req.SetBodyValue("Img", "Base64ImageData===========");

	string msg = req.GetMsg();
	PrintMsg(msg.c_str());

	EasyDarwinDeviceSnapUpdateReq parse(msg.c_str());
	
	//get image method 1
	string image;
	parse.GetImageData(image);

	//get image method 2
	string img = parse.GetBodyValue("Img");
	cout << image << "\n\n" <<endl;

	cout << img << endl;
}
