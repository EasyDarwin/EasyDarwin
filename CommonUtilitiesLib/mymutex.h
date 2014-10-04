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
/*
    File:       mymutex.h

    Contains:   xxx put contents here xxx

    Written by: Greg Vaughan

    Change History (most recent first):

         <2>    10/27/99    GBV     update for beaker

    To Do:
*/

#ifndef _MYMUTEX_H_
#define _MYMUTEX_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <mach/mach.h>
#include <pthread.h>

typedef void* mymutex_t;
mymutex_t mymutex_alloc();
void mymutex_free(mymutex_t);

void mymutex_lock(mymutex_t);
int mymutex_try_lock(mymutex_t);
void mymutex_unlock(mymutex_t);

#ifdef __cplusplus
}
#endif

#endif
