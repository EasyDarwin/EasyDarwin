/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyUtil.cpp
 * Author: wellsen
 * 
 * Created on 2014年11月22日, 上午10:16
*/

#include <EasyUtil.h>

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
#include <boost/lexical_cast.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>  
#include <boost/archive/iterators/binary_from_base64.hpp>  
#include <boost/archive/iterators/transform_width.hpp>   

std::string EasyUtil::TimeT2String(EasyDarwinTimeFormat whatFormat, unsigned long time)
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
    case EASY_TIME_FORMAT_YYYYMMDDHHMMSS:
        sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,\
												local.tm_hour, local.tm_min, local.tm_sec);
        break;
	case EASY_TIME_FORMAT_YYYYMMDDHHMMSSEx:
	    sprintf(timeStr, "%04d%02d%02d%02d%02d%02d", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,\
												local.tm_hour, local.tm_min, local.tm_sec);
        break;
    case EASY_TIME_FORMAT_YYYYMMDD:
        sprintf(timeStr, "%04d-%02d-%02d", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
        break;
        
    case EASY_TIME_FORMAT_HHMMSS:
        sprintf(timeStr, "%02d:%02d:%02d", local.tm_hour, local.tm_min, local.tm_sec);
        break;
        
    default:
        break;
    }
    return std::string(timeStr);
}

unsigned long EasyUtil::String2TimeT(EasyDarwinTimeFormat whatFormat, std::string timeString)
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
    case EASY_TIME_FORMAT_YYYYMMDDHHMMSS:
        sscanf(timeString.c_str(), "%04d-%02d-%02d %02d:%02d:%02d", &local.tm_year, &local.tm_mon, &local.tm_mday,\
												&local.tm_hour, &local.tm_min, &local.tm_sec);
        local.tm_year -= 1900;
        local.tm_mon -= 1;
        break;
        
    case EASY_TIME_FORMAT_YYYYMMDD:
        sscanf(timeString.c_str(), "%04d-%02d-%02d", &local.tm_year, &local.tm_mon, &local.tm_mday);
        local.tm_year -= 1900;
        local.tm_mon -= 1;
        break;
        
    case EASY_TIME_FORMAT_HHMMSS:
        sscanf(timeString.c_str(), "%02d:%02d:%02d", &local.tm_hour, &local.tm_min, &local.tm_sec);
        break;
        
    default:
        break;
    }
    
    return mktime(&local);
}

unsigned long EasyUtil::String2TimeT(std::string dateYYMMDD/*2014-11-23*/, std::string timeHHMMSS/*08:30:00*/)
{
    std::string strTime = dateYYMMDD + " " + timeHHMMSS;
    return String2TimeT(EASY_TIME_FORMAT_YYYYMMDDHHMMSS, strTime);
}

std::string EasyUtil::NowTime(EasyDarwinTimeFormat whatFormat)
{
    return TimeT2String(whatFormat, NowTime());
}

unsigned long EasyUtil::NowTime()
{
    time_t now = time(NULL);
    return (unsigned long)now;
}

std::string EasyUtil::GetUUID()
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

int EasyUtil::String2Int(std::string value)
{
	try
	{
		return boost::lexical_cast<int>(value);
	}
	catch (std::exception &e)
	{
		printf("EasyUtil::String2Int error: %s\n", e.what());
		return -1;
	}	
}

std::string EasyUtil::Int2String(int value)
{
	try
	{
		return boost::lexical_cast<std::string>(value);
	}
	catch (std::exception &e)
	{
		printf("EasyUtil::String2Int error: %s\n", e.what());		
	}
	return std::string();
}

bool EasyUtil::Base64Decode(string& sInput, string& sOutput)
{
	typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<string::const_iterator>, 8, 6> Base64DecodeIterator;
	stringstream result;
	try
	{
		copy(Base64DecodeIterator(sInput.begin()), Base64DecodeIterator(sInput.end()), ostream_iterator<char>(result));
	}
	catch (...)
	{
		return false;
	}
	sOutput = result.str();

	return !sOutput.empty();
}

bool EasyUtil::Base64Encode(string& sInput, string& sOutput)
{
	typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<string::const_iterator, 6, 8> > Base64EncodeIterator;
	stringstream result;
	copy(Base64EncodeIterator(sInput.begin()), Base64EncodeIterator(sInput.end()), ostream_iterator<char>(result));
	size_t equal_count = (3 - sInput.length() % 3) % 3;
	for (size_t i = 0; i < equal_count; i++)
	{
		result.put('=');
	}
	sOutput = result.str();

	return !sOutput.empty();
}

string EasyUtil::Base64Encode(const char* src, size_t len)
{
	string sInput, sOutput;
	sInput.assign(src, len);
	Base64Encode(sInput, sOutput);
	return sOutput;
}

string EasyUtil::Base64Decode(const char* src, size_t len)
{
	try
	{
		string sInput, sOutput;
		sInput.assign(src, len);
		Base64Decode(sInput, sOutput);
		return sOutput;
	}
	catch (std::exception &e)
	{
		printf("util::Base64Decode error: %s\n", e.what());
	}
	catch (...)
	{
		printf("util::Base64Decode error\n");
	}
	return std::string();
}

void EasyUtil::DelChar(std::string & sInput, char ch)
{
	int begin = 0;
	begin = sInput.find(ch,begin);//查找ch在sInput的第一个位置

	while(begin != std::string::npos)  //表示字符串中存在ch
	{
		sInput.replace(begin, 1, "");  // 用空串替换sInput中从begin开始的1个字符
		begin = sInput.find(ch,begin);  //查找ch在替换后的sInput中第一次出现的位置
	}
}
