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
	 File:       UDPDemuxer.cpp

	 Contains:   Implements objects defined in UDPDemuxer.h



 */

#include "UDPDemuxer.h"

#include <errno.h>


OS_Error UDPDemuxer::RegisterTask(UInt32 inRemoteAddr, UInt16 inRemotePort,
	UDPDemuxerTask* inTaskP)
{
	Assert(NULL != inTaskP);
	OSMutexLocker locker(&fMutex);
	if (this->GetTask(inRemoteAddr, inRemotePort) != NULL)
		return EPERM;
	inTaskP->set(inRemoteAddr, inRemotePort);
	fHashTable.Add(inTaskP);
	return OS_NoErr;
}

OS_Error UDPDemuxer::UnregisterTask(UInt32 inRemoteAddr, UInt16 inRemotePort,
	UDPDemuxerTask* inTaskP)
{
	OSMutexLocker locker(&fMutex);
	//remove by executing a lookup based on key information
	UDPDemuxerTask* theTask = this->GetTask(inRemoteAddr, inRemotePort);

	if ((NULL != theTask) && (theTask == inTaskP))
	{
		fHashTable.Remove(theTask);
		return OS_NoErr;
	}
	else
		return EPERM;
}

UDPDemuxerTask* UDPDemuxer::GetTask(UInt32 inRemoteAddr, UInt16 inRemotePort)
{
	UDPDemuxerKey theKey(inRemoteAddr, inRemotePort);
	return fHashTable.Map(&theKey);
}
