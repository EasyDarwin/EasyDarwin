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
 *	File:	atomic_subr.s
 *
 *	History:
 *		11-Feb-1999 Umesh Vaishampayan (umeshv@apple.com)
 *			Added atomic_or().
 *
 *		12-Oct-1998	Umesh Vaishampayan (umeshv@apple.com)
 *			Changed simple_ to spin_ so as to coexist with cthreads till
 *			the merge to the system framework.
 *
 *		8-Oct-1998	Umesh Vaishampayan (umeshv@apple.com)
 *			Created from the kernel code to be in a dynamic shared library.
 *			Kernel code created by: Bill Angell	(angell@apple.com)
 */

#include <architecture/ppc/asm_help.h>

/*
 *	void spin_lock_init(spin_lock_t)
 *
 *		Initialize a spin lock.
 *		These locks should be cache aligned and a multiple of cache size.
 */
LEAF(_spin_lock_init)
		li	r0,	0				/* set lock to free == 0 */
		stw	r0,	0(r3)			/* Initialize the lock */
		blr

/*
 *	void spin_lock_unlock(spin_lock_t)
 *
 *		Unconditionally release lock.
 */
LEAF(_spin_lock_unlock)
		sync					/* Flush writes done under lock */
		li	r0,	0				/* set lock to free */
		stw	r0,	0(r3)
		blr

/*
 *	unsigned int spin_lock_lock(spin_lock_t)
 *
 *		Try to acquire spin-lock. Return success (1).
 */
LEAF(_spin_lock_lock)
		mr		r5,r3				/* Get the address of the lock */

Lcktry:
		lwarx	r6,0,r5				/* Grab the lock value */
		li		r3,1				/* Set the return value */
		mr.		r6,r6				/* Is it locked? */
		bne-	Lcksniff			/* Yeah, wait for it to clear... */
		stwcx.	r3,0,r5				/* Try to sieze that darn lock */
		beq+	Lckgot				/* We got it, yahoo... */
		b		Lcktry				/* Try again if the store failed... */

Lcksniff:
		lwz		r3,0(r5)			/* Get that lock in here */
		mr.		r3,r3				/* Is it free yet? */
		beq+	Lcktry				/* Yeah, try for it again... */
		b		Lcksniff			/* keep trying... */

Lckgot:
		isync						/* Make sure we don't use a */
									/* speculativily loaded value */
		blr

/*
 *	unsigned int spin_lock_bit(spin_lock_t, unsigned int bits)
 *
 *		Try to acquire spin-lock. The second parameter is the bit mask to 
 *		test and set. multiple bits may be set.
 *		Return success (1).
 */
LEAF(_spin_lock_bit)
Lbittry:
		lwarx	r6,0,r3				/* Grab the lock value */
		and.	r0,r6,r4			/* See if any of the lock bits are on */
		or		r6,r6,r4			/* Turn on the lock bits */
		bne-	Lbitsniff			/* Yeah, wait for it to clear... */
		stwcx.	r6,0,r3				/* Try to sieze that there durn lock */
		beq+	Lbitgot				/* We got it, yahoo... */
		b		Lbittry				/* Try again if the store failed... */

Lbitsniff:
		lwz		r6,0(r3)			/* Get that lock in here */
		and.	r0,r6,r4			/* See if any of the lock bits are on */
		beq+	Lbittry				/* Yeah, try for it again... */
		b		Lbitsniff			/* keep trying... */

Lbitgot:
		li		r3,1				/* Set good return code */
		isync						/* Make sure we don't use a */
									/* speculativily loaded value */
		blr


/*
 *	unsigned int spin_unlock_bit(spin_lock_t, unsigned int bits)
 *
 *		Release bit based spin-lock. The second parameter is the bit mask to 
 *		clear. Multiple bits may be cleared.
 */
LEAF(_spin_unlock_bit)
Lubittry:
		lwarx	r0,0,r3				/* Grab the lock value */
		andc	r0,r0,r4			/* Clear the lock bits */
		stwcx.	r0,0,r3				/* Try to clear that there durn lock */
		bne-	Lubittry			/* Try again, couldn't save it... */
		blr							/* Leave... */			

/*
 *	unsigned int spin_lock_try(spin_lock_t)
 *
 *		Try to acquire spin-lock. Return success (1) or failure (0).
 */
LEAF(_spin_lock_try)
		li	r4,	1					/* value to be stored... 1==taken */

L_lock_try_loop:	
		lwarx	r5,	0,r3			/* Ld from addr of arg and reserve */
		cmpwi	r5,	0				/* TEST... */
		bne-	L_lock_try_failed	/* branch if taken. Predict free */

		stwcx.  r4,	0,r3			/* And SET (if still reserved) */
		bne-	L_lock_try_loop		/* If set failed, loop back */

		isync
		li		r3,1				/* Set that the lock was free */
		blr

L_lock_try_failed:
		li		r3,0				/* FAILURE - lock was taken */
		blr

