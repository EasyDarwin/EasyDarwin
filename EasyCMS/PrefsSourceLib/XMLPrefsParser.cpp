/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 */
 /*
	 Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	 Github: https://github.com/EasyDarwin
	 WEChat: EasyDarwin
	 Website: http://www.EasyDarwin.org
 */
 /*
	 File:       XMLPrefsParser.cpp
	 Contains:   Prototype implementation of XMLPrefsParser object.
 */

#include <sys/stat.h>

#ifndef __Win32__
#include <unistd.h>
#endif

#include "XMLPrefsParser.h"
#include "OSHeaders.h"

static const UInt32 kPrefArrayMinSize = 20;

static char* kMainTag = "CONFIG";
static char* kServer = "SERVER";
static char* kModule = "MODULE";
static char* kPref = "PREF";
static char* kListPref = "LIST-PREF";
static char* kEmptyObject = "EMPTY-OBJECT";
static char* kObject = "OBJECT";
static char* kObjectList = "LIST-OBJECT";
static char* kValue = "VALUE";
static char* kNameAttr = "NAME";
static char* kTypeAttr = "TYPE";

static char* kFileHeader[] =
{
	nullptr
};

XMLPrefsParser::XMLPrefsParser(char* inPath)
	: XMLParser(inPath)
{}

XMLPrefsParser::~XMLPrefsParser()
{}


ContainerRef XMLPrefsParser::getConfigurationTag()
{
	ContainerRef result = GetRootTag();
	if (result == nullptr)
	{
		result = new XMLTag(kMainTag);
		SetRootTag(result);
	}

	return result;
}

ContainerRef XMLPrefsParser::GetRefForModule(char* inModuleName, bool create)
{
	if (inModuleName == nullptr)
		return GetRefForServer();

	ContainerRef result = getConfigurationTag()->GetEmbeddedTagByNameAndAttr(kModule, kNameAttr, inModuleName);
	if (result == nullptr)
	{
		result = new XMLTag(kModule);
		result->AddAttribute(kNameAttr, (char*)inModuleName);
		GetRootTag()->AddEmbeddedTag(result);
	}

	return result;
}

ContainerRef XMLPrefsParser::GetRefForServer()
{
	ContainerRef result = getConfigurationTag()->GetEmbeddedTagByName(kServer);
	if (result == nullptr)
	{
		result = new XMLTag(kServer);
		GetRootTag()->AddEmbeddedTag(result);
	}

	return result;
}

UInt32 XMLPrefsParser::GetNumPrefValues(ContainerRef pref)
{
	if (!strcmp(pref->GetTagName(), kPref))
	{
		if (pref->GetValue() == nullptr)
			return 0;
		else
			return 1;
	}
	else if (!strcmp(pref->GetTagName(), kObject))
		return 1;
	else if (!strcmp(pref->GetTagName(), kEmptyObject))
		return 0;

	return pref->GetNumEmbeddedTags();  // it must be a list
}

UInt32 XMLPrefsParser::GetNumPrefsByContainer(ContainerRef container)
{
	return container->GetNumEmbeddedTags();
}

char* XMLPrefsParser::GetPrefValueByIndex(ContainerRef container, const UInt32 inPrefsIndex, const UInt32 inValueIndex,
	char** outPrefName, char** outDataType)
{
	if (outPrefName != nullptr)
		*outPrefName = nullptr;
	if (outPrefName != nullptr)
		*outDataType = nullptr;
	XMLTag* pref = container->GetEmbeddedTag(inPrefsIndex);
	if (pref == nullptr)
		return nullptr;

	return GetPrefValueByRef(pref, inValueIndex, outPrefName, outDataType);
}

char* XMLPrefsParser::GetPrefValueByRef(ContainerRef pref, const UInt32 inValueIndex,
	char** outPrefName, char** outDataType)
{
	if (outPrefName != nullptr)
		*outPrefName = pref->GetAttributeValue(kNameAttr);
	if (outDataType != nullptr)
	{
		*outDataType = pref->GetAttributeValue(kTypeAttr);
		if (*outDataType == nullptr)
			*outDataType = "CharArray";
	}

	if (!strcmp(pref->GetTagName(), kPref))
	{
		if (inValueIndex > 0)
			return nullptr;
		else
			return pref->GetValue();
	}

	if (!strcmp(pref->GetTagName(), kListPref))
	{
		XMLTag* value = pref->GetEmbeddedTag(inValueIndex);
		if (value != nullptr)
			return value->GetValue();
	}

	if (!strcmp(pref->GetTagName(), kObject) || !strcmp(pref->GetTagName(), kObjectList))
		*outDataType = "QTSS_Object";

	return nullptr;
}

ContainerRef XMLPrefsParser::GetObjectValue(ContainerRef pref, const UInt32 inValueIndex)
{
	if (!strcmp(pref->GetTagName(), kObject) && (inValueIndex == 0))
		return pref;
	if (!strcmp(pref->GetTagName(), kObjectList))
		return pref->GetEmbeddedTag(inValueIndex);

	return nullptr;
}

ContainerRef XMLPrefsParser::GetPrefRefByName(ContainerRef container,
	const char* inPrefName)
{
	return container->GetEmbeddedTagByAttr(kNameAttr, inPrefName);
}

ContainerRef XMLPrefsParser::GetPrefRefByIndex(ContainerRef container,
	const UInt32 inPrefsIndex)
{
	return container->GetEmbeddedTag(inPrefsIndex);
}

