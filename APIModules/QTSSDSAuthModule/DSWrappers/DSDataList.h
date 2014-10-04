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
	File:       DSDataList.h
 
	Contains:   Class definition for Open Directory data list
				(wraps tDataListPtr)
 
	Created By: chris jalbert
 
	Created:	26 July 2000
 */


#ifndef _DSDataList_h
#define _DSDataList_h

// ANSI / POSIX Headers
#include <stdarg.h>		// for varargs stuff
#include <string.h>		// for memset()

// STL and Std C++ Library Headers
#include <memory>		// for auto_ptr<>

// Framework Headers
#include <DirectoryService/DirServicesTypes.h>
#include <DirectoryService/DirServicesUtils.h>

// Project Headers
#include "DSDataNode.h"


namespace DirectoryServices {

//-----------------------------------------------------------------------------
// ¥ DSDataList - simple wrapper for tDataBufferPtr.
//	This should be considered a private class, primarily used by DSNode.
//	All methods are inlined for performance reasons (sorry).
//	Logically, a tDataList is a collection of tDataNode's, so this
//	implementation offers GetCount() and operator[](u_long) methods.
//-----------------------------------------------------------------------------

class DSDataList
{
public:
	/**** Instance methods. ****/
	// ctor and dtor.
/*	Not used anywhere and conflicts with next ctor, which is more useful.
	DSDataList	( tDirReference	inDirRef,
					const char	*inString, ... ) throw (DSException)
			: mDirRef (inDirRef)
			{	va_list	args ; va_start (args, inString) ;
				std::memset (&mList, 0, sizeof (mList)) ;
				tDirStatus		nError = ::dsBuildListFromStringsAllocV (
											inDirRef, &mList, inString, args) ;
				va_end (args) ;
				if (nError) throw_ds_error (nError) ; }
*/
	DSDataList	( tDirReference	inDirRef,
					const char *inPath ) throw (DSException)
			: mDirRef (inDirRef)
			{	std::memset (&mList, 0, sizeof (mList)) ;
				tDirStatus	nError = ::dsBuildListFromPathAlloc (inDirRef, &mList, inPath, "/") ;
				if (nError) throw_ds_error (nError) ; }
	DSDataList	( const DSDataList& inOrg ) throw (DSException)
			: mDirRef (inOrg.mDirRef)
			{	tDataList	*dlp = ::dsDataListCopyList (inOrg.mDirRef, &inOrg.mList) ;
				if (!dlp) throw_ds_error (eDSAllocationFailed) ;
				mList = *dlp ; std::free (dlp) ; }
		// The following constructor changes the ownership of the list buffer!
	DSDataList	( tDirReference		inDirRef,
					tDataListPtr	inList = 0 ) throw (DSException)
			: mDirRef (inDirRef)
			{	if (inList) {
					mList = *inList ; std::memset (inList, 0, sizeof (mList)) ;
				} else std::memset (&mList, 0, sizeof (mList)) ; }
	~DSDataList	( void ) throw ()
			{ ::dsDataListDeallocate (mDirRef, &mList) ; }

	// Inline accessors.
	UInt32	count	( void ) const throw ()
			{ return ::dsDataListGetNodeCount (&mList) ; }
	size_t			length	( void ) const throw ()
			{ return (size_t) ::dsGetDataLength (&mList) ; }
		// GetPath()'s return value will be freed when it goes out of scope.
		// If it is important, COPY IT to another auto_ptr (which will
		// properly invalidate the original).
	std::auto_ptr<const char>	path	( const char *inSep = "/" ) const
			{ return std::auto_ptr<const char> (::dsGetPathFromList (mDirRef,
											&mList, inSep)) ; }

	// Casting operators.
					operator tDataListPtr() throw ()
						{ return &mList ; }
					operator const tDataList*() const throw ()
						{ return &mList ; }
	DSDataNode*		operator[] ( UInt32 inIndex ) const throw (DSException)
			{	tDataNodePtr	dnTemp ;
				tDirStatus		nError = ::dsDataListGetNodeAlloc (
											mDirRef, &mList, inIndex, &dnTemp) ;
				if (nError) throw_ds_error (nError) ;
				return new DSDataNode (mDirRef, dnTemp) ; }

	// Setters.
	void	append	( const char *inString ) throw (DSException)
			{	tDirStatus	nError = ::dsAppendStringToListAlloc (
											mDirRef, &mList, inString) ;
				if (nError) throw_ds_error (nError) ; }

protected:
	/**** Instance data. ****/
	tDirReference	mDirRef ;
	tDataList		mList ;
} ;

}	// namespace DirectoryServices

#endif	/* _DSDataList_h */
