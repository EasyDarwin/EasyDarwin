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
#include <sys/types.h>
#include <sys/stat.h>

#ifndef __Win32__
#include <unistd.h>
#endif

#include "XMLParser.h"
#include "OSMemory.h"

XMLParser::XMLParser(char* inPath, DTDVerifier* verifier)
	: fRootTag(NULL), fFilePath(NULL)
{
	StrPtrLen thePath(inPath);
	fFilePath = thePath.GetAsCString();
	fFile.Set(inPath);
	fVerifier = verifier;
}

XMLParser::~XMLParser()
{
	if (fRootTag)
		delete fRootTag;

	delete[] fFilePath;
}

Bool16 XMLParser::ParseFile(char* errorBuffer, int errorBufferSize)
{
	if (fRootTag != NULL)
	{
		delete fRootTag;    // flush old data
		fRootTag = NULL;
	}

	fFile.Set(fFilePath);

	if (errorBufferSize < 500) errorBuffer = NULL;  // Just a hack to avoid checking everywhere
	if ((fFile.GetLength() == 0) || fFile.IsDir())
	{
		if (errorBuffer != NULL)
			qtss_snprintf(errorBuffer, errorBufferSize, "Couldn't read xml file");
		return false;   // we don't have a valid file;
	}

	char* fileData = NEW char[(SInt32)(fFile.GetLength() + 1)];
	UInt32 theLengthRead = 0;
	fFile.Read(0, fileData, (UInt32)fFile.GetLength(), &theLengthRead);

	StrPtrLen theDataPtr(fileData, theLengthRead);
	StringParser theParser(&theDataPtr);

	fRootTag = NEW XMLTag();
	Bool16 result = fRootTag->ParseTag(&theParser, fVerifier, errorBuffer, errorBufferSize);
	if (!result)
	{
		// got error parsing file
		delete fRootTag;
		fRootTag = NULL;
	}

	delete fileData;

	fFile.Close();

	return result;
}

Bool16  XMLParser::DoesFileExist()
{
	Bool16 itExists = false;
	fFile.Set(fFilePath);
	if ((fFile.GetLength() > 0) && (!fFile.IsDir()))
		itExists = true;
	fFile.Close();

	return itExists;
}

Bool16  XMLParser::DoesFileExistAsDirectory()
{
	Bool16 itExists = false;
	fFile.Set(fFilePath);
	if (fFile.IsDir())
		itExists = true;
	fFile.Close();

	return itExists;
}

Bool16  XMLParser::CanWriteFile()
{
	//
	// First check if it exists for reading
	FILE* theFile = ::fopen(fFilePath, "r");
	if (theFile == NULL)
		return true;

	::fclose(theFile);

	//
	// File exists for reading, check if we can write it
	theFile = ::fopen(fFilePath, "a");
	if (theFile == NULL)
		return false;

	//
	// We can read and write
	::fclose(theFile);
	return true;
}

void XMLParser::SetRootTag(XMLTag* tag)
{
	if (fRootTag != NULL)
		delete fRootTag;
	fRootTag = tag;
}

void XMLParser::WriteToFile(char** fileHeader)
{
	char theBuffer[8192];
	ResizeableStringFormatter formatter(theBuffer, 8192);

	//
	// Write the file header
	for (UInt32 a = 0; fileHeader[a] != NULL; a++)
	{
		formatter.Put(fileHeader[a]);
		formatter.Put(kEOLString);
	}

	if (fRootTag)
		fRootTag->FormatData(&formatter, 0);

	//
	// New libC code. This seems to work better on Win32
	formatter.PutTerminator();
	FILE* theFile = ::fopen(fFilePath, "w");
	if (theFile == NULL)
		return;

	qtss_fprintf(theFile, "%s", formatter.GetBufPtr());
	::fclose(theFile);

#if __MacOSX__
	(void) ::chown(fFilePath, 76, 80);//owner qtss, group admin
#endif

#ifndef __Win32__
	::chmod(fFilePath, S_IRUSR | S_IWUSR | S_IRGRP);
#endif
}

