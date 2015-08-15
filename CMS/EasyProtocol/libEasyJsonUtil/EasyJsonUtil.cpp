/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include <EasyJsonUtil.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>  
#include <boost/typeof/typeof.hpp>  
#include <iostream>
#include <stdarg.h>

using boost::property_tree::ptree;
//============================================

EasyJsonAttributes::EasyJsonAttributes()
{
	attrs = new EasyJsonAttrs;
}

EasyJsonAttributes::~EasyJsonAttributes()
{
	delete attrs;
}

EasyJsonAttrs& EasyJsonAttributes::GetAttrs()
{
    return *attrs;
}

void EasyJsonAttributes::AddFloatAttribute(const char* name, double value)
{
    attrs->insert(std::make_pair(name, boost::lexical_cast<std::string>(value)));
}

void EasyJsonAttributes::AddIntAttribute(const char* name, int value)
{
    attrs->insert(std::make_pair(name, boost::lexical_cast<std::string>(value)));
}

void EasyJsonAttributes::AddStringAttribute(const char* name, const char* value)
{
    attrs->insert(std::make_pair(name, boost::lexical_cast<std::string>(value)));
}

void EasyJsonAttributes::Clear()
{
    attrs->clear();
}

//====================================
EasyJsonTag::EasyJsonTag(const std::string &tag)
{
	str = new std::string(tag);
}

EasyJsonTag::EasyJsonTag(const char* tag)
{
	str = new std::string(tag);
}

EasyJsonTag::EasyJsonTag(const EasyJsonTag &tag)
{
    str = new std::string(tag.ToString());
}

EasyJsonTag::~EasyJsonTag()
{
	if (str != NULL)
	{
		delete str;
	}
}

EasyJsonTag EasyJsonTag::operator+(const EasyJsonTag &tag)
{
	return ToString() + "." + tag.ToString();
}

std::string EasyJsonTag::ToString() const
{
	if (str != NULL)
	{
		return *str;
	}
	return std::string();
}

//====================================

EasyJsonUtil::EasyJsonUtil()
{
    json = new ptree;
}

EasyJsonUtil::EasyJsonUtil(const EasyJsonObject jsonObj)
{
    json = new ptree;
    if (jsonObj != NULL)
    {
        *((ptree*) json) = *((ptree*) jsonObj);
    }
}

EasyJsonUtil::EasyJsonUtil(const EasyJsonUtil &jsonUtil)
{
    json = new ptree;
    *((ptree*) json) = *((ptree*) jsonUtil.GetJsonObject());
}

EasyJsonUtil::~EasyJsonUtil()
{
    if (json != NULL)
    {
        delete (ptree*) json;
        json = NULL;
    }
}

void EasyJsonUtil::Print()
{
    std::string sJson;
    if (Write(sJson, false))
    {
        std::cout << sJson << std::endl;
    }
}

const char* EasyJsonUtil::MyName()
{
    return "libeasyjsonutil";
}

EasyJsonObject EasyJsonUtil::GetJsonObject() const
{
    return json;
}

bool EasyJsonUtil::Read(const std::string &sJsonFileorBuffer, bool isFile)
{
    try
    {
        if (!isFile)
        {
            std::stringstream stream(sJsonFileorBuffer);
            boost::property_tree::read_json(stream, *((ptree*) json));
        }
        else
        {
			boost::property_tree::read_json(sJsonFileorBuffer, *((ptree*)json));
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]read json failed: %s\n", MyName(), e.what());
        return false;
    }
    return true;
}

bool EasyJsonUtil::Write(std::string &sJsonFileorBuffer, bool isFile)
{
    try
    {
        if (isFile)
        {
            boost::property_tree::write_json(sJsonFileorBuffer, *((ptree*) json), std::locale());
        }
        else
        {
            std::stringstream stream;
			boost::property_tree::write_json(stream, *((ptree*)json));
            sJsonFileorBuffer = std::string(stream.str());
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]write json failed: %s\n", MyName(), e.what());
        return false;
    }
    return true;
}

