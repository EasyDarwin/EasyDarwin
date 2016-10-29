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
	 File:       OSRef.h

	 Contains:   Class supports creating unique string IDs to object pointers. A grouping
				 of an object and its string ID may be stored in an OSRefTable, and the
				 associated object pointer can be looked up by string ID.

				 Refs can only be removed from the table when no one is using the ref,
				 therefore allowing clients to arbitrate access to objects in a preemptive,
				 multithreaded environment.




 */

#ifndef _OSREF_H_
#define _OSREF_H_

#include "StrPtrLen.h"
#include "OSHashTable.h"
#include "OSCond.h"
#include "OSHeaders.h"

class OSRefKey;

class OSRefTableUtils
{
private:

	static UInt32   HashString(StrPtrLen* inString);

	friend class OSRef;
	friend class OSRefKey;
};

class OSRef
{
public:

	OSRef() : fObjectP(nullptr), fRefCount(0), fHashValue(0), fNextHashEntry(nullptr)
	{
#if DEBUG
		fInATable = false;
		fSwapCalled = false;
#endif          
	}
	OSRef(const StrPtrLen& inString, void* inObjectP)
		: fRefCount(0), fNextHashEntry(nullptr)
	{
		Set(inString, inObjectP);
	}
	~OSRef() {}

	void Set(const StrPtrLen& inString, void* inObjectP)
	{
#if DEBUG
		fInATable = false;
		fSwapCalled = false;
#endif          
		fString = inString; fObjectP = inObjectP;
		fHashValue = OSRefTableUtils::HashString(&fString);
	}

#if DEBUG
	bool  IsInTable() { return fInATable; }
#endif
	void**  GetObjectPtr() { return &fObjectP; }
	void*   GetObject() { return fObjectP; }
	UInt32  GetRefCount() { return fRefCount; }
	StrPtrLen *GetString() { return &fString; }
private:

	//value
	void*   fObjectP;
	//key
	StrPtrLen   fString;

	//refcounting
	UInt32  fRefCount;
#if DEBUG
	bool  fInATable;
	bool  fSwapCalled;
#endif
	OSCond  fCond;//to block threads waiting for this ref.

	UInt32              fHashValue;
	OSRef*              fNextHashEntry;

	friend class OSRefKey;
	friend class OSHashTable<OSRef, OSRefKey>;
	friend class OSHashTableIter<OSRef, OSRefKey>;
	friend class OSRefTable;

};


class OSRefKey
{
public:

	//CONSTRUCTOR / DESTRUCTOR:
	OSRefKey(StrPtrLen* inStringP)
		: fStringP(inStringP)
	{
		fHashValue = OSRefTableUtils::HashString(inStringP);
	}

	~OSRefKey() {}


	//ACCESSORS:
	StrPtrLen*  GetString() { return fStringP; }


private:

	//PRIVATE ACCESSORS:    
	SInt32      GetHashKey() { return fHashValue; }

	//these functions are only used by the hash table itself. This constructor
	//will break the "Set" functions.
	OSRefKey(OSRef* elem) : fStringP(&elem->fString),
		fHashValue(elem->fHashValue)
	{
	}

	friend int operator ==(const OSRefKey& key1, const OSRefKey& key2)
	{
		if (key1.fStringP->Equal(*key2.fStringP))
			return true;
		return false;
	}

	//data:
	StrPtrLen *fStringP;
	UInt32  fHashValue;

	friend class OSHashTable<OSRef, OSRefKey>;
};

typedef OSHashTable<OSRef, OSRefKey> OSRefHashTable;
typedef OSHashTableIter<OSRef, OSRefKey> OSRefHashTableIter;

class OSRefTable
{
public:

	enum
	{
		kDefaultTableSize = 1193 //UInt32
	};

	//tableSize doesn't indicate the max number of Refs that can be added
	//(it's unlimited), but is rather just how big to make the hash table
	OSRefTable(UInt32 tableSize = kDefaultTableSize) : fTable(tableSize), fMutex() {}
	~OSRefTable() {}

	//Allows access to the mutex in case you need to lock the table down
	//between operations
	OSMutex*    GetMutex() { return &fMutex; }
	OSRefHashTable* GetHashTable() { return &fTable; }

	//Registers a Ref in the table. Once the Ref is in, clients may resolve
	//the ref by using its string ID. You must setup the Ref before passing it
	//in here, ie., setup the string and object pointers
	//This function will succeed unless the string identifier is not unique,
	//in which case it will return QTSS_DupName
	//This function is atomic wrt this ref table.
	OS_Error        Register(OSRef* ref);

	// RegisterOrResolve
	// If the ID of the input ref is unique, this function is equivalent to
	// Register, and returns nullptr.
	// If there is a duplicate ID already in the map, this funcion
	// leave it, resolves it, and returns it.
	OSRef*              RegisterOrResolve(OSRef* inRef);

	//This function may block. You can only remove a Ref from the table
	//when the refCount drops to the level specified. If several threads have
	//the ref currently, the calling thread will wait until the other threads
	//stop using the ref (by calling Release, below)
	//This function is atomic wrt this ref table.
	void        UnRegister(OSRef* ref, UInt32 refCount = 0);

	// Same as UnRegister, but guarenteed not to block. Will return
	// true if ref was sucessfully unregistered, false otherwise
	bool      TryUnRegister(OSRef* ref, UInt32 refCount = 0);

	//Resolve. This function uses the provided key string to identify and grab
	//the Ref keyed by that string. Once the Ref is resolved, it is safe to use
	//(it cannot be removed from the Ref table) until you call Release. Because
	//of that, you MUST call release in a timely manner, and be aware of potential
	//deadlocks because you now own a resource being contended over.
	//This function is atomic wrt this ref table.
	OSRef*      Resolve(StrPtrLen*  inString);

	//Release. Release a Ref, and drops its refCount. After calling this, the
	//Ref is no longer safe to use, as it may be removed from the ref table.
	void        Release(OSRef*  inRef);

	// Swap. This atomically removes any existing Ref in the table with the new
	// ref's ID, and replaces it with this new Ref. If there is no matching Ref
	// already in the table, this function does nothing.
	//
	// Be aware that this creates a situation where clients may have a Ref resolved
	// that is no longer in the table. The old Ref must STILL be UnRegistered normally.
	// Once Swap completes sucessfully, clients that call resolve on the ID will get
	// the new OSRef object.
	void        Swap(OSRef* newRef);

	UInt32      GetNumRefsInTable() { UInt64 result = fTable.GetNumEntries(); Assert(result < kUInt32_Max); return (UInt32)result; }

private:


	//all this object needs to do its job is an atomic hashtable
	OSRefHashTable  fTable;
	OSMutex         fMutex;
};


class OSRefReleaser
{
public:

	OSRefReleaser(OSRefTable* inTable, OSRef* inRef) : fOSRefTable(inTable), fOSRef(inRef) {}
	~OSRefReleaser() { fOSRefTable->Release(fOSRef); }

	OSRef*          GetRef() { return fOSRef; }

private:

	OSRefTable*     fOSRefTable;
	OSRef*          fOSRef;
};



#endif //_OSREF_H_
