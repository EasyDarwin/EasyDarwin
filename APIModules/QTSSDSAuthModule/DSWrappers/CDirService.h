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
	File:       CDirService.h
 
	Contains:   Defines DSException.
 
	Created By: chris jalbert
 
	Created:	26 July 2000
 */


#ifndef _CDirService_h
#define _CDirService_h

// Framework Headers
#include <DirectoryService/DirServicesTypes.h>

namespace DirectoryServices {

const tDirReference kDSDirRefNull = 0 ;

// This file is only meaningful if exceptions are enabled.
#if __EXCEPTIONS

// STL and Std C++ Library Headers
#include <stdexcept>	// for standard exceptions


//-----------------------------------------------------------------------------
// ¥ DSException - exception class that wraps a tDirStatus result code.
//-----------------------------------------------------------------------------

class DSException : public std::runtime_error {
	public:
		typedef std::runtime_error inherited ;
					DSException	( tDirStatus err ) ;
		tDirStatus	status		( void ) const	{ return mErr ; }
	private:
		tDirStatus	mErr ;
		char		mErrStr [12] ;
} ;
#define throw_ds_error(err)	throw DSException(err)

#else	/* __EXCEPTIONS */

//-----------------------------------------------------------------------------
// ¥ DSException - with exceptions disabled, this is dummy code.
//-----------------------------------------------------------------------------

class DSException {
	public:
		DSException	( tDirStatus ) { }
} ;

// Define away the throw spec.
#define throw(spec)
#define throw_ds_error(err)

#endif	/* __EXCEPTIONS */

}	// namespace DirectoryServices

#endif	/* _CDirService_h */
