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

#ifndef _DSS_SYS_EV_H_
#define _DSS_SYS_EV_H_



#if !defined(__Win32__) && !defined(__solaris__) && !defined(__sgi__) && !defined(__osf__) && !defined(__hpux__)
    #include <sys/queue.h>
#endif

#if MACOSXEVENTQUEUE
    #include <sys/ev.h>
#else

#include "common.h"

typedef struct eventreq *er_t;


#ifdef _KERNEL

#define EV_RBYTES 0x1
#define EV_WBYTES 0x2
#define EV_RWBYTES (EV_RBYTES|EV_WBYTES)
#define EV_RCLOSED 0x4
#define EV_RCONN  0x8
#define EV_ERRORS  0x10
#define EV_WCLOSED 0x20
#define EV_WCONN   0x40
#define EV_OOBD    0x80
#define EV_OOBM    0x100

struct eventqelt {
  TAILQ_ENTRY(eventqelt)  ee_slist;
  TAILQ_ENTRY(eventqelt)  ee_plist;
  struct eventreq  ee_req;
  struct proc *    ee_proc;
  u_int            ee_flags;
#define EV_QUEUED 1
  u_int            ee_eventmask;
  struct socket   *ee_sp;
};

#endif /* _KERNEL */


int select_watchevent(struct eventreq *req, int which);
int select_modwatch(struct eventreq *req, int which);
int select_waitevent(struct eventreq *req, void* onlyForMOSX);
void select_startevents();
int select_removeevent(int which);

#endif /* !MACOSXEVENTQUEUE */

#endif /* _DSS_SYS_EV_H_ */
