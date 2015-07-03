/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyDSSUtil.cpp
 * Author: wellsen
 * 
 * Created on 2014年11月22日, 上午10:16
 */

#include <EasyDSSUtil.h>

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <string.h>
#include <stdio.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

std::string EasyDSSUtil::TimeT2String(EasyDSSTimeFormat whatFormat, unsigned long time)
{
    struct tm local;
    time_t t = time;
    
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    
    char timeStr[64];
    memset(timeStr, 0, 64);
    switch (whatFormat)
    {
    case EASYDSS_TIME_FORMAT_YYYYMMDDHHMMSS:
        sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,\
												local.tm_hour, local.tm_min, local.tm_sec);
        break;
        
    case EASYDSS_TIME_FORMAT_YYYYMMDD:
        sprintf(timeStr, "%04d-%02d-%02d", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
        break;
        
    case EASYDSS_TIME_FORMAT_HHMMSS:
        sprintf(timeStr, "%02d:%02d:%02d", local.tm_hour, local.tm_min, local.tm_sec);
        break;
        
    default:
        break;
    }
    return std::string(timeStr);
}

unsigned long EasyDSSUtil::String2TimeT(EasyDSSTimeFormat whatFormat, std::string timeString)
{
    struct tm local;
       
    time_t now = time(NULL);
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    
    switch (whatFormat)
    {
    case EASYDSS_TIME_FORMAT_YYYYMMDDHHMMSS:
        sscanf(timeString.c_str(), "%04d-%02d-%02d %02d:%02d:%02d", &local.tm_year, &local.tm_mon, &local.tm_mday,\
												&local.tm_hour, &local.tm_min, &local.tm_sec);
        local.tm_year -= 1900;
        local.tm_mon -= 1;
        break;
        
    case EASYDSS_TIME_FORMAT_YYYYMMDD:
        sscanf(timeString.c_str(), "%04d-%02d-%02d", &local.tm_year, &local.tm_mon, &local.tm_mday);
        local.tm_year -= 1900;
        local.tm_mon -= 1;
        break;
        
    case EASYDSS_TIME_FORMAT_HHMMSS:
        sscanf(timeString.c_str(), "%02d:%02d:%02d", &local.tm_hour, &local.tm_min, &local.tm_sec);
        break;
        
    default:
        break;
    }
    
    return mktime(&local);
}

unsigned long EasyDSSUtil::String2TimeT(std::string dateYYMMDD/*2014-11-23*/, std::string timeHHMMSS/*08:30:00*/)
{
    std::string strTime = dateYYMMDD + " " + timeHHMMSS;
    return String2TimeT(EASYDSS_TIME_FORMAT_YYYYMMDDHHMMSS, strTime);
}

std::string EasyDSSUtil::NowTime(EasyDSSTimeFormat whatFormat)
{
    return TimeT2String(whatFormat, NowTime());
}

unsigned long EasyDSSUtil::NowTime()
{
    time_t now = time(NULL);
    return (unsigned long)now;
}

std::string EasyDSSUtil::GetUUID()
{
	try
	{
		boost::uuids::random_generator rnd_gen;
		boost::uuids::uuid u = rnd_gen();
		std::string s = boost::uuids::to_string(u);
		boost::erase_all(s, "-");
		return s;
	}
	catch (std::exception &e)
	{
		printf("SysUtil::GetUUID error: %s\n", e.what());
		return std::string();
	}	
}
