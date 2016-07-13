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
	File:       QueryParamList.cpp

	Contains:   Implementation of QueryParamList class

	The QueryParamList class is used to parse and build a searchable list
	of name/value pairs from a RFC1808 QueryString that has been encoded
	using the html 'form encoding' rules.

*/

#ifndef __query_param_list__
#define __query_param_list__


//#include "QueryParamList.h"

#include "PLDoubleLinkedList.h"
#include "StrPtrLen.h"


class QueryParamListElement {

public:
	QueryParamListElement(char* name, char* value)
	{
		mName = name;
		mValue = value;

	}

	virtual ~QueryParamListElement()
	{
		delete[] mName;
		delete[] mValue;
	}

	char    *mName;
	char    *mValue;

};


class QueryParamList
{
public:
	QueryParamList(char* queryString);
	QueryParamList(StrPtrLen* querySPL);
	~QueryParamList() { delete fNameValueQueryParamlist; }

	void AddNameValuePairToList(char* name, char* value);
	const char *DoFindCGIValueForParam(char* name);
	void PrintAll(char* idString);

protected:
	void            BulidList(StrPtrLen* querySPL);
	void            DecodeArg(char* ioCodedPtr);
	enum {
		// escaping states
		kLastWasText
		, kPctEscape
		, kRcvHexDigitOne
	};

	Bool16          IsHex(char c);

	PLDoubleLinkedList<QueryParamListElement>* fNameValueQueryParamlist;


};





#endif
