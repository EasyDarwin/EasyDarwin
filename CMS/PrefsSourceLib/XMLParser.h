/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#ifndef __XMLParser_h__
#define __XMLParser_h__

#include "StringParser.h"
#include "OSQueue.h"
#include "OSFileSource.h"
#include "ResizeableStringFormatter.h"

class DTDVerifier
{
public:
    virtual bool IsValidSubtag(char* tagName, char* subTagName) = 0;
    virtual bool IsValidAttributeName(char* tagName, char* attrName) = 0;
    virtual bool IsValidAttributeValue(char* tagName, char* attrName, char* attrValue) = 0;
    virtual char* GetRequiredAttribute(char* tagName, int index) = 0;
    virtual bool CanHaveValue(char* tagName) = 0;
	virtual ~DTDVerifier(){};
};

class XMLTag
{
public:
    XMLTag();
    XMLTag(char* tagName);
    ~XMLTag();
    
    bool ParseTag(StringParser* parser, DTDVerifier* verifier, char* errorBuffer = NULL, int errorBufferSize = 0);
    
    char* GetAttributeValue(const char* attrName);
    char* GetValue() { return fValue; }
    char* GetTagName() { return fTag; }
    
    UInt32 GetNumEmbeddedTags() { return fEmbeddedTags.GetLength(); }
    
    XMLTag* GetEmbeddedTag(const UInt32 index = 0);
    XMLTag* GetEmbeddedTagByName(const char* tagName, const UInt32 index = 0);
    XMLTag* GetEmbeddedTagByAttr(const char* attrName, const char* attrValue, const UInt32 index = 0);
    XMLTag* GetEmbeddedTagByNameAndAttr(const char* tagName, const char* attrName, const char* attrValue, const UInt32 index = 0);
    
    void AddAttribute(char* attrName, char* attrValue);
    void RemoveAttribute(char* attrName);
    void AddEmbeddedTag(XMLTag* tag);
    void RemoveEmbeddedTag(XMLTag* tag);
    
    void SetTagName( char* name);
    void SetValue( char* value);
    
    void FormatData(ResizeableStringFormatter* formatter, UInt32 indent);

private:
    void ConsumeIfComment(StringParser* parser);

    char* fTag;
    char* fValue;
    OSQueue fAttributes;
    OSQueue fEmbeddedTags;
    
    OSQueueElem fElem;

    static UInt8 sNonNameMask[];        // stop when you hit a word
};

class XMLAttribute
{
public:
    XMLAttribute();
    ~XMLAttribute();
    
    char* fAttrName;
    char* fAttrValue;
    
    OSQueueElem fElem;
};

class XMLParser
{
public:
    XMLParser( char* inPath, DTDVerifier* verifier = NULL);
    ~XMLParser();
    
    // Check for existence, man.
    Bool16  DoesFileExist();
    Bool16  DoesFileExistAsDirectory();
    Bool16  CanWriteFile();
    
    Bool16  ParseFile(char* errorBuffer = NULL, int errorBufferSize = 0);
        
    XMLTag* GetRootTag() { return fRootTag; }
    void SetRootTag(XMLTag* tag);
    
    void WriteToFile(char** fileHeader);
    
private:
    XMLTag*         fRootTag;
        
    OSFileSource    fFile;
    char*           fFilePath;
    DTDVerifier*    fVerifier;
};

#endif