ContainerRef XMLPrefsParser::AddPref(ContainerRef container, char* inPrefName,
	char* inPrefDataType)
{
	XMLTag* pref = container->GetEmbeddedTagByAttr(kNameAttr, inPrefName);
	if (pref != nullptr)
		return pref;    // it already exists

	pref = new XMLTag(kPref);   // start it out as a pref
	pref->AddAttribute(kNameAttr, inPrefName);
	if (!strcmp(inPrefDataType, "QTSS_Object"))
		pref->SetTagName(kEmptyObject);
	else if (strcmp(inPrefDataType, "CharArray"))
		pref->AddAttribute(kTypeAttr, (char*)inPrefDataType);

	container->AddEmbeddedTag(pref);

	return pref;
}

void XMLPrefsParser::AddPrefValue(ContainerRef pref, char* inNewValue)
{
	if (!strcmp(pref->GetTagName(), kPref))     // is this a PREF tag
	{
		if (pref->GetValue() == nullptr)
		{
			// easy case, no existing value, so just add a vlue
			pref->SetValue(inNewValue);
			return;
		}
		else
		{
			// it already has a value, so change the pref to be a list pref and go to code below
			char* firstValue = pref->GetValue();
			XMLTag* value = new XMLTag(kValue);
			value->SetValue(firstValue);

			pref->SetTagName(kListPref);
			pref->SetValue(nullptr);
			pref->AddEmbeddedTag(value);
		}
	}

	// we want to fall through from second case above, so this isn't an else
	if (!strcmp(pref->GetTagName(), kListPref))
	{
		XMLTag* value = new XMLTag(kValue);
		value->SetValue(inNewValue);
		pref->AddEmbeddedTag(value);
	}
}

void XMLPrefsParser::AddNewObject(ContainerRef pref)
{
	if (!strcmp(pref->GetTagName(), kEmptyObject))
	{
		// just flag that this is now a real object instead of a placeholder
		pref->SetTagName(kObject);
		return;
	}

	if (!strcmp(pref->GetTagName(), kObject))
	{
		// change the object to be an object list and go to code below
		XMLTag* subObject = new XMLTag(kObject);
		XMLTag* objectPref;
		// copy all this objects tags into the new listed object
		while ((objectPref = pref->GetEmbeddedTag()) != nullptr)
		{
			pref->RemoveEmbeddedTag(objectPref);
			subObject->AddEmbeddedTag(objectPref);
		}

		pref->SetTagName(kObjectList);
		pref->AddEmbeddedTag(subObject);
	}

	// we want to fall through from second case above, so this isn't an else
	if (!strcmp(pref->GetTagName(), kObjectList))
	{
		XMLTag* subObject = new XMLTag(kObject);
		pref->AddEmbeddedTag(subObject);
	}
}

void XMLPrefsParser::ChangePrefType(ContainerRef pref, char* inNewPrefDataType)
{
	pref->RemoveAttribute(kTypeAttr);   // remove it if it exists
	if (strcmp(inNewPrefDataType, "CharArray"))
		pref->AddAttribute(kTypeAttr, inNewPrefDataType);
}

void XMLPrefsParser::SetPrefValue(ContainerRef pref, const UInt32 inValueIndex,
	char* inNewValue)
{
	UInt32 numValues = GetNumPrefValues(pref);

	if (((numValues == 0) || (numValues == 1)) && (inValueIndex == 0))
	{
		pref->SetValue(inNewValue);
	}
	else if (inValueIndex == numValues) // this is an additional value
		AddPrefValue(pref, inNewValue);
	else
	{
		XMLTag* value = pref->GetEmbeddedTag(inValueIndex);
		if (value != nullptr)
			value->SetValue(inNewValue);
	}
}

void XMLPrefsParser::RemovePrefValue(ContainerRef pref, const UInt32 inValueIndex)
{
	UInt32 numValues = GetNumPrefValues(pref);
	if (inValueIndex >= numValues)
		return;

	if (numValues == 1)
	{
		delete pref;    // just remove the whole pref
	}
	else if (numValues == 2)
	{
		XMLTag* value = pref->GetEmbeddedTag(inValueIndex); // get the one we're removing
		delete value;                                       // delete it
		value = pref->GetEmbeddedTag(0);         			// get the remaining tag index always 0 for 2 vals
		pref->RemoveEmbeddedTag(value);                     // pull it out of the parent
		if (!strcmp(pref->GetTagName(), kObjectList))
		{
			pref->SetTagName(kObject);  // set it back to a simple pref
			// move all this objects tags into the parent
			XMLTag* objectPref;
			while ((objectPref = value->GetEmbeddedTag()) != nullptr)
			{
				value->RemoveEmbeddedTag(objectPref);
				pref->AddEmbeddedTag(objectPref);
			}
		}
		else
		{
			char* temp = value->GetValue();
			pref->SetTagName(kPref);    // set it back to a simple pref
			pref->SetValue(temp);
		}

		delete value;   // get rid of the other one
	}
	else
	{
		XMLTag* value = pref->GetEmbeddedTag(inValueIndex);
		if (value)
			delete value;
	}
}

void XMLPrefsParser::RemovePref(ContainerRef pref)
{
	delete pref;
}

int XMLPrefsParser::Parse()
{
	char error[500];

	if (!ParseFile(error, sizeof(error)))
	{
		qtss_printf("%s\n", error);
		return -1;
	}



	// the above routine checks that it's a valid XML file, we should check that
	// all the tags conform to our prefs format

	return 0;
}

int XMLPrefsParser::WritePrefsFile()
{
	getConfigurationTag();  // force it to be created if it doesn't exist
	WriteToFile(kFileHeader);
	return 0;
}
