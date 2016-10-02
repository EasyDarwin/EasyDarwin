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
	 File:       RTSPSourceInfo.cpp

	 Contains:



 */

#include "RTSPSourceInfo.h"
#include "StringParser.h"
#include "SDPSourceInfo.h"
#include "OSMemory.h"
#include "StringFormatter.h"
#include "SocketUtils.h"
#include "RelayOutput.h"
#include "StringTranslator.h"
#include "SDPUtils.h"

StrPtrLen RTSPSourceInfo::sKeyString("rtsp_source");
StrPtrLen RTSPSourceInfo::sAnnouncedKeyString("announced_rtsp_source");

// StrPtrLen's for various keywords on the relay_source & relay_destination lines
static StrPtrLen sOutAddr("out_addr");
static StrPtrLen sDestAddr("dest_addr");
static StrPtrLen sDestPorts("dest_ports");
static StrPtrLen sTtl("ttl");

char* RTSPOutputInfo::CopyString(const char* srcStr)
{
	char* dstStr = NULL;

	if (srcStr != NULL)
	{
		UInt32 len = ::strlen(srcStr);
		dstStr = NEW char[len + 1];
		::memcpy(dstStr, srcStr, len);
		dstStr[len] = '\0';
	}

	return dstStr;
}

void RTSPOutputInfo::Copy(const RTSPOutputInfo& copy)
{
	fIsAnnounced = copy.fIsAnnounced;
	fAnnouncePort = copy.fAnnouncePort;

	if (copy.fDestURl != NULL)
		fDestURl = RTSPOutputInfo::CopyString(copy.fDestURl);
	if (copy.fUserName != NULL)
		fUserName = RTSPOutputInfo::CopyString(copy.fUserName);
	if (copy.fPassword != NULL)
		fPassword = RTSPOutputInfo::CopyString(copy.fPassword);
}

RTSPSourceInfo::RTSPSourceInfo(const RTSPSourceInfo& copy)
	:RCFSourceInfo(copy),
	fHostAddr(copy.fHostAddr), fHostPort(copy.fHostPort), fLocalAddr(copy.fLocalAddr),
	fNumSetupsComplete(copy.fNumSetupsComplete), fDescribeComplete(copy.fDescribeComplete),
	fAnnounce(copy.fAnnounce), fAnnounceIP(copy.fAnnounceIP), fAnnounceActualIP(copy.fAnnounceIP), fQueueElem()
{
	fQueueElem.SetEnclosingObject(this);
	fSourceURL = RTSPOutputInfo::CopyString(copy.fSourceURL);
	fUserName = RTSPOutputInfo::CopyString(copy.fUserName);
	fPassword = RTSPOutputInfo::CopyString(copy.fPassword);
	fAnnounceURL = RTSPOutputInfo::CopyString(copy.fAnnounceURL);

	fRTSPInfoArray = NEW RTSPOutputInfo[fNumOutputs];
	for (UInt32 index = 0; index < fNumOutputs; index++)
		fRTSPInfoArray[index].Copy(copy.fRTSPInfoArray[index]);

	// These aren't set anyway and shouldn't be copied around
	fClientSocket = NULL;
	fClient = NULL;
	fRelaySessionCreatorTask = NULL;
	fSession = NULL;
	fSessionQueue = NULL;

	if ((copy.fLocalSDP).Ptr != NULL)
		fLocalSDP.Set((copy.fLocalSDP).GetAsCString(), (copy.fLocalSDP).Len);

}

RTSPSourceInfo::~RTSPSourceInfo()
{
	OSMutexLocker locker(RelayOutput::GetQueueMutex());

	if (fRelaySessionCreatorTask != NULL)
		fRelaySessionCreatorTask->fInfo = NULL;

	if (fDescribeComplete)
	{
		Assert(fClientSocket != NULL);
		Assert(fClient != NULL);
		// the task will delete these objects when it is done with the teardown
		TeardownTask* task = new TeardownTask(fClientSocket, fClient);
		task->Signal(Task::kStartEvent);
	}
	else
	{
		if (fClientSocket != NULL) delete fClientSocket;
		if (fClient != NULL) delete fClient;
	}

	if (fQueueElem.IsMemberOfAnyQueue())
		fQueueElem.Remove();

	if (fLocalSDP.Ptr != NULL) delete fLocalSDP.Ptr;
	fLocalSDP.Len = 0;

	if (fSourceURL != NULL) delete fSourceURL;
	if (fAnnounceURL != NULL) delete fAnnounceURL;

	if (fRTSPInfoArray != NULL) delete[] fRTSPInfoArray;
}

