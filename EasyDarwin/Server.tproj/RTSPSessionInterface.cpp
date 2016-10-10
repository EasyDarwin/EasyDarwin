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
	 File:       RTSPSessionInterface.cpp

	 Contains:   Implementation of RTSPSessionInterface object.
 */


#include "RTSPSessionInterface.h"
#include "QTSServerInterface.h"
#include "OSMemory.h"
#include "RTSPProtocol.h"

#include <errno.h>


#if DEBUG
#define RTSP_SESSION_INTERFACE_DEBUGGING 1
#else
#define RTSP_SESSION_INTERFACE_DEBUGGING 0
#endif



unsigned int            RTSPSessionInterface::sSessionIDCounter = kFirstRTSPSessionID;
bool                  RTSPSessionInterface::sDoBase64Decoding = true;
UInt32					RTSPSessionInterface::sOptionsRequestBody[kMaxRandomDataSize / sizeof(UInt32)];

QTSSAttrInfoDict::AttrInfo  RTSPSessionInterface::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
	/* 0 */ { "qtssRTSPSesID",              nullptr,           qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
	/* 1 */ { "qtssRTSPSesLocalAddr",       SetupParams,    qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
	/* 2 */ { "qtssRTSPSesLocalAddrStr",    SetupParams,    qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
	/* 3 */ { "qtssRTSPSesLocalDNS",        SetupParams,    qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
	/* 4 */ { "qtssRTSPSesRemoteAddr",      SetupParams,    qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
	/* 5 */ { "qtssRTSPSesRemoteAddrStr",   SetupParams,    qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
	/* 6 */ { "qtssRTSPSesEventCntxt",      nullptr,           qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
	/* 7 */ { "qtssRTSPSesType",            nullptr,           qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
	/* 8 */ { "qtssRTSPSesStreamRef",       nullptr,           qtssAttrDataTypeQTSS_StreamRef, qtssAttrModeRead | qtssAttrModePreempSafe },

	/* 9 */ { "qtssRTSPSesLastUserName",    nullptr,           qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe  },
	/* 10 */{ "qtssRTSPSesLastUserPassword",nullptr,           qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe  },
	/* 11 */{ "qtssRTSPSesLastURLRealm",    nullptr,           qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe  },

	/* 12 */{ "qtssRTSPSesLocalPort",       SetupParams,    qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
	/* 13 */{ "qtssRTSPSesRemotePort",      SetupParams,    qtssAttrDataTypeUInt16,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeCacheable },
	/* 14 */{ "qtssRTSPSes3GPPObject",      nullptr,           qtssAttrDataTypeQTSS_Object,qtssAttrModeRead | qtssAttrModePreempSafe },

	/* 15 */{ "qtssRTSPSesLastDigestChallenge",nullptr,        qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe  }


};


void    RTSPSessionInterface::Initialize()
{
	for (UInt32 x = 0; x < qtssRTSPSesNumParams; x++)
		QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kRTSPSessionDictIndex)->
		SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr, sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);

	// DJM PROTOTYPE
	::srand((unsigned int)OS::Microseconds());
	for (unsigned int i = 0; i < kMaxRandomDataSize / sizeof(UInt32); i++)
		RTSPSessionInterface::sOptionsRequestBody[i] = ::rand();
	((char *)RTSPSessionInterface::sOptionsRequestBody)[0] = 0; //always set first byte so it doesn't hit any client parser bugs for \r or \n.

}


RTSPSessionInterface::RTSPSessionInterface()
	: QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kRTSPSessionDictIndex)),
	Task(),
	fTimeoutTask(nullptr, QTSServerInterface::GetServer()->GetPrefs()->GetRealRTSPTimeoutInSecs() * 1000),
	fInputStream(&fSocket),
	fOutputStream(&fSocket, &fTimeoutTask),
	fSessionMutex(),
	fTCPCoalesceBuffer(nullptr),
	fNumInCoalesceBuffer(0),
	fSocket(nullptr, Socket::kNonBlockingSocketType),
	fOutputSocketP(&fSocket),
	fInputSocketP(&fSocket),
	fSessionType(qtssRTSPSession),
	fLiveSession(true),
	fObjectHolders(0),
	fCurChannelNum(0),
	fChNumToSessIDMap(nullptr),
	fRequestBodyLen(-1),
	fSentOptionsRequest(false),
	fOptionsRequestSendTime(-1),
	fRoundTripTime(-1),
	fRoundTripTimeCalculation(true),
	fRTSPSession3GPP(QTSServerInterface::GetServer()->GetPrefs()->Get3GPPEnabled()),
	fRTSPSession3GPPPtr(&fRTSPSession3GPP)
{

	fTimeoutTask.SetTask(this);
	fSocket.SetTask(this);
	fStreamRef = this;

	fSessionID = (UInt32)atomic_add(&sSessionIDCounter, 1);
	this->SetVal(qtssRTSPSesID, &fSessionID, sizeof(fSessionID));
	this->SetVal(qtssRTSPSesEventCntxt, &fOutputSocketP, sizeof(fOutputSocketP));
	this->SetVal(qtssRTSPSesType, &fSessionType, sizeof(fSessionType));
	this->SetVal(qtssRTSPSesStreamRef, &fStreamRef, sizeof(fStreamRef));
	this->SetVal(qtssRTSPSes3GPPObject, &fRTSPSession3GPPPtr, sizeof(fRTSPSession3GPPPtr));

	this->SetEmptyVal(qtssRTSPSesLastUserName, &fUserNameBuf[0], kMaxUserNameLen);
	this->SetEmptyVal(qtssRTSPSesLastUserPassword, &fUserPasswordBuf[0], kMaxUserPasswordLen);
	this->SetEmptyVal(qtssRTSPSesLastURLRealm, &fUserRealmBuf[0], kMaxUserRealmLen);


	fInputStream.ShowRTSP(QTSServerInterface::GetServer()->GetPrefs()->GetRTSPDebugPrintfs());
	fOutputStream.ShowRTSP(QTSServerInterface::GetServer()->GetPrefs()->GetRTSPDebugPrintfs());
}


RTSPSessionInterface::~RTSPSessionInterface()
{
	// If the input socket is != output socket, the input socket was created dynamically
	if (fInputSocketP != fOutputSocketP)
		delete fInputSocketP;

	delete[] fTCPCoalesceBuffer;

	for (UInt8 x = 0; x < (fCurChannelNum >> 1); x++)
		delete[] fChNumToSessIDMap[x].Ptr;
	delete[] fChNumToSessIDMap;
}

void RTSPSessionInterface::DecrementObjectHolderCount()
{

#if __Win32__
	//maybe don't need this special case but for now on Win32 we do it the old way since the killEvent code hasn't been verified on Windows.
	this->Signal(Task::kReadEvent);//have the object wakeup in case it can go away.
	atomic_sub(&fObjectHolders, 1);
#else
	if (0 == atomic_sub(&fObjectHolders, 1))
		this->Signal(Task::kKillEvent);
#endif

}

QTSS_Error RTSPSessionInterface::Write(void* inBuffer, UInt32 inLength,
	UInt32* outLenWritten, UInt32 inFlags)
{
	UInt32 sendType = RTSPResponseStream::kDontBuffer;
	if ((inFlags & qtssWriteFlagsBufferData) != 0)
		sendType = RTSPResponseStream::kAlwaysBuffer;

	iovec theVec[2];
	theVec[1].iov_base = (char*)inBuffer;
	theVec[1].iov_len = inLength;
	return fOutputStream.WriteV(theVec, 2, inLength, outLenWritten, sendType);
}

QTSS_Error RTSPSessionInterface::WriteV(iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength, UInt32* outLenWritten)
{
	return fOutputStream.WriteV(inVec, inNumVectors, inTotalLength, outLenWritten, RTSPResponseStream::kDontBuffer);
}

QTSS_Error RTSPSessionInterface::Read(void* ioBuffer, UInt32 inLength, UInt32* outLenRead)
{
	//
	// Don't let callers of this function accidently creep past the end of the
	// request body.  If the request body size isn't known, fRequestBodyLen will be -1

	if (fRequestBodyLen == 0)
		return QTSS_NoMoreData;

	if ((fRequestBodyLen > 0) && ((SInt32)inLength > fRequestBodyLen))
		inLength = fRequestBodyLen;

	UInt32 theLenRead = 0;
	QTSS_Error theErr = fInputStream.Read(ioBuffer, inLength, &theLenRead);

	if (fRequestBodyLen >= 0)
		fRequestBodyLen -= theLenRead;

	if (outLenRead != nullptr)
		*outLenRead = theLenRead;

	return theErr;
}

QTSS_Error RTSPSessionInterface::RequestEvent(QTSS_EventType inEventMask)
{
	if (inEventMask & QTSS_ReadableEvent)
		fInputSocketP->RequestEvent(EV_RE);
	if (inEventMask & QTSS_WriteableEvent)
		fOutputSocketP->RequestEvent(EV_WR);

	return QTSS_NoErr;
}

UInt8 RTSPSessionInterface::GetTwoChannelNumbers(StrPtrLen* inRTSPSessionID)
{
	//
	// Allocate a TCP coalesce buffer if still needed
	if (fTCPCoalesceBuffer != nullptr)
		fTCPCoalesceBuffer = new char[kTCPCoalesceBufferSize];

	//
	// Allocate 2 channel numbers
	UInt8 theChannelNum = fCurChannelNum;
	fCurChannelNum += 2;

	//
	// Reallocate the Ch# to Session ID Map
	UInt32 numChannelEntries = fCurChannelNum >> 1;
	StrPtrLen* newMap = NEW StrPtrLen[numChannelEntries];
	if (fChNumToSessIDMap != nullptr)
	{
		Assert(numChannelEntries > 1);
		::memcpy(newMap, fChNumToSessIDMap, sizeof(StrPtrLen) * (numChannelEntries - 1));
		delete[] fChNumToSessIDMap;
	}
	fChNumToSessIDMap = newMap;

	//
	// Put this sessionID to the proper place in the map
	fChNumToSessIDMap[numChannelEntries - 1].Set(inRTSPSessionID->GetAsCString(), inRTSPSessionID->Len);

	return theChannelNum;
}

StrPtrLen*  RTSPSessionInterface::GetSessionIDForChannelNum(UInt8 inChannelNum)
{
	if (inChannelNum < fCurChannelNum)
		return &fChNumToSessIDMap[inChannelNum >> 1];
	else
		return nullptr;
}

/*********************************
/
/   InterleavedWrite
/
/   Write the given RTP packet out on the RTSP channel in interleaved format.
/
*/

QTSS_Error RTSPSessionInterface::InterleavedWrite(void* inBuffer, UInt32 inLen, UInt32* outLenWritten, unsigned char channel)
{

	if (inLen == 0 && fNumInCoalesceBuffer == 0)
	{
		if (outLenWritten != nullptr)
			*outLenWritten = 0;
		return QTSS_NoErr;
	}

	// First attempt to grab the RTSPSession mutex. This is to prevent writing data to
	// the connection at the same time an RTSPRequest is being processed. We cannot
	// wait for this mutex to be freed (there would be a deadlock possibility), so
	// just try to grab it, and if we can't, then just report it as an EAGAIN
	if (this->GetSessionMutex()->TryLock() == false)
	{
		return EAGAIN;
	}

	// DMS - this struct should be packed.
	//rt todo -- is this struct more portable (byte alignment could be a problem)?
	struct  RTPInterleaveHeader
	{
		unsigned char header;
		unsigned char channel;
		UInt16      len;
	};

	struct  iovec               iov[3];
	QTSS_Error                  err = QTSS_NoErr;



	// flush rules
	if ((inLen > kTCPCoalesceDirectWriteSize || inLen == 0) && fNumInCoalesceBuffer > 0
		|| (inLen + fNumInCoalesceBuffer + kInteleaveHeaderSize > kTCPCoalesceBufferSize) && fNumInCoalesceBuffer > 0
		)
	{
		UInt32      buffLenWritten;

		// skip iov[0], WriteV uses it
		iov[1].iov_base = fTCPCoalesceBuffer;
		iov[1].iov_len = fNumInCoalesceBuffer;

		err = this->GetOutputStream()->WriteV(iov, 2, fNumInCoalesceBuffer, &buffLenWritten, RTSPResponseStream::kAllOrNothing);

#if RTSP_SESSION_INTERFACE_DEBUGGING 
		qtss_printf("InterleavedWrite: flushing %li\n", fNumInCoalesceBuffer);
#endif

		if (err == QTSS_NoErr)
			fNumInCoalesceBuffer = 0;
	}



	if (err == QTSS_NoErr)
	{

		if (inLen > kTCPCoalesceDirectWriteSize)
		{
			struct RTPInterleaveHeader  rih;

			// write direct to stream
			rih.header = '$';
			rih.channel = channel;
			rih.len = htons((UInt16)inLen);

			iov[1].iov_base = (char*)&rih;
			iov[1].iov_len = sizeof(rih);

			iov[2].iov_base = (char*)inBuffer;
			iov[2].iov_len = inLen;

			err = this->GetOutputStream()->WriteV(iov, 3, inLen + sizeof(rih), outLenWritten, RTSPResponseStream::kAllOrNothing);

#if RTSP_SESSION_INTERFACE_DEBUGGING 
			qtss_printf("InterleavedWrite: bypass %li\n", inLen);
#endif

		}
		else
		{
			// coalesce with other small writes

			fTCPCoalesceBuffer[fNumInCoalesceBuffer] = '$';
			fNumInCoalesceBuffer++;;

			fTCPCoalesceBuffer[fNumInCoalesceBuffer] = channel;
			fNumInCoalesceBuffer++;

			//*((short*)&fTCPCoalesceBuffer[fNumInCoalesceBuffer]) = htons(inLen);
			// if we ever turn TCPCoalesce back on, this should be optimized
			// for processors w/o alignment restrictions as above.

			SInt16  pcketLen = htons((UInt16)inLen);
			::memcpy(&fTCPCoalesceBuffer[fNumInCoalesceBuffer], &pcketLen, 2);
			fNumInCoalesceBuffer += 2;

			::memcpy(&fTCPCoalesceBuffer[fNumInCoalesceBuffer], inBuffer, inLen);
			fNumInCoalesceBuffer += inLen;

#if RTSP_SESSION_INTERFACE_DEBUGGING 
			qtss_printf("InterleavedWrite: coalesce %li, total bufff %li\n", inLen, fNumInCoalesceBuffer);
#endif
		}
	}

	if (err == QTSS_NoErr)
	{
		/*  if no error sure to correct outLenWritten, cuz WriteV above includes the interleave header count

			 GetOutputStream()->WriteV guarantees all or nothing for writes
			 if no error, then all was written.
		*/
		if (outLenWritten != nullptr)
			*outLenWritten = inLen;
	}

	this->GetSessionMutex()->Unlock();


	return err;

}

/*
	take the TCP socket away from a RTSP session that's
	waiting to be snarfed.

*/

void    RTSPSessionInterface::SnarfInputSocket(RTSPSessionInterface* fromRTSPSession)
{
	Assert(fromRTSPSession != nullptr);
	Assert(fromRTSPSession->fOutputSocketP != nullptr);

	// grab the unused, but already read fromsocket data
	// this should be the first RTSP request
	if (sDoBase64Decoding)
		fInputStream.IsBase64Encoded(true); // client sends all data base64 encoded
	fInputStream.SnarfRetreat(fromRTSPSession->fInputStream);

	if (fInputSocketP == fOutputSocketP)
		fInputSocketP = NEW TCPSocket(this, Socket::kNonBlockingSocketType);
	else
		fInputSocketP->Cleanup();   // if this is a socket replacing an old socket, we need
									// to make sure the file descriptor gets closed
	fInputSocketP->SnarfSocket(fromRTSPSession->fSocket);

	// fInputStream, meet your new input socket
	fInputStream.AttachToSocket(fInputSocketP);
}

void* RTSPSessionInterface::SetupParams(QTSSDictionary* inSession, UInt32* /*outLen*/)
{
	RTSPSessionInterface* theSession = (RTSPSessionInterface*)inSession;

	theSession->fLocalAddr = theSession->fSocket.GetLocalAddr();
	theSession->fRemoteAddr = theSession->fSocket.GetRemoteAddr();

	theSession->fLocalPort = theSession->fSocket.GetLocalPort();
	theSession->fRemotePort = theSession->fSocket.GetRemotePort();

	StrPtrLen* theLocalAddrStr = theSession->fSocket.GetLocalAddrStr();
	StrPtrLen* theLocalDNSStr = theSession->fSocket.GetLocalDNSStr();
	StrPtrLen* theRemoteAddrStr = theSession->fSocket.GetRemoteAddrStr();
	if (theLocalAddrStr == nullptr || theLocalDNSStr == nullptr || theRemoteAddrStr == nullptr)
	{    //the socket is bad most likely values are all 0. If the socket had an error we shouldn't even be here.
		 //theLocalDNSStr is set to localAddr if it is unavailable, so it should be present at this point as well.
		Assert(0);   //for debugging
		return nullptr; //nothing to set
	}
	theSession->SetVal(qtssRTSPSesLocalAddr, &theSession->fLocalAddr, sizeof(theSession->fLocalAddr));
	theSession->SetVal(qtssRTSPSesLocalAddrStr, theLocalAddrStr->Ptr, theLocalAddrStr->Len);
	theSession->SetVal(qtssRTSPSesLocalDNS, theLocalDNSStr->Ptr, theLocalDNSStr->Len);
	theSession->SetVal(qtssRTSPSesRemoteAddr, &theSession->fRemoteAddr, sizeof(theSession->fRemoteAddr));
	theSession->SetVal(qtssRTSPSesRemoteAddrStr, theRemoteAddrStr->Ptr, theRemoteAddrStr->Len);

	theSession->SetVal(qtssRTSPSesLocalPort, &theSession->fLocalPort, sizeof(theSession->fLocalPort));
	theSession->SetVal(qtssRTSPSesRemotePort, &theSession->fRemotePort, sizeof(theSession->fRemotePort));
	return nullptr;
}

void RTSPSessionInterface::SaveOutputStream()
{
	Assert(fOldOutputStreamBuffer.Ptr == nullptr);
	fOldOutputStreamBuffer.Ptr = NEW char[fOutputStream.GetBytesWritten()];
	fOldOutputStreamBuffer.Len = fOutputStream.GetBytesWritten();
	::memcpy(fOldOutputStreamBuffer.Ptr, fOutputStream.GetBufPtr(), fOldOutputStreamBuffer.Len);
}

void RTSPSessionInterface::RevertOutputStream()
{
	Assert(fOldOutputStreamBuffer.Ptr != nullptr);
	Assert(fOldOutputStreamBuffer.Len != 0);
	static StrPtrLen theRTTStr(";rtt=", 5);

	if (fOldOutputStreamBuffer.Ptr != nullptr)
	{
		//fOutputStream.Put(fOldOutputStreamBuffer);		
		StringParser theStreamParser(&fOldOutputStreamBuffer);
		StrPtrLen theHeader;
		StrPtrLen theEOL;
		StrPtrLen theField;
		StrPtrLen theValue;
		while (theStreamParser.GetDataRemaining() != 0)
		{
			theStreamParser.ConsumeUntil(&theHeader, StringParser::sEOLMask);
			if (theHeader.Len != 0)
			{
				fOutputStream.Put(theHeader);

				StringParser theHeaderParser(&theHeader);
				theHeaderParser.ConsumeUntil(&theField, ':');
				if (theHeaderParser.PeekFast() == ':')
				{
					if (theField.Equal(RTSPProtocol::GetHeaderString(qtssXDynamicRateHeader)))
					{
						fOutputStream.Put(theRTTStr);
						fOutputStream.Put(fRoundTripTime);
					}
				}
			}
			theStreamParser.ConsumeEOL(&theEOL);
			fOutputStream.Put(theEOL);
		}

		fOldOutputStreamBuffer.Delete();
	}
}

void RTSPSessionInterface::SendOptionsRequest()
{
	static StrPtrLen	sOptionsRequestHeader("OPTIONS * RTSP/1.0\r\nContent-Type: application/x-random-data\r\nContent-Length: 1400\r\n\r\n");

	fOutputStream.Put(sOptionsRequestHeader);
	fOutputStream.Put((char*)(RTSPSessionInterface::sOptionsRequestBody), 1400);

	fOptionsRequestSendTime = OS::Milliseconds();
	fSentOptionsRequest = true;
	fRoundTripTimeCalculation = false;
}
