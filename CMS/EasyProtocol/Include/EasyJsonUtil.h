/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef Easy_Json_UTIL
#define Easy_Json_UTIL

#include <string>
#include <map>
#include <list>

#if 0//(defined(_WIN32))
#ifndef _DLL_
#define EasyJson_API
#else
#ifdef EasyJson_API_EXPORTS
#define EasyJson_API __declspec(dllexport) 
#else  
#define EasyJson_API __declspec(dllimport)
#endif  
#endif
#elif defined(__linux__)
#define EasyJson_API
#endif
#define EasyJson_API
typedef void* EasyJsonObject;
typedef std::map<std::string, std::string> EasyJsonAttrs;



class EasyJson_API EasyJsonTag
{
public:	
	EasyJsonTag(const std::string &tag);
	EasyJsonTag(const char* tag);
	EasyJsonTag(const EasyJsonTag &tag);
	~EasyJsonTag();
	EasyJsonTag operator+(const EasyJsonTag &tag);
	
	std::string ToString() const;

private:
	std::string *str;
};


class EasyJson_API EasyJsonAttributes
{
public:
	EasyJsonAttributes();
	virtual ~EasyJsonAttributes();

public:
	void AddIntAttribute(const char* name, int value);
	void AddStringAttribute(const char* name, const char* value);
	void AddFloatAttribute(const char* name, double value);
	void Clear();
	EasyJsonAttrs& GetAttrs();

private:
	EasyJsonAttrs *attrs;
};

class EasyJsonUtil;

typedef void(*fForeachChild)(EasyJsonUtil &childJson, void *pUserData);
typedef std::list<EasyJsonUtil> EasyJsonList;

class EasyJson_API EasyJsonUtil
{
public:
	EasyJsonUtil();
	EasyJsonUtil(const EasyJsonObject jsonObj);
    EasyJsonUtil(const EasyJsonUtil &jsonUtil);
	virtual ~EasyJsonUtil();

public:

	static const char* MyName();

	bool Read(const std::string &sJsonFileorBuffer, bool isFile = false);
	bool Write(std::string &sJsonFileorBuffer, bool isFile = false);
	void Clear();
	void Print();
	
	bool TagExist(std::string sJsonTag);
	bool IsEmpty();

	bool GetValueAsString(std::string sJsonTag, std::string &value);
	bool GetValueAsInt(std::string sJsonTag, int &value);
	bool GetValueAsDouble(std::string sJsonTag, double &value);

	bool GetAttributeAsString(std::string sJsonTag, std::string sAttributeName, std::string &attr);
	bool GetAttributeAsInt(std::string sJsonTag, std::string sAttributeName, int &attr);
	bool GetAttributeAsDouble(std::string sJsonTag, std::string sAttributeName, double &attr);

	bool ForeachChild(std::string sJsonParentTag, std::string sJsonChildTag, fForeachChild foreachFunc, void *pUserData);
    bool GetAllChild(std::string sJsonParentTag, std::string sJsonChildTag, EasyJsonList &childList);

	bool SetStringValue(std::string sJsonTag, const char* value, EasyJsonAttributes *pAttrs = NULL);
	bool SetIntValue(std::string sJsonTag, int value, EasyJsonAttributes *pAttrs = NULL);
	bool SetFloatValue(std::string sJsonTag, double value, EasyJsonAttributes *pAttrs = NULL);

	bool SetAttributes(std::string sJsonTag, EasyJsonAttributes &attrs);

	bool SetChildWithIntValue(std::string sJsonParentTag, std::string sJsonChildTag, int value, EasyJsonAttributes *pAttrs = NULL);
	bool SetChildWithFloatValue(std::string sJsonParentTag, std::string sJsonChildTag, double value, EasyJsonAttributes *pAttrs = NULL);
	bool SetChildWithStringValue(std::string sJsonParentTag, std::string sJsonChildTag, std::string value, EasyJsonAttributes *pAttrs = NULL);

	bool AddChild(std::string sJsonParentTag, std::string sChildName, EasyJsonUtil &child, EasyJsonAttributes *pAttrs = NULL);
    bool Add(std::string sJsonParentTag, EasyJsonUtil &child, EasyJsonAttributes *pAttrs = NULL);
	bool AddArray(std::string sJsonParentTag, EasyJsonUtil &arry);
    
public:
	EasyJsonObject GetJsonObject() const; 
	EasyJsonObject GetChild(std::string sJsonTag);
	EasyJsonObject GetChildCreateNewIfNil(std::string sJsonTag);


private:
	EasyJsonObject json;
};

#endif