void EasyJsonUtil::Clear()
{
    ((ptree*) json)->clear();
}

bool EasyJsonUtil::TagExist(std::string sJsonTag)
{
    if (((ptree*) json)->get_child_optional(sJsonTag))
    {
        return true;
    }

    return false;
}

bool EasyJsonUtil::IsEmpty()
{
	return ((ptree*) json)->empty();
}

EasyJsonObject EasyJsonUtil::GetChild(std::string sJsonTag)
{
    if (TagExist(sJsonTag))
    {
        return &(((ptree*) json)->get_child(sJsonTag));
    }

    return NULL;
}

bool EasyJsonUtil::ForeachChild(std::string sJsonParentTag, std::string sJsonChildTag, fForeachChild foreachFunc, void* pUserData)
{
    try
    {
        EasyJsonObject childObj = GetChild(sJsonParentTag);
        if (childObj == NULL)
        {
            printf("[%s]ForeachChild[%s.%s] failed: no such node\n", MyName(), sJsonParentTag.c_str(), sJsonChildTag.c_str());
            return false;
        }

        if (foreachFunc == NULL)
        {
            printf("[%s]ForeachChild[%s.%s] failed: fForeachChild callback function is not set\n", MyName(), sJsonParentTag.c_str(), sJsonChildTag.c_str());
            return false;
        }

        ptree &pt = *((ptree*) childObj);

        BOOST_FOREACH(const ptree::value_type &value, pt)
        {
            if (value.first == sJsonChildTag)
            {
                ptree pt_child;
                pt_child.add_child(value.first, value.second);
                EasyJsonUtil childJson((EasyJsonObject) & pt_child);
                foreachFunc(childJson, pUserData);
            }
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]ForeachChild[%s.%s] failed: %s\n", MyName(), sJsonParentTag.c_str(), sJsonChildTag.c_str(), e.what());
        return false;
    }

    return true;
}

bool EasyJsonUtil::GetAllChild(std::string sJsonParentTag, std::string sJsonChildTag, EasyJsonList& childList)
{
    try
    {
       /* EasyJsonObject childObj = GetChild(sJsonParentTag);
        if (childObj == NULL)
        {
            printf("[%s]GetAllChild[%s.%s] failed: no such node\n", MyName(), sJsonParentTag.c_str(), sJsonChildTag.c_str());
            return false;
        }

        ptree &pt = *((ptree*) childObj);*/
		ptree &pt = *((ptree*)json);
        BOOST_FOREACH(const ptree::value_type &value, pt)
        {
            //if (value.first == sJsonChildTag)
            {
                //ptree pt_child;
                //pt_child.add_child(value.first, value.second);
				EasyJsonUtil childJson((EasyJsonObject)& value.second);
                childList.push_back(childJson);                
            }
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]GetAllChild[%s.%s] failed: %s\n", MyName(), sJsonParentTag.c_str(), sJsonChildTag.c_str(), e.what());
        return false;
    }

    return true;
}


template<class type>
bool GetAttribute(EasyJsonUtil *jsonUtil, std::string& sJsonTag, std::string sAttributeName, type& attr)
{
    EasyJsonObject json = jsonUtil->GetJsonObject();

    try
    {
        attr = ((ptree*) json)->get_child(sJsonTag).get<type>("<jsonattr>." + sAttributeName);
    }
    catch (std::exception &e)
    {
        printf("[%s]GetAttribute [%s.%s] error: %s\n", EasyJsonUtil::MyName(), sJsonTag.c_str(), sAttributeName.c_str(), e.what());
        return false;
    }

    return true;
}

template<class type>
bool GetValue(EasyJsonUtil *jsonUtil, std::string& sJsonTag, type& value)
{
    EasyJsonObject json = jsonUtil->GetJsonObject();

    try
    {
        value = ((ptree*) json)->get<type>(sJsonTag);
    }
    catch (std::exception &e)
    {
        printf("[%s]GetValue [%s] error: %s\n", EasyJsonUtil::MyName(), sJsonTag.c_str(), e.what());
        return false;
    }

    return true;
}