UInt8 XMLTag::sNonNameMask[] =
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0-9 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //10-19 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //20-29
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //30-39 
	1, 1, 1, 1, 1, 0, 0, 1, 0, 0, //40-49 '.' and '-' are name chars
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //50-59 ':' is a name char
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //60-69 //stop on every character except a letter or number
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
	0, 1, 1, 1, 1, 0, 1, 0, 0, 0, //90-99 '_' is a name char
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, //120-129
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //130-139
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //140-149
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //150-159
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //160-169
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //170-179
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //180-189
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //190-199
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //200-209
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //210-219
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //220-229
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //230-239
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //240-249
	1, 1, 1, 1, 1, 1             //250-255
};

XMLTag::XMLTag() :
	fTag(NULL),
	fValue(NULL),
	fElem(NULL)
{
	fElem = this;
}

XMLTag::XMLTag(char* tagName) :
	fTag(NULL),
	fValue(NULL),
	fElem(NULL)
{
	fElem = this;
	StrPtrLen temp(tagName);
	fTag = temp.GetAsCString();
}

XMLTag::~XMLTag()
{
	if (fTag)
		delete[] fTag;
	if (fValue)
		delete[] fValue;

	OSQueueElem* elem;
	while ((elem = fAttributes.DeQueue()) != NULL)
	{
		XMLAttribute* attr = (XMLAttribute*)elem->GetEnclosingObject();
		delete attr;
	}

	while ((elem = fEmbeddedTags.DeQueue()) != NULL)
	{
		XMLTag* tag = (XMLTag*)elem->GetEnclosingObject();
		delete tag;
	}

	if (fElem.IsMemberOfAnyQueue())
		fElem.InQueue()->Remove(&fElem);    // remove from parent tag
}

void XMLTag::ConsumeIfComment(StringParser* parser)
{
	if ((parser->GetDataRemaining() > 2) && ((*parser)[1] == '-') && ((*parser)[2] == '-'))
	{
		// this is a comment, so skip to end of comment
		parser->ConsumeLength(NULL, 2); // skip '--'

		// look for -->
		while ((parser->GetDataRemaining() > 2) && ((parser->PeekFast() != '-') ||
			((*parser)[1] != '-') || ((*parser)[2] != '>')))
		{
			if (parser->PeekFast() == '-') parser->ConsumeLength(NULL, 1);
			parser->ConsumeUntil(NULL, '-');
		}

		if (parser->GetDataRemaining() > 2) parser->ConsumeLength(NULL, 3); // consume -->
	}
}

