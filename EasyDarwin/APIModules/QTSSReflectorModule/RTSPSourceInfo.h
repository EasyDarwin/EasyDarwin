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
	 File:       RTSPSourceInfo.h

	 Contains:


 */

#ifndef __RTSP_SOURCE_INFO_H__
#define __RTSP_SOURCE_INFO_H__

#include "QTSS.h"
#include "StrPtrLen.h"
#include "RCFSourceInfo.h"
#include "RTSPClient.h"
#include "XMLParser.h"
#include "ClientSocket.h"
#include "RelaySession.h"

using namespace EasyDarwin;

class RelaySessionCreator;

class RTSPOutputInfo
{
public:
	RTSPOutputInfo() : fIsAnnounced(false),
		fAnnouncePort(554),
		fDestURl(NULL),
		fUserName(NULL),
		fPassword(NULL) {}

	~RTSPOutputInfo()
	{
		if (fDestURl != NULL) delete fDestURl;
		if (fUserName != NULL) delete fUserName;
		if (fPassword != NULL) delete fPassword;
	}

	static char* CopyString(const char* srcStr);
	void Copy(const RTSPOutputInfo& copy); // copies dynamically allocated data too
	bool Equal(const RTSPOutputInfo* inInfo)
	{
		return ((inInfo != NULL) && (fIsAnnounced == inInfo->fIsAnnounced) && (fAnnouncePort == inInfo->fAnnouncePort)
			&& (strcmp(fDestURl, inInfo->fDestURl) == 0));
	}


	bool      fIsAnnounced;
	UInt16      fAnnouncePort;
	char*       fDestURl;
	char*       fUserName;
	char*       fPassword;
};

class RTSPSourceInfo : public RCFSourceInfo
{
public:

	// Specify whether the client should be blocking or non-blocking
	RTSPSourceInfo(bool inAnnounce) : fSourceURL(NULL),
		fHostAddr(0),
		fHostPort(0),
		fLocalAddr(0),
		fUserName(NULL),
		fPassword(NULL),
		fRTSPInfoArray(NULL),
		fClientSocket(NULL),
		fClient(NULL),
		fNumSetupsComplete(0),
		fDescribeComplete(false),
		fAnnounce(inAnnounce),
		fAnnounceURL(NULL),
		fAnnounceIP(0),
		fAnnounceActualIP(0),
		fRelaySessionCreatorTask(NULL),
		fSession(NULL),
		fSessionQueue(NULL),
		fQueueElem() {
		fQueueElem.SetEnclosingObject(this);
	}

	RTSPSourceInfo(const RTSPSourceInfo& copy); // Does copy dynamically allocated data
												// Doesn't copy fClientSocket and fClient ptrs

	virtual ~RTSPSourceInfo();

	// Call this before calling ParsePrefs / Describe
	void InitClient(UInt32 inSocketType);

	void SetClientInfo(UInt32 inAddr, UInt16 inPort, char* inURL, UInt32 inLocalAddr = 0);

	// Call this immediately after the constructor. This object will parse
	// the config file and extract the necessary information to connect to an rtsp server.
	// Specify the config file line index where the "rtsp_source" line resides
	QTSS_Error  ParsePrefs(XMLTag* relayTag, bool inAnnounce);

	// Connects, sends a DESCRIBE, and parses the incoming SDP data. After this
	// function completes sucessfully, GetLocalSDP returns the data, and the
	// SourceInfo & DestInfo arrays will be set up. Also sends SETUPs for all the
	// tracks, and finishes by issuing a PLAY.
	//
	// These functions return QTSS_NoErr if the transaction has completed
	// successfully. Otherwise, they return:
	//
	// EAGAIN: the transaction is still in progress, the call should be reissued
	// QTSS_RequestFailed: the remote host responded with an error.
	// Any other error means that the remote host was unavailable or refused the connection
	QTSS_Error  Describe();
	QTSS_Error  SetupAndPlay();

	// This function works the same way as the above ones, and should be
	// called before destroying the object to let the remote host know that
	// we are going away.
	QTSS_Error  Teardown();