bool EasyJsonUtil::GetAttributeAsDouble(std::string sJsonTag, std::string sAttributeName, double& attr)
{
    return GetAttribute<double>(this, sJsonTag, sAttributeName, attr);
}

bool EasyJsonUtil::GetAttributeAsInt(std::string sJsonTag, std::string sAttributeName, int& attr)
{
    return GetAttribute<int>(this, sJsonTag, sAttributeName, attr);
}

bool EasyJsonUtil::GetAttributeAsString(std::string sJsonTag, std::string sAttributeName, std::string& attr)
{
    return GetAttribute<std::string>(this, sJsonTag, sAttributeName, attr);
}

bool EasyJsonUtil::GetValueAsDouble(std::string sJsonTag, double& value)
{
    return GetValue<double>(this, sJsonTag, value);
}

bool EasyJsonUtil::GetValueAsInt(std::string sJsonTag, int& value)
{
    return GetValue<int>(this, sJsonTag, value);
}

bool EasyJsonUtil::GetValueAsString(std::string sJsonTag, std::string& value)
{
    return GetValue<std::string>(this, sJsonTag, value);
}

template<class type>
bool SetValue(EasyJsonUtil *jsonUtil, std::string sJsonTag, type& value, EasyJsonAttributes *pAttrs = NULL)
{
    EasyJsonObject json = jsonUtil->GetJsonObject();

    try
    {
        /*if (!jsonUtil->TagExist(sJsonTag))
        {
            ((ptree*) json)->add(sJsonTag, "");
        }

        ptree *pt = (ptree*) jsonUtil->GetChild(sJsonTag);

        pt->put_value(value);

        if (pAttrs != NULL)
        {

            BOOST_FOREACH(EasyJsonAttrs::value_type &value, pAttrs->GetAttrs())
            {
                pt->add("<jsonattr>." + value.first, value.second);
            }
        }*/
		((ptree*)json)->put(sJsonTag, value);
    }
    catch (std::exception &e)
    {
        printf("[%s]SetValue [%s] error: %s\n", EasyJsonUtil::MyName(), sJsonTag.c_str(), e.what());
        return false;
    }

    return true;
}

bool EasyJsonUtil::SetFloatValue(std::string sJsonTag, double value, EasyJsonAttributes *pAttrs)
{
    return SetValue<double>(this, sJsonTag, value, pAttrs);
}

bool EasyJsonUtil::SetIntValue(std::string sJsonTag, int value, EasyJsonAttributes *pAttrs)
{
    return SetValue<int>(this, sJsonTag, value, pAttrs);
}

bool EasyJsonUtil::SetStringValue(std::string sJsonTag, const char* value, EasyJsonAttributes *pAttrs)
{
    std::string sValue = std::string(value);
    return SetValue<std::string>(this, sJsonTag, sValue, pAttrs);
}

bool EasyJsonUtil::SetAttributes(std::string sJsonTag, EasyJsonAttributes& attrs)
{
    try
    {
        if (!TagExist(sJsonTag))
        {
            SetStringValue(sJsonTag, "");
        }

        ptree &pt = ((ptree*) json)->get_child(sJsonTag);

        BOOST_FOREACH(EasyJsonAttrs::value_type &value, attrs.GetAttrs())
        {
            pt.add("<jsonattr>." + value.first, value.second);
        }
    }
    catch (std::exception &e)
    {
        printf("[%s]SetAttributes [%s] error: %s\n", EasyJsonUtil::MyName(), sJsonTag.c_str(), e.what());
        return false;
    }

    return true;
}

