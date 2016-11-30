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
	 File:       QTSSMP3StreamingModule.h

	 Contains:   Handle ShoutCast/IceCast-style MP3 streaming.

	 Written by: Steve Ussery

 */

#ifndef __QTSSMP3STREAMINGMODULE_H__
#define __QTSSMP3STREAMINGMODULE_H__

#include "QTSS.h"
#include "OSQueue.h"
#include "OSMutex.h"
#include "OSHashTable.h"

extern "C"
{
	EXPORT QTSS_Error QTSSMP3StreamingModule_Main(void* inPrivateArgs);
}

#define kURLBufferSize 1024
#define kSongNameBufferSize 1024
#define kHeaderBufferSize 1024
#define kRequestBufferSize 512
#define kHostNameBufferSize 256
#define kUserAgentBufferSize 256
#define kClientMetaInt 24576

// The client poll interval in milliseconds
#define kClientPollInterval 253

// FORWARD CLASS DEFINES

class MP3ClientSessionRef;
class MP3ClientSession;
class MP3ClientQueue;
class MP3BroadcasterSession;
class MP3BroadcasterQueue;
class MP3SessionRef;
class MP3SessionRefKey;
class MP3SessionTable;

enum {
	kMP3UndefinedSessionType = 0,
	kMP3BroadcasterSessionType,
	kMP3ClientSessionType
};

//
// MP3Session -- This is a base class to hold all the MP3 Session state info.
// We will subclass it for MP3 Broadcaster sessions and MP3 Client sessions.
// (See the MP3BroadcasterSession & MP3ClientSession classes defined below.)
//
class MP3Session
{
public:
	MP3Session();

	MP3Session(QTSS_RTSPSessionObject sess, QTSS_StreamRef stream);

	virtual         ~MP3Session();

	// The IsA() method is a mechanism for subclasses of this class to
	// identify their type. It is kMP3UndefinedSessionType for this
	// base class.

	virtual UInt8   IsA() const;

	// Session data field accessor methods

	QTSS_RTSPSessionObject  GetSession() { return fSession; }

	void            SetSessionID(UInt32 sessID) { fSessID = sessID; }

	UInt32          GetSessionID() { return fSessID; }

	QTSS_StreamRef  GetStreamRef() { return fStream; }

	UInt16          GetState() { return fState; }

	void            SetState(UInt16 state) { fState = state; }

	UInt16          GetResult() { return fResult; }

	void            SetResult(UInt16 result) { fResult = result; }

	static SInt32   GetTotalNumMP3Sessions() { return sTotalNumMP3Sessions; }

protected:

	void            SetSession(QTSS_RTSPSessionObject session) { fSession = session; }

	void            SetStreamRef(QTSS_StreamRef stream) { fStream = stream; }

private:

	static SInt32   sTotalNumMP3Sessions;

	UInt16          fState;
	UInt16          fResult;
	QTSS_StreamRef  fStream;
	QTSS_RTSPSessionObject  fSession;
	UInt32          fSessID;
};

//
// MP3BroadcasterSession -- This is a class to hold all the MP3 Broadcaster
// session-related state info. There is a global queue of these managed by the
// server. Each instance of this class must have a unique mountpoint name
// string which clients will use to identify the broadcast stream they are
// "tuning" into.
//
class MP3BroadcasterSession : public MP3Session
{
public:


	// MP3BroadcasterSession states

	enum {
		kBroadcasterInitState = 0,
		kBroadcasterValidatePasswordState,
		kBroadcasterGetHeaderState,
		kBroadcasterRecvDataState,
		kBroadcasterShutDownState
	};

	MP3BroadcasterSession(QTSS_RTSPSessionObject sess, QTSS_StreamRef stream);

	virtual         ~MP3BroadcasterSession();

	// The IsA() method is a mechanism for subclasses of the MP3Session class
	// to identify their type. It is kMP3BroadcasterSessionType for this
	// subclass.

	virtual UInt8   IsA() const;

	// Session state control methods

	QTSS_Error      ExecuteState();

	void            InitBroadcastSessionState();

	void            AcceptBroadcastSessionState();

	void            AcceptPasswordState();

	void            AcceptDataStreamState();

	void            ShutDownState();

	// MP3 Client handling methods

	QTSS_Error      AddClient(QTSS_RTSPSessionObject sess, QTSS_StreamRef stream);

	QTSS_Error      RemoveClient(QTSS_RTSPSessionObject sess);

	Bool16          IsMyClient(QTSS_RTSPSessionObject sess);

	// Session data field accessor methods

	void            SetMountpoint(char* mp);

	void            SetMountpoint(StrPtrLen& mp);

