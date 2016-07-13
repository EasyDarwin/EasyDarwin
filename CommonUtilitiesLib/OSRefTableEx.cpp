/*
    Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
    Github: https://github.com/EasyDarwin
    WEChat: EasyDarwin
    Website: http://www.EasyDarwin.org
*/

#include"OSRefTableEx.h"
#include <errno.h>

OSRefTableEx::OSRefEx * OSRefTableEx::Resolve(const string& key)//引用对象，存在返回指针，否则返回NULL
{
    OSMutexLocker locker(&m_Mutex);
    if (m_Map.find(key) == m_Map.end())//不存在
        return NULL;
    m_Map[key]->AddRef(1);

    return m_Map[key];
}

OS_Error OSRefTableEx::Release(const string& key)//释放引用
{
    OSMutexLocker locker(&m_Mutex);
    if (m_Map.find(key) == m_Map.end())//不存在
        return EPERM;
    //make sure to wakeup anyone who may be waiting for this resource to be released
    m_Map[key]->AddRef(-1);//减少引用
    m_Map[key]->GetCondPtr()->Signal();

    return OS_NoErr;
}

OS_Error OSRefTableEx::Register(const string& key, void* pObject)//加入到map中
{
    Assert(pObject != NULL);
    if (pObject == NULL)
        return EPERM;

    OSMutexLocker locker(&m_Mutex);//互斥
    if (m_Map.find(key) != m_Map.end())//已经出现，拒绝重复的key
        return EPERM;
    OSRefEx *RefTemp = new OSRefEx(pObject);//生成value，在此new，在UnRegister里delete
    m_Map[key] = RefTemp;//加入到map中

    return OS_NoErr;
}

OS_Error OSRefTableEx::UnRegister(const string& key)//从map中移除，必须引用计数为0才允许移除
{
    OSMutexLocker locker(&m_Mutex);
    if (m_Map.find(key) == m_Map.end())//不存在当前key
        return EPERM;
    //make sure that no one else is using the object
    while (m_Map[key]->GetRefNum() > 0)
        m_Map[key]->GetCondPtr()->Wait(&m_Mutex);

    delete m_Map[key];//释放
    m_Map.erase(key);//移除

    return OS_NoErr;
}

OS_Error OSRefTableEx::TryUnRegister(const string& key)
{
    OSMutexLocker locker(&m_Mutex);
    if (m_Map.find(key) == m_Map.end())//不存在当前key
        return EPERM;
    if (m_Map[key]->GetRefNum() > 0)
        return EPERM;
    // At this point, this is guarenteed not to block, because
    // we've already checked that the refCount is low.
    delete m_Map[key];//释放
    m_Map.erase(key);//移除

    return OS_NoErr;
}