template<class type>
bool SetChildValue(EasyJsonUtil *jsonUtil, std::string sJsonParentTag, std::string sJsonChildTag, type& value, EasyJsonAttributes *pAttrs)
{
    EasyJsonObject json = jsonUtil->GetJsonObject();

    try
    {
        ptree pt_child;
        pt_child.put_value(value);
        if (pAttrs != NULL)
        {
            BOOST_FOREACH(EasyJsonAttrs::value_type &value, pAttrs->GetAttrs())
            {
                pt_child.add("<jsonattr>." + value.first, value.second);
            }
        }

        if (!((ptree*) json)->get_child_optional(sJsonParentTag))
        {
            jsonUtil->SetStringValue(sJsonParentTag, "");
        }
        ptree &pt = ((ptree*) json)->get_child(sJsonParentTag);
        pt.add_child(sJsonChildTag, pt_child);
    }
    catch (std::exception &e)
    {
        printf("[%s]SetChildValue [%s.%s] error: %s\n", EasyJsonUtil::MyName(), sJsonParentTag.c_str(), sJsonChildTag.c_str(), e.what());
        return false;
    }

    return true;
}

bool EasyJsonUtil::SetChildWithFloatValue(std::string sJsonParentTag, std::string sJsonChildTag, double value, EasyJsonAttributes* pAttrs)
{
    return SetChildValue<double>(this, sJsonParentTag, sJsonChildTag, value, pAttrs);
}

bool EasyJsonUtil::SetChildWithIntValue(std::string sJsonParentTag, std::string sJsonChildTag, int value, EasyJsonAttributes* pAttrs)
{
    return SetChildValue<int>(this, sJsonParentTag, sJsonChildTag, value, pAttrs);
}

bool EasyJsonUtil::SetChildWithStringValue(std::string sJsonParentTag, std::string sJsonChildTag, std::string value, EasyJsonAttributes* pAttrs)
{
    return SetChildValue<std::string>(this, sJsonParentTag, sJsonChildTag, value, pAttrs);
}

EasyJsonObject EasyJsonUtil::GetChildCreateNewIfNil(std::string sJsonTag)
{
    EasyJsonObject obj = GetChild(sJsonTag);
    if (obj == NULL)
    {
        SetStringValue(sJsonTag, "");
        obj = GetChild(sJsonTag);
    }

    return obj;
}

bool EasyJsonUtil::AddChild(std::string sJsonParentTag, std::string sChildName, EasyJsonUtil &child, EasyJsonAttributes* pAttrs)
{
    try
    {
        ptree *pt = (ptree*) GetChildCreateNewIfNil(sJsonParentTag);

        ptree &pt_child = *((ptree*) child.GetJsonObject());
        if (pAttrs != NULL)
        {

            BOOST_FOREACH(EasyJsonAttrs::value_type &value, pAttrs->GetAttrs())
            {
                pt_child.add("<jsonattr>." + value.first, value.second);
            }
        }

        pt->add_child(sChildName, pt_child);
    }
    catch (std::exception &e)
    {
        printf("[%s]AddChild [%s.%s] error: %s\n", EasyJsonUtil::MyName(), sJsonParentTag.c_str(), sChildName.c_str(), e.what());
        return false;
    }

    return true;
}

bool EasyJsonUtil::AddArray(std::string sJsonParentTag, EasyJsonUtil &arry)
{
	try
    {
        ptree *pt = (ptree*) GetChildCreateNewIfNil(sJsonParentTag);

        ptree &pt_child = *((ptree*) arry.GetJsonObject());
       
		pt->push_back(std::make_pair("", pt_child));
    }
    catch (std::exception &e)
    {
        printf("[%s]AddArray [%s] error: %s\n", EasyJsonUtil::MyName(), sJsonParentTag.c_str(), e.what());
        return false;
    }

    return true;
}

bool EasyJsonUtil::Add(std::string sJsonParentTag, EasyJsonUtil& child, EasyJsonAttributes* pAttrs)
{
    try
    {               
        ptree &pt_child = *((ptree*) child.GetJsonObject());
        if (pAttrs != NULL)
        {

            BOOST_FOREACH(EasyJsonAttrs::value_type &value, pAttrs->GetAttrs())
            {
                pt_child.add("<jsonattr>." + value.first, value.second);
            }
        }

        ((ptree*) json)->add_child(sJsonParentTag, pt_child);
    }
    catch (std::exception &e)
    {
        printf("[%s]AddChild [%s] error: %s\n", EasyJsonUtil::MyName(), sJsonParentTag.c_str(), e.what());
        return false;
    }

    return true;
}

