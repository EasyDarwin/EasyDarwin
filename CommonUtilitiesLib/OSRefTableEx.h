#ifndef _OSREFTABLEEX_H_
#define _OSREFTABLEEX_H_

//因为darwin的引用表不允许相同的key，但他的字符串hash函数又不能做到这一点。因此考虑使用STL中的map进行代替。
//注意这个类是后来加上的，不是Darwin自带的

#include <unordered_map>

using namespace std;

#include "OSCond.h"
#include "OSHeaders.h"

class OSRefTableEx
{
public:
	class OSRefEx
	{

	public:
		OSRefEx() : mp_Object(nullptr), m_Count(0) {}
		OSRefEx(void* pobject) : mp_Object(pobject), m_Count(0) {}
		void* GetObjectPtr() const { return mp_Object; }
		int GetRefNum() const { return m_Count; }
		OSCond* GetCondPtr() { return &m_Cond; }
		void AddRef(int num) { m_Count += num; }

	private:
		void* mp_Object;//引用的对象或其余数据
		int m_Count;//当前引用对象计数，只有为0时才允许对象销毁
		OSCond  m_Cond;//to block threads waiting for this ref.
	};

	OSRefEx*     Resolve(const string& key);//引用对象
	OS_Error     Release(const string& key);//释放引用
	OS_Error     Register(const string& key, void* pObject);//加入到map中
	OS_Error     UnRegister(const string& key);//从map中移除

	OS_Error TryUnRegister(const string& key);//尝试移除，如果引用为0,则移除并返回true，否则返回false
	int GetEleNumInMap() const { return m_Map.size(); }//获得map中的元素个数
	OSMutex *GetMutex() { return &m_Mutex; }//给外面提供互斥接口
	unordered_map<string, OSRefEx*>* GetMap() { return &m_Map; }

private:
	unordered_map<string, OSRefEx*> m_Map;//以string为key，以OSRefEx为value
	OSMutex         m_Mutex;//提供对map的互斥操作
};
typedef unordered_map<string, OSRefTableEx::OSRefEx*> OSHashMap;
typedef unordered_map<string, OSRefTableEx::OSRefEx*>::iterator OSRefIt;

class OSRefReleaserEx
{
public:
	OSRefReleaserEx(OSRefTableEx* pTable, const string& key) : fOSRefTable(pTable), fOSRefKey(key) {}
	~OSRefReleaserEx() { fOSRefTable->Release(fOSRefKey); }

	string GetRefKey() const { return fOSRefKey; }

private:
	OSRefTableEx*	fOSRefTable;
	string			fOSRefKey;
};

#endif //_OSREFTABLEEX_H_