	// This function uses the Parsed SDP file, and strips out all the network information,
	// producing an SDP file that appears to be local.
	virtual char*   GetLocalSDP(UInt32* newSDPLen);
	virtual char*   GetAnnounceSDP(UInt32 ipAddr, UInt32* newSDPLen);
	virtual StrPtrLen*  GetSourceID() { return fClient->GetURL(); }

	// This object looks for this keyword in the FilePrefsSource, where it
	// expects the IP address, port, and URL.
	static StrPtrLen&   GetRTSPSourceString() { return sKeyString; }

	RTSPClient* GetRTSPClient() { return fClient; }
	TCPClientSocket* GetClientSocket() { return fClientSocket; }

	bool      IsDescribeComplete() { return fDescribeComplete; }

	RTSPOutputInfo* GetRTSPOutputInfo(UInt32 index) { return &fRTSPInfoArray[index]; }
	char* GetSourceURL() { return fSourceURL; }

	virtual bool IsRTSPSourceInfo() { return true; }
	virtual bool Equal(SourceInfo* inInfo);

	bool IsAnnounce() { return fAnnounce; }

	char* GetAnnounceURL() { return fAnnounceURL; }
	UInt32 GetAnnounceIP() { return fAnnounceIP; }

	UInt32 GetAnnounceActualIP() { return fAnnounceActualIP; }
	void   SetAnnounceActualIP(UInt32 inActualIP) { fAnnounceActualIP = inActualIP; }

	UInt32 GetHostAddr() { return fHostAddr; }
	UInt32 GetHostPort() { return fHostPort; }

	char* GetUsername() { return fUserName; }
	char* GetPassword() { return fPassword; }

	RelaySession* GetRelaySession() { return fSession; }

	void SetSourceParameters(UInt32 inHostAddr, UInt16 inHostPort, StrPtrLen& inURL);

	void StartSessionCreatorTask(OSQueue* inSessionQueue, OSQueue* inSourceQueue);

	SInt64 RunCreateSession();

protected:
	virtual void ParseAnnouncedDestination(XMLTag* destTag, UInt32 index);
	virtual void AllocateOutputArray(UInt32 numOutputs);

private:
	class RelaySessionCreator : public Task
	{
	public:
		RelaySessionCreator(RTSPSourceInfo* inInfo) : fInfo(inInfo) { this->SetTaskName("RTSPSourceInfo::RelaySessionCreator"); }

		virtual SInt64 Run();

		RTSPSourceInfo* fInfo;
	};

	class TeardownTask : public Task
	{
	public:
		TeardownTask(TCPClientSocket* clientSocket, RTSPClient* client);
		virtual ~TeardownTask();

		virtual SInt64 Run();

	private:
		TCPClientSocket*    fClientSocket;
		RTSPClient*         fClient;
	};

	char*                   fSourceURL;
	UInt32                  fHostAddr;
	UInt16                  fHostPort;
	UInt32                  fLocalAddr;
	char*                   fUserName;
	char*                   fPassword;
	RTSPOutputInfo*         fRTSPInfoArray;
	TCPClientSocket*        fClientSocket;
	RTSPClient*             fClient;
	UInt32                  fNumSetupsComplete;
	bool                  fDescribeComplete;
	StrPtrLen               fLocalSDP;

	bool                  fAnnounce;
	char*                   fAnnounceURL;
	UInt32                  fAnnounceIP;
	UInt32                  fAnnounceActualIP;
	RelaySessionCreator*    fRelaySessionCreatorTask;

	enum    // relay session creation states
	{
		kSendingDescribe = 0,
		kSendingSetup = 1,
		kSendingPlay = 2,
		kDone = 3
	};
	UInt32          fSessionCreationState;

	RelaySession* fSession;
	OSQueue* fSessionQueue;

	OSQueueElem fQueueElem;

	static StrPtrLen sKeyString;
	static StrPtrLen sAnnouncedKeyString;
};
#endif // __RTSP_SOURCE_INFO_H__

