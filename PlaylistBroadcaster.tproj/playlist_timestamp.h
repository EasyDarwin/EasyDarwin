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
 *
 *  History:
 *      26-Oct-1998 Umesh Vaishampayan (umeshv@apple.com)
 *          Made the header c++ friendly.
 *
 *      23-Oct-1998 Umesh Vaishampayan (umeshv@apple.com)
 *          Created.
 */

#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Get a 64 bit timestamp */
extern SInt64 timestamp(void);

struct timescale {
    SInt64 tsc_numerator;
    SInt64 tsc_denominator;
};

/*
 * Get numerator and denominator to convert value returned 
 * by timestamp() to microseconds
 */
extern void utimescale(struct timescale *tscp);

#ifdef __cplusplus
}
#endif

#endif /* _TIMESTAMP_H_ */