void RTSPSourceInfo::InitClient(UInt32 inSocketType)
{
	fClientSocket = NEW TCPClientSocket(inSocketType);
	fClient = NEW RTSPClient(fClientSocket, false, RelaySession::sRelayUserAgent);
}

void RTSPSourceInfo::SetClientInfo(UInt32 inAddr, UInt16 inPort, char* inURL, UInt32 inLocalAddr)
{
	if (fClientSocket != NULL)
		fClientSocket->Set(inAddr, inPort);

	StrPtrLen inURLPtrLen(inURL);

	if (fClient != NULL)
		fClient->Set(inURLPtrLen);

	if (inLocalAddr != 0)
		fClientSocket->GetSocket()->Bind(inLocalAddr, 0);
}

QTSS_Error RTSPSourceInfo::ParsePrefs(XMLTag* relayTag, bool inAnnounce)
{
	XMLTag* prefTag;
	UInt32 localAddr = 0;
	UInt32 theHostAddr = 0;
	UInt16 theHostPort = 554;
	char* userName = NULL;
	char* password = NULL;
	StrPtrLen theURL;

	fAnnounce = inAnnounce;

	XMLTag* sourceTag = relayTag->GetEmbeddedTagByNameAndAttr("OBJECT", "CLASS", "source");
	if (sourceTag == NULL)
		return QTSS_ValueNotFound;

	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "in_addr");
	if (prefTag != NULL)
	{
		char* inAddrStr = prefTag->GetValue();
		if (inAddrStr != NULL)
			localAddr = SocketUtils::ConvertStringToAddr(inAddrStr);
	}
	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "source_addr");
	if (prefTag != NULL)
	{
		char* destAddrStr = prefTag->GetValue();
		if (destAddrStr != NULL)
			theHostAddr = SocketUtils::ConvertStringToAddr(destAddrStr);
	}
	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "rtsp_port");
	if (prefTag != NULL)
	{
		char* portStr = prefTag->GetValue();
		if (portStr != NULL)
			theHostPort = atoi(portStr);
	}
	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "name");
	if (prefTag != NULL)
	{
		userName = prefTag->GetValue();
	}
	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "password");
	if (prefTag != NULL)
	{
		password = prefTag->GetValue();
	}
	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "url");
	char urlBuff[1024];
	if (prefTag != NULL)
	{
		char* urlString = prefTag->GetValue();
		StringTranslator::EncodeURL(urlString, strlen(urlString) + 1, urlBuff, sizeof(urlBuff));
		theURL.Set(urlBuff);
	}

	if (fAnnounce)
	{
		fAnnounceURL = theURL.GetAsCString();
		fAnnounceIP = theHostAddr;
	}
	else
	{
		fHostAddr = theHostAddr;
		fHostPort = theHostPort;
		fSourceURL = theURL.GetAsCString();
		fUserName = RTSPOutputInfo::CopyString(userName);
		fPassword = RTSPOutputInfo::CopyString(password);
	}

	return QTSS_NoErr;
}

