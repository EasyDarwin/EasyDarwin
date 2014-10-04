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
 *      11-Feb-1999 Umesh Vaishampayan (umeshv@apple.com)
 *          Added atomic_or().
 *
 *      26-Oct-1998 Umesh Vaishampayan (umeshv@apple.com)
 *          Made the header c++ friendly.
 *
 *      12-Oct-1998 Umesh Vaishampayan (umeshv@apple.com)
 *          Changed simple_ to spin_ so as to coexist with cthreads till
 *          the merge to the system framework.
 *
 *      8-Oct-1998  Umesh Vaishampayan (umeshv@apple.com)
 *          Created from the kernel code to be in a dynamic shared library.
 *          Kernel code created by: Bill Angell (angell@apple.com)
 */

#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Locking routines */

struct spin_lock {          /* sizeof cache line */
    unsigned int lock_data;
    unsigned int pad[7];
};

typedef struct spin_lock *spin_lock_t;

extern void spin_lock_init(spin_lock_t);

extern void spin_lock_unlock(spin_lock_t);

extern unsigned int spin_lock_lock(spin_lock_t);

extern unsigned int spin_lock_bit(spin_lock_t, unsigned int bits);

extern unsigned int spin_unlock_bit(spin_lock_t, unsigned int bits);

extern unsigned int spin_lock_try(spin_lock_t);

extern unsigned int spin_lock_held(spin_lock_t);

/* Other atomic routines */

extern unsigned int compare_and_store(unsigned int oval,
                                unsigned int nval, unsigned int *area);

extern unsigned int atomic_add(unsigned int *area, int val);

extern unsigned int atomic_or(unsigned int *area, unsigned int mask);

extern unsigned int atomic_sub(unsigned int *area, int val);

extern void queue_atomic(unsigned int *anchor,
                    unsigned int *elem, unsigned int disp);

extern void queue_atomic_list(unsigned int *anchor,
                        unsigned int *first, unsigned int *last,
                        unsigned int disp);

extern unsigned int *dequeue_atomic(unsigned int *anchor, unsigned int disp);

#ifdef __cplusplus
}
#endif

#endif /* _ATOMIC_H_ */
