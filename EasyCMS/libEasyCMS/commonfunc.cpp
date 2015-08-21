/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
#include "commonfunc.h"
#include <io.h>
#include <algorithm>
#include "HTTPProtocol.h"
#include "HTTPRequest.h"
#include <string.h>

bool IsFileExist(const char *szfile)
{
	if(szfile == NULL)
	{
		return false;
	}

	if(access(szfile, 0) == 0) //ÎÄ¼þ´æÔÚ
	{
		return true;
	}
	else
	{
		return false;
	}
	return true;
}

string MakeTimeInt2String(time_t tmTime, int nFlags)
{
	struct tm* tmNow;
	string strDate;
	tmNow=localtime(&tmTime);
	char cpYearMonDay[100] = {0};
	if (nFlags == 0)
	{
		sprintf(cpYearMonDay,  "%04d-%02d-%02d %02d:%02d:%02d",
			tmNow->tm_year+1900, tmNow->tm_mon+1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
	}
	else if (nFlags == 1)
	{
		sprintf(cpYearMonDay, "%02d:%02d:%02d",
			tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
	}
	else if (nFlags == 2)
	{
		sprintf(cpYearMonDay,  "%02d:%02d",
			tmNow->tm_hour, tmNow->tm_min);
	}
	else if (nFlags == 3)
	{
		sprintf(cpYearMonDay, "%04d-%02d-%02d %02d",
			tmNow->tm_year+1900, tmNow->tm_mon+1, tmNow->tm_mday, tmNow->tm_hour);
	}
	else if (nFlags == 4)
	{
		sprintf(cpYearMonDay,  "%04d%02d%02d%02d%02d%02d",
			tmNow->tm_year+1900, tmNow->tm_mon+1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
	}
	strDate = cpYearMonDay;
	return strDate;
}


time_t MakeTimeString2Int(char const *szDate)
{
	string file_time(szDate);
	if (file_time.length()<10)
	{
		return 0;
	}

	//12:34:30
	int stop_year=atoi(file_time.substr(0,4).c_str());
	int stop_month=atoi(file_time.substr(5,2).c_str());
	int stop_day=atoi(file_time.substr(8,2).c_str());

	int stop_hour=atoi(file_time.substr(11,2).c_str());
	int stop_minute=atoi(file_time.substr(14,2).c_str());
	int stop_second=atoi(file_time.substr(17,2).c_str());

	return MakeTimeGroup(stop_year,stop_month,stop_day,stop_hour,stop_minute,stop_second);
}

time_t MakeTimeGroup(int year, int month, int day, int hour, int minute, int second)
{
	struct tm tmdate;
	tmdate.tm_year = year - 1900;
	tmdate.tm_mon = month - 1;
	tmdate.tm_mday = day;

	tmdate.tm_hour = hour;
	tmdate.tm_min = minute;
	tmdate.tm_sec = second;
	tmdate.tm_isdst = -1;

	return mktime(&tmdate);
}

void Toupperstring(string &str)
{
	transform(str.begin(), str.end(), str.begin(), (int(*)(int))toupper);
}

void PackHttpPostHeader(const char *szHost, string &strHttp, unsigned int uDataLen)
{
	char reqHeader[1024] = {0};
	sprintf(reqHeader, 
		"POST / HTTP/1.1\n"
		"Host: %s\n"  
		"Encrypt-Type: None\n"
		"Checksum: None\n"
		"Accept: text/xml\n"
		"Content-Length: %d\r\n\r\n", szHost, uDataLen);
	strHttp = reqHeader;
}

void PackHttpAckHeader(const char *szServer, char *ackHeader, unsigned int uDataLen)
{
	sprintf(ackHeader,  "HTTP/1.1 200 OK\n"
					    "Server: %s\n"
					    "Content-Length: %d\r\n\r\n", szServer, uDataLen);
}

int ParseHttpData(char *httpData, char *&pXML)
{
	StrPtrLen strTmp("HTTP-Parse");
	StrPtrLen sHttpData(httpData);
	HTTPRequest http(&strTmp, &sHttpData);
	if (http.Parse() != QTSS_NoErr)
	{
		return -1;
	}
	
	StrPtrLen* lengthPtr = http.GetHeaderValue(httpContentLengthHeader);
	pXML = lengthPtr->Ptr;
	
 
 	
 	char *pPos = strstr(pXML, "\r\n\r\n");
 	if (pPos == NULL)
 	{
 		return -1;
 	}
 	
 	pXML = pPos+4;

	return atoi(lengthPtr->GetAsCString());;
}
