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
	 File:       StrPtrLen.cpp

	 Contains:   Implementation of class defined in StrPtrLen.h.




 */


#include <ctype.h>
#include "StrPtrLen.h"
#include "MyAssert.h"
#include "OS.h"
#include "OSMemory.h"


UInt8 StrPtrLen::sCaseInsensitiveMask[] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //0-9 
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, //10-19 
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, //20-29
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39, //30-39 
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49, //40-49
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59, //50-59
	60, 61, 62, 63, 64, 97, 98, 99, 100, 101, //60-69 //stop on every character except a letter
	102, 103, 104, 105, 106, 107, 108, 109, 110, 111, //70-79
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, //80-89
	122, 91, 92, 93, 94, 95, 96, 97, 98, 99, //90-99
	100, 101, 102, 103, 104, 105, 106, 107, 108, 109, //100-109
	110, 111, 112, 113, 114, 115, 116, 117, 118, 119, //110-119
	120, 121, 122, 123, 124, 125, 126, 127, 128, 129 //120-129
};

UInt8 StrPtrLen::sNonPrintChars[] =
{
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0-9     // stop
	0, 1, 1, 0, 1, 1, 1, 1, 1, 1, //10-19    //'\r' & '\n' are not stop conditions
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //20-29
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, //30-39   
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40-49
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //60-69  
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, //170-179
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //180-189
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //190-199
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //200-209
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //210-219
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //220-229
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //230-239
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //240-249
	1, 1, 1, 1, 1, 1             //250-255
};

char* StrPtrLen::GetAsCString() const
{
	// convert to a "NEW'd" zero terminated char array
	// caler is reponsible for the newly allocated memory
	char *theString = NEW char[Len + 1];

	if (Ptr && Len > 0)
		::memcpy(theString, Ptr, Len);

	theString[Len] = 0;

	return theString;
}


bool StrPtrLen::Equal(const StrPtrLen& compare) const
{
	if (NULL == compare.Ptr && NULL == Ptr)
		return true;

	if ((NULL == compare.Ptr) || (NULL == Ptr))
		return false;

	if ((compare.Len == Len) && (memcmp(compare.Ptr, Ptr, Len) == 0))
		return true;
	else
		return false;
}

bool StrPtrLen::Equal(const char* compare) const
{
	if (NULL == compare && NULL == Ptr)
		return true;

	if ((NULL == compare) || (NULL == Ptr))
		return false;

	if ((::strlen(compare) == Len) && (memcmp(compare, Ptr, Len) == 0))
		return true;
	else
		return false;
}



bool StrPtrLen::NumEqualIgnoreCase(const char* compare, const UInt32 len) const
{
	// compare thru the first "len: bytes
	Assert(compare != NULL);

	if (len <= Len)
	{
		for (UInt32 x = 0; x < len; x++)
			if (sCaseInsensitiveMask[(UInt8)Ptr[x]] != sCaseInsensitiveMask[(UInt8)compare[x]])
				return false;
		return true;
	}
	return false;
}

bool StrPtrLen::EqualIgnoreCase(const char* compare, const UInt32 len) const
{
	Assert(compare != NULL);
	if (len == Len)
	{
		for (UInt32 x = 0; x < len; x++)
			if (sCaseInsensitiveMask[(UInt8)Ptr[x]] != sCaseInsensitiveMask[(UInt8)compare[x]])
				return false;
		return true;
	}
	return false;
}

char* StrPtrLen::FindStringCase(char* queryCharStr, StrPtrLen* resultStr, bool caseSensitive) const
{
	// Be careful about exiting this method from the middle. This routine deletes allocated memory at the end.
	// 

	if (resultStr)
		resultStr->Set(NULL, 0);

	Assert(NULL != queryCharStr);
	if (NULL == queryCharStr) return NULL;
	if (NULL == Ptr) return NULL;
	if (0 == Len) return NULL;


	StrPtrLen queryStr(queryCharStr);
	char *editSource = NULL;
	char *resultChar = NULL;
	char lastSourceChar = Ptr[Len - 1];

	if (lastSourceChar != 0) // need to modify for termination. 
	{
		editSource = NEW char[Len + 1]; // Ptr could be a static string so make a copy
		::memcpy(editSource, Ptr, Len);
		editSource[Len] = 0; // this won't work on static strings so we are modifing a new string here
	}

	char *queryString = queryCharStr;
	char *dupSourceString = NULL;
	char *dupQueryString = NULL;
	char *sourceString = Ptr;
	UInt32 foundLen = 0;

	if (editSource != NULL) // a copy of the source ptr and len 0 terminated
		sourceString = editSource;

	if (!caseSensitive)
	{
		dupSourceString = ::strdup(sourceString);
		dupQueryString = ::strdup(queryCharStr);
		if (dupSourceString && dupQueryString)
		{
			sourceString = StrPtrLen(dupSourceString).ToUpper();
			queryString = StrPtrLen(dupQueryString).ToUpper();
			resultChar = ::strstr(sourceString, queryString);

			::free(dupSourceString);
			::free(dupQueryString);
		}
	}
	else
	{
		resultChar = ::strstr(sourceString, queryString);
	}

	if (resultChar != NULL) // get the start offset
	{
		foundLen = resultChar - sourceString;
		resultChar = Ptr + foundLen;  // return a pointer in the source buffer
		if (resultChar > (Ptr + Len)) // make sure it is in the buffer
			resultChar = NULL;
	}

	if (editSource != NULL)
		delete[] editSource;

	if (resultStr != NULL && resultChar != NULL)
		resultStr->Set(resultChar, queryStr.Len);

#if STRPTRLENTESTING    
	qtss_printf("StrPtrLen::FindStringCase found string=%s\n", resultChar);
#endif

	return resultChar;
}


