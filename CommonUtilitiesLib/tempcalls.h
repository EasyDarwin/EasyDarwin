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
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER
// these optimized calls are implemented in stdclib and declared in sys/ev.h on 10.5 and later for better performance.

/*    
    int watchevent(struct eventreq *u_req, int u_eventmask);
    int waitevent(struct eventreq *u_req, struct timeval *tv);
    int modwatch(struct eventreq *u_req, int u_eventmask);
*/

#else 
    // use syscall for the performance calls.
    #define watchevent(a,b) syscall(231,(a),(b))
    #define waitevent(a,b) syscall(232,(a),(b))
    #define modwatch(a,b) syscall(233,(a),(b))
#endif //AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER
