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
    File:       DirectoryInfo.cpp

    Contains:   Implementation of class defined in .h file
                    
    
*/

#include "DirectoryInfo.h"

Bool16 SessionListElement::Equal(QTSS_ClientSessionObject* inSessionPtr)
{
	UInt32 *theSessionID = 0;
	UInt32 *inSessionID = 0;
	UInt32 theLen = 0;
	
	(void)QTSS_GetValuePtr(fSession, qtssCliSesCounterID, 0, (void **)&theSessionID, &theLen);
	Assert(theLen != 0);
	(void)QTSS_GetValuePtr(*inSessionPtr, qtssCliSesCounterID, 0, (void **)&inSessionID, &theLen);
	Assert(theLen != 0);
	
	if (*theSessionID == *inSessionID)
		return true;
	
	return false;
}

UInt32 SessionListElement::CurrentBitRate()
{
	UInt32 *theBitRate = 0;
	UInt32 theLen = 0;
	
	(void)QTSS_GetValuePtr(fSession, qtssCliSesCurrentBitRate, 0, (void **)&theBitRate, &theLen);
	Assert(theLen != 0);
		
	return *theBitRate;
}

static bool IsSession(PLDoubleLinkedListNode<SessionListElement>* node,  void* userData)
{
	/*
        used by ForEachUntil to find a SessionListElement with a given Session
        userData is a pointer to the QTSS_ClientSessionObject we want to find
        
    */
	return node->fElement->Equal((QTSS_ClientSessionObject *)userData);
}

static void AddCurrentBitRate(PLDoubleLinkedListNode<SessionListElement>* node,  void* userData)
{
	*(UInt64 *)userData += node->fElement->CurrentBitRate();
}

DirectoryInfo::~DirectoryInfo()
{
	fMutex.Lock();
	fClientSessionList->ClearList();
	fNumSessions = 0;
	fHomeDir.Delete();
	fMutex.Unlock();
}

void DirectoryInfo::AddSession(QTSS_ClientSessionObject *sessionPtr)
{
	fMutex.Lock();
	
	SessionListElement *theElement = NEW SessionListElement(sessionPtr);
	PLDoubleLinkedListNode<SessionListElement> *sessionNode = new PLDoubleLinkedListNode<SessionListElement> (theElement);
	fClientSessionList->AddNode(sessionNode);
	fNumSessions++;
	
	fMutex.Unlock();	
}

void DirectoryInfo::RemoveSession(QTSS_ClientSessionObject *sessionPtr)
{
	fMutex.Lock();
	
	PLDoubleLinkedListNode<SessionListElement> *node = NULL;
    
    node = fClientSessionList->ForEachUntil(IsSession, (void *)sessionPtr);
	if (node != NULL)
	{
		fClientSessionList->RemoveNode(node);
		fNumSessions--;
	}
	
	fMutex.Unlock();	
}

UInt64 DirectoryInfo::CurrentTotalBandwidthInKbps()
{
	fMutex.Lock();
	UInt64 totalBandwidth = 0;
	fClientSessionList->ForEach(AddCurrentBitRate, &totalBandwidth);
	fMutex.Unlock();
		
	return (totalBandwidth/1024);
}
