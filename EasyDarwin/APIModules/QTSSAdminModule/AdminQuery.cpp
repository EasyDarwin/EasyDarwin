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
	 File:       AdminQuery.cpp

	 Contains:   Implements Admin Query



 */

#ifndef __Win32__
#include <unistd.h>     /* for getopt() et al */
#endif

#include <time.h>
#include <stdio.h>      /* for //qtss_printf */
#include <stdlib.h>     /* for getloadavg & other useful stuff */
#include "QTSSAdminModule.h"
#include "OSArrayObjectDeleter.h"
#include "StringParser.h"
#include "StrPtrLen.h"
#include "OSMutex.h"
#include "OSRef.h"
#include "AdminQuery.h"
#include "StringTranslator.h"

 /*
 r = recurse -> walk downward in hierarchy
 v = verbose -> return full path in name
 a = access -> return read/write access
 t = type -> return type of value ** perhaps better not to support
 f = filter -> return filter
 p = path -> return path
 i = indexed -> return indexed representation of attributes
 d = debug -> return debug info on errors
 */



StrPtrLen * QueryURI::NextSegment(StrPtrLen *currentPathPtr, StrPtrLen *outNextPtr)
{
	StrPtrLen *result = NULL;
	StrPtrLen *theURLPtr = GetURL();
	if (outNextPtr)
		outNextPtr->Set(NULL, 0);

	if (currentPathPtr && outNextPtr && theURLPtr && currentPathPtr->Len > 0)
	{
		if ((currentPathPtr->Ptr >= theURLPtr->Ptr)
			&& (currentPathPtr->Ptr < &theURLPtr->Ptr[theURLPtr->Len])
			)
		{
			//qtss_printf("theURLPtr="); PRINT_STR(theURLPtr);
			//qtss_printf("QueryURI::NextSegment currentPathPtr="); PRINT_STR(currentPathPtr);

			UInt32 len = (PointerSizedInt)&(theURLPtr->Ptr[theURLPtr->Len - 1]) - ((PointerSizedInt)currentPathPtr->Ptr + (currentPathPtr->Len - 1));
			char *startPtr = (char *)((PointerSizedInt)currentPathPtr->Ptr + currentPathPtr->Len);
			StrPtrLen tempPath(startPtr, len);


			StringParser URLparser(&tempPath);
			URLparser.ConsumeLength(NULL, 1);
			URLparser.ConsumeUntil(outNextPtr, (UInt8*)sNotQueryData);
			result = outNextPtr;

			//qtss_printf("QueryURI::NextSegment nextPathPtr=");PRINT_STR(outNextPtr);
		}

	}

	return result;
};


QueryURI::URIField QueryURI::sURIFields[] =
{   /* fAttrName, len, id, fDataptr*/
	{ "modules",    strlen("modules"),      eModuleID,  NULL    },
	{ "admin",      strlen("admin"),        eRootID,    NULL    },
	{ "URL",        strlen("URL"),          eURL,       NULL    },
	{ "QUERY",      strlen("QUERY"),        eQuery,     NULL    },
	{ "parameters", strlen("parameters"),   eParameters,NULL    },
	{ "snapshot",   strlen("snapshot"),     eSnapshot,  NULL    },
	{ "command",    strlen("command"),      eCommand,   NULL    },
	{ "value",      strlen("value"),        eValue,     NULL    },
	{ "type",       strlen("type"),         eType,      NULL    },
	{ "access",     strlen("access"),       eAccess,    NULL    },
	{ "name",       strlen("name"),         eName,      NULL    },
	{ "filter1",    strlen("filter1"),      eFilter1,   NULL    },
	{ "filter2",    strlen("filter2"),      eFilter2,   NULL    },
	{ "filter3",    strlen("filter3"),      eFilter3,   NULL    },
	{ "filter4",    strlen("filter4"),      eFilter4,   NULL    },
	{ "filter5",    strlen("filter5"),      eFilter5,   NULL    },
	{ "filter6",    strlen("filter6"),      eFilter6,   NULL    },
	{ "filter7",    strlen("filter7"),      eFilter7,   NULL    },
	{ "filter8",    strlen("filter8"),      eFilter8,   NULL    },
	{ "filter9",    strlen("filter9"),      eFilter9,   NULL    },
	{ "filter10",   strlen("filter10"),     eFilter10,  NULL    },
	{ "",   0,      -1,     NULL    }
};

