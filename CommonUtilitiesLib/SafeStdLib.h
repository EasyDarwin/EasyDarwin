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
	 File:       SafeStdLib.h

	 Contains:   Thread safe std lib calls


 */

#ifndef _INTERNAL_STDLIB_H_
#define _INTERNAL_STDLIB_H_

#include <time.h>
#include "OSHeaders.h"

#define kTimeStrSize 32
#define kErrorStrSize 256
extern int qtss_maxprintf(const char *fmt, ...);
extern void qtss_setmaxprintfcharsinK(UInt32 newMaxCharsInK);
extern UInt32 qtss_getmaxprintfcharsinK();

#ifndef USE_DEFAULT_STD_LIB


#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __USE_MAX_PRINTF__
#define qtss_printf qtss_maxprintf
#else
	extern int qtss_printf(const char *fmt, ...);

#endif

	extern int qtss_sprintf(char *buffer, const char *fmt, ...);
	extern int qtss_fprintf(FILE *file, const char *fmt, ...);
	extern int qtss_snprintf(char *str, size_t size, const char *format, ...);
	extern size_t qtss_strftime(char *buf, size_t maxsize, const char *format, const struct tm *timeptr);

	// These calls return the pointer passed into the call as the result.

	extern char *qtss_strerror(int errnum, char* buffer, int buffLen);
	extern char *qtss_ctime(const time_t *timep, char* buffer, int buffLen);
	extern char *qtss_asctime(const struct tm *timeptr, char* buffer, int buffLen);
	extern struct tm *qtss_gmtime(const time_t *, struct tm *result);
	extern struct tm *qtss_localtime(const time_t *, struct tm *result);

#ifdef __cplusplus
}
#endif


#else //USE_DEFAULT_STD_LIB

#define qtss_sprintf sprintf

#define qtss_fprintf fprintf

#ifdef __USE_MAX_PRINTF__
#define qtss_printf qtss_maxprintf
#else
#define qtss_printf printf
#endif

#if __Win32__
#define qtss_snprintf _snprintf
#else

#define qtss_snprintf snprintf

#endif



#define qtss_strftime strftime

// Use our calls for the following.
// These calls return the pointer passed into the call as the result.

extern char* qtss_strerror(int errnum, char* buffer, int buffLen);
extern char* qtss_ctime(const time_t* timep, char* buffer, int buffLen);
extern char* qtss_asctime(const struct tm* timeptr, char* buffer, int buffLen);
extern struct tm* qtss_gmtime(const time_t*, struct tm* result);
extern struct tm* qtss_localtime(const time_t*, struct tm* result);

#endif  //USE_DEFAULT_STD_LIB
#endif //_INTERNAL_STDLIB_H_
