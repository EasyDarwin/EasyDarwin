/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include"OSMapEx.h"
#ifndef _WIN32
#include<arpa/inet.h>
#endif
string OSMapEx::GenerateSessionId()//生成32位的SessionId
{
	SInt64 theMicroseconds = OS::Microseconds();//Windows下1ms内多次执行会造成产生的随机数时一样的，因为Windows下微秒的获取也是靠毫秒*1000来进行的
	::srand((unsigned int)theMicroseconds);
	UInt16 the1Random = ::rand();

	::srand((unsigned int)the1Random);
	UInt16 the2Random = ::rand();

	::srand((unsigned int)the2Random);
	UInt16 the3Random = ::rand();

	::srand((unsigned int)the3Random);
	UInt16 the4Random = ::rand();

	::srand((unsigned int)the4Random);
	UInt16 the5Random = ::rand();

	::srand((unsigned int)the5Random);
	UInt16 the6Random = ::rand();

	::srand((unsigned int)the6Random);
	UInt16 the7Random = ::rand();

	::srand((unsigned int)the7Random);
	UInt16 the8Random = ::rand();

	char chTemp[33]={0};
	sprintf(chTemp,"%04X%04X%04X%04X%04X%04X%04X%04X",the1Random,the2Random,the3Random,the4Random,the5Random,the6Random,the7Random,the8Random);
	return string(chTemp);
}
bool OSMapEx::Insert(string& strSessionId,SInt64 lastingTime)
{
	OSMutexLocker mutexLocker(&m_Mutex);
	if(m_Map.find(strSessionId)!=m_Map.end())//已经存在
		return false;
	else
	{
		strMapData strTemp(lastingTime+OS::Microseconds(),1);//1表示不自动删除，直到超时再删除
		m_Map.insert(MapType::value_type(strSessionId,strTemp));
		return true;
	}
}
string OSMapEx::GererateAndInsert(SInt64 lastingTime)
{
	OSMutexLocker mutexLocker(&m_Mutex);
	string  strSessionId;
	do
	{
		strSessionId=OSMapEx::GenerateSessionId();//生成sessionid
	}
	while(m_Map.find(strSessionId)!=m_Map.end());//如果是重复的就一直生成直到不是重复的
	strMapData strTemp(lastingTime+OS::Microseconds(),0);//0表示自动删除，验证1次就删除
	m_Map.insert(MapType::value_type(strSessionId,strTemp));
	return strSessionId;
}
bool OSMapEx::FindAndDelete(string& strSessionID)//查找并删除，线程安全,找到返回true,否则返回false
{
	OSMutexLocker mutexLocker(&m_Mutex);
	SInt64 sNowTime=OS::Microseconds();//获取当前时间
	bool bReVal=true;
	MapType::iterator l_it=m_Map.find(strSessionID);
	if(l_it!=m_Map.end())//找到了
	{
		if (l_it->second.m_LastingTime<sNowTime)//如果失效了返回false
			bReVal=false;
		if(l_it->second.m_AllExist==0)//如果是自动删除类型就删除
			m_Map.erase(l_it);
		return bReVal;
	}
	else
		return false;
}
void OSMapEx::CheckTimeoutAndDelete()//遍历map里的SessionID,删除失效的SessionID
{
	//unsigned int num=0;
	OSMutexLocker mutexLocker(&m_Mutex);
	SInt64 sNowTime=OS::Microseconds();//获取当前时间
	for (MapType::iterator i=m_Map.begin(); i!=m_Map.end(); /*i++*/)  
	{  
		if (i->second.m_LastingTime<sNowTime)//失效
		{  
			m_Map.erase(i++);  //i++包含1备份i;2i=i+1;3返回备份的i
			//num++;
		}  
		else  
		{  
			i++;  
		}  
	}
	//cout<<"失效"<<num<<"个"<<endl;
}
//for redis
string OSMapEx::GenerateSessionIdForRedis(string strIP,UInt16 uPort)
{
	SInt64 theMicroseconds = OS::Microseconds();//Windows下1ms内多次执行会造成产生的随机数时一样的，因为Windows下微秒的获取也是靠毫秒*1000来进行的
	::srand((unsigned int)theMicroseconds);
	UInt16 the1Random = ::rand();

	::srand((unsigned int)the1Random);
	UInt16 the2Random = ::rand();

	::srand((unsigned int)the2Random);
	UInt16 the3Random = ::rand();

	::srand((unsigned int)the3Random);
	UInt16 the4Random = ::rand();

	::srand((unsigned int)the4Random);
	UInt16 the5Random = ::rand();

	UInt32 uIP=inet_addr(strIP.c_str());

	char chTemp[33]={0};
	sprintf(chTemp,"%08X%04X%04X%04X%04X%04X%04X",uIP,uPort,the1Random,the2Random,the3Random,the4Random,the5Random);//32位16进制，16字节，前六个字节用IP和端口填充
	return string(chTemp);
}
//for redis