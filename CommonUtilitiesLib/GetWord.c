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

#include "GetWord.h"

char* GetWord(char* toWordPtr, char* fromStrPtr, SInt32 limit)
{
	// get a word from a string
	// copy result into toWordPtr, return one past end of the 
	// word, limit is max for toWordPtr
	// fromStrPtr

	// trim any leading white space
	while ((unsigned char)*fromStrPtr <= 0x20 && *fromStrPtr)
		fromStrPtr++;

	// copy until we find more white space
	while (limit && (unsigned char)*fromStrPtr > 0x20 && *fromStrPtr)
	{
		*toWordPtr++ = *fromStrPtr++;
		limit--;
	}

	*toWordPtr = 0x00;

	return (char *)fromStrPtr;
}

char* GetQuotedWord(char* toWordPtr, char* fromStrPtr, SInt32 limit)
{
	// get a quote encoded word from a string
	// make include white space
	int lastWasQuote = 0;

	// trim any leading white space
	while (((unsigned char)*fromStrPtr <= 0x20) && *fromStrPtr)
		fromStrPtr++;


	if ((unsigned char)*fromStrPtr == '"')
	{   // must lead with quote sign after white space
		fromStrPtr++;



		// copy until we find the last single quote
		while (limit && *fromStrPtr)
		{
			if ((unsigned char)*fromStrPtr == '"')
			{
				if (lastWasQuote)
				{
					*toWordPtr++ = '"';
					lastWasQuote = 0;
					limit--;
				}
				else
					lastWasQuote = 1;
			}
			else
			{
				if (!lastWasQuote)
				{
					*toWordPtr++ = *fromStrPtr;
					limit--;
				}
				else // we're done, hit a quote by itself
					break;

			}

			// consume the char we read
			fromStrPtr++;

		}
	}

	*toWordPtr = 0x00;

	return (char *)fromStrPtr;
}
