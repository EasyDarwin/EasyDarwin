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
#include "../../../include/EasyDSSProtocol/AVSXmlUtil.h"
#include <iostream>

using namespace std;

/*
 * 
 */

const char* xmltestdata = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
"<EasyVideo>"
"<Header>"
"<Version>1.0</Version>"
"<SessionID></SessionID>"
"<CSeq></CSeq>"
"<MessageType>MSG_CS_CREATE_SESSION_ACK</MessageType>"
"<ErrorNum></ErrorNum>"
"<ErrorString></ErrorString>"
"</Header>"
"<Body>"
"<Redirect>"
"<ServerAddress></ServerAddress>"
"<ServerPort>1206</ServerPort>"
"</Redirect>"
"<Key><![CDATA[abcdefgaddcdedg]]></Key>"
"</Body>"
"</EasyVideo>";

void ForeachChild(AVSXmlUtil &child, void *pUser);

void testRead()
{
    AVSXmlUtil xml;
    if(!xml.Read("easydarwin.json", true))
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

	/*
	AVSXmlList files, childs;
	files.GetAllChild("", "", childs);
	while (!childs.empty())
	{
		AVSXmlUtil child = childs.front();
		
		string filename;
		if (child.GetValueAsString("filename", filename))
		{
			printf("AVSXmlUtil.GetValue[%s] = %s\n", "filename", filename.c_str());
		}

		AVSXmlUtil imagesRoot = child.GetChild("images");
		AVSXmlList images;
		imagesRoot.GetAllChild("", "", images);
		while (!images.empty())
		{
			AVSXmlUtil image = images.front();
			string  value;
			if (image.GetValueAsString("url", value))
			{
				printf("AVSXmlUtil.GetValue[%s] = %s\n", "url", value.c_str());
			}
			int iValue;
			if (image.GetValueAsInt("width", iValue))
			{
				printf("AVSXmlUtil.GetValue[%s] = %d\n", "width", iValue);
			}
			images.pop_front();
		}

		childs.pop_front();
	}
	string filename;
	if (files.GetValueAsString("filename", filename))
	{
		printf("AVSXmlUtil.GetValue[%s] = %s", "filename\n", filename.c_str());
	}

    string version;

    string xmltag = "uploadid";
    if(xml.GetValueAsString(xmltag, version))
    {
        printf("AVSXmlUtil.GetValue[%s] = %s\n", xmltag.c_str(), version.c_str());
    }
    
    double fVersion;
    if(xml.GetValueAsDouble(xmltag, fVersion))
    {
        printf("AVSXmlUtil.GetValue[%s] = %f\n", xmltag.c_str(), fVersion);
    }
    
    xmltag = "EasyVideo.Body.Redirect.ServerPort";
    int port = -1;
    if(xml.GetValueAsInt(xmltag, port))
    {
        printf("AVSXmlUtil.GetValue[%s] = %d\n", xmltag.c_str(), port);
    }
    
    xmltag = "EasyVideo.Body.Key";
    string sKey;
    if(xml.GetValueAsString(xmltag, sKey))
    {
        printf("AVSXmlUtil.GetValue[%s] = %s\n", xmltag.c_str(), sKey.c_str());
    }
	*/
}

void testWrite()
{
	AVSXmlUtil  EasyDarwin, header, body, device;
	header.SetFloatValue("Version", 1.0);
	header.SetStringValue("TerminalType", "Camera");
	header.SetIntValue("CSeq", 1);

	device.SetStringValue("SerialNumber", "abcdefg");
	device.SetStringValue("DssIP", "dss.com");
	//body.add
	EasyDarwin.AddChild("EasyDarwin","Header", header);
	
	AVSXmlUtil arr;

	
	for(int i = 0; i < 3; i++)
	{
		arr.AddArray("", device);
	}

	body.Add("Devices", arr);
	EasyDarwin.AddChild("EasyDarwin","Body", body);
	
	string filename = "easydarwin.json";
	bool ret =EasyDarwin.Write(filename, true);

	/*
    AVSXmlUtil xml;
    xml.SetStringValue("EasyVideo.Header.Version", "1.0");
    xml.SetStringValue("EasyVideo.Header.MessageType", "MSG_CS_CREATE_SESSION_ACK");
    xml.SetIntValue("EasyVideo.Body.Redirect.ServerPort", 1026);
    xml.SetStringValue("EasyVideo.Body.Key", "![CDATA[XXXXXXXXXXXX]]");
    xml.SetStringValue("EasyVideo.Body.Redirect.ServerAddress", "192.168.100.14");
    xml.SetIntValue("EasyVideo.Header.ErrorNum", -1);
    for(int i = 0; i < 5; i++)
    {
        AVSXmlAttributes attrs;
        attrs.AddStringAttribute("name", "deveic_name");
        attrs.AddIntAttribute("id", i);
        attrs.AddStringAttribute("ip", "192.168.100.1");
        attrs.AddIntAttribute("port", 8554);
        xml.SetChildWithIntValue("EasyVideo.Body.Devices", "Device", i,  &attrs);
    }
    AVSXmlAttributes attrs;
    attrs.AddIntAttribute("Count", 5);
    xml.SetAttributes("EasyVideo.Body.Devices", attrs);
    
    string sXml;
    xml.Write(sXml);    
    cout << sXml << endl;
    xml.SetStringValue("EasyVideo.Body.Key", "<![CDATA[ andnew=====//adlfka;el;kadsf;lk ]]>");
       
//    attrs.Clear();
//    attrs.AddIntAttribute("Count", 5);
//    xml.SetStringValue("EasyVideo.Body.DeviceList", "",  &attrs);
    
    for(int i = 0; i < 5; i++)
    {
        AVSXmlUtil child;
        child.SetStringValue("DeviceSerial", "AABBCCDDEEFF1");
        child.SetIntValue("DeviceName", i);
        child.SetStringValue("EffectiveAccessDate.From", "2014-08-31");
        child.SetStringValue("EffectiveAccessDate.To", "2014-09-02");
        child.SetStringValue("EffectiveAccessTime.From", "12:00:00");
        child.SetStringValue("EffectiveAccessTime.To", "21:00:00");
        child.SetIntValue("Status", i);
        
        attrs.Clear();
        attrs.AddIntAttribute("id", i + 1);
        xml.AddChild("EasyVideo.Body.DeviceList", "Device", child, &attrs);
    }
    
    attrs.Clear();
    attrs.AddIntAttribute("Count", 5);
    xml.SetAttributes("EasyVideo.Body.DeviceList", attrs);
    
    sXml = "test.xml";
    xml.Write(sXml, true);
    
    AVSLOG_DEBUG("==========copy xml from others==============");
    AVSXmlUtil xml2(xml.GetXmlObject());
    xml2.Print();  
	
	cout << "===============================================" << endl;

	AVSXmlTag root = "EasyVideo", body = "Body", device = "Devices";

	xml.ForeachChild((root + body + device).ToString(), "Device", ForeachChild, NULL);
	*/
}


void ForeachChild(AVSXmlUtil &child, void *pUser)
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
    AVSXmlUtil xml;
    xml.Read("test.xml", true);
    xml.Print();
    cout << "===============================================" <<endl;
    
    AVSXmlTag root = "EasyVideo", body = "Body", device ="Devices";

    xml.ForeachChild((root + body + device).ToString(), "Device", ForeachChild, NULL);
    //xml.ForeachChild("EasyVideo.Body.Devices", "Device", ForeachChild, NULL);
}

int main(int argc, char** argv)
{
    testRead();
    
    testWrite();
    
    //testReadChild();
        
    return 0;
}

