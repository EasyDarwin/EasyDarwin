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
	File:       DSBuffer.cpp
 
	Contains:   Class implementation for Open Directory buffers
				(wraps tDataBufferPtr)
 
	Created By: chris jalbert
 
	Created:	28 November 2000
 */


// ANSI / POSIX headers

// STL and Std C++ Library Headers
#include <stdexcept>	// for standard exceptions

// Framework Headers
#include <DirectoryService/DirServices.h>

// Project Headers
#include "DSBuffer.h"


using namespace DirectoryServices ;


// ----------------------------------------------------------------------------
//	¥ DSBuffer Class Globals
//	These private typedefs, globals, and functions are not declared statically
//	in the class definition because I want to hide the implementation details
//	and reduce unrelated dependencies in the class header.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//	¥ DSBuffer Protected Instance Methods
// ----------------------------------------------------------------------------
#pragma mark **** DSBuffer Protected Instance Methods ****

// ----------------------------------------------------------------------------
// ¥ÊGrow
//	All memory management (even in the c'tors) are handled in this method.
//	An inNewSize value of 0 implies the use of the default buffer size!
//	To leave the buffer alone, call with an argument value of 1.
// ----------------------------------------------------------------------------

tDataBufferPtr DSBuffer::grow ( size_t inNewSize )
{
	if (!inNewSize)
		inNewSize = kDefaultSize ;
	if (mBuffer && (inNewSize <= mBuffer->fBufferSize))
		return mBuffer ;

	register size_t	ulTemp = 16 ;
	if (inNewSize == kDefaultSize)
		ulTemp = inNewSize ;
	else
		for ( ; ulTemp < inNewSize ; ulTemp <<= 1) ;

	register tDataBufferPtr	bufNew = ::dsDataBufferAllocate (mDirRef, ulTemp) ;
	if (!bufNew)
		throw_ds_error (eDSAllocationFailed) ;
	if (mBuffer && (ulTemp = mBuffer->fBufferLength))
		std::memcpy (bufNew->fBufferData, mBuffer->fBufferData, ulTemp) ;
	else
		ulTemp = 0 ;
	bufNew->fBufferLength = ulTemp ;
	::dsDataBufferDeAllocate (mDirRef, mBuffer) ;
	return (mBuffer = bufNew) ;
}
