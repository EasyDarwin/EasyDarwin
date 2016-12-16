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
	File:       ReflectorSession.cpp
	Contains:   Implementation of object defined in ReflectorSession.h.
*/

#include "ReflectorSession.h"
#include "SocketUtils.h"

#include "OSMemory.h"
#include "OS.h"

#include "QTSServerInterface.h"
#include "sdpCache.h"

#ifndef __Win32__
#include <unistd.h>
#endif

#if DEBUG
#define REFLECTOR_SESSION_DEBUGGING 0
#else
#define REFLECTOR_SESSION_DEBUGGING 0
#endif

FileDeleter::FileDeleter(StrPtrLen* inSDPPath)
{
	Assert(inSDPPath);
	fFilePath.Len = inSDPPath->Len;
	fFilePath.Ptr = NEW char[inSDPPath->Len + 1];
	Assert(fFilePath.Ptr);
	memcpy(fFilePath.Ptr, inSDPPath->Ptr, inSDPPath->Len);
	fFilePath.Ptr[inSDPPath->Len] = 0;
}

FileDeleter::~FileDeleter()
{
	//qtss_printf("FileDeleter::~FileDeleter delete = %s \n",fFilePath.Ptr);
	::unlink(fFilePath.Ptr);
	delete fFilePath.Ptr;
	fFilePath.Ptr = nullptr;
	fFilePath.Len = 0;
}

void ReflectorSession::Initialize()
{
	;
}

ReflectorSession::ReflectorSession(StrPtrLen* inSourceID, UInt32 inChannelNum, SourceInfo* inInfo) : Task(),
	fIsSetup(false),
	fSessionName(inSourceID->GetAsCString()),
	fChannelNum(inChannelNum),
	fHLSLive(false),
	fQueueElem(),
	fNumOutputs(0),
	fStreamArray(nullptr),
	fSourceInfo(inInfo),
	fSocketStream(nullptr),
	fBroadcasterSession(nullptr),
	fInitTimeMS(OS::Milliseconds()),
	fNoneOutputStartTimeMS(OS::Milliseconds()),
	fHasBufferedStreams(false),
	fHasVideoKeyFrameUpdate(false)
{
	fQueueElem.SetEnclosingObject(this);
	if (inSourceID != nullptr)
	{
		char streamID[QTSS_MAX_NAME_LENGTH + 10] = { 0 };
		if (inSourceID->Len > QTSS_MAX_NAME_LENGTH)
			inSourceID->Len = QTSS_MAX_NAME_LENGTH;

		sprintf(streamID, "%s%s%d", inSourceID->Ptr, EASY_KEY_SPLITER, fChannelNum);
		fSourceID.Ptr = NEW char[::strlen(streamID) + 1];
		::strncpy(fSourceID.Ptr, streamID, strlen(streamID));
		fSourceID.Ptr[strlen(streamID)] = '\0';
		fSourceID.Len = strlen(streamID);
		fRef.Set(fSourceID, this);

		this->SetSessionName();
	}

	this->Signal(Task::kStartEvent);
}


ReflectorSession::~ReflectorSession()
{
	this->StopHLSSession();

	// For each stream, check to see if the ReflectorStream should be deleted
	for (UInt32 x = 0; x < fSourceInfo->GetNumStreams(); x++)
	{
		if (fStreamArray[x] == nullptr)
			continue;

		fStreamArray[x]->SetMyReflectorSession(nullptr);

		delete fStreamArray[x];
		fStreamArray[x] = nullptr;
	}

	CSdpCache::GetInstance()->eraseSdpMap(GetSourceID()->GetAsCString());

	// We own this object when it is given to us, so delete it now
	delete[] fStreamArray;
	delete fSourceInfo;
	fLocalSDP.Delete();

	if (fSourceID.Ptr)
	{
		QTSS_RoleParams theParams;
		theParams.easyStreamInfoParams.inStreamName = fSessionName.Ptr;
		theParams.easyStreamInfoParams.inChannel = fChannelNum;
		theParams.easyStreamInfoParams.inAction = easyRedisActionDelete;
		UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisUpdateStreamInfoRole);
		for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
		{
			qtss_printf("从redis中删除推流名称%s\n", fSourceID.Ptr);
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisUpdateStreamInfoRole, currentModule);
			(void)theModule->CallDispatch(Easy_RedisUpdateStreamInfo_Role, &theParams);
		}
	}

	fSourceID.Delete();
	fSessionName.Delete();
}

QTSS_Error ReflectorSession::SetSessionName()
{
	if (fSourceID.Len > 0)
	{
		QTSS_RoleParams theParams;
		theParams.easyStreamInfoParams.inStreamName = fSessionName.Ptr;
		theParams.easyStreamInfoParams.inChannel = fChannelNum;
		theParams.easyStreamInfoParams.inNumOutputs = fNumOutputs;
		theParams.easyStreamInfoParams.inAction = easyRedisActionSet;
		auto numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisUpdateStreamInfoRole);
		for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
		{
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisUpdateStreamInfoRole, currentModule);
			(void)theModule->CallDispatch(Easy_RedisUpdateStreamInfo_Role, &theParams);
		}
	}
	return QTSS_NoErr;
}

