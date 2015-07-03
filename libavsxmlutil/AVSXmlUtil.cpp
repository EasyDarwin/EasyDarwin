/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include <AVSXmlUtil.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>  
#include <boost/typeof/typeof.hpp>  
#include <iostream>
#include <stdarg.h>

using boost::property_tree::ptree;
//============================================

AVSXmlAttributes::AVSXmlAttributes()
{
	attrs = new AVSXmlAttrs;
}

AVSXmlAttributes::~AVSXmlAttributes()
{
	delete attrs;
}

AVSXmlAttrs& AVSXmlAttributes::GetAttrs()
{
    return *attrs;
}

void AVSXmlAttributes::AddFloatAttribute(const char* name, double value)
{
    attrs->insert(std::make_pair(name, boost::lexical_cast<std::string>(value)));
}

void AVSXmlAttributes::AddIntAttribute(const char* name, int value)
{
    attrs->insert(std::make_pair(name, boost::lexical_cast<std::string>(value)));
}

void AVSXmlAttributes::AddStringAttribute(const char* name, const char* value)
{
    attrs->insert(std::make_pair(name, boost::lexical_cast<std::string>(value)));
}

void AVSXmlAttributes::Clear()
{
    attrs->clear();
}

//====================================
AVSXmlTag::AVSXmlTag(const std::string &tag)
{
	str = new std::string(tag);
}

AVSXmlTag::AVSXmlTag(const char* tag)
{
	str = new std::string(tag);
}

AVSXmlTag::AVSXmlTag(const AVSXmlTag &tag)
{
    str = new std::string(tag.ToString());
}

AVSXmlTag::~AVSXmlTag()
{
	if (str != NULL)
	{
		delete str;
	}
}

AVSXmlTag AVSXmlTag::operator+(const AVSXmlTag &tag)
{
	return ToString() + "." + tag.ToString();
}

std::string AVSXmlTag::ToString() const
{
	if (str != NULL)
	{
		return *str;
	}
	return std::string();
}

//====================================

AVSXmlUtil::AVSXmlUtil()
{
    xml = new ptree;
}

AVSXmlUtil::AVSXmlUtil(const AVSXmlObject xmlObj)
{
    xml = new ptree;
    if (xmlObj != NULL)
    {
        *((ptree*) xml) = *((ptree*) xmlObj);
    }
}

AVSXmlUtil::AVSXmlUtil(const AVSXmlUtil &xmlUtil)
{
    xml = new ptree;
    *((ptree*) xml) = *((ptree*) xmlUtil.GetXmlObject());
}

AVSXmlUtil::~AVSXmlUtil()
{
    if (xml != NULL)
    {
        delete (ptree*) xml;
        xml = NULL;
    }
}

void AVSXmlUtil::Print()
{
    std::string sXml;
    if (Write(sXml, false))
    {
        std::cout << sXml << std::endl;
    }
}

const char* AVSXmlUtil::MyName()
{
    return "libavsxmlutil";
}

AVSXmlObject AVSXmlUtil::GetXmlObject() const
{
    return xml;
}

bool AVSXmlUtil::Read(const std::string &sXmlFileorBuffer, bool isFile)
{
    try
    {
        if (!isFile)
        {
            std::stringstream stream(sXmlFileorBuffer);
            boost::property_tree::read_json(stream, *((ptree*) xml));
        }
        else
        {
			boost::property_tree::read_json(sXmlFileorBuffer, *((ptree*)xml));
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]read xml failed: %s\n", MyName(), e.what());
        return false;
    }
    return true;
}

bool AVSXmlUtil::Write(std::string &sXmlFileorBuffer, bool isFile)
{
    try
    {
        if (isFile)
        {
            boost::property_tree::write_json(sXmlFileorBuffer, *((ptree*) xml), std::locale());
        }
        else
        {
            std::stringstream stream;
			boost::property_tree::write_json(stream, *((ptree*)xml));
            sXmlFileorBuffer = std::string(stream.str());
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]write xml failed: %s\n", MyName(), e.what());
        return false;
    }
    return true;
}

