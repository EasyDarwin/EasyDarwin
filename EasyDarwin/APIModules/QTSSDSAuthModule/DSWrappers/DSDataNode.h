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
	File:       DSDataNode.h
 
	Contains:   Class definition for Open Directory data node
				(wraps tDataNodePtr)
 
	Created By: chris jalbert
 
	Created:	26 July 2000
 */


#ifndef _DSDataNode_h
#define _DSDataNode_h


// Framework Headers
#include <DirectoryService/DirServicesTypes.h>
#include <DirectoryService/DirServicesUtils.h>

// Project Headers
#include "CDirService.h"


namespace DirectoryServices {

//-----------------------------------------------------------------------------
// ¥ DSDataNode - simple wrapper for tDataBufferPtr.
//	This should be considered a private class, primarily used by DSNode.
//	All methods are inlined for performance reasons (sorry).
//	tDataNode's are identical to tDataBuffer's in implementation, however,
//	nodes are treated as opaque objects with DS accessor functions. As a
//	result, DSDataNode is not a subclass of DSBuffer.
//-----------------------------------------------------------------------------

class DSDataNode
{
public:
	/**** Instance methods. ****/
	// ctor and dtor.
	DSDataNode	( tDirReference		inDirRef,
					size_t			inBufSize,
					size_t			inBufUsed,
					const void		*inData ) throw (DSException)
			: mDirRef (inDirRef),
				mNode (::dsDataNodeAllocateBlock (inDirRef,
						(UInt32) inBufSize, (UInt32) inBufUsed,
						(void *) inData))
			{ if (!mNode) throw_ds_error (eDSAllocationFailed) ; }
	DSDataNode	( tDirReference		inDirRef,
					const char		*inString ) throw (DSException)
			: mDirRef (inDirRef),
				mNode (::dsDataNodeAllocateString (inDirRef, inString))
			{ if (!mNode) throw_ds_error (eDSAllocationFailed) ; }
			// Used by DSDataList
	DSDataNode	( tDirReference		inDirRef,
					tDataNode		*inNode ) throw (DSException)
			: mDirRef (inDirRef), mNode (inNode)
			{ if (!mNode) throw_ds_error (eDSAllocationFailed) ; }
			// Something of a "copy" constructor
	DSDataNode	( tDirReference		inDirRef,
					const tDataNode	*inNode ) throw (DSException)
			: mDirRef (inDirRef),
				mNode (::dsDataNodeAllocateBlock (inDirRef,
								inNode->fBufferSize, inNode->fBufferLength,
								(void *) inNode->fBufferData))
			{ if (!mNode) throw_ds_error (eDSAllocationFailed) ; }
	~DSDataNode	( void ) throw ()
			{ if (mNode) ::dsDataNodeDeAllocate (mDirRef, mNode) ; }

	// Inline accessors.
	size_t	capacity	( void ) const throw ()
			{ return (size_t) ::dsDataNodeGetSize (mNode) ; }
	size_t	size		( void ) const throw ()
			{ return (size_t) ::dsDataNodeGetSize (mNode) ; }
	size_t	length		( void ) const throw ()
			{ return (size_t) ::dsDataNodeGetLength (mNode) ; }
	void	resize		( size_t inLength ) throw (DSException)
			{ tDirStatus	nError = ::dsDataNodeSetLength (mNode, inLength) ;
				if (nError) throw_ds_error (nError) ; }

	// Casting operators.
	operator tDataNodePtr() const throw ()
			{ return mNode ; }

protected:
	/**** Instance data. ****/
	tDirReference	mDirRef ;
	tDataNodePtr	mNode ;
} ;

}	// namespace DirectoryServices

#endif	/* _DSDataNode_h */