/*
 *	unsigned int spin_lock_held(spin_lock_t)
 *
 *      Return 1 if lock is held
 *      N.B.  Racy, of course.
 */
LEAF(_spin_lock_held)
		isync						/* Make sure we don't use a */
									/* speculativily fetched lock */
		lwz		r3, 0(r3)			/* Return value of lock */
		blr

/*
 *	unsigned int compare_and_store(unsigned int oval,
 *									unsigned int nval, unsigned int *area)
 *
 *		Compare oval to area if equal, store nval, and return true
 *		else return false and no store
 *		This is an atomic operation
 */
LEAF(_compare_and_store)
		mr		r6,r3				/* Save the old value */			

Lcstry:
		lwarx	r9,0,r5				/* Grab the area value */
		li		r3,1				/* Assume it works */
		cmplw	cr0,r9,r6			/* Does it match the old value? */
		bne-	Lcsfail				/* No, it must have changed... */
		stwcx.	r4,0,r5				/* Try to save the new value */
		bne-	Lcstry				/* Didn't get it, try again... */
		isync						/* Just hold up prefetch */
		blr							/* Return... */

Lcsfail:
		li		r3,0				/* Set failure */
		blr

/*
 *	unsigned int atomic_add(unsigned int *area, int val)
 *
 *		Atomically add the second parameter to the first.
 *		Returns the result.
 */
LEAF(_atomic_add)
		mr		r6,r3				/* Save the area */			

Laddtry:
		lwarx	r3,0,r6				/* Grab the area value */
		add		r3,r3,r4			/* Add the value */
		stwcx.	r3,0,r6				/* Try to save the new value */
		bne-	Laddtry				/* Didn't get it, try again... */
		blr							/* Return... */


/*
 *	unsigned int atomic_or(unsigned int *area, unsigned int mask)
 *
 *		Atomically or the mask into *area.
 *		Returns the old value.
 */
LEAF(_atomic_or)
		mr		r6,r3				/* Save the area */			

Lortry:
		lwarx	r3,0,r6				/* Grab the area value */
		or		r5,r3,r4			/* or the mask */
		stwcx.	r5,0,r6				/* Try to save the new value */
		bne-	Lortry				/* Didn't get it, try again... */
		blr							/* Return the old value... */


/*
 *	unsigned int atomic_sub(unsigned int *area, int val)
 *
 *		Atomically subtract the second parameter from the first.
 *		Returns the result.
 */
LEAF(_atomic_sub)
		mr		r6,r3				/* Save the area */			

Lsubtry:
		lwarx	r3,0,r6				/* Grab the area value */
		sub		r3,r3,r4			/* Subtract the value */
		stwcx.	r3,0,r6				/* Try to save the new value */
		bne-	Lsubtry				/* Didn't get it, try again... */
		blr							/* Return... */

/*
 *	void queue_atomic(unsigned int * anchor,
 *						unsigned int * elem, unsigned int disp)
 *
 *		Atomically inserts the element at the head of the list
 *		anchor is the pointer to the first element
 *		element is the pointer to the element to insert
 *		disp is the displacement into the element to the chain pointer
 */
LEAF(_queue_atomic)
		mr		r7,r4				/* Make end point the same as start */
		mr		r8,r5				/* Copy the displacement also */
		b		Lqueue_comm		/* Join common code... */

/*
 *	void queue_atomic_list(unsigned int * anchor,
 *							unsigned int * first, unsigned int * last,
 *							unsigned int disp)
 *
 *		Atomically inserts the list of elements at the head of the list
 *		anchor is the pointer to the first element
 *		first is the pointer to the first element to insert
 *		last is the pointer to the last element to insert
 *		disp is the displacement into the element to the chain pointer
 */
LEAF(_queue_atomic_list)
		mr		r7,r5				/* Make end point the same as start */
		mr		r8,r6				/* Copy the displacement also */

Lqueue_comm:
		lwarx	r9,0,r3				/* Pick up the anchor */
		stwx	r9,r8,r7			/* Chain that to the end of the new stuff */
		stwcx.	r4,0,r3				/* Try to chain into the front */
		bne-	Lqueue_comm			/* Didn't make it, try again... */
		blr							/* Return... */

/*
 *	unsigned int *dequeue_atomic(unsigned int *anchor, unsigned int disp)
 *
 *		Atomically removes the first element in a list and returns it.
 *		anchor is the pointer to the first element
 *		disp is the displacement into the element to the chain pointer
 *		Returns element if found, 0 if empty.
 */
LEAF(_dequeue_atomic)
		mr		r5,r3				/* Save the anchor */

Ldequeue_comm:
		lwarx	r3,0,r5				/* Pick up the anchor */
		mr.		r3,r3				/* Is the list empty? */
		beqlr-						/* Leave it list empty... */
		lwzx	r9,r4,r3			/* Get the next in line */
		stwcx.	r9,0,r5				/* Try to chain into the front */
		bne-	Ldequeue_comm	/* Didn't make it, try again... */
		blr							/* Return... */