void AVSXmlUtil::Clear()
{
    ((ptree*) xml)->clear();
}

bool AVSXmlUtil::TagExist(std::string sXmlTag)
{
    if (((ptree*) xml)->get_child_optional(sXmlTag))
    {
        return true;
    }

    return false;
}

AVSXmlObject AVSXmlUtil::GetChild(std::string sXmlTag)
{
    if (TagExist(sXmlTag))
    {
        return &(((ptree*) xml)->get_child(sXmlTag));
    }

    return NULL;
}

bool AVSXmlUtil::ForeachChild(std::string sXmlParentTag, std::string sXmlChildTag, fForeachChild foreachFunc, void* pUserData)
{
    try
    {
        AVSXmlObject childObj = GetChild(sXmlParentTag);
        if (childObj == NULL)
        {
            printf("[%s]ForeachChild[%s.%s] failed: no such node\n", MyName(), sXmlParentTag.c_str(), sXmlChildTag.c_str());
            return false;
        }

        if (foreachFunc == NULL)
        {
            printf("[%s]ForeachChild[%s.%s] failed: fForeachChild callback function is not set\n", MyName(), sXmlParentTag.c_str(), sXmlChildTag.c_str());
            return false;
        }

        ptree &pt = *((ptree*) childObj);

        BOOST_FOREACH(const ptree::value_type &value, pt)
        {
            if (value.first == sXmlChildTag)
            {
                ptree pt_child;
                pt_child.add_child(value.first, value.second);
                AVSXmlUtil childXml((AVSXmlObject) & pt_child);
                foreachFunc(childXml, pUserData);
            }
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]ForeachChild[%s.%s] failed: %s\n", MyName(), sXmlParentTag.c_str(), sXmlChildTag.c_str(), e.what());
        return false;
    }

    return true;
}

bool AVSXmlUtil::GetAllChild(std::string sXmlParentTag, std::string sXmlChildTag, AVSXmlList& childList)
{
    try
    {
       /* AVSXmlObject childObj = GetChild(sXmlParentTag);
        if (childObj == NULL)
        {
            printf("[%s]GetAllChild[%s.%s] failed: no such node\n", MyName(), sXmlParentTag.c_str(), sXmlChildTag.c_str());
            return false;
        }

        ptree &pt = *((ptree*) childObj);*/
		ptree &pt = *((ptree*)xml);
        BOOST_FOREACH(const ptree::value_type &value, pt)
        {
            //if (value.first == sXmlChildTag)
            {
                //ptree pt_child;
                //pt_child.add_child(value.first, value.second);
				AVSXmlUtil childXml((AVSXmlObject)& value.second);
                childList.push_back(childXml);                
            }
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]GetAllChild[%s.%s] failed: %s\n", MyName(), sXmlParentTag.c_str(), sXmlChildTag.c_str(), e.what());
        return false;
    }

    return true;
}


template<class type>
bool GetAttribute(AVSXmlUtil *xmlUtil, std::string& sXmlTag, std::string sAttributeName, type& attr)
{
    AVSXmlObject xml = xmlUtil->GetXmlObject();

    try
    {
        attr = ((ptree*) xml)->get_child(sXmlTag).get<type>("<xmlattr>." + sAttributeName);
    }
    catch (std::exception &e)
    {
        printf("[%s]GetAttribute [%s.%s] error: %s\n", AVSXmlUtil::MyName(), sXmlTag.c_str(), sAttributeName.c_str(), e.what());
        return false;
    }

    return true;
}