UInt32 StrPtrLen::RemoveWhitespace()
{
	if (Ptr == NULL || Len == 0)
		return 0;

	char *EndPtr = Ptr + Len; // one past last char
	char *destPtr = Ptr;
	char *srcPtr = Ptr;

	Len = 0;
	while (srcPtr < EndPtr)
	{

		if (*srcPtr != ' ' && *srcPtr != '\t')
		{
			if (srcPtr != destPtr)
				*destPtr = *srcPtr;

			destPtr++;
			Len++;
		}
		srcPtr++;
	}

	return Len;
}

UInt32 StrPtrLen::TrimLeadingWhitespace()
{
	if (Ptr == NULL || Len == 0)
		return 0;

	char *EndPtr = Ptr + Len; //one past last char

	while (Ptr < EndPtr)
	{
		if (*Ptr != ' ' && *Ptr != '\t')
			break;

		Ptr += 1;
		Len -= 1;
	}

	return Len;
}

UInt32 StrPtrLen::TrimTrailingWhitespace()
{
	if (Ptr == NULL || Len == 0)
		return 0;

	char *theCharPtr = Ptr + (Len - 1); // last char

	while (theCharPtr >= Ptr)
	{
		if (*theCharPtr != ' ' && *theCharPtr != '\t')
			break;

		theCharPtr -= 1;
		Len -= 1;
	}

	return Len;
}

void StrPtrLen::PrintStr()
{
	char *thestr = GetAsCString();

	UInt32 i = 0;
	for (; i < Len; i++)
	{
		if (StrPtrLen::sNonPrintChars[(UInt8)Ptr[i]])
		{
			thestr[i] = 0;
			break;
		}

	}

	if (thestr != NULL)
	{
		qtss_printf(thestr);
		delete[]thestr;
	}
}

void StrPtrLen::PrintStr(char* appendStr)
{
	StrPtrLen::PrintStr();
	if (appendStr != NULL)
		qtss_printf(appendStr);
}

void StrPtrLen::PrintStr(char* prependStr, char* appendStr)
{
	if (prependStr != NULL)
		qtss_printf(prependStr);

	StrPtrLen::PrintStr();

	if (appendStr != NULL)
		qtss_printf(appendStr);
}


void StrPtrLen::PrintStrEOL(char* stopStr, char* appendStr)
{


	char *thestr = GetAsCString();

	SInt32 i = 0;
	for (; i < (SInt32)Len; i++)
	{
		if (StrPtrLen::sNonPrintChars[(UInt8)Ptr[i]])
		{
			thestr[i] = 0;
			break;
		}

	}

	for (i = 0; thestr[i] != 0; i++)
	{
		if (thestr[i] == '%' && thestr[i + 1] != '%')
		{
			thestr[i] = '$';
		}
	}

	SInt32 stopLen = 0;
	if (stopStr != NULL)
		stopLen = ::strlen(stopStr);

	if (stopLen > 0 && stopLen <= i)
	{
		char* stopPtr = ::strstr(thestr, stopStr);
		if (stopPtr != NULL)
		{
			stopPtr += stopLen;
			*stopPtr = 0;
			i = stopPtr - thestr;
		}
	}

	char * theStrLine = thestr;
	char * nextLine = NULL;
	char * theChar = NULL;
	static char *cr = "\\r";
	static char *lf = "\\n\n";
	SInt32 tempLen = i;
	for (i = 0; i < tempLen; i++)
	{
		if (theStrLine[i] == '\r')
		{
			theChar = cr;
			theStrLine[i] = 0;
			nextLine = &theStrLine[i + 1];
		}
		else if (theStrLine[i] == '\n')
		{
			theChar = lf;
			theStrLine[i] = 0;
			nextLine = &theStrLine[i + 1];
		}

		if (nextLine != NULL)
		{
			qtss_printf(theStrLine);
			qtss_printf(theChar);

			theStrLine = nextLine;
			nextLine = NULL;
			tempLen -= (i + 1);
			i = -1;
		}
	}
	qtss_printf(theStrLine);
	delete[]thestr;

	if (appendStr != NULL)
		qtss_printf(appendStr);

}




