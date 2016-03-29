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
 * Created on 2014年11月15日, 下午8:15
 */

#include <cstdlib>
#include "ProtocolTest.h"
#include <stdio.h>

using namespace std;

/*
 * 
 */
int main(int argc, char** argv)
{
    ProtocolTest test;
	//test.TestEasyDSSSnapUpdateReq();
	//test.TestEasyDSSSnapUpdateAck();
	test.TestRegisterReq();
	test.TestRegisterRsp();
	test.TestDeviceStreamReq();
	test.TestDeviceStreamRsp();

	test.TestDeviceStreamStopReq();
	test.TestDeviceStreamStopRsp();
	
	test.TestDeviceListRsp();
	test.TestCameraListRsp();
	test.TestClientStartStreamRsp();
	
	test.TestDeviceSnapReq();
	test.TestDeviceSnapRsp();

	getchar();
    return 0;
}

