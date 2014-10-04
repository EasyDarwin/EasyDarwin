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
    File:       DynamicModuleStdLib.cpp

    Contains:   Thread safe std lib calls for QTSS API dynamic modules
    

*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "SafeStdLib.h"
#include "OSMutex.h"
#include "OS.h"
#include "QTSS.h"


#ifndef USE_DEFAULT_STD_LIB

#if __Win32__

#define VSNprintf _vsnprintf

#else

#define VSNprintf vsnprintf

#endif

int qtss_printf(char *fmt,  ...)
{
    if (fmt == NULL)
        return 1;

    QTSS_LockStdLib();
    va_list args;
    va_start(args,fmt);
    int result =  ::vprintf(fmt, args);
    va_end(args);
    QTSS_UnlockStdLib();
    
    return result;
}

int qtss_sprintf(char *buffer, const char *fmt,  ...)
{
    if (buffer == NULL)
        return -1;

    QTSS_LockStdLib();
    va_list args;
    va_start(args,fmt);
    int result =  ::vsprintf(buffer, fmt, args);
    va_end(args);
    QTSS_UnlockStdLib();
    
    return result;
}

int qtss_fprintf(FILE *file, const char *fmt,  ...)
{
    if (file == NULL)
        return -1;

    QTSS_LockStdLib();
    va_list args;
    va_start(args,fmt);
    int result =  ::vfprintf(file, fmt, args);
    va_end(args);
    QTSS_UnlockStdLib();
    
    return result;
}

int  qtss_snprintf(char *str, size_t size, const char *fmt, ...)
{
    if (str == NULL)
        return -1;

    QTSS_LockStdLib();
    va_list args;
    va_start(args,fmt);
    int result =  ::VSNprintf(str, size, fmt, args);
    va_end(args);
    QTSS_UnlockStdLib();
    
    return result;
}

size_t qtss_strftime(char *buf, size_t maxsize, const char *format, const struct tm *timeptr)
{
    if (buf == NULL)
        return 0;

    QTSS_LockStdLib();
    size_t result = ::strftime(buf, maxsize, format, timeptr); 
    QTSS_UnlockStdLib();
    
    return result;
}

#endif //USE_DEFAULT_STD_LIB

char *qtss_strerror(int errnum, char* buffer, int buffLen)
{
    QTSS_LockStdLib();
	(void) ::strncpy( buffer, ::strerror(errnum), buffLen);
	buffer[buffLen -1] = 0;  //make sure it is null terminated even if truncated.
    QTSS_UnlockStdLib();

    return buffer;
}

char *qtss_ctime(const time_t *timep, char* buffer, int buffLen)
{
#if __MacOSX__
    Assert(buffLen >= 26);
    return ::ctime_r(timep, buffer);
#else
    QTSS_LockStdLib();
    (void) ::strncpy( buffer, ::ctime(timep), buffLen);
    buffer[buffLen -1] = 0;  //make sure it is null terminated even if truncated.
    QTSS_UnlockStdLib();

    return buffer;
#endif
}

char *qtss_asctime(const struct tm *timeptr, char* buffer, int buffLen)
{
#if __MacOSX__
    Assert(buffLen >= 26);
    return ::asctime_r(timeptr, buffer);
#else
    QTSS_LockStdLib();
    (void) ::strncpy( buffer, ::asctime(timeptr), buffLen);
    buffer[buffLen -1] = 0;  //make sure it is null terminated even if truncated.
    QTSS_UnlockStdLib();

    return buffer;
#endif
}

struct tm *qtss_gmtime(const time_t *timep, struct tm *result)
{
#if __MacOSX__
    return ::gmtime_r(timep, result);
#else
   QTSS_LockStdLib();
    struct tm *time_result = ::gmtime(timep);
    *result = *time_result;
    QTSS_UnlockStdLib();
    
    return result;
#endif
}

struct tm *qtss_localtime(const time_t *timep, struct tm *result)
{
#if __MacOSX__
    return ::localtime_r(timep, result);
#else
    QTSS_LockStdLib();
    struct tm *time_result = ::localtime(timep);
    *result = *time_result;
    QTSS_UnlockStdLib();
    
    return result;
#endif
}

