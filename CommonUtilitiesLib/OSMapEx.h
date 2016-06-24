/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
//接受客户端认证，产生32位sessionid，加入到map中。
//然后EasyDarWin用这个32位的sessionid来进行认证，认证之后就删除。
//同时每一个ID都有其时效性，可以开启一个定时器来将那些失效的sessionid进行删除
//邵帅2016.03.12

#ifndef __OSMAPEX__H__
#define __OSMAPEX__H__

#include<set>
#include<map>
#include<string>
//#include <iostream>
using namespace std;
#define sLastingTimeOneMin 500*1000//SessionID的默认持续时间，500毫秒
#define sLastingTimeOneYear (SInt64)60*1000*1000*60*24*365//持续时间1年
#include"OS.h"
#include"OSMutex.h"//互斥操作

//SesssionID Map
struct strMapData
{
	char m_AllExist;
	SInt64 m_LastingTime;
	strMapData():m_AllExist(0),m_LastingTime(0){}
	strMapData(SInt64 LastingTime,char AllExist=0):m_LastingTime(LastingTime),m_AllExist(AllExist){}
};
typedef map<string,strMapData> MapType;

class OSMapEx
{
public:
	static string GenerateSessionIdForRedis(string strIP,UInt16 uPort);//生成sessionid，可重入，不保证生成的唯一性
	static string GenerateSessionId();//生成sessionid,可重入，不保证生成的唯一性
	bool Insert(string& strSessionID,SInt64 lastingTime=sLastingTimeOneYear);//插入，线程安全，返回true成功插入，否则重复插入,用于一些长期存在的SessionID
	//bool Delete(string&strSessionID);//删除,线程安全，成功删除返回true,否则false
	
	string GererateAndInsert(SInt64 lastingTime=sLastingTimeOneMin);//生成一个唯一的sessionID并插入到map中，返回sessionid,线程安全，lastingTime为微秒单位
	bool FindAndDelete(string&strSessionID);//查找并删除，线程安全,找到且在时效范围内返回true,否则返回false

	void  CheckTimeoutAndDelete();//检查超时，超时删除
private:
	MapType m_Map;//key位32位sessionid，value为1970年到超时时间的微妙数
	OSMutex m_Mutex;
};
//SesssionID Map
#endif