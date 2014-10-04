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
    File:       DirectoryInfo.h

    Contains:   Stores an array of client sessions, # of client sessions,
				and the home directory
                    
    
*/
#ifndef _DIRECTORYINFO_H_
#define _DIRECTORYINFO_H_
#include "QTSS.h"
#include "StrPtrLen.h"
#include "OSRef.h"
#include "OSMemory.h"
#include "PLDoubleLinkedList.h"

class SessionListElement {
    public:
        SessionListElement(QTSS_ClientSessionObject *inSessionPtr) { fSession = *inSessionPtr; }
		
        virtual ~SessionListElement() { fSession = NULL; }
		
		Bool16 Equal(QTSS_ClientSessionObject* inSessionPtr);
        UInt32 CurrentBitRate();
	
	private:
		QTSS_ClientSessionObject fSession;
};

class DirectoryInfo
{
    public:
	DirectoryInfo(StrPtrLen *inHomeDir):fNumSessions(0), fHomeDir(inHomeDir->GetAsCString())
	{
		fClientSessionList = NEW PLDoubleLinkedList<SessionListElement>;
		fRef.Set(fHomeDir, (void *)this);
	}
	
	~DirectoryInfo();
	OSRef*	GetRef() { return &fRef; }
	void	AddSession(QTSS_ClientSessionObject *sessionPtr);
	void	RemoveSession(QTSS_ClientSessionObject *sessionPtr);
	UInt64	CurrentTotalBandwidthInKbps();
	UInt32	NumSessions() { return fNumSessions; }

	private:
		OSRef											fRef;
		OSMutex											fMutex;
		PLDoubleLinkedList<SessionListElement>			*fClientSessionList;
		UInt32											fNumSessions;
		StrPtrLen										fHomeDir;
};


#endif // _DIRECTORYINFO_H_ 