#if STRPTRLENTESTING
bool  StrPtrLen::Test()
{
	static char* test1 = "2347.;.][';[;]abcdefghijklmnopqrstuvwxyz#%#$$#";
	static char* test2 = "2347.;.][';[;]ABCDEFGHIJKLMNOPQRSTUVWXYZ#%#$$#";
	static char* test3 = "Content-Type:";
	static char* test4 = "cONTent-TYPe:";
	static char* test5 = "cONTnnt-TYPe:";
	static char* test6 = "cONTent-TY";

	static char* test7 = "ontent-Type:";
	static char* test8 = "ONTent-TYPe:";
	static char* test9 = "-TYPe:";
	static char* test10 = ":";

	StrPtrLen theVictim1(test1, strlen(test1));
	if (!theVictim1.EqualIgnoreCase(test2, strlen(test2)))
		return false;

	if (theVictim1.EqualIgnoreCase(test3, strlen(test3)))
		return false;
	if (!theVictim1.EqualIgnoreCase(test1, strlen(test1)))
		return false;

	StrPtrLen theVictim2(test3, strlen(test3));
	if (!theVictim2.EqualIgnoreCase(test4, strlen(test4)))
		return false;
	if (theVictim2.EqualIgnoreCase(test5, strlen(test5)))
		return false;
	if (theVictim2.EqualIgnoreCase(test6, strlen(test6)))
		return false;

	StrPtrLen outResultStr;
	if (!theVictim1.FindStringIgnoreCase(test2, &outResultStr))
		return false;
	if (theVictim1.FindStringIgnoreCase(test3, &outResultStr))
		return false;
	if (!theVictim1.FindStringIgnoreCase(test1, &outResultStr))
		return false;
	if (!theVictim2.FindStringIgnoreCase(test4))
		return false;
	if (theVictim2.FindStringIgnoreCase(test5))
		return false;
	if (!theVictim2.FindStringIgnoreCase(test6))
		return false;
	if (!theVictim2.FindStringIgnoreCase(test7))
		return false;
	if (!theVictim2.FindStringIgnoreCase(test8))
		return false;
	if (!theVictim2.FindStringIgnoreCase(test9))
		return false;
	if (!theVictim2.FindStringIgnoreCase(test10))
		return false;

	if (theVictim1.FindString(test2, &outResultStr))
		return false;
	if (theVictim1.FindString(test3, &outResultStr))
		return false;
	if (!theVictim1.FindString(test1, &outResultStr))
		return false;
	if (theVictim2.FindString(test4))
		return false;
	if (theVictim2.FindString(test5))
		return false;
	if (theVictim2.FindString(test6))
		return false;
	if (!theVictim2.FindString(test7))
		return false;
	if (theVictim2.FindString(test8))
		return false;
	if (theVictim2.FindString(test9))
		return false;
	if (!theVictim2.FindString(test10))
		return false;

	StrPtrLen query;
	query.Set(test2);
	if (theVictim1.FindString(query, &outResultStr))
		return false;
	if (outResultStr.Len > 0)
		return false;
	if (outResultStr.Ptr != NULL)
		return false;

	query.Set(test3);
	if (theVictim1.FindString(query, &outResultStr))
		return false;
	if (outResultStr.Len > 0)
		return false;
	if (outResultStr.Ptr != NULL)
		return false;

	query.Set(test1);
	if (!theVictim1.FindString(query, &outResultStr))
		return false;
	if (!outResultStr.Equal(query))
		return false;

	query.Set(test4);
	if (query.Equal(theVictim2.FindString(query)))
		return false;

	query.Set(test5);
	if (query.Equal(theVictim2.FindString(query)))
		return false;

	query.Set(test6);
	if (query.Equal(theVictim2.FindString(query)))
		return false;

	query.Set(test7);
	if (!query.Equal(theVictim2.FindString(query)))
		return false;

	query.Set(test8);
	if (query.Equal(theVictim2.FindString(query)))
		return false;

	query.Set(test9);
	if (query.Equal(theVictim2.FindString(query)))
		return false;

	query.Set(test10);
	if (!query.Equal(theVictim2.FindString(query)))
		return false;

	query.Set(test10);
	if (!query.Equal(theVictim2.FindString(query)))
		return false;

	StrPtrLen partialStaticSource(test1, 5);
	query.Set("abcd");
	if (query.Equal(partialStaticSource.FindString(query)))
		return false;

	query.Set("47");
	if (query.Equal(partialStaticSource.FindString(query))) // success = !equal because the char str is longer than len
		return false;

	if (query.FindString(partialStaticSource.FindString(query))) // success = !found because the 0 term src is not in query
		return false;

	partialStaticSource.FindString(query, &outResultStr);
	if (!outResultStr.Equal(query)) // success =found the result Ptr and Len is the same as the query
		return false;

	return true;
}
#endif