char *QueryURI::sCommandDefs[] =
{
	"GET",
	"SET",
	"ADD",
	"DEL"
};


UInt8 QueryURI::sNotQueryData[] = // query stops
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0-9     
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //10-19    
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //20-29
	1, 1, 1, 1, 0, 1, 1, 1, 1, 1, //30-39   
	1, 1, 0, 1, 1, 0, 0, 1, 0, 0, //40-49   allow * . and - and all numbers
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //50-59  allow :
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //60-69 //stop on every character except a letter
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
	0, 1, 1, 1, 1, 0, 1, 0, 0, 0, //90-99 _ is a word
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
	1, 1, 1, 1, 1, 0             //250-255
};


UInt8 QueryURI::sWhiteQuoteOrEOL[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //0-9     // \t is a stop
	1, 0, 0, 1, 0, 0, 0, 0, 0, 0, //10-19    //'\r' & '\n' are stop conditions
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
	0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //30-39   ' ' , '"' is a stop
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
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
	0, 0, 0, 0, 0, 0             //250-255
};

UInt8 QueryURI::sWhitespaceOrQuoteMask[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //0-9     // \t is a stop
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //10-19   
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
	0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //30-39   ' ' , '"' is a stop
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
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
	0, 0, 0, 0, 0, 0             //250-255
};


QueryURI::QueryURI(StrPtrLen *inStream)
{
	fIsAdminQuery = false;
	fUseSnapShot = false;
	fURIFieldsPtr = &sURIFields[0];
	fTheCommand = -1;

	for (short testField = 0; testField < eNumAttributes; testField++)
		fURIFieldsPtr[testField].fData = NULL;

	memset(fURIBuffer, 0, QueryURI::eMaxBufferSize);
	memset(fQueryBuffer, 0, QueryURI::eMaxBufferSize);
	fParamBits = 0;
	fSnapshotID = 0;
	fAccessFlags = 0;
	fQueryHasResponse = false;
	fLastPath[0] = 0;
	fIsPref = false;
	fNumFilters = 0;
	fHasQuery = false;
	URLParse(inStream);
	SetQueryData();

}


void QueryURI::SetSnapShot()
{
	StrPtrLen *snapshotSPL = GetSnapshot();
	if (snapshotSPL != NULL)
	{
		StringParser parser(snapshotSPL);
		fSnapshotID = parser.ConsumeInteger(NULL);
		fUseSnapShot = true;
	}
};

void QueryURI::SetCommand()
{
	StrPtrLen commandDef;
	StrPtrLen *queryCommandPtr;
	SInt16 commandIndex;

	queryCommandPtr = this->GetCommand();
	if (queryCommandPtr == NULL || queryCommandPtr->Len == 0) // Default command
	{
		fTheCommand = kGETCommand;
		return;
	}

	for (commandIndex = 0; commandIndex < kLastCommand; commandIndex++)
	{
		bool foundCommand = queryCommandPtr->EqualIgnoreCase(sCommandDefs[commandIndex], strlen(sCommandDefs[commandIndex]));
		if (foundCommand)
		{
			fTheCommand = commandIndex;
		}
	}
}

