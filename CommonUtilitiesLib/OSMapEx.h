/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
//���ܿͻ�����֤������32λsessionid�����뵽map�С�
//Ȼ��EasyDarwin�����32λ��sessionid��������֤����֤֮���ɾ����
//ͬʱÿһ��ID������ʱЧ�ԣ����Կ���һ����ʱ��������ЩʧЧ��sessionid����ɾ��
//��˧2016.03.12

#ifndef __OSMAPEX__H__
#define __OSMAPEX__H__

#include<map>
#include<string>
#include"OSMutex.h"//�������
//#include <iostream>
using namespace std;

#define sLastingTimeOneMin 500*1000//SessionID��Ĭ�ϳ���ʱ�䣬500����
#define sLastingTimeOneYear (SInt64)60*1000*1000*60*24*365//����ʱ��1��

//SesssionID Map
struct strMapData
{
	char m_AllExist;
	SInt64 m_LastingTime;
	strMapData() :m_AllExist(0), m_LastingTime(0) {}
	strMapData(SInt64 LastingTime, char AllExist = 0) :m_AllExist(AllExist), m_LastingTime(LastingTime) {}
};
typedef map<string, strMapData> MapType;

class OSMapEx
{
public:
	static string GenerateSessionIdForRedis(string strIP, UInt16 uPort);//����sessionid�������룬����֤���ɵ�Ψһ��
	static string GenerateSessionId();//����sessionid,�����룬����֤���ɵ�Ψһ��
	bool Insert(const string& strSessionID, SInt64 lastingTime = sLastingTimeOneYear);//���룬�̰߳�ȫ������true�ɹ����룬�����ظ�����,����һЩ���ڴ��ڵ�SessionID
	//bool Delete(string&strSessionID);//ɾ��,�̰߳�ȫ���ɹ�ɾ������true,����false

	string GererateAndInsert(SInt64 lastingTime = sLastingTimeOneMin);//����һ��Ψһ��sessionID�����뵽map�У�����sessionid,�̰߳�ȫ��lastingTimeΪ΢�뵥λ
	bool FindAndDelete(const string& strSessionID);//���Ҳ�ɾ�����̰߳�ȫ,�ҵ�����ʱЧ��Χ�ڷ���true,���򷵻�false

	void  CheckTimeoutAndDelete();//��鳬ʱ����ʱɾ��
private:
	MapType m_Map;//keyλ32λsessionid��valueΪ1970�굽��ʱʱ���΢����
	OSMutex m_Mutex;

};
//SesssionID Map
#endif