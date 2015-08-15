/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   main.cpp
 * Author: wellsen
 *
 * Created on 2014年10月31日, 下午1:52
 */

#include <cstdlib>
#include "../../include/EasyJsonUtil.h"
#include <iostream>

using namespace std;

void ForeachChild(EasyJsonUtil &child, void *pUser);

void testRead()
{
    EasyJsonUtil json;
    if(!json.Read("easydarwin.json", true))
    {
        printf("EasyJsonUtil read json errror\n");
    }
    
	EasyJsonObject obj = json.GetChild("EasyDarwin");
	if (obj == NULL)
	{
		printf("not found EasyDarwin\n");
		return;
	}

	EasyJsonUtil easydarwin(obj);

	obj = easydarwin.GetChild("Header");
	if (obj == NULL)
	{
		printf("not found Header\n");
		return;
	}
	EasyJsonUtil header(obj);
	string value;
	header.GetValueAsString("Version", value);
	printf("header.version = %s\n", value.c_str());
	header.GetValueAsString("TerminalType", value);
	printf("header.TerminalType = %s\n", value.c_str());
	EasyJsonUtil body = easydarwin.GetChild("Body");

	body.GetValueAsString("SerialNumber", value);
	printf("body.SerialNumber = %s\n", value.c_str());
}

void testWrite()
{
	EasyJsonUtil  EasyDarwin, header, body, device;
	header.SetFloatValue("Version", 1.0);
	header.SetStringValue("TerminalType", "Camera");
	header.SetIntValue("CSeq", 1);

	device.SetStringValue("SerialNumber", "abcdefg");
	device.SetStringValue("DssIP", "dss.com");
	//body.add
	EasyDarwin.AddChild("EasyDarwin","Header", header);
	
	EasyJsonUtil arr;

	
	for(int i = 0; i < 3; i++)
	{
		arr.AddArray("", device);
	}

	body.Add("Devices", arr);
	EasyDarwin.AddChild("EasyDarwin","Body", body);
	
	string filename = "easydarwin.json";
	bool ret =EasyDarwin.Write(filename, true);
}


void ForeachChild(EasyJsonUtil &child, void *pUser)
{
    //child.Print();
    int value = -1, port = -1;
    child.GetValueAsInt("Device", value);
    string ip, name;
    child.GetAttributeAsString("Device", "ip", ip);
    child.GetAttributeAsString("Device", "name", name);
    child.GetAttributeAsInt("Device", "port", port);
    
    printf("[ForeachCallBack]value = %d, ip = %s, port = %d, name = %s", value, ip.c_str(), port, name.c_str());
}

void testReadChild()
{
    EasyJsonUtil json;
    json.Read("test.json", true);
    json.Print();
    cout << "===============================================" <<endl;
    
    EasyJsonTag root = "EasyVideo", body = "Body", device ="Devices";

    json.ForeachChild((root + body + device).ToString(), "Device", ForeachChild, NULL);
    //json.ForeachChild("EasyVideo.Body.Devices", "Device", ForeachChild, NULL);
}

int main(int argc, char** argv)
{
    testRead();
    
    testWrite();
    
    //testReadChild();

	getchar();
        
    return 0;
}

