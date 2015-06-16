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
 
	Contains:   Implements DSException.
 
	Created By: chris jalbert
 
	Created:	26 July 2000
 */


// This file is only meaningful if exceptions are enabled.
#if USE_EXCEPTIONS

// STL and Std C++ Library Headers
#include <cstdlib>		// for abs()
#include <stdexcept>	// for standard exceptions

// Project Headers
#include "CDirService.h"

using namespace DirectoryServices ;
using namespace std ;


// ----------------------------------------------------------------------------
//	¥ CDirService Class Globals
//	These private typedefs, globals, and functions are not declared statically
//	in the class definition because I want to hide the implementation details
//	and reduce unrelated dependencies in the class header.
// ----------------------------------------------------------------------------

static const char * const	_DSErr = "DirectoryService error: " ;

/*
 * Convert an UInt32 to ASCII for printf purposes, returning
 * a pointer to the first character of the string representation.
 * Borrowed from vfprintf.c.
 */
static char *_ltoa (
	SInt32			val,
	char			*endp)
{
	register char	*cp = endp ;
	register SInt32	sval = std::abs (val) ;

	// Terminate the string.
	*--cp = '\0' ;

	do {
		*--cp = '0' + (sval % 10) ;
		sval /= 10 ;
	} while (sval != 0) ;

	// Handle signed values.
	if (val < 0)
		*--cp = '-' ;
	return cp ;
}

DSException::DSException (tDirStatus err)
	: inherited (string (_DSErr) + _ltoa (err, &mErrStr[sizeof (mErrStr)])),
				mErr (err)
{
}

#endif	/* USE_EXCEPTIONS */