bool XMLTag::ParseTag(StringParser* parser, DTDVerifier* verifier, char* errorBuffer, int errorBufferSize)
{
	while (true)
	{
		if (!parser->GetThru(NULL, '<'))
		{
			if (errorBuffer != NULL)
				qtss_snprintf(errorBuffer, errorBufferSize, "Couldn't find a valid tag");
			return false;   // couldn't find beginning of tag
		}

		char c = parser->PeekFast();
		if (c == '/')
		{
			if (errorBuffer != NULL)
				qtss_snprintf(errorBuffer, errorBufferSize, "End tag with no begin tag on line %d", parser->GetCurrentLineNumber());
			return false;   // we shouldn't be seeing a close tag here
		}

		if ((c != '!') && (c != '?'))
			break;  // this should be the beginning of a regular tag

		ConsumeIfComment(parser);
		// otherwise this is a processing instruction or a c-data, so look for the next tag
	}

	int tagStartLine = parser->GetCurrentLineNumber();

	StrPtrLen temp;
	parser->ConsumeUntil(&temp, sNonNameMask);
	if (temp.Len == 0)
	{
		if (errorBuffer != NULL)
		{
			if (parser->GetDataRemaining() == 0)
				qtss_snprintf(errorBuffer, errorBufferSize, "Unexpected end of file on line %d", parser->GetCurrentLineNumber());
			else
				qtss_snprintf(errorBuffer, errorBufferSize, "Unexpected character (%c) on line %d", parser->PeekFast(), parser->GetCurrentLineNumber());
		}
		return false;   // bad file
	}

	fTag = temp.GetAsCString();

	parser->ConsumeWhitespace();
	while ((parser->PeekFast() != '>') && (parser->PeekFast() != '/'))
	{
		// we must have an attribute value for this tag
		XMLAttribute* attr = new XMLAttribute;
		fAttributes.EnQueue(&attr->fElem);
		parser->ConsumeUntil(&temp, sNonNameMask);
		if (temp.Len == 0)
		{
			if (errorBuffer != NULL)
			{
				if (parser->GetDataRemaining() == 0)
					qtss_snprintf(errorBuffer, errorBufferSize, "Unexpected end of file on line %d", parser->GetCurrentLineNumber());
				else
					qtss_snprintf(errorBuffer, errorBufferSize, "Unexpected character (%c) on line %d", parser->PeekFast(), parser->GetCurrentLineNumber());
			}
			return false;   // bad file
		}

		attr->fAttrName = temp.GetAsCString();

		if (!parser->Expect('='))
		{
			if (errorBuffer != NULL)
				qtss_snprintf(errorBuffer, errorBufferSize, "Missing '=' after attribute %s on line %d", attr->fAttrName, parser->GetCurrentLineNumber());
			return false;   // bad attribute specification
		}
		if (!parser->Expect('"'))
		{
			if (errorBuffer != NULL)
				qtss_snprintf(errorBuffer, errorBufferSize, "Attribute %s value not in quotes on line %d", attr->fAttrName, parser->GetCurrentLineNumber());
			return false;   // bad attribute specification
		}

		parser->ConsumeUntil(&temp, '"');
		attr->fAttrValue = temp.GetAsCString();
		if (!parser->Expect('"'))
		{
			if (errorBuffer != NULL)
				qtss_snprintf(errorBuffer, errorBufferSize, "Attribute %s value not in quotes on line %d", attr->fAttrName, parser->GetCurrentLineNumber());
			return false;   // bad attribute specification
		}

		if (verifier && !verifier->IsValidAttributeName(fTag, attr->fAttrName))
		{
			if (errorBuffer != NULL)
				qtss_snprintf(errorBuffer, errorBufferSize, "Attribute %s not allowed in tag %s on line %d", attr->fAttrName, fTag, parser->GetCurrentLineNumber());
			return false;   // bad attribute specification
		}

		if (verifier && !verifier->IsValidAttributeValue(fTag, attr->fAttrName, attr->fAttrValue))
		{
			if (errorBuffer != NULL)
				qtss_snprintf(errorBuffer, errorBufferSize, "Bad value for attribute %s on line %d", attr->fAttrName, parser->GetCurrentLineNumber());
			return false;   // bad attribute specification
		}

		parser->ConsumeWhitespace();
	}

	if (parser->PeekFast() == '/')
	{
		// this is an empty element tag, i.e. no contents or end tag (e.g <TAG attr="value" />
		parser->Expect('/');
		if (!parser->Expect('>'))
		{
			if (errorBuffer != NULL)
				qtss_snprintf(errorBuffer, errorBufferSize, "'>' must follow '/' on line %d", parser->GetCurrentLineNumber());
			return false;   // bad attribute specification
		}

		return true;    // we're done with this tag
	}

	if (!parser->Expect('>'))
	{
		if (errorBuffer != NULL)
			qtss_snprintf(errorBuffer, errorBufferSize, "Bad format for tag <%s> on line %d", fTag, parser->GetCurrentLineNumber());
		return false;   // bad attribute specification
	}

	while (true)
	{
		parser->ConsumeUntil(&temp, '<');   // this is either value or whitespace
		if (parser->GetDataRemaining() < 4)
		{
			if (errorBuffer != NULL)
				qtss_snprintf(errorBuffer, errorBufferSize, "Reached end of file without end for tag <%s> declared on line %d", fTag, tagStartLine);
			return false;
		}
		if ((*parser)[1] == '/')
		{
			// we'll only assign a value if there were no embedded tags
			if (fEmbeddedTags.GetLength() == 0 && (!verifier || verifier->CanHaveValue(fTag)))
				fValue = temp.GetAsCString();
			else
			{
				// otherwise this needs to have been just whitespace
				StringParser tempParser(&temp);
				tempParser.ConsumeWhitespace();
				if (tempParser.GetDataRemaining() > 0)
				{
					if (errorBuffer)
					{
						if (fEmbeddedTags.GetLength() > 0)
							qtss_snprintf(errorBuffer, errorBufferSize, "Unexpected text outside of tag on line %d", tagStartLine);
						else
							qtss_snprintf(errorBuffer, errorBufferSize, "Tag <%s> on line %d not allowed to have data", fTag, tagStartLine);
					}
				}
			}
			break;  // we're all done with this tag
		}

		if (((*parser)[1] != '!') && ((*parser)[1] != '?'))
		{
			// this must be the beginning of an embedded tag
			XMLTag* tag = NEW XMLTag();
			fEmbeddedTags.EnQueue(&tag->fElem);
			if (!tag->ParseTag(parser, verifier, errorBuffer, errorBufferSize))
				return false;

			if (verifier && !verifier->IsValidSubtag(fTag, tag->GetTagName()))
			{
				if (errorBuffer != NULL)
					qtss_snprintf(errorBuffer, errorBufferSize, "Tag %s not allowed in tag %s on line %d", tag->GetTagName(), fTag, parser->GetCurrentLineNumber());
				return false;   // bad attribute specification
			}
		}
		else
		{
			parser->ConsumeLength(NULL, 1); // skip '<'
			ConsumeIfComment(parser);
		}
	}

	parser->ConsumeLength(NULL, 2); // skip '</'
	parser->ConsumeUntil(&temp, sNonNameMask);
	if (!temp.Equal(fTag))
	{
		char* newTag = temp.GetAsCString();
		if (errorBuffer != NULL)
			qtss_snprintf(errorBuffer, errorBufferSize, "End tag </%s> on line %d doesn't match tag <%s> declared on line %d", newTag, parser->GetCurrentLineNumber(), fTag, tagStartLine);
		delete newTag;
		return false;   // bad attribute specification
	}

	if (!parser->GetThru(NULL, '>'))
	{
		if (errorBuffer != NULL)
			qtss_snprintf(errorBuffer, errorBufferSize, "Couldn't find end of tag <%s> declared on line %d", fTag, tagStartLine);
		return false;   // bad attribute specification
	}

	return true;
}