template<class type>
bool GetValue(AVSXmlUtil *xmlUtil, std::string& sXmlTag, type& value)
{
    AVSXmlObject xml = xmlUtil->GetXmlObject();

    try
    {
        value = ((ptree*) xml)->get<type>(sXmlTag);
    }
    catch (std::exception &e)
    {
        printf("[%s]GetValue [%s] error: %s\n", AVSXmlUtil::MyName(), sXmlTag.c_str(), e.what());
        return false;
    }

    return true;
}

bool AVSXmlUtil::GetAttributeAsDouble(std::string sXmlTag, std::string sAttributeName, double& attr)
{
    return GetAttribute<double>(this, sXmlTag, sAttributeName, attr);
}

bool AVSXmlUtil::GetAttributeAsInt(std::string sXmlTag, std::string sAttributeName, int& attr)
{
    return GetAttribute<int>(this, sXmlTag, sAttributeName, attr);
}

bool AVSXmlUtil::GetAttributeAsString(std::string sXmlTag, std::string sAttributeName, std::string& attr)
{
    return GetAttribute<std::string>(this, sXmlTag, sAttributeName, attr);
}

bool AVSXmlUtil::GetValueAsDouble(std::string sXmlTag, double& value)
{
    return GetValue<double>(this, sXmlTag, value);
}

bool AVSXmlUtil::GetValueAsInt(std::string sXmlTag, int& value)
{
    return GetValue<int>(this, sXmlTag, value);
}

bool AVSXmlUtil::GetValueAsString(std::string sXmlTag, std::string& value)
{
    return GetValue<std::string>(this, sXmlTag, value);
}

template<class type>
bool SetValue(AVSXmlUtil *xmlUtil, std::string sXmlTag, type& value, AVSXmlAttributes *pAttrs = NULL)
{
    AVSXmlObject xml = xmlUtil->GetXmlObject();

    try
    {
        /*if (!xmlUtil->TagExist(sXmlTag))
        {
            ((ptree*) xml)->add(sXmlTag, "");
        }

        ptree *pt = (ptree*) xmlUtil->GetChild(sXmlTag);

        pt->put_value(value);

        if (pAttrs != NULL)
        {

            BOOST_FOREACH(AVSXmlAttrs::value_type &value, pAttrs->GetAttrs())
            {
                pt->add("<xmlattr>." + value.first, value.second);
            }
        }*/
		((ptree*)xml)->put(sXmlTag, value);
    }
    catch (std::exception &e)
    {
        printf("[%s]SetValue [%s] error: %s\n", AVSXmlUtil::MyName(), sXmlTag.c_str(), e.what());
        return false;
    }

    return true;
}

bool AVSXmlUtil::SetFloatValue(std::string sXmlTag, double value, AVSXmlAttributes *pAttrs)
{
    return SetValue<double>(this, sXmlTag, value, pAttrs);
}

bool AVSXmlUtil::SetIntValue(std::string sXmlTag, int value, AVSXmlAttributes *pAttrs)
{
    return SetValue<int>(this, sXmlTag, value, pAttrs);
}

bool AVSXmlUtil::SetStringValue(std::string sXmlTag, const char* value, AVSXmlAttributes *pAttrs)
{
    std::string sValue = std::string(value);
    return SetValue<std::string>(this, sXmlTag, sValue, pAttrs);
}

bool AVSXmlUtil::SetAttributes(std::string sXmlTag, AVSXmlAttributes& attrs)
{
    try
    {
        if (!TagExist(sXmlTag))
        {
            SetStringValue(sXmlTag, "");
        }

        ptree &pt = ((ptree*) xml)->get_child(sXmlTag);

        BOOST_FOREACH(AVSXmlAttrs::value_type &value, attrs.GetAttrs())
        {
            pt.add("<xmlattr>." + value.first, value.second);
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]SetAttributes [%s] error: %s\n", AVSXmlUtil::MyName(), sXmlTag.c_str(), e.what());
        return false;
    }

    return true;
}

