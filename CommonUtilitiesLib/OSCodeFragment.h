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
	 File:       OSDynamicLoader.h

	 Contains:   OS abstraction for loading code fragments.



 */

#ifndef _OS_CODEFRAGMENT_H_
#define _OS_CODEFRAGMENT_H_

#include <stdlib.h>
#include "SafeStdLib.h"
#include "OSHeaders.h"

#ifdef __MacOSX__
#include <CoreFoundation/CFBundle.h>
#endif

class OSCodeFragment
{
public:

	static void Initialize();

	OSCodeFragment(const char* inPath);
	~OSCodeFragment();

	bool  IsValid() { return (fFragmentP != NULL); }
	void*   GetSymbol(const char* inSymbolName);

private:

#ifdef __Win32__
	HMODULE fFragmentP;
#elif __MacOSX__
	CFBundleRef fFragmentP;
#else
	void*   fFragmentP;
#endif
};

#endif//_OS_CODEFRAGMENT_H_
