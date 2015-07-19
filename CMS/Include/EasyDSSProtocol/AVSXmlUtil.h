/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef AVS_XML_UTIL
#define AVS_XML_UTIL

#include <string>
#include <map>
#include <list>

#if 0//(defined(_WIN32))
#ifndef _DLL_
#define AVSXML_API
#else
#ifdef AVSXML_API_EXPORTS
#define AVSXML_API __declspec(dllexport) 
#else  
#define AVSXML_API __declspec(dllimport)
#endif  
#endif
#elif defined(__linux__)
#define AVSXML_API
#endif
#define AVSXML_API
typedef void* AVSXmlObject;
typedef std::map<std::string, std::string> AVSXmlAttrs;



class AVSXML_API AVSXmlTag
{
public:	
	AVSXmlTag(const std::string &tag);
	AVSXmlTag(const char* tag);
	AVSXmlTag(const AVSXmlTag &tag);
	~AVSXmlTag();
	AVSXmlTag operator+(const AVSXmlTag &tag);
	
	std::string ToString() const;

private:
	std::string *str;
};


class AVSXML_API AVSXmlAttributes
{
public:
	AVSXmlAttributes();
	virtual ~AVSXmlAttributes();

public:
	void AddIntAttribute(const char* name, int value);
	void AddStringAttribute(const char* name, const char* value);
	void AddFloatAttribute(const char* name, double value);
	void Clear();
	AVSXmlAttrs& GetAttrs();

private:
	AVSXmlAttrs *attrs;
};

class AVSXmlUtil;

typedef void(*fForeachChild)(AVSXmlUtil &childXml, void *pUserData);
typedef std::list<AVSXmlUtil> AVSXmlList;

class AVSXML_API AVSXmlUtil
{
public:
	AVSXmlUtil();
	AVSXmlUtil(const AVSXmlObject xmlObj);
    AVSXmlUtil(const AVSXmlUtil &xmlUtil);
	virtual ~AVSXmlUtil();

public:

	static const char* MyName();

	bool Read(const std::string &sXmlFileorBuffer, bool isFile = false);
	bool Write(std::string &sXmlFileorBuffer, bool isFile = false);
	void Clear();
	void Print();
	
	bool TagExist(std::string sXmlTag);
	bool IsEmpty();

	bool GetValueAsString(std::string sXmlTag, std::string &value);
	bool GetValueAsInt(std::string sXmlTag, int &value);
	bool GetValueAsDouble(std::string sXmlTag, double &value);

	bool GetAttributeAsString(std::string sXmlTag, std::string sAttributeName, std::string &attr);
	bool GetAttributeAsInt(std::string sXmlTag, std::string sAttributeName, int &attr);
	bool GetAttributeAsDouble(std::string sXmlTag, std::string sAttributeName, double &attr);

	bool ForeachChild(std::string sXmlParentTag, std::string sXmlChildTag, fForeachChild foreachFunc, void *pUserData);
    bool GetAllChild(std::string sXmlParentTag, std::string sXmlChildTag, AVSXmlList &childList);

	bool SetStringValue(std::string sXmlTag, const char* value, AVSXmlAttributes *pAttrs = NULL);
	bool SetIntValue(std::string sXmlTag, int value, AVSXmlAttributes *pAttrs = NULL);
	bool SetFloatValue(std::string sXmlTag, double value, AVSXmlAttributes *pAttrs = NULL);

	bool SetAttributes(std::string sXmlTag, AVSXmlAttributes &attrs);

	bool SetChildWithIntValue(std::string sXmlParentTag, std::string sXmlChildTag, int value, AVSXmlAttributes *pAttrs = NULL);
	bool SetChildWithFloatValue(std::string sXmlParentTag, std::string sXmlChildTag, double value, AVSXmlAttributes *pAttrs = NULL);
	bool SetChildWithStringValue(std::string sXmlParentTag, std::string sXmlChildTag, std::string value, AVSXmlAttributes *pAttrs = NULL);

	bool AddChild(std::string sXmlParentTag, std::string sChildName, AVSXmlUtil &child, AVSXmlAttributes *pAttrs = NULL);
    bool Add(std::string sXmlParentTag, AVSXmlUtil &child, AVSXmlAttributes *pAttrs = NULL);
	bool AddArray(std::string sXmlParentTag, AVSXmlUtil &arry);
    
public:
	AVSXmlObject GetXmlObject() const; 
	AVSXmlObject GetChild(std::string sXmlTag);
	AVSXmlObject GetChildCreateNewIfNil(std::string sXmlTag);


private:
	AVSXmlObject xml;
};

#endif
