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
	 File:       StringFormatter.cpp

	 Contains:   Implementation of StringFormatter class.




 */

#include <string.h>
#include <stdarg.h>
#include "StringFormatter.h"
#include "MyAssert.h"

char*   StringFormatter::sEOL = "\r\n";
UInt32  StringFormatter::sEOLLen = 2;

void StringFormatter::Put(const SInt32 num)
{
	char buff[32];
	qtss_sprintf(buff, "%" _S32BITARG_ "", num);
	Put(buff);
}

void StringFormatter::Put(char* buffer, UInt32 bufferSize)
{
	//optimization for writing 1 character
	if ((bufferSize == 1) && (fCurrentPut != fEndPut)) {
		*(fCurrentPut++) = *buffer;
		fBytesWritten++;
		return;
	}

	//loop until the input buffer size is smaller than the space in the output
	//buffer. Call BufferIsFull at each pass through the loop
	UInt32 spaceLeft = this->GetSpaceLeft();
	UInt32 spaceInBuffer = spaceLeft - 1;
	UInt32 resizedSpaceLeft = 0;

	while ((spaceInBuffer < bufferSize) || (spaceLeft == 0)) // too big for destination
	{
		if (spaceLeft > 0)
		{
			//copy as much as possible; truncating the result
			::memcpy(fCurrentPut, buffer, spaceInBuffer);
			fCurrentPut += spaceInBuffer;
			fBytesWritten += spaceInBuffer;
			buffer += spaceInBuffer;
			bufferSize -= spaceInBuffer;
		}
		this->BufferIsFull(fStartPut, this->GetCurrentOffset()); // resize buffer
		resizedSpaceLeft = this->GetSpaceLeft();
		if (spaceLeft == resizedSpaceLeft) // couldn't resize, nothing left to do
		{
			return; // done. There is either nothing to do or nothing we can do because the BufferIsFull
		}
		spaceLeft = resizedSpaceLeft;
		spaceInBuffer = spaceLeft - 1;
	}

	//copy the remaining chunk into the buffer
	::memcpy(fCurrentPut, buffer, bufferSize);
	fCurrentPut += bufferSize;
	fBytesWritten += bufferSize;

}

//Puts a printf-style formatted string; except that the NUL terminator is not written.  If the buffer is too small, returns false and does not
//Alter the buffer.  Will not count the '\0' terminator as among the bytes written
Bool16 StringFormatter::PutFmtStr(const char* fmt, ...)
{
	Assert(fmt != NULL);

	va_list args;
	for (;;)
	{
		va_start(args, fmt);
		int length = ::vsnprintf(fCurrentPut, this->GetSpaceLeft(), fmt, args);
		va_end(args);

		if (length < 0)
			return false;
		if (static_cast<UInt32>(length) >= this->GetSpaceLeft()) //was not able to write all the output
		{
			if (this->BufferIsFull(fStartPut, this->GetCurrentOffset()))
				continue;
			//can only output a portion of the string
			UInt32 bytesWritten = fEndPut - fCurrentPut - 1; //We don't want to include the NUL terminator
			fBytesWritten += bytesWritten;
			fCurrentPut += bytesWritten;
			return false;
		}
		else
		{
			fBytesWritten += length;
			fCurrentPut += length;
		}
		return true;
	}
}

