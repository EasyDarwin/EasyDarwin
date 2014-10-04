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
    File:       ResizeableStringFormatter.cpp

    Contains:   Implements object defined in ResizeableStringFormatter.h
    
    
*/

#include "ResizeableStringFormatter.h"
#include "OSMemory.h"

Bool16 ResizeableStringFormatter::BufferIsFull(char* inBuffer, UInt32 inBufferLen)
{
    //allocate a buffer twice as big as the old one, and copy over the contents
    UInt32 theNewBufferSize = this->GetTotalBufferSize() * 2;
    if (theNewBufferSize == 0)
        theNewBufferSize = 64;
        
    char* theNewBuffer = NEW char[theNewBufferSize];
    ::memcpy(theNewBuffer, inBuffer, inBufferLen);

    //if the old buffer was dynamically allocated also, we'd better delete it.
    if (inBuffer != fOriginalBuffer)
        delete [] inBuffer;
    
    fStartPut = theNewBuffer;
    fCurrentPut = theNewBuffer + inBufferLen;
    fEndPut = theNewBuffer + theNewBufferSize;
	return true;
}
