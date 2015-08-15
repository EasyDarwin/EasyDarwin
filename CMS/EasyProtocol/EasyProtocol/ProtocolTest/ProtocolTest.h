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

	void TestDeviceListRsp();

	void TestDeviceSnapReq();

private:
    void PrintMsg(const char *msg);
};

#endif	/* PROTOCOLTEST_H */

