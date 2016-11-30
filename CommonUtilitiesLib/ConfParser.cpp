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


#include "ConfParser.h"
#include "OSMemory.h"

#include "MyAssert.h"
#include "OSHeaders.h"

#include "GetWord.h"
#include "Trim.h"

#include <string.h> 
#include <stdio.h>  

static Bool16 SampleConfigSetter(const char* paramName, const char* paramValue[], void* userData);
static void DisplayConfigErr(const char* fname, int lineCount, const char* lineBuff, const char* errMessage);

void TestParseConfigFile()
{
	ParseConfigFile(false, "qtss.conf", SampleConfigSetter, NULL);

}

static Bool16 SampleConfigSetter(const char* paramName, const char* paramValue[], void* /*userData*/)
{
	qtss_printf("param: %s", paramName);

	int x = 0;

	while (paramValue[x])
	{
		qtss_printf(" value(%" _S32BITARG_ "): %s ", (SInt32)x, (char *)paramValue[x]);
		x++;
	}

	qtss_printf("\n");

	return false;
}


static void DisplayConfigErr(const char* fname, int lineCount, const char* lineBuff, const char* errMessage)
{

	qtss_printf("- Configuration file error:\n");


	if (lineCount)
		qtss_printf("  file: %s, line# %i\n", fname, lineCount);
	else
		qtss_printf("  file: %s\n", fname);

	if (lineBuff)
		qtss_printf("  text: %s", lineBuff); // lineBuff already includes a \n

	if (errMessage)
		qtss_printf("  reason: %s\n", errMessage); // lineBuff already includes a \n
}


int ParseConfigFile(
	Bool16  allowNullValues
	, const char* fname
	, Bool16(*ConfigSetter)(const char* paramName, const char* paramValue[], void* userData)
	, void* userData)
{
	int     error = -1;

	Assert(fname);
	Assert(ConfigSetter);

	if (!fname) return error;
	if (!ConfigSetter) return error;

	FILE* configFile = fopen(fname, "r");

	//  Assert( configFile );

	if (configFile)
	{
		SInt32    lineBuffSize = kConfParserMaxLineSize;
		SInt32    wordBuffSize = kConfParserMaxParamSize;

		char    lineBuff[kConfParserMaxLineSize];
		char    wordBuff[kConfParserMaxParamSize];

		char    *next;

		// debug assistance -- CW debugger won't display large char arrays as strings
		//char* l = lineBuff;
		//char* w = wordBuff;


		int lineCount = 0;
		do
		{
			next = lineBuff;

			// get a line ( fgets adds \n+ 0x00 )

			if (fgets(lineBuff, lineBuffSize, configFile) == NULL)
				break;

			++lineCount;
			error = 0; // allow empty lines at beginning of file.

			// trim the leading whitespace
			next = TrimLeft(lineBuff);

			if (*next)
			{

				if (*next == '#')
				{
					// it's a comment
					// prabably do nothing in release version?

					//  qtss_printf( "comment: %s" , &lineBuff[1] );

					error = 0;

				}
				else
				{
					char* param;

					// grab the param name, quoted or not
					if (*next == '"')
						next = GetQuotedWord(wordBuff, next, wordBuffSize);
					else
						next = GetWord(wordBuff, next, wordBuffSize);

					Assert(*wordBuff);

					param = NEW char[strlen(wordBuff) + 1];

					Assert(param);

					if (param)
					{
						const char* values[kConfParserMaxParamValues + 1];
						int         maxValues = 0;

						strcpy(param, wordBuff);

						values[maxValues] = NULL;

						while (maxValues < kConfParserMaxParamValues && *next)
						{
							// ace
							next = TrimLeft(next);

							if (*next)
							{
								if (*next == '"')
									next = GetQuotedWord(wordBuff, next, wordBuffSize);
								else
									next = GetWord(wordBuff, next, wordBuffSize);

								char* value = NEW char[strlen(wordBuff) + 1];

								Assert(value);

								if (value)
								{
									strcpy(value, wordBuff);

									values[maxValues++] = value;
									values[maxValues] = 0;
								}

							}

						}

						if ((maxValues > 0 || allowNullValues) && !(*ConfigSetter)(param, values, userData))
							error = 0;
						else
						{
							error = -1;
							if (maxValues > 0)
								DisplayConfigErr(fname, lineCount, lineBuff, "Parameter could not be set.");
							else
								DisplayConfigErr(fname, lineCount, lineBuff, "No value to set.");
						}

						delete[] param;

						maxValues = 0;

						while (values[maxValues])
						{
							char** tempValues = (char**)values; // Need to do this to delete a const
							delete[] tempValues[maxValues];
							maxValues++;
						}
					}
				}
			}
		} while (feof(configFile) == 0 && error == 0);

		(void)fclose(configFile);
	}
	//  else
	//  {
	//      qtss_printf("Couldn't open config file at: %s\n", fname);
	//  }

	return error;

}