void QueryURI::SetAccessFlags()
{

	StrPtrLen tempStr;
	SInt16 theChar;
	StringParser parser(GetAccess());
	fAccessFlags = 0;
	while (parser.GetDataRemaining() != 0)
	{
		parser.ConsumeLength(&tempStr, 1);
		if (tempStr.Len > 0)
		{
			theChar = *tempStr.Ptr;
			switch (theChar)
			{
			case 'r': fAccessFlags |= qtssAttrModeRead;
				break;

			case 'w': fAccessFlags |= qtssAttrModeWrite;
				break;

			case 'p': fAccessFlags |= qtssAttrModePreempSafe;
				break;

				//case 'd': fAccessFlags |= qtssAttrModeRemoveable;
				//break;
			}

		}
		tempStr.Len = 0;
	}
}

void QueryURI::SetParamBits(UInt32 forcebits)
{
	StrPtrLen tempStr;
	SInt16 theChar;

	StringParser parser(GetParameters());
	fParamBits = forcebits;
	while (parser.GetDataRemaining() != 0)
	{
		parser.ConsumeLength(&tempStr, 1);
		if (tempStr.Len > 0)
		{
			theChar = *tempStr.Ptr;
			switch (theChar)
			{
			case 'r': fParamBits |= kRecurseParam;
				break;

			case 'v': fParamBits |= kVerboseParam;
				break;

			case 'a': fParamBits |= kAccessParam;
				break;

			case 't': fParamBits |= kTypeParam;
				break;

			case 'f': fParamBits |= kFilterParam;
				break;

			case 'p': fParamBits |= kPathParam;
				break;

			case 'd': fParamBits |= kDebugParam;
				break;

			case 'i': fParamBits |= kIndexParam;
				break;
			}

		}
		tempStr.Len = 0;
	}
}

QueryURI::~QueryURI()
{
	/*
		for (int count = 0; fURIFieldsPtr[count].fID != -1 ; count++)
		{   //qtss_printf("QueryURI::~QueryURI delete %s=",fURIFieldsPtr[count].fFieldName); PRINT_STR(fURIFieldsPtr[count].fData);
			if (fURIFieldsPtr[count].fData && fURIFieldsPtr[count].fData->Ptr)
				delete fURIFieldsPtr[count].fData->Ptr;
			fURIFieldsPtr[count].fData = NULL;
		}
	*/
}

UInt32 QueryURI::CheckInvalidIterator(char* evalMessageBuff)
{
	UInt32 result = 0;

	StringParser parser(GetURL());
	parser.ConsumeUntil(NULL, '*');
	if (parser.PeekFast() == '*')
	{
		result = 405;
		static char *message = "* iterator not valid";
		qtss_sprintf(evalMessageBuff, "%s", message);
	}

	return result;

}

UInt32 QueryURI::CheckInvalidArrayIterator(char* evalMessageBuff)
{
	UInt32 result = 0;

	StringParser parser(GetURL());
	parser.ConsumeUntil(NULL, ':');
	if (parser.PeekFast() == ':')
	{
		result = 405;
		static char *message = ": array iterator not valid";
		qtss_sprintf(evalMessageBuff, "%s", message);
	}

	return result;

}

UInt32 QueryURI::CheckInvalidRecurseParam(char* evalMessageBuff)
{
	UInt32 result = 0;

	if (RecurseParam())
	{
		result = 405;
		static char *message = "(r)ecurse parameter not valid";
		qtss_sprintf(evalMessageBuff, "%s", message);
	}

	return result;

}