QTSS_Error ReflectorSession::StartHLSSession()
{
	QTSS_Error theErr = QTSS_NoErr;

	if (!fHLSLive)
	{
		// Get the ip addr out of the prefs dictionary
		UInt16 thePort = 554;
		UInt32 theLen = sizeof(UInt16);
		theErr = QTSServerInterface::GetServer()->GetPrefs()->GetValue(qtssPrefsRTSPPorts, 0, &thePort, &theLen);
		Assert(theErr == QTSS_NoErr);

		//构造本地URL
		char url[QTSS_MAX_URL_LENGTH] = { 0 };
		qtss_sprintf(url, "rtsp://127.0.0.1:%d/%s", thePort, fSourceID.Ptr);

		Easy_StartHLSession(fSourceID.Ptr, url, 0, nullptr);
		//if(QTSS_NoErr == theErr)
		fHLSLive = true;
	}

	return theErr;
}

QTSS_Error ReflectorSession::StopHLSSession()
{
	QTSS_Error theErr = QTSS_NoErr;

	if (fHLSLive)
	{
		theErr = Easy_StopHLSession(fSourceID.Ptr);
		if (QTSS_NoErr == theErr)
			fHLSLive = false;
	}

	return theErr;
}

QTSS_Error ReflectorSession::SetupReflectorSession(SourceInfo* inInfo, QTSS_StandardRTSP_Params* inParams, UInt32 inFlags, bool filterState, UInt32 filterTimeout)
{
	// use the current SourceInfo
	if (inInfo == nullptr)
		inInfo = fSourceInfo;

	// Store a reference to this sourceInfo permanently
	Assert((fSourceInfo == NULL) || (inInfo == fSourceInfo));
	fSourceInfo = inInfo;

	// this must be set to the new SDP.
	fLocalSDP.Delete();
	fLocalSDP.Ptr = inInfo->GetLocalSDP(&fLocalSDP.Len);

	if (fStreamArray != nullptr)
	{
		delete fStreamArray; // keep the array list synchronized with the source info.
	}

	fStreamArray = NEW ReflectorStream*[fSourceInfo->GetNumStreams()];
	::memset(fStreamArray, 0, fSourceInfo->GetNumStreams() * sizeof(ReflectorStream*));

	for (UInt32 x = 0; x < fSourceInfo->GetNumStreams(); x++)
	{
		fStreamArray[x] = NEW ReflectorStream(fSourceInfo->GetStreamInfo(x));
		// Obviously, we may encounter an error binding the reflector sockets.
		// If that happens, we'll just abort here, which will leave the ReflectorStream
		// array in an inconsistent state, so we need to make sure in our cleanup
		// code to check for NULL.
		QTSS_Error theError = fStreamArray[x]->BindSockets(inParams, inFlags, filterState, filterTimeout);
		if (theError != QTSS_NoErr)
		{
			delete fStreamArray[x];
			fStreamArray[x] = nullptr;
			return theError;
		}
		fStreamArray[x]->SetMyReflectorSession(this);

		fStreamArray[x]->SetEnableBuffer(this->fHasBufferedStreams);// buffering is done by the stream's sender

		// If the port was 0, update it to reflect what the actual RTP port is.
		fSourceInfo->GetStreamInfo(x)->fPort = fStreamArray[x]->GetStreamInfo()->fPort;
		//qtss_printf("ReflectorSession::SetupReflectorSession fSourceInfo->GetStreamInfo(x)->fPort= %u\n",fSourceInfo->GetStreamInfo(x)->fPort);   
	}

	if (inFlags & kMarkSetup)
		fIsSetup = true;

	return QTSS_NoErr;
}

void ReflectorSession::AddBroadcasterClientSession(QTSS_StandardRTSP_Params* inParams)
{
	if (NULL == fStreamArray || NULL == inParams)
		return;

	for (UInt32 x = 0; x < fSourceInfo->GetNumStreams(); x++)
	{
		if (fStreamArray[x] != nullptr)
		{   //qtss_printf("AddBroadcasterSession=%"   _U32BITARG_   "\n",inParams->inClientSession);
			((ReflectorSocket*)fStreamArray[x]->GetSocketPair()->GetSocketA())->AddBroadcasterSession(inParams->inClientSession);
			((ReflectorSocket*)fStreamArray[x]->GetSocketPair()->GetSocketB())->AddBroadcasterSession(inParams->inClientSession);
		}
	}
	fBroadcasterSession = inParams->inClientSession;
}