QTSS_Error  RTSPSourceInfo::Describe()
{
	QTSS_Error theErr = QTSS_NoErr;

	if (!fDescribeComplete)
	{
		// Work on the describe
		theErr = fClient->SendDescribe();
		if (theErr != QTSS_NoErr)
			return theErr;
		if (fClient->GetStatus() != 200)
			return QTSS_RequestFailed;

		// If the above function returns QTSS_NoErr, we've gotten the describe response,
		// so process it.
		SDPSourceInfo theSourceInfo(fClient->GetContentBody(), fClient->GetContentLength());

		// Copy the Source Info into our local SourceInfo.
		fNumStreams = theSourceInfo.GetNumStreams();
		fStreamArray = NEW StreamInfo[fNumStreams];

		for (UInt32 x = 0; x < fNumStreams; x++)
		{
			// Copy fPayloadType, fPayloadName, fTrackID, fBufferDelay
			fStreamArray[x].Copy(*theSourceInfo.GetStreamInfo(x));

			// Copy all stream info data. Also set fSrcIPAddr to be the host addr
			fStreamArray[x].fSrcIPAddr = fClientSocket->GetHostAddr();
			fStreamArray[x].fDestIPAddr = fClientSocket->GetLocalAddr();
			fStreamArray[x].fPort = 0;
			fStreamArray[x].fTimeToLive = 0;
		}
	}

	// Ok, describe is complete, copy out the SDP information.

	fLocalSDP.Ptr = NEW char[fClient->GetContentLength() + 1];

	// Look for an "a=range" line in the SDP. If there is one, remove it.

	static StrPtrLen sRangeStr("a=range:");
	StrPtrLen theSDPPtr(fClient->GetContentBody(), fClient->GetContentLength());
	StringParser theSDPParser(&theSDPPtr);

	do
	{
		// Loop until we reach the end of the SDP or hit a a=range line.
		StrPtrLen theSDPLine(theSDPParser.GetCurrentPosition(), theSDPParser.GetDataRemaining());
		if ((theSDPLine.Len > sRangeStr.Len) && (theSDPLine.NumEqualIgnoreCase(sRangeStr.Ptr, sRangeStr.Len)))
			break;

	} while (theSDPParser.GetThruEOL(NULL));

	// Copy what we have so far
	::memcpy(fLocalSDP.Ptr, fClient->GetContentBody(), theSDPParser.GetDataParsedLen());
	fLocalSDP.Len = theSDPParser.GetDataParsedLen();

	// Skip over the range (if it exists)
	(void)theSDPParser.GetThruEOL(NULL);

	// Copy the rest of the SDP
	::memcpy(fLocalSDP.Ptr + fLocalSDP.Len, theSDPParser.GetCurrentPosition(), theSDPParser.GetDataRemaining());
	fLocalSDP.Len += theSDPParser.GetDataRemaining();

#define _WRITE_SDP_ 0

#if _WRITE_SDP_
	FILE* outputFile = ::fopen("rtspclient.sdp", "w");
	if (outputFile != NULL)
	{
		fLocalSDP.Ptr[fLocalSDP.Len] = '\0';
		qtss_fprintf(outputFile, "%s", fLocalSDP.Ptr);
		::fclose(outputFile);
		qtss_printf("Wrote sdp to rtspclient.sdp\n");
	}
	else
		qtss_printf("Failed to write sdp\n");
#endif
	fDescribeComplete = true;
	return QTSS_NoErr;
}

QTSS_Error RTSPSourceInfo::SetupAndPlay()
{
	QTSS_Error theErr = QTSS_NoErr;

	// Do all the setups. This is async, so when a setup doesn't complete
	// immediately, return an error, and we'll pick up where we left off.
	while (fNumSetupsComplete < fNumStreams)
	{
		theErr = fClient->SendUDPSetup(fStreamArray[fNumSetupsComplete].fTrackID, fStreamArray[fNumSetupsComplete].fPort);
		if (theErr != QTSS_NoErr)
			return theErr;
		else if (fClient->GetStatus() != 200)
			return QTSS_RequestFailed;
		else
			fNumSetupsComplete++;
	}

	// We've done all the setups. Now send a play.
	theErr = fClient->SendPlay(0);
	if (theErr != QTSS_NoErr)
		return theErr;
	if (fClient->GetStatus() != 200)
		return QTSS_RequestFailed;

	return QTSS_NoErr;
}

QTSS_Error RTSPSourceInfo::Teardown()
{
	return (QTSS_Error)fClient->SendTeardown();
}

char*   RTSPSourceInfo::GetLocalSDP(UInt32* newSDPLen)
{
	*newSDPLen = fLocalSDP.Len;
	return fLocalSDP.GetAsCString();
}

