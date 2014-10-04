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
	File:       DSBuffer.h
 
	Contains:   Class definition for Open Directory buffers
				(wraps tDataBufferPtr)
 
	Created By: chris jalbert
 
	Created:	26 July 2000
 */


#ifndef _DSBuffer_h
#define _DSBuffer_h


// Framework Headers
#include <DirectoryService/DirServicesTypes.h>
#include <DirectoryService/DirServicesUtils.h>

// Project Headers
#include "CDirService.h"


namespace DirectoryServices {

//-----------------------------------------------------------------------------
// ¥ DSBuffer - a wrapper for tDataBufferPtr.
//	This should be considered a private class, primarily used by DSNode.
//	All methods except Grow() are inlined for performance reasons (sorry).
//-----------------------------------------------------------------------------

class DSBuffer
{
public:
	/**** Typedefs, enums, and constants. ****/
	enum { kDefaultSize = 128 } ;

public:
	/**** Instance methods. ****/
	// ctor and dtor.
	DSBuffer	( tDirReference	inDirRef = 0,
					size_t		inBufferSize = kDefaultSize ) throw (DSException)
			: mDirRef (inDirRef), mBuffer (0)
			{ if (!grow (inBufferSize)) throw_ds_error (eDSAllocationFailed) ; }
	~DSBuffer	( void ) throw ()
			{ if (mBuffer) ::dsDataBufferDeAllocate (mDirRef, mBuffer) ; }

	// Inline accessors.
	size_t	capacity	( void ) const throw ()
			{ return (size_t) mBuffer->fBufferSize ; }
	size_t	size		( void ) const throw ()
			{ return (size_t) mBuffer->fBufferSize ; }
	size_t	length		( void ) const throw ()
			{ return (size_t) mBuffer->fBufferLength ; }
	const char	*c_str	( void ) const throw ()
			{ return (const char *) mBuffer->fBufferData ; }
	const void	*data	( void ) const throw ()
			{ return (const void *) mBuffer->fBufferData ; }

	// Inline setters.
	void	clear	( void ) throw ()
			{ mBuffer->fBufferLength = 0 ; }
	void	resize	( size_t inLength ) throw (DSException)
			{	if (inLength > mBuffer->fBufferSize)
					throw_ds_error (eDSBufferTooSmall) ;
				mBuffer->fBufferLength = inLength ; }
	void	set		( const char *inString ) throw (DSException)
			{ clear () ; append (inString) ; }
	void	set		( const void *inData, size_t inLength ) throw (DSException)
			{ clear () ; append (inData, inLength) ; }
	void	append	( const char *inString ) throw (DSException)
			{ append ((const void *) inString, 1 + strlen (inString)) ; }
	void	append	( const void *inData, size_t inLength ) throw (DSException)
			{	grow (mBuffer->fBufferLength + inLength) ;
				char *cpBuf = mBuffer->fBufferData + mBuffer->fBufferLength ;
				std::memcpy (cpBuf, inData, inLength) ;
				mBuffer->fBufferLength += inLength ; }

	// Casting operators.
	tDataBufferPtr	operator->() throw ()
			{ return mBuffer ; }
	operator tDataBufferPtr() const throw ()
			{ return mBuffer ; }
	operator const char*() const throw ()
			{ return this->c_str () ; }
	operator const void*() const throw ()
			{ return this->data () ; }

protected:
	/**** Instance methods accessible only to class and subclasses. ****/
	tDataBufferPtr	grow	( size_t inNewSize ) throw (DSException) ;

	/**** Instance data. ****/
	tDirReference	mDirRef ;
	tDataBufferPtr	mBuffer ;
} ;

}	// namespace DirectoryServices

#endif	/* _DSBuffer_h */
