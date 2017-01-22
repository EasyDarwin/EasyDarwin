/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
 */
#define __Win32__ 1
#define _MT 1

/* Defines needed to compile windows headers */


#ifdef _WIN64

#ifndef _X64_
#define _X64_ 1
#endif

/* Pro4 compilers should automatically define this to appropriate value */
#ifndef _M_IA64
#define _M_IA64 600
#endif

#else

#ifndef _X86_
    #define _X86_ 1
#endif

/* Pro4 compilers should automatically define this to appropriate value */
#ifndef _M_IX86
    #define _M_IX86 500
#endif

#endif // _WIN64


#ifndef WIN32
    /* same as definition in OLE2.h where 100 implies WinNT version 1.0 */
    #define WIN32 100
#endif

#ifndef _WIN32
    #define _WIN32 1
#endif

#ifndef _MSC_VER
    /* we implement the feature set of CL 9.0 (MSVC++ 2) */
    #define _MSC_VER 900
#endif

#ifndef _CRTAPI1
    #define _CRTAPI1 __cdecl
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0400
#endif

#ifndef _WIN32_IE
    #define _WIN32_IE 0x0400
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include "PlatformHeader.h"
#include "revision.h"
#include <fcntl.h>