UInt32  QueryURI::EvalQuery(UInt32 *forceResultPtr, char *forceMessagePtr)
{
	UInt32 result = 0;
	const SInt16 messageLen = 512;
	char evalMessageBuff[messageLen] = { 0 };
	StrPtrLen evalMessage;
	evalMessage.Set(evalMessageBuff, messageLen);
	if (forceResultPtr != NULL)
	{
		result = *forceResultPtr;
		switch (*forceResultPtr)
		{
		case 404:
			qtss_sprintf(fQueryMessageBuff, "reason=\"No data found\"");
			fQueryEvalMessage.Set(fQueryMessageBuff, strlen(fQueryMessageBuff));
			break;

		default:
			{   SInt32 theID = GetCommandID();
			StrPtrLen *commandPtr = GetCommand();
			if (theID < 0)
			{
				if (commandPtr)
				{
					qtss_sprintf(fQueryMessageBuff, "reason=\"%s for command %s\"", forceMessagePtr, commandPtr->Ptr);
				}
			}
			else
			{
				qtss_sprintf(fQueryMessageBuff, "reason=\"%s for command %s\"", forceMessagePtr, QueryURI::sCommandDefs[GetCommandID()]);
			}
			}

			fQueryEvalMessage.Set(fQueryMessageBuff, strlen(fQueryMessageBuff));
		}
	}
	else
	{

		switch (GetCommandID())
		{
		case kGETCommand:
			{   // special case test. A query with no parameters is a get. A query with parameters requires a command.
				if (fHasQuery && (this->GetCommand() == NULL || this->GetCommand()->Len == 0))
				{
					result = 400;
					static char *message = "reason=\"command parameter is missing\"";
					qtss_sprintf(evalMessageBuff, "%s", message);
					fQueryEvalMessage.Set(evalMessageBuff, strlen(evalMessageBuff));
					fQueryEvalResult = result;
					return result;
				}
			}
			break;

		case kSETCommand:
			{
				if (NULL == GetValue())
				{
					result = 400;
					static char *message = "No value";
					qtss_sprintf(evalMessageBuff, "%s", message);
					break;
				}

				result = CheckInvalidRecurseParam((char *)evalMessageBuff);
				if (result != 0)
					break;

				result = CheckInvalidIterator((char *)evalMessageBuff);
				if (result != 0)
					break;

				result = CheckInvalidArrayIterator((char *)evalMessageBuff);
				if (result != 0)
					break;

			}
			break;

		case kADDCommand:
			{

				if (0)
				{
					result = 501;
					static char *message = "No implementation";
					qtss_sprintf(evalMessageBuff, "%s", message);
					break;
				}

				if (NULL == GetValue())
				{
					result = 400;
					static char *message = "Attribute value not defined";
					qtss_sprintf(evalMessageBuff, "%s", message);
					break;
				}

				result = CheckInvalidRecurseParam((char *)evalMessageBuff);
				if (result != 0)
					break;

				result = CheckInvalidIterator((char *)evalMessageBuff);
				if (result != 0)
					break;

				result = CheckInvalidArrayIterator((char *)evalMessageBuff);
				if (result != 0)
					break;

			}
			break;

		case kDELCommand:
			{

				result = CheckInvalidRecurseParam((char *)evalMessageBuff);
				if (result != 0)
					break;

				result = CheckInvalidIterator((char *)evalMessageBuff);
				if (result != 0)
					break;

				result = CheckInvalidArrayIterator((char *)evalMessageBuff);
				if (result != 0)
					break;

			}
			break;

		default:
			{
				result = 501;
				static char *message = "No implementation";
				qtss_sprintf(evalMessageBuff, "%s", message);
				break;
			}


		}

		if (result != 0)
		{
			SInt32 theID = GetCommandID();
			StrPtrLen *commandPtr = GetCommand();
			if (theID < 0)
			{
				if (commandPtr)
				{
					qtss_sprintf(fQueryMessageBuff, "reason=\"%s for command %s\"", evalMessage.Ptr, commandPtr->Ptr);
				}
			}
			else
			{   //qtss_printf("Set fQueryMessageBuff=%s\n",evalMessage.Ptr);
				qtss_sprintf(fQueryMessageBuff, "reason=\"%s for command %s\"", evalMessage.Ptr, QueryURI::sCommandDefs[GetCommandID()]);
			}
		}
		else
		{
			qtss_sprintf(fQueryMessageBuff, "reason=\"OK\"");
		}

		fQueryEvalMessage.Set(fQueryMessageBuff, strlen(fQueryMessageBuff));
		//qtss_printf("fQueryMessageBuff=%s\n",fQueryMessageBuff);
	}
	fQueryEvalResult = result;
	return result;
}