char*   RTSPSourceInfo::GetAnnounceSDP(UInt32 ipAddr, UInt32* newSDPLen)
{
	char *announceSDP = NEW char[fLocalSDP.Len * 2];
	StringFormatter announceSDPFormatter(announceSDP, fLocalSDP.Len * 2);

	StrPtrLen sdpLine;
	StringParser sdpParser(&fLocalSDP);
	bool added = false;

	while (sdpParser.GetDataRemaining() > 0)
	{
		//stop when we reach an empty line.
		sdpParser.GetThruEOL(&sdpLine);
		if (sdpLine.Len == 0)
			continue;

		switch (*sdpLine.Ptr)
		{
		case 'c':
			break;  // remove any existing c lines
		case 'm':
			{
				if (!added)
				{
					added = true;
					// add a c line before the first m line
					char ipStr[50];
					char buff[50];
					StrPtrLen temp(buff);

					struct in_addr theIPAddr;
					theIPAddr.s_addr = htonl(ipAddr);
					SocketUtils::ConvertAddrToString(theIPAddr, &temp);

					qtss_sprintf(ipStr, "c=IN IP4 %s", buff);
					StrPtrLen tempLine(ipStr);
					announceSDPFormatter.Put(tempLine);
					announceSDPFormatter.PutEOL();
				}

				announceSDPFormatter.Put(sdpLine);
				announceSDPFormatter.PutEOL();
				break;//ignore connection information
			}
		default:
			{
				announceSDPFormatter.Put(sdpLine);
				announceSDPFormatter.PutEOL();
			}
		}
	}

	*newSDPLen = (UInt32)announceSDPFormatter.GetCurrentOffset();
	announceSDP[*newSDPLen] = 0;

	StrPtrLen theSDPStr(announceSDP);
	SDPContainer rawSDPContainer;
	if (!rawSDPContainer.SetSDPBuffer(&theSDPStr))
	{
		return NULL; // it is screwed up do nothing the sdp will be deleted automatically
	}

	SDPLineSorter sortedSDP(&rawSDPContainer);
	return sortedSDP.GetSortedSDPCopy(); // return a new copy of the sorted SDP
}

void RTSPSourceInfo::ParseAnnouncedDestination(XMLTag* destTag, UInt32 index)
{
	XMLTag* prefTag;

	fRTSPInfoArray[index].fIsAnnounced = true;

	prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "out_addr");
	if (prefTag != NULL)
	{
		char* outAddrStr = prefTag->GetValue();
		if (outAddrStr != NULL)
			fOutputArray[index].fLocalAddr = SocketUtils::ConvertStringToAddr(outAddrStr);
	}
	prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "dest_addr");
	if (prefTag != NULL)
	{
		char* destAddrStr = prefTag->GetValue();
		if (destAddrStr != NULL)
			fOutputArray[index].fDestAddr = SocketUtils::ConvertStringToAddr(destAddrStr);
	}
	prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "rtsp_port");
	if (prefTag != NULL)
	{
		char* portStr = prefTag->GetValue();
		if (portStr != NULL)
			fRTSPInfoArray[index].fAnnouncePort = atoi(portStr);
	}
	prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "name");
	if (prefTag != NULL)
	{
		StrPtrLen userName(prefTag->GetValue());
		fRTSPInfoArray[index].fUserName = userName.GetAsCString();
	}
	prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "url");
	if (prefTag != NULL)
	{
		char urlBuff[1024];
		char* urlString = prefTag->GetValue();
		StringTranslator::EncodeURL(urlString, strlen(urlString) + 1, urlBuff, sizeof(urlBuff));
		StrPtrLen destURL(urlBuff);
		fRTSPInfoArray[index].fDestURl = destURL.GetAsCString();
	}
	prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "password");
	if (prefTag != NULL)
	{
		StrPtrLen password(prefTag->GetValue());
		fRTSPInfoArray[index].fPassword = password.GetAsCString();
	}
}

void RTSPSourceInfo::AllocateOutputArray(UInt32 numOutputs)
{
	// Allocate the proper number of relay outputs
	RCFSourceInfo::AllocateOutputArray(numOutputs);
	fRTSPInfoArray = new RTSPOutputInfo[numOutputs];
}

bool RTSPSourceInfo::Equal(SourceInfo* inInfo)
{
	if (!inInfo->IsRTSPSourceInfo())
		return false;

	//  RTSPSourceInfo* info = dynamic_cast<RTSPSourceInfo *>(inInfo);
	RTSPSourceInfo* info = (RTSPSourceInfo *)(inInfo);

	//  if (info == NULL)
	//      return false;

	if (!fAnnounce)
	{
		StrPtrLen source(fSourceURL);
		if (source.Equal(info->GetSourceURL()) && (fHostAddr == info->GetHostAddr())
			&& (fHostPort == info->GetHostPort()))
			return true;
	}
	else
	{
		StrPtrLen announceURL(fAnnounceURL);
		if ((fAnnounceIP == info->GetAnnounceIP()) && announceURL.Equal(info->GetAnnounceURL()))
			return true;
	}

	return false;
}