template<class type>
bool SetChildValue(AVSXmlUtil *xmlUtil, std::string sXmlParentTag, std::string sXmlChildTag, type& value, AVSXmlAttributes *pAttrs)
{
    AVSXmlObject xml = xmlUtil->GetXmlObject();

    try
    {
        ptree pt_child;
        pt_child.put_value(value);
        if (pAttrs != NULL)
        {
            BOOST_FOREACH(AVSXmlAttrs::value_type &value, pAttrs->GetAttrs())
            {
                pt_child.add("<xmlattr>." + value.first, value.second);
            }
        }

        if (!((ptree*) xml)->get_child_optional(sXmlParentTag))
        {
            xmlUtil->SetStringValue(sXmlParentTag, "");
        }
        ptree &pt = ((ptree*) xml)->get_child(sXmlParentTag);
        pt.add_child(sXmlChildTag, pt_child);
    }
    catch (std::exception &e)
    {
        printf("[%s]SetChildValue [%s.%s] error: %s\n", AVSXmlUtil::MyName(), sXmlParentTag.c_str(), sXmlChildTag.c_str(), e.what());
        return false;
    }

    return true;
}

bool AVSXmlUtil::SetChildWithFloatValue(std::string sXmlParentTag, std::string sXmlChildTag, double value, AVSXmlAttributes* pAttrs)
{
    return SetChildValue<double>(this, sXmlParentTag, sXmlChildTag, value, pAttrs);
}

bool AVSXmlUtil::SetChildWithIntValue(std::string sXmlParentTag, std::string sXmlChildTag, int value, AVSXmlAttributes* pAttrs)
{
    return SetChildValue<int>(this, sXmlParentTag, sXmlChildTag, value, pAttrs);
}

bool AVSXmlUtil::SetChildWithStringValue(std::string sXmlParentTag, std::string sXmlChildTag, std::string value, AVSXmlAttributes* pAttrs)
{
    return SetChildValue<std::string>(this, sXmlParentTag, sXmlChildTag, value, pAttrs);
}

AVSXmlObject AVSXmlUtil::GetChildCreateNewIfNil(std::string sXmlTag)
{
    AVSXmlObject obj = GetChild(sXmlTag);
    if (obj == NULL)
    {
        SetStringValue(sXmlTag, "");
        obj = GetChild(sXmlTag);
    }

    return obj;
}

bool AVSXmlUtil::AddChild(std::string sXmlParentTag, std::string sChildName, AVSXmlUtil &child, AVSXmlAttributes* pAttrs)
{
    try
    {
        ptree *pt = (ptree*) GetChildCreateNewIfNil(sXmlParentTag);

        ptree &pt_child = *((ptree*) child.GetXmlObject());
        if (pAttrs != NULL)
        {

            BOOST_FOREACH(AVSXmlAttrs::value_type &value, pAttrs->GetAttrs())
            {
                pt_child.add("<xmlattr>." + value.first, value.second);
            }
        }

        pt->add_child(sChildName, pt_child);
    }
    catch (std::exception &e)
    {
        printf("[%s]AddChild [%s.%s] error: %s\n", AVSXmlUtil::MyName(), sXmlParentTag.c_str(), sChildName.c_str(), e.what());
        return false;
    }

    return true;
}

bool AVSXmlUtil::Add(std::string sXmlParentTag, AVSXmlUtil& child, AVSXmlAttributes* pAttrs)
{
    try
    {               
        ptree &pt_child = *((ptree*) child.GetXmlObject());
        if (pAttrs != NULL)
        {

            BOOST_FOREACH(AVSXmlAttrs::value_type &value, pAttrs->GetAttrs())
            {
                pt_child.add("<xmlattr>." + value.first, value.second);
            }
        }

        ((ptree*) xml)->add_child(sXmlParentTag, pt_child);
    }
    catch (std::exception &e)
    {
        printf("[%s]AddChild [%s] error: %s\n", AVSXmlUtil::MyName(), sXmlParentTag.c_str(), e.what());
        return false;
    }

    return true;
}