void QueryURI::ParseQueryString(StringParser *parserPtr, StrPtrLen *urlStreamPtr)
{
	StrPtrLen queryStr;
	StringParser tempQueryParse(parserPtr->GetStream());// point to local copy xx
	char *stopCharPtr = NULL;
	tempQueryParse.ConsumeUntil(NULL, sWhiteQuoteOrEOL); // stop on whitespace '"' 
	tempQueryParse.ConsumeWhitespace();
	tempQueryParse.ConsumeUntil(NULL, '?'); // stop at start of query
	tempQueryParse.Expect('?');
	char* startCharPtr = tempQueryParse.GetCurrentPosition();
	//qtss_printf("QueryURI::ParseQueryString start Position = '%s'\n",startCharPtr);
	while (tempQueryParse.GetDataRemaining() > 0)
	{

		tempQueryParse.ConsumeUntil(NULL, sWhiteQuoteOrEOL); // stop on whitespace '"' 
		stopCharPtr = tempQueryParse.GetCurrentPosition();
		if (*stopCharPtr == '"') // if quote read to next quote
		{
			tempQueryParse.ConsumeLength(NULL, 1);
			tempQueryParse.ConsumeUntil(NULL, '"');
			tempQueryParse.ConsumeLength(NULL, 1);
			//qtss_printf("QueryURI::ParseQueryString is quote GetCurrentPosition = '%s' len = %"   _U32BITARG_   "\n",stopCharPtr, strlen(stopCharPtr));
		}
		else
		{
			//qtss_printf("QueryURI::ParseQueryString white or EOL GetCurrentPosition = '%s' len = %"   _U32BITARG_   "\n",stopCharPtr, strlen(stopCharPtr));
			if (*stopCharPtr == ' ')
			{
				tempQueryParse.ConsumeWhitespace();
				continue;
			}

			break;

		}
	}
	UInt32 len = (UInt32)((PointerSizedInt)stopCharPtr - (PointerSizedInt)startCharPtr);
	if (len < QueryURI::eMaxBufferSize)
	{
		if (len > 0)
			fHasQuery = true;
		queryStr.Set(fQueryBuffer, len);
		memcpy(fQueryBuffer, startCharPtr, len);
		fURIFieldSPL[eQuery].Set(queryStr.Ptr, queryStr.Len);
		fURIFieldsPtr[eQuery].fData = &fURIFieldSPL[eQuery];
	}

	//qtss_printf("Query String = '%s' Query len = %"   _U32BITARG_   " parseLen = %"   _U32BITARG_   "\n",queryStr.Ptr, queryStr.Len,len);

};

void QueryURI::ParseURLString(StringParser *parserPtr, StrPtrLen *urlStreamPtr)
{
	parserPtr->ConsumeWhitespace();
	parserPtr->ConsumeUntilWhitespace(urlStreamPtr);

	fAdminFullURI.Set(fURIBuffer, urlStreamPtr->Len);

	if (urlStreamPtr->Len < QueryURI::eMaxBufferSize)
		memcpy(fURIBuffer, urlStreamPtr->Ptr, urlStreamPtr->Len); // make a local copy in fAdminFullURI

	//qtss_printf("QueryURI::ParseURLString fURIBuffer =%s len = %"   _U32BITARG_   "\n", fURIBuffer,urlStreamPtr->Len);

	StringParser tempURLParse(&fAdminFullURI);// point to local copy    
	tempURLParse.ConsumeUntil(&fURIFieldSPL[eURL], '?'); // pull out URL
	fURIFieldsPtr[eURL].fData = &fURIFieldSPL[eURL];
	//qtss_printf("QueryURI::ParseURLString fURIFieldsPtr[eURL]="); PRINT_STR(fURIFieldsPtr[eURL].fData);    
};


