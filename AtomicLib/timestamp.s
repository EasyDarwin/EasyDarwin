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
 *	File:	timestamp.s
 *
 *	History:
 *		12-Feb-1999 Umesh Vaishampayan (umeshv@apple.com)
 *			Integrated scaledtimestamp() written by
 *			Joe Sokol (sokol1@apple.com)
 *
 *		23-Oct-1998	Umesh Vaishampayan (umeshv@apple.com)
 *			Created.
 */

#include <architecture/ppc/asm_help.h>

/*
 *	long long timestamp(void)
 *
 *		Read the PPC timebase.
 */
LEAF(_timestamp)
Lagain:
	mftbu   r3
	mftb    r4
	mftbu   r6
	cmpw    r6, r3
	bne-    Lagain
	blr


/*
 *	long long scaledtimestamp(double scale)
 *
 *		Read the PPC timebase. Convert the time base value based on
 *		scale.
 *
 *		Caveat: scale can not be 0, NaN, Inf. It's upto the caller
 *				to validate scale before calling this.
 */

LEAF(_scaledtimestamp)
Lagain1:
	mftbu   r3
	mftb    r4
	mftbu   r6
	cmpw    r6,r3
	bne-    Lagain1

; r3 and r4 have the time base.
; convert the long long value to double
L_LLtoD:	
	cntlzw	r0,r3
	cmplwi	cr0,r0,31
	bc	12,1,L2
	subfic	r10,r0,63
	subfic	r11,r10,52
	subfic	r0,r11,32
	srw	r0,r4,r0
	slw	r9,r3,r11
	or	r3,r9,r0
	b	L6
L2:
	cntlzw	r0,r4
	subfic	r10,r0,31
	subfic	r11,r10,52
	cmplwi	cr0,r11,31
	bc	4,1,L4
	addi	r0,r11,-32
	slw	r3,r4,r0
	li	r4,0
	b	L3
L4:
	subfic	r0,r11,32
	srw	r3,r4,r0
L6:
	slw	r4,r4,r11
L3:
	addi	r0,r10,1023
	slwi	r0,r0,20
	rlwimi	r3,r0,0,0,11
	stw	r3,-8(r1)
	stw	r4,-4(r1)
	lfd	f0,-8(r1)		; load the double representation of time base

	fmul	f0,f0,f1	; f1 has scale to convert timestamp

; convert the double to long long
L_DtoLL:		
	stfd	f0,-8(r1)
	lwz	r3,-8(r1)
	lwz	r4,-4(r1)
	srwi	r0,r3,20
	rlwinm	r9,r3,0,12,31
	subfic	r8,r0,1075
	cmplwi	cr0,r8,31
	oris	r3,r9,0x10
	bc	4,1,L8
	addi	r0,r8,-32
	srw	r4,r3,r0
	li	r3,0
	blr
L8:
	subfic	r0,r8,32
	slw	r0,r3,r0
	srw	r9,r4,r8
	or	r4,r9,r0
	srw	r3,r3,r8
	blr

