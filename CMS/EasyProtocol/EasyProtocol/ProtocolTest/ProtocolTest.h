/* 
 * File:   ProtocolTest.h
 * Author: wellsen
 *
 * Created on 2014年11月15日, 下午8:20
 */

#ifndef PROTOCOLTEST_H
#define	PROTOCOLTEST_H

#include <EasyDSSProtocol.h>
#include <EasyDSSUtil.h>

class ProtocolTest
{
public:
    ProtocolTest();
    virtual ~ProtocolTest();
    
public:
    //void TestCreateSessionReq();
    //void TestCreateSessionAck();
	void TestRegisterReq();

	void TestDeviceListRsp();

	void TestDeviceSnapReq();

private:
    void PrintMsg(const char *msg);
};

#endif	/* PROTOCOLTEST_H */