void QueryURI::URLParse(StrPtrLen *inStream)
{
	if (inStream != NULL)
	{
		char * decodedRequest = new char[inStream->Len + 1];
		Assert(decodedRequest != NULL);
		decodedRequest[inStream->Len] = 0;
		OSCharArrayDeleter decodedRequestDeleter(decodedRequest);

		StringParser tempParser(inStream);
		StrPtrLen URLToParse;
		SInt32 URLoffset = 0;

		if (inStream->Len > 0)
		{   // skip past the HTTP command for the StringTranslator::DecodeURL but keep it in our decoded Request buffer
			tempParser.ConsumeWhitespace();
			tempParser.ConsumeWord(NULL);
			tempParser.ConsumeWhitespace();
			URLToParse.Set(tempParser.GetCurrentPosition(), tempParser.GetDataRemaining());  // this should be a '/' and is required by the DecodeURL routine
			URLoffset = tempParser.GetDataParsedLen();
			memcpy(decodedRequest, inStream->Ptr, URLoffset);
		}

		SInt32 decodedLen = StringTranslator::DecodeURL(URLToParse.Ptr, URLToParse.Len, &decodedRequest[URLoffset], inStream->Len);
		StrPtrLen decodedRequestStr(decodedRequest, decodedLen);

		StringParser parser(&decodedRequestStr);
		StrPtrLen startFields;
		StrPtrLen adminURI;
		StrPtrLen streamURL;

		do // once
		{

			if (decodedRequestStr.Len < 1)
			{
				//qtss_printf("no string to parse \n");
				break;
			}

			if (decodedRequestStr.Len > QueryURI::eMaxBufferSize - 1)
			{
				//qtss_printf("URL string bigger than Buffer size=%"   _U32BITARG_   "\n",decodedRequestStr.Len);
				break;
			}

			StrPtrLen httpRequest;
			parser.ConsumeWord(&httpRequest);

			static StrPtrLen sPost("POST");
			static StrPtrLen sGet("GET");
			if (false == httpRequest.Equal(sPost) && false == httpRequest.Equal(sGet))    //bail if not a GET or POST
			{
				//qtss_printf("not a POST or GET \n");
				break;
			}

			ParseURLString(&parser, &streamURL);
			ParseQueryString(&parser, &streamURL);

			if (fURIFieldSPL[eURL].Len > 0)
			{
				StrPtrLen tempStr;

				StringParser URIParser(fURIFieldsPtr[eURL].fData); // parser now pointing to internal buffer root of URL

				if (!URIParser.Expect('/'))
				{
					//qtss_printf("no starting slash\n");
					break;
				}

				URIParser.ConsumeWord(&tempStr);
				if (!(tempStr.Len != 0 && tempStr.Equal(StrPtrLen(fURIFieldsPtr[eModuleID].fFieldName, fURIFieldsPtr[eModuleID].fFieldLen))))//check "modules" request
				{
					//qtss_printf("no %s in URL\n",fURIFieldsPtr[eModuleID].fFieldName);
					break;
				}
				fURIFieldSPL[eModuleID] = tempStr;
				fURIFieldsPtr[eModuleID].fData = &fURIFieldSPL[eModuleID];

				if (!URIParser.Expect('/'))
				{
					//qtss_printf("no trailing slash for modules\n");
					break;
				}
				URIParser.ConsumeWord(&tempStr);
				if (!(tempStr.Len != 0 && tempStr.Equal(StrPtrLen(fURIFieldsPtr[eRootID].fFieldName, fURIFieldsPtr[eRootID].fFieldLen))))//check "modules" request
				{
					//qtss_printf("no %s in URL\n", fURIFieldsPtr[eRootID].fFieldName);
					break;
				}

				fIsAdminQuery = true; // ok it is for us

				fURIFieldSPL[eRootID] = tempStr;
				fURIFieldsPtr[eRootID].fData = &fURIFieldSPL[eRootID];

			}



			if (fURIFieldSPL[eQuery].Len > 0) // has query fields (step past ?)
			{
				StringParser queryParser(fURIFieldsPtr[eQuery].fData);
				StrPtrLen tempStr(fURIFieldSPL[eQuery].Ptr, fURIFieldSPL[eQuery].Len);
				StrPtrLen tempData;

				//qtss_printf("queryParser=");PRINT_STR(fURIFieldsPtr[eQuery].fData);
				while (queryParser.GetDataRemaining() != 0)
				{
					tempData.Set(NULL, 0);
					if (queryParser.GetDataRemaining())queryParser.ConsumeWhitespace();
					if (queryParser.GetDataRemaining())queryParser.ConsumeUntil(&tempStr, (UInt8*)sNotQueryData);
					if (tempStr.Len == 0)
					{
						//qtss_printf("no query name\n");
						if (queryParser.GetDataRemaining())queryParser.ConsumeLength(NULL, 1);
						continue;
					}
					if (queryParser.GetDataRemaining())queryParser.ConsumeWhitespace();
					if (!queryParser.Expect('='))
					{
						//qtss_printf("no '=' for query name ");PRINT_STR(&tempStr);
						if (queryParser.GetDataRemaining())queryParser.ConsumeLength(NULL, 1);
						continue;
					}
					if (queryParser.GetDataRemaining())queryParser.ConsumeWhitespace();
					char testQuote = queryParser.PeekFast();
					if (testQuote == '"')
					{
						if (queryParser.GetDataRemaining())queryParser.ConsumeLength(NULL, 1);
						if (queryParser.GetDataRemaining())queryParser.ConsumeUntil(&tempData, '"');
						if (queryParser.GetDataRemaining())queryParser.ConsumeLength(NULL, 1);
					}
					else
					{
						if (queryParser.GetDataRemaining())queryParser.ConsumeUntil(&tempData, (UInt8*)sNotQueryData);
					}

					if (tempData.Len == 0)
					{
						if (queryParser.GetDataRemaining())queryParser.Expect('+');
						if (queryParser.GetDataRemaining())queryParser.ConsumeWhitespace();
						//qtss_printf("no query data for ");PRINT_STR(&tempStr);
						continue;
					}

					if (queryParser.GetDataRemaining())queryParser.ConsumeWhitespace();
					if (queryParser.GetDataRemaining())queryParser.Expect('+');
					if (queryParser.GetDataRemaining())queryParser.ConsumeWhitespace();

					//qtss_printf("set data =%s\n", tempData.Ptr);

					StrPtrLen   definedID;
					UInt32      fieldID;
					for (short testField = 0; testField < eNumAttributes; testField++)
					{
						definedID.Set(fURIFieldsPtr[testField].fFieldName, fURIFieldsPtr[testField].fFieldLen);
						fieldID = fURIFieldsPtr[testField].fID;
						if (definedID.EqualIgnoreCase(tempStr.Ptr, tempStr.Len)) // test (fURIFieldsPtr[fieldID].fData == NULL) to make first time setting only
						{   // set the field value always takes the last appearance of the name=value pair
							if (fieldID >= eFilter1)
							{
								UInt32 emptyId = eFilter1 + fNumFilters;
								if (tempData.Len > 0)
								{
									fNumFilters++;
									fURIFieldSPL[emptyId].Set(tempData.Ptr, tempData.Len);
									fURIFieldsPtr[emptyId].fData = &fURIFieldSPL[emptyId];
								}
							}
							else
							{
								fURIFieldSPL[fieldID].Set(tempData.Ptr, tempData.Len);
								fURIFieldsPtr[testField].fData = &fURIFieldSPL[fieldID];
							}
						}
					}

				}

			}

		} while (false);

		/*
				for (int count = 0; fURIFieldsPtr[count].fID != -1 ; count++)
				{   //qtss_printf("QueryURI::URLParse %s=",fURIFieldsPtr[count].fFieldName); PRINT_STR(fURIFieldsPtr[count].fData);
				}
		*/

	}
}

