
#ifndef _OSREFTABLEEX_H_
#define _OSREFTABLEEX_H_
//因为darwin的引用表不允许相同的key，但他的字符串hash函数又不能做到这一点。因此考虑使用STL中的map进行代替。
//注意这个类是后来加上的，不是Darwin自带的
#include<map>
#include<string>
using namespace std;

#include "OSCond.h"
#include "OSHeaders.h"
class OSRefTableEx
{
public:
	class OSRefEx
	{
	private:
		void *mp_Object;//引用的对象或其余数据
		int m_Count;//当前引用对象计数，只有为0时才允许对象销毁
		OSCond  m_Cond;//to block threads waiting for this ref.
	public:
		OSRefEx() :mp_Object(NULL), m_Count(0) {}
		OSRefEx(void * pobject) :mp_Object(pobject), m_Count(0) {}
	public:
		void *GetObjectPtr() const { return mp_Object; }
		int GetRefNum() const { return m_Count; }
		OSCond *GetCondPtr() { return &m_Cond; }
		void AddRef(int num) { m_Count += num; }
	};
private:
	map<string, OSRefTableEx::OSRefEx*> m_Map;//以string为key，以OSRefEx为value
	OSMutex         m_Mutex;//提供对map的互斥操作
public:
	OSRefEx *    Resolve(const string& keystring);//引用对象
	OS_Error     Release(const string& keystring);//释放引用
	OS_Error     Register(const string& keystring, void* pobject);//加入到map中
	OS_Error     UnRegister(const string& keystring);//从map中移除

	OS_Error TryUnRegister(const string& keystring);//尝试移除，如果引用为0,则移除并返回true，否则返回false
public:
	int GetEleNumInMap() const { return m_Map.size(); }//获得map中的元素个数
	OSMutex *GetMutex() { return &m_Mutex; }//给外面提供互斥接口
	map<string, OSRefTableEx::OSRefEx*> *GetMap() { return &m_Map; }
};
typedef map<string, OSRefTableEx::OSRefEx*> OSHashMap;
typedef map<string, OSRefTableEx::OSRefEx*>::iterator OSRefIt;

class OSRefReleaserEx
{
public:

	OSRefReleaserEx(OSRefTableEx* inTable, const string& inKeyString) : fOSRefTable(inTable), fOSRefKey(inKeyString) {}
	~OSRefReleaserEx() { fOSRefTable->Release(fOSRefKey); }

	string GetRefKey() const { return fOSRefKey; }

private:

	OSRefTableEx*	fOSRefTable;
	string			fOSRefKey;
};

#endif _OSREFTABLEEX_H_