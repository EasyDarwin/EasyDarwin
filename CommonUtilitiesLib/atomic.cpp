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

#include "atomic.h"
#include "OSMutex.h"

static OSMutex sAtomicMutex;

unsigned int atomic_add(unsigned int* area, int val)
{
	OSMutexLocker locker(&sAtomicMutex);
	*area += val;
	return *area;
}

unsigned int atomic_sub(unsigned int* area, int val)
{
	return atomic_add(area, -val);
}

unsigned int atomic_or(unsigned int* area, unsigned int val)
{
	unsigned int oldval;

	OSMutexLocker locker(&sAtomicMutex);
	oldval = *area;
	*area = oldval | val;
	return oldval;
}

unsigned int compare_and_store(unsigned int oval, unsigned int nval, unsigned int* area)
{
	int rv;
	OSMutexLocker locker(&sAtomicMutex);
	if (oval == *area)
	{
		rv = 1;
		*area = nval;
	}
	else
		rv = 0;
	return rv;
}
