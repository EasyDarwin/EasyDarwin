/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyUtil.h
 * Author: wellsen
 *
 * Created on 2014年11月22日, 上午10:16
 */

#ifndef EASYDSS_UTIL_H
#define	EASYDSS_UTIL_H

#include <EasyProtocolDef.h>
#include <string>

class EASYDSS_API EasyUtil
{
public:
    static std::string TimeT2String(EasyDSSTimeFormat whatFormat, unsigned long time);
    
    //if whatFormat == EASYDSS_TIME_FORMAT_YYYYMMDD, the time will be set to now,
    //if whatFormat == EASYDSS_TIME_FORMAT_HHMMSS, the date will be set to today   
    static unsigned long String2TimeT(EasyDSSTimeFormat whatFormat, std::string timeString);
    
    static unsigned long String2TimeT(std::string dateYYMMDD/*2014-11-23*/, std::string timeHHMMSS/*08:30:00*/);
    
    static std::string NowTime(EasyDSSTimeFormat whatFormat);
    
    static unsigned long NowTime();

	static std::string GetUUID();

	static int String2Int(std::string value);
    
};

#endif	/* EASYDSS_UTIL_H */