char* XMLTag::GetAttributeValue(const char* attrName)
{
	for (OSQueueIter iter(&fAttributes); !iter.IsDone(); iter.Next())
	{
		XMLAttribute* attr = (XMLAttribute*)iter.GetCurrent()->GetEnclosingObject();
		if (!strcmp(attr->fAttrName, attrName))
			return attr->fAttrValue;
	}

	return NULL;
}

XMLTag* XMLTag::GetEmbeddedTag(const UInt32 index)
{
	if (fEmbeddedTags.GetLength() <= index)
		return NULL;

	OSQueueIter iter(&fEmbeddedTags);
	for (UInt32 i = 0; i < index; i++)
	{
		iter.Next();
	}
	OSQueueElem* result = iter.GetCurrent();

	return (XMLTag*)result->GetEnclosingObject();
}

XMLTag* XMLTag::GetEmbeddedTagByName(const char* tagName, const UInt32 index)
{
	if (fEmbeddedTags.GetLength() <= index)
		return NULL;

	XMLTag* result = NULL;
	UInt32 curIndex = 0;
	for (OSQueueIter iter(&fEmbeddedTags); !iter.IsDone(); iter.Next())
	{
		XMLTag* temp = (XMLTag*)iter.GetCurrent()->GetEnclosingObject();
		if (!strcmp(temp->GetTagName(), tagName))
		{
			if (curIndex == index)
			{
				result = temp;
				break;
			}

			curIndex++;
		}
	}

	return result;
}

XMLTag* XMLTag::GetEmbeddedTagByAttr(const char* attrName, const char* attrValue, const UInt32 index)
{
	if (fEmbeddedTags.GetLength() <= index)
		return NULL;

	XMLTag* result = NULL;
	UInt32 curIndex = 0;
	for (OSQueueIter iter(&fEmbeddedTags); !iter.IsDone(); iter.Next())
	{
		XMLTag* temp = (XMLTag*)iter.GetCurrent()->GetEnclosingObject();
		if ((temp->GetAttributeValue(attrName) != NULL) && (!strcmp(temp->GetAttributeValue(attrName), attrValue)))
		{
			if (curIndex == index)
			{
				result = temp;
				break;
			}

			curIndex++;
		}
	}

	return result;
}

