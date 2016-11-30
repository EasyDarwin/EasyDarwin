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
	 File:       ResizeableStringFormatter.h

	 Contains:   Derived from StringFormatter, this transparently grows the
				 output buffer if the original buffer is too small to hold all
				 the data being placed in it



 */

#ifndef __RESIZEABLE_STRING_FORMATTER_H__
#define __RESIZEABLE_STRING_FORMATTER_H__

#include "StringFormatter.h"

class ResizeableStringFormatter : public StringFormatter
{
public:
	// Pass in inBuffer=NULL and inBufSize=0 to dynamically allocate the initial buffer.
	ResizeableStringFormatter(char* inBuffer = NULL, UInt32 inBufSize = 0)
		: StringFormatter(inBuffer, inBufSize), fOriginalBuffer(inBuffer) {}

	//If we've been forced to increase the buffer size, fStartPut WILL be a dynamically allocated
	//buffer, and it WON'T be equal to fOriginalBuffer (obviously).
	virtual ~ResizeableStringFormatter() { if (fStartPut != fOriginalBuffer) delete[] fStartPut; }

private:

	// This function will get called by StringFormatter if the current
	// output buffer is full. This object allocates a buffer that's twice
	// as big as the old one.
	virtual Bool16    BufferIsFull(char* inBuffer, UInt32 inBufferLen);

	char*           fOriginalBuffer;

};

#endif //__RESIZEABLE_STRING_FORMATTER_H__
