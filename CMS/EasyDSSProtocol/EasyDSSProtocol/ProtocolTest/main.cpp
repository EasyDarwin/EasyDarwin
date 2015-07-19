/* 
 * File:   main.cpp
 * Author: wellsen
 *
 * Created on 2014年11月15日, 下午8:15
 */

#include <cstdlib>
#include "ProtocolTest.h"

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
    return 0;
}