XMLTag* XMLTag::GetEmbeddedTagByNameAndAttr(const char* tagName, const char* attrName, const char* attrValue, const UInt32 index)
{
	if (fEmbeddedTags.GetLength() <= index)
		return NULL;

	XMLTag* result = NULL;
	UInt32 curIndex = 0;
	for (OSQueueIter iter(&fEmbeddedTags); !iter.IsDone(); iter.Next())
	{
		XMLTag* temp = (XMLTag*)iter.GetCurrent()->GetEnclosingObject();
		if (!strcmp(temp->GetTagName(), tagName) && (temp->GetAttributeValue(attrName) != NULL) &&
			(!strcmp(temp->GetAttributeValue(attrName), attrValue)))
		{
			if (curIndex == index)
			{
				result = temp;
				break;
			}

			curIndex++;
		}
	}

	return result;
}

void XMLTag::AddAttribute(char* attrName, char* attrValue)
{
	XMLAttribute* attr = NEW XMLAttribute;
	StrPtrLen temp(attrName);
	attr->fAttrName = temp.GetAsCString();
	temp.Set(attrValue);
	attr->fAttrValue = temp.GetAsCString();

	fAttributes.EnQueue(&attr->fElem);
}

void XMLTag::RemoveAttribute(char* attrName)
{
	for (OSQueueIter iter(&fAttributes); !iter.IsDone(); iter.Next())
	{
		XMLAttribute* attr = (XMLAttribute*)iter.GetCurrent()->GetEnclosingObject();
		if (!strcmp(attr->fAttrName, attrName))
		{
			fAttributes.Remove(&attr->fElem);
			delete attr;
			return;
		}
	}
}

void XMLTag::AddEmbeddedTag(XMLTag* tag)
{
	fEmbeddedTags.EnQueue(&tag->fElem);
}

void XMLTag::RemoveEmbeddedTag(XMLTag* tag)
{
	fEmbeddedTags.Remove(&tag->fElem);
}

void XMLTag::SetTagName(char* name)
{
	Assert(name != NULL);  // can't have a tag without a name!

	if (fTag != NULL)
		delete fTag;

	StrPtrLen temp(name);
	fTag = temp.GetAsCString();
}

void XMLTag::SetValue(char* value)
{
	if (fEmbeddedTags.GetLength() > 0)
		return;     // can't have a value with embedded tags

	if (fValue != NULL)
		delete fValue;

	if (value == NULL)
		fValue = NULL;
	else
	{
		StrPtrLen temp(value);
		fValue = temp.GetAsCString();
	}
}

void XMLTag::FormatData(ResizeableStringFormatter* formatter, UInt32 indent)
{
	for (UInt32 i = 0; i < indent; i++) formatter->PutChar('\t');

	formatter->PutChar('<');
	formatter->Put(fTag);
	if (fAttributes.GetLength() > 0)
	{
		formatter->PutChar(' ');
		for (OSQueueIter iter(&fAttributes); !iter.IsDone(); iter.Next())
		{
			XMLAttribute* attr = (XMLAttribute*)iter.GetCurrent()->GetEnclosingObject();
			formatter->Put(attr->fAttrName);
			formatter->Put("=\"");
			formatter->Put(attr->fAttrValue);
			formatter->Put("\" ");
		}
	}
	formatter->PutChar('>');

	if (fEmbeddedTags.GetLength() == 0)
	{
		if (fValue > 0)
			formatter->Put(fValue);
	}
	else
	{
		formatter->Put(kEOLString);
		for (OSQueueIter iter(&fEmbeddedTags); !iter.IsDone(); iter.Next())
		{
			XMLTag* current = (XMLTag*)iter.GetCurrent()->GetEnclosingObject();
			current->FormatData(formatter, indent + 1);
		}

		for (UInt32 i = 0; i < indent; i++) formatter->PutChar('\t');
	}

	formatter->Put("</");
	formatter->Put(fTag);
	formatter->PutChar('>');
	formatter->Put(kEOLString);
}

XMLAttribute::XMLAttribute()
	: fAttrName(NULL),
	fAttrValue(NULL)
{
	fElem = this;
}

XMLAttribute::~XMLAttribute()
{
	if (fAttrName)
		delete[] fAttrName;
	if (fAttrValue)
		delete[] fAttrValue;
}