	Bool16          MountpointEqual(char* mp);

	Bool16          MountpointEqual(StrPtrLen& mp);

	char*           GetMountpoint() { return fMountpoint; }

	char*           GetHeader() { return fHeader; }

	void            SetSongName(char* sn);

	char*           GetSongName() { return fSongName; }

	// Session data handlers

	QTSS_Error      SendOKResponse();

	QTSS_Error      GetBroadcastHeaders();

	QTSS_Error      GetBroadcastData();

	QTSS_Error      SendClientsData();

	void            PreflightClients();

protected:

	void            TerminateHeaders();

private:

	MP3BroadcasterSession();

	MP3ClientQueue* fMP3ClientQueue;
	UInt32          fDataBufferLen;
	UInt32          fDataBufferSize;
	char            fMountpoint[kURLBufferSize];
	char            fHeader[kHeaderBufferSize];
	char            fSongName[kSongNameBufferSize];
	char*           fBuffer;
	OSMutex         fSongNameMutex;
	Bool16          fNewSongName;
};

//
// MP3ClientSession -- This is a class to hold all the MP3 client
// session-related state info. These class instances are always owned by a
// instance of the MP3BroadcasterSession class which has a queue of these.
//
class MP3ClientSession : public MP3Session
{
public:


	// MP3ClientSession states

	enum {
		kClientInitState = 0,
		kClientSendResponse,
		kClientSendDataState,
		kClientShutDownState
	};

	MP3ClientSession(QTSS_RTSPSessionObject sess,
		QTSS_StreamRef stream,
		MP3BroadcasterSession* owner);

	virtual         ~MP3ClientSession();

	// The IsA() method is a mechanism for subclasses of the MP3Session class
	// to identify their type. It is kMP3ClientSessionType for this subclass.

	virtual UInt8   IsA() const;

	// Session stream handlers

	QTSS_Error      SendResponse();

	QTSS_Error      SendMP3Data(char* buffer, UInt32 bufferlen);

	QTSS_Error      RetrySendData();

	QTSS_Error      SendMetaData();

	// Session data field accessor methods

	void            SetHeader(char* header);

	char*           GetHeader() { return fHeader; }

	char*           GetHostName() { return fHostName; }

	char*           GetUserAgent() { return fUserAgent; }

	void            SetSongName(char* sn);

	char*           GetSongName() { return fSongName; }

	MP3BroadcasterSession*  GetOwner() { return fOwner; }

	void            SetOwner(MP3BroadcasterSession* owner) { fOwner = owner; }

	Bool16          WasBlocked() { return fWasBlocked; }

	Bool16          WantsContentLength() { return fNeedsContentLength; }

	void            SetRequest(char* req) { ::strcpy(fRequestBuffer, req); }

	char*           GetRequest() { return fRequestBuffer; }

	UInt32          GetTotalCount() { return fBytesSent; }

	SInt64          GetConnectTime() { return fConnectTime; }

private:

	MP3ClientSession();

	void            UpdateBitRateInternal(const SInt64& curTime);

	void            ParseRequestParams(QTSS_StreamRef stream);

	UInt32          fBytesSent;
	UInt32          fCurrentBitRate;
	UInt32          fLastBitRateBytes;
	SInt64          fLastBitRateUpdateTime;
	SInt64          fConnectTime;
	UInt32          fSendCount;
	UInt32          fRetryCount;
	MP3BroadcasterSession*  fOwner;
	char            fHeader[kHeaderBufferSize];
	char            fHostName[kHostNameBufferSize];
	char            fUserAgent[kUserAgentBufferSize];
	char            fSongName[kSongNameBufferSize];
	OSMutex         fSongNameMutex;
	Bool16          fNewSongName;
	Bool16          fWasBlocked;
	SInt64          fBlockTime;
	Bool16          fNeedsContentLength;
	Bool16          fWantsMetaData;
	char            fRequestBuffer[kRequestBufferSize];
	QTSS_Object     fQTSSObject;
};

//
// MP3SessionRef -- This class is just a wrapper class for handling the
// mapping of RTSP Session refs to the corresponding MP3 Session class refs.
// It will be the an element of our hash table in the MP3SessionTable class.
//
class MP3SessionRef
{
public:
	MP3SessionRef(MP3Session* mp3Session);

	~MP3SessionRef();

private:

	MP3Session*     GetMP3Session() { return fMP3Session; }

	QTSS_RTSPSessionObject  GetSession() { return fMP3Session->GetSession(); }

	MP3SessionRef*  fNextHashEntry;
	PointerSizedInt fHashValue;
	MP3Session*     fMP3Session;

