/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/

#include"OSRefTableEx.h"
#include <errno.h>

OSRefTableEx::OSRefEx * OSRefTableEx::Resolve(string keystring)//引用对象，存在返回指针，否则返回NULL
{
	OSMutexLocker locker(&m_Mutex);
	if(m_Map.find(keystring) == m_Map.end())//不存在
		return NULL;
	m_Map[keystring]->AddRef(1);
	return m_Map[keystring];
}

OS_Error OSRefTableEx::Release(string keystring)//释放引用
{
	OSMutexLocker locker(&m_Mutex);
	if(m_Map.find(keystring) == m_Map.end())//不存在
		return EPERM;
	//make sure to wakeup anyone who may be waiting for this resource to be released
	m_Map[keystring]->AddRef(-1);//减少引用
	m_Map[keystring]->GetCondPtr()->Signal();
	return OS_NoErr;
}

OS_Error OSRefTableEx::Register(string keystring,void* pobject)//加入到map中
{
	Assert(pobject != NULL);
	if(pobject == NULL)
		return EPERM;

	OSMutexLocker locker(&m_Mutex);//互斥
	if(m_Map.find(keystring) != m_Map.end())//已经出现，拒绝重复的key
		return EPERM;
	OSRefTableEx::OSRefEx *RefTemp = new OSRefTableEx::OSRefEx(pobject);//生成value，在此new，在UnRegister里delete
	m_Map[keystring] = RefTemp;//加入到map中
	return OS_NoErr;
}

OS_Error OSRefTableEx::UnRegister(string keystring)//从map中移除，必须引用计数为0才允许移除
{
	OSMutexLocker locker(&m_Mutex);
	if(m_Map.find(keystring) == m_Map.end())//不存在当前key
		return EPERM;
	//make sure that no one else is using the object
	while (m_Map[keystring]->GetRefNum()>0)
		m_Map[keystring]->GetCondPtr()->Wait(&m_Mutex);
	
	delete m_Map[keystring];//释放
	m_Map.erase(keystring);//移除
	return OS_NoErr;
}

OS_Error OSRefTableEx::TryUnRegister(string keystring)
{
	OSMutexLocker locker(&m_Mutex);
	if(m_Map.find(keystring) == m_Map.end())//不存在当前key
		return EPERM;
	if(m_Map[keystring]->GetRefNum() > 0)
		return EPERM;
	// At this point, this is guarenteed not to block, because
	// we've already checked that the refCount is low.
	delete m_Map[keystring];//释放
	m_Map.erase(keystring);//移除
	return OS_NoErr;
}