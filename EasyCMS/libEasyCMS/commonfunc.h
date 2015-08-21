/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
#ifndef _COMMON_FUNC_H
#define _COMMON_FUNC_H
#include <string>
#include <time.h>
using namespace std;
//#define printlog qtss_printf

bool   IsFileExist(const char *szfile);
string MakeTimeInt2String(time_t tmTime, int nFlags=0);
time_t MakeTimeString2Int(char const *szDate);
time_t MakeTimeGroup(int year, int month, int day, int hour, int minute, int second);
void Toupperstring(string &str);

void PackHttpPostHeader(const char *szHost, string &strHttp, unsigned int uDataLen);
void PackHttpAckHeader(const char *szServer, char *ackHeader, unsigned int uDataLen);
int ParseHttpData(char *httpData, char *&pXML);
#endif