void RTSPSourceInfo::SetSourceParameters(UInt32 inHostAddr, UInt16 inHostPort, StrPtrLen& inURL)
{
	fHostAddr = inHostAddr;
	fHostPort = inHostPort;
	fSourceURL = inURL.GetAsCString();
}

void RTSPSourceInfo::StartSessionCreatorTask(OSQueue* inSessionQueue, OSQueue* inSourceQueue)
{
	InitClient(Socket::kNonBlockingSocketType);
	SetClientInfo(fHostAddr, fHostPort, fSourceURL, fLocalAddr);
	if (fUserName != NULL)
		fClient->SetName(fUserName);
	if (fPassword != NULL)
		fClient->SetPassword(fPassword);

	fSessionQueue = inSessionQueue;

	if (fAnnounce)
		inSourceQueue->EnQueue(&fQueueElem);

	fSessionCreationState = kSendingDescribe;
	fRelaySessionCreatorTask = NEW RelaySessionCreator(this);
	fRelaySessionCreatorTask->Signal(Task::kStartEvent);
}

SInt64 RTSPSourceInfo::RelaySessionCreator::Run()
{
	OSMutexLocker locker(RelayOutput::GetQueueMutex());
	SInt64 result = -1;
	if (fInfo != NULL)
		result = fInfo->RunCreateSession();

	return result;
}