void    ReflectorSession::AddOutput(ReflectorOutput* inOutput, bool isClient)
{
	Assert(fSourceInfo->GetNumStreams() > 0);

	// We need to make sure that this output goes into the same bucket for each ReflectorStream.
	SInt32 bucket = -1;
	SInt32 lastBucket = -1;

	while (true)
	{
		UInt32 x = 0;
		for (; x < fSourceInfo->GetNumStreams(); x++)
		{
			bucket = fStreamArray[x]->AddOutput(inOutput, bucket);
			if (bucket == -1)   // If this output couldn't be added to this bucket,
				break;          // break and try again
			else
			{
				lastBucket = bucket; // Remember the last successful bucket placement.
				if (isClient)
					fStreamArray[x]->IncEyeCount();
			}
		}

		if (bucket == -1)
		{
			// If there was some kind of conflict adding this output to this bucket,
			// we need to remove it from the streams to which it was added.
			for (UInt32 y = 0; y < x; y++)
			{
				fStreamArray[y]->RemoveOutput(inOutput);
				if (isClient)
					fStreamArray[y]->DecEyeCount();
			}

			// Because there was an error, we need to start the whole process over again,
			// this time starting from a higher bucket
			lastBucket = bucket = lastBucket + 1;
		}
		else
			break;
	}
	//(void)atomic_add(&fNumOutputs, 1);
	++fNumOutputs;
}

void    ReflectorSession::RemoveOutput(ReflectorOutput* inOutput, bool isClient)
{
	//(void)atomic_sub(&fNumOutputs, 1);
	--fNumOutputs;
	for (UInt32 y = 0; y < fSourceInfo->GetNumStreams(); y++)
	{
		fStreamArray[y]->RemoveOutput(inOutput);
		if (isClient)
			fStreamArray[y]->DecEyeCount();
	}

	if (fNumOutputs == 0)
	{
		this->SetNoneOutputStartTimeMS();
		//QTSS_RoleParams theParams;
		//theParams.easyFreeStreamParams.inStreamName = GetSourceID()->Ptr;
		//auto numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kEasyCMSFreeStreamRole);
		//for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
		//{
		//	auto theModule = QTSServerInterface::GetModule(QTSSModule::kEasyCMSFreeStreamRole, currentModule);
		//	(void)theModule->CallDispatch(Easy_CMSFreeStream_Role, &theParams);
		//}
	}
}

void ReflectorSession::TearDownAllOutputs()
{
	for (UInt32 y = 0; y < fSourceInfo->GetNumStreams(); y++)
		fStreamArray[y]->TearDownAllOutputs();
}

void    ReflectorSession::RemoveSessionFromOutput(QTSS_ClientSessionObject inSession)
{
	for (UInt32 x = 0; x < fSourceInfo->GetNumStreams(); x++)
	{
		((ReflectorSocket*)fStreamArray[x]->GetSocketPair()->GetSocketA())->RemoveBroadcasterSession(inSession);
		((ReflectorSocket*)fStreamArray[x]->GetSocketPair()->GetSocketB())->RemoveBroadcasterSession(inSession);
	}
	fBroadcasterSession = nullptr;
}

UInt32  ReflectorSession::GetBitRate()
{
	UInt32 retval = 0;
	for (UInt32 x = 0; x < fSourceInfo->GetNumStreams(); x++)
		retval += fStreamArray[x]->GetBitRate();
	return retval;
}

bool ReflectorSession::Equal(SourceInfo* inInfo)
{
	return fSourceInfo->Equal(inInfo);
}

void*   ReflectorSession::GetStreamCookie(UInt32 inStreamID)
{
	for (UInt32 x = 0; x < fSourceInfo->GetNumStreams(); x++)
	{
		if (fSourceInfo->GetStreamInfo(x)->fTrackID == inStreamID)
			return fStreamArray[x]->GetStreamCookie();
	}
	return nullptr;
}

SInt64 ReflectorSession::Run()
{
	EventFlags events = this->GetEvents();

	if (events & Task::kKillEvent)
		return -1;

	SInt64 sNowTime = OS::Milliseconds();
	SInt64 sNoneTime = GetNoneOutputStartTimeMS();
	if ((GetNumOutputs() == 0) && (sNowTime - sNoneTime >= 30*1000))
	{
		QTSS_RoleParams theParams;
		theParams.easyFreeStreamParams.inStreamName = GetSourceID()->Ptr;
		auto numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kEasyCMSFreeStreamRole);
		for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
		{
			auto theModule = QTSServerInterface::GetModule(QTSSModule::kEasyCMSFreeStreamRole, currentModule);
			(void)theModule->CallDispatch(Easy_CMSFreeStream_Role, &theParams);
		}
	}
	else
	{
		QTSS_RoleParams theParams;
		theParams.easyStreamInfoParams.inStreamName = fSessionName.Ptr;
		theParams.easyStreamInfoParams.inChannel = fChannelNum;
		theParams.easyStreamInfoParams.inNumOutputs = fNumOutputs;
		theParams.easyStreamInfoParams.inAction = easyRedisActionSet;
		auto numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisUpdateStreamInfoRole);
		for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
		{
			auto theModule = QTSServerInterface::GetModule(QTSSModule::kRedisUpdateStreamInfoRole, currentModule);
			(void)theModule->CallDispatch(Easy_RedisUpdateStreamInfo_Role, &theParams);
		}
	}

	return QTSServerInterface::GetServer()->GetPrefs()->GetRTPSessionTimeoutInSecs()*1000;
}