	friend class MP3SessionRefKey;
	friend class OSHashTable<MP3SessionRef, MP3SessionRefKey>;
	friend class OSHashTableIter<MP3SessionRef, MP3SessionRefKey>;
	friend class MP3SessionTable;
};

//
// MP3SessionRefKey -- This class is used to generate hash keys for looking
// up values in our MP3SessionTable.
//
class MP3SessionRefKey
{
public:
	MP3SessionRefKey(MP3SessionRef* mp3SessRef);

	MP3SessionRefKey(QTSS_RTSPSessionObject rtspSessRef);

	~MP3SessionRefKey();

private:

	PointerSizedInt	GetHashKey() { return fHashValue; }

	friend int operator ==(const MP3SessionRefKey &key1, const MP3SessionRefKey &key2)
	{
		return (key1.fHashValue == key2.fHashValue);
	}

	MP3SessionRef*  fKeyValue;
	PointerSizedInt fHashValue;
	MP3Session*     fMP3Session;

	friend class OSHashTable<MP3SessionRef, MP3SessionRefKey>;
};

typedef OSHashTable<MP3SessionRef, MP3SessionRefKey> MP3SessRefHashTable;
typedef OSHashTableIter<MP3SessionRef, MP3SessionRefKey> MP3SessRefHashTableIter;

//
// MP3SessionTable -- This class provides a way to map RTSP Session references
// into the corresponding MP3Session class instances if any.
//
class MP3SessionTable
{
public:

	enum
	{
		kDefaultTableSize = 19
	};

	// tableSize is the number of hash buckets not the number of entries.

	MP3SessionTable(UInt32 tableSize = kDefaultTableSize);

	~MP3SessionTable();

	UInt32          GetNumSessionsInTable() { return (UInt32)fTable.GetNumEntries(); }

	// Attempt to add a new MP3Session ref to the table's map.
	// returns true on success or false if it fails.
	Bool16          RegisterSession(MP3Session* session);

	// Given an QTSS_RTSPSessionObject resolve it into a MP3Session class
	// reference. Returns NULL if there's none in out map.
	MP3Session*     Resolve(QTSS_RTSPSessionObject rtspSession);

	// Attempt to remove a  MP3Session ref from the table's map.
	// returns true on success or false if it fails.
	Bool16          UnRegisterSession(MP3Session* session);

private:

	MP3SessRefHashTable fTable;
	OSMutex             fMutex;
};

//
// MP3ClientQueue -- This is a class manages a queue of MP3Client objects.
// Every MP3BroadcasterSession class instance contains one of these objects.
//
class MP3ClientQueue
{
public:

	MP3ClientQueue();

	~MP3ClientQueue();

	UInt32          GetNumClients() { return fQueue.GetLength(); }

	QTSS_Error      AddClient(QTSS_RTSPSessionObject clientsess, QTSS_StreamRef clientstream,
		MP3BroadcasterSession* owner);

	QTSS_Error      RemoveClient(QTSS_RTSPSessionObject clientsess);

	Bool16          InQueue(QTSS_RTSPSessionObject clientsess);

	QTSS_Error      SendToAllClients(char* buffer, UInt32 bufferlen);

	void            PreflightClients(char* songname);

private:

	void            TerminateClients();

	OSMutex         fMutex;
	OSQueue         fQueue;
};

//
// MP3BroadcasterQueue -- This is a class manages a queue of MP3Broadcaster objects.
// There is only one global instance of this class owned by the server.
//
class MP3BroadcasterQueue
{
public:

	MP3BroadcasterQueue();

	~MP3BroadcasterQueue();

	UInt32          GetNumBroadcasters() { return fQueue.GetLength(); }

	QTSS_Error      CreateBroadcaster(QTSS_RTSPSessionObject bcastsess, QTSS_StreamRef bcaststream, StrPtrLen& mountpt);

	QTSS_Error      RemoveBroadcaster(QTSS_RTSPSessionObject bcastsess);

	QTSS_Error      RemoveClient(QTSS_RTSPSessionObject clientsess);

	Bool16          InQueue(QTSS_RTSPSessionObject bcastsess);

	Bool16          IsActiveClient(QTSS_RTSPSessionObject clientsess);

	MP3BroadcasterSession*  FindByMountPoint(char* mountpoint);

	MP3BroadcasterSession*  FindByMountPoint(StrPtrLen& mountpoint);

	MP3BroadcasterSession*  FindBySession(QTSS_RTSPSessionObject bcastsess);

	void            TerminateAllBroadcastSessions();

private:

	OSMutex         fMutex;
	OSQueue         fQueue;
};

#endif // __QTSSMP3STREAMINGMODULE_H__