SInt64 RTSPSourceInfo::RunCreateSession()
{
	OS_Error osErr = OS_NoErr;
	SInt64 result = 500;

	if (fSessionCreationState == kSendingDescribe)
	{
		if (!fDescribeComplete)
		{
			osErr = fClient->SendDescribe();

			if (osErr == OS_NoErr)
			{
				if (fClient->GetStatus() == 200)
				{
					// we've gotten the describe response, so process it.
					SDPSourceInfo theSourceInfo(fClient->GetContentBody(), fClient->GetContentLength());

					// Copy the Source Info into our local SourceInfo.
					fNumStreams = theSourceInfo.GetNumStreams();
					fStreamArray = NEW StreamInfo[fNumStreams];

					for (UInt32 x = 0; x < fNumStreams; x++)
					{
						// Copy fPayloadType, fPayloadName, fTrackID, fBufferDelay
						fStreamArray[x].Copy(*theSourceInfo.GetStreamInfo(x));

						// Copy all stream info data. Also set fSrcIPAddr to be the host addr
						fStreamArray[x].fSrcIPAddr = fClientSocket->GetHostAddr();
						fStreamArray[x].fDestIPAddr = fClientSocket->GetLocalAddr();
						fStreamArray[x].fPort = 0;
						fStreamArray[x].fTimeToLive = 0;
					}
				}
				else
					osErr = ENOTCONN;
			}
		}

		//describe is complete 
		if (osErr == OS_NoErr)
		{
			//copy out the SDP information
			fLocalSDP.Ptr = NEW char[fClient->GetContentLength() + 1];

			// Look for an "a=range" line in the SDP. If there is one, remove it.
			static StrPtrLen sRangeStr("a=range:");
			StrPtrLen theSDPPtr(fClient->GetContentBody(), fClient->GetContentLength());
			StringParser theSDPParser(&theSDPPtr);

			do
			{
				// Loop until we reach the end of the SDP or hit a a=range line.
				StrPtrLen theSDPLine(theSDPParser.GetCurrentPosition(), theSDPParser.GetDataRemaining());
				if ((theSDPLine.Len > sRangeStr.Len) && (theSDPLine.NumEqualIgnoreCase(sRangeStr.Ptr, sRangeStr.Len)))
					break;
			} while (theSDPParser.GetThruEOL(NULL));

			// Copy what we have so far
			::memcpy(fLocalSDP.Ptr, fClient->GetContentBody(), theSDPParser.GetDataParsedLen());
			fLocalSDP.Len = theSDPParser.GetDataParsedLen();

			// Skip over the range (if it exists)
			(void)theSDPParser.GetThruEOL(NULL);

			// Copy the rest of the SDP
			::memcpy(fLocalSDP.Ptr + fLocalSDP.Len, theSDPParser.GetCurrentPosition(), theSDPParser.GetDataRemaining());
			fLocalSDP.Len += theSDPParser.GetDataRemaining();

#define _WRITE_SDP_ 0

#if _WRITE_SDP_
			FILE* outputFile = ::fopen("rtspclient.sdp", "w");
			if (outputFile != NULL)
			{
				fLocalSDP.Ptr[fLocalSDP.Len] = '\0';
				qtss_fprintf(outputFile, "%s", fLocalSDP.Ptr);
				::fclose(outputFile);
				qtss_printf("Wrote sdp to rtspclient.sdp\n");
			}
			else
				qtss_printf("Failed to write sdp\n");
#endif
			fDescribeComplete = true;

			fSession = NEW RelaySession(NULL, this);
			if (fSession->SetupRelaySession(this) == OS_NoErr)
			{
				fSessionCreationState = kSendingSetup;
			}
			else
			{
				osErr = ENOTCONN;
			}
		}
	}

	while ((fSessionCreationState == kSendingSetup) && (osErr == OS_NoErr))
	{
		osErr = fClient->SendUDPSetup(fStreamArray[fNumSetupsComplete].fTrackID, fStreamArray[fNumSetupsComplete].fPort);
		if (osErr == OS_NoErr)
		{
			if (fClient->GetStatus() == 200)
			{
				fNumSetupsComplete++;
				if (fNumSetupsComplete == fNumStreams)
					fSessionCreationState = kSendingPlay;
			}
			else
				osErr = ENOTCONN;
		}
	}

	if (fSessionCreationState == kSendingPlay)
	{
		osErr = fClient->SendPlay(0);
		if (osErr == OS_NoErr)
		{
			if (fClient->GetStatus() == 200)
				fSessionCreationState = kDone;
			else
				osErr = ENOTCONN;
		}
	}

	if (fSessionCreationState == kDone)
	{
		// If session was correctly set up, 
		// add the outputs
		if (fSession != NULL)
		{
			// Format SourceInfo HTML for the stats web page
			fSession->FormatHTML(fClient->GetURL());

			OSMutexLocker locker(RelayOutput::GetQueueMutex());
			fSessionQueue->EnQueue(fSession->GetQueueElem());

			for (UInt32 x = 0; x < fNumOutputs; x++)
			{
				SourceInfo::OutputInfo* theOutputInfo = GetOutputInfo(x);
				if (theOutputInfo->fAlreadySetup)
					continue;       // shouldn't ever happen

				RelayOutput* theOutput = NEW RelayOutput(this, x, fSession, true);
				if (theOutput->IsValid())
					fSession->AddOutput(theOutput, false);
				else
					delete theOutput;
			}
		}
		fClientSocket->GetSocket()->SetTask(NULL); //detach the task from the socket
		result = -1;    // let the task die
		fRelaySessionCreatorTask = NULL;
	}

	if ((osErr == EINPROGRESS) || (osErr == EAGAIN))
	{
		// Request an async event
		fClientSocket->GetSocket()->SetTask(fRelaySessionCreatorTask);
		fClientSocket->GetSocket()->RequestEvent(fClientSocket->GetEventMask());
	}
	else if (osErr != OS_NoErr)
	{
		// We encountered some fatal error with the socket. Record this as a connection failure
		// delete the session
		// delete the session
		if (fSession != NULL)
		{
			delete fSession;
			fSession = NULL;
		}

		fClientSocket->GetSocket()->SetTask(NULL); //detach the task from the socket
		result = -1;    // let the task die
		fRelaySessionCreatorTask = NULL;
	}

	return result;
}


RTSPSourceInfo::TeardownTask::TeardownTask(TCPClientSocket* clientSocket, RTSPClient* client)
{
	this->SetTaskName("RTSPSourceInfo::TeardownTask");
	fClientSocket = clientSocket;
	fClient = client;
}

RTSPSourceInfo::TeardownTask::~TeardownTask()
{
	delete fClientSocket;
	delete fClient;
}

SInt64 RTSPSourceInfo::TeardownTask::Run()
{
	OS_Error err = fClient->SendTeardown();

	if ((err == EINPROGRESS) || (err == EAGAIN))
	{
		// Request an async event
		fClientSocket->GetSocket()->SetTask(this);
		fClientSocket->GetSocket()->RequestEvent(fClientSocket->GetEventMask());
		return 250;
	}
	fClientSocket->GetSocket()->SetTask(NULL); //detach the task from the socket
	return -1;  // we're out of here, this will cause the destructor to be called
}

