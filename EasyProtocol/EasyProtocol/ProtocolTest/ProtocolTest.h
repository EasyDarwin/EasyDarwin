/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   ProtocolTest.h
 * Author: wellsen
 *
 * Created on 2014年11月15日, 下午8:20
 */

#ifndef PROTOCOLTEST_H
#define	PROTOCOLTEST_H

#include <EasyProtocol.h>
#include <EasyUtil.h>

class ProtocolTest
{
public:
    ProtocolTest();
    virtual ~ProtocolTest();
    
public:
    //void TestCreateSessionReq();
    //void TestCreateSessionAck();
	void TestRegisterReq();
	void TestRegisterRsp();

	void TestDeviceStreamReq();
	void TestDeviceStreamRsp();

	void TestDeviceStreamStopReq();
	void TestDeviceStreamStopRsp();

	void TestDeviceListRsp();
	void TestCameraListRsp();

	void TestClientStartStreamRsp();

	void TestDeviceSnapReq();
	void TestDeviceSnapRsp();

private:
    void PrintMsg(const char *msg);
};

#endif	/* PROTOCOLTEST_H */

