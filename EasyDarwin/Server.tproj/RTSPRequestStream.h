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
	 File:       RTSPRequestStream.h

	 Contains:   Provides a stream abstraction for RTSP. Given a socket, this object
				 can read in data until an entire RTSP request header is available.
				 (do this by calling ReadRequest). It handles RTSP pipelining (request
				 headers are produced serially even if multiple headers arrive simultaneously),
				 & RTSP request data.
 */

#ifndef __RTSPREQUESTSTREAM_H__
#define __RTSPREQUESTSTREAM_H__


 //INCLUDES
#include "StrPtrLen.h"
#include "TCPSocket.h"
#include "QTSS.h"

class RTSPRequestStream
{
public:

	//CONSTRUCTOR / DESTRUCTOR
	RTSPRequestStream(TCPSocket* sock);

	// We may have to delete this memory if it was allocated due to base64 decoding
	~RTSPRequestStream() { if (fRequest.Ptr != &fRequestBuffer[0]) delete[] fRequest.Ptr; }

	//ReadRequest
	//This function will not block.
	//Attempts to read data into the stream, stopping when we hit the EOL - EOL that
	//ends an RTSP header.
	//
	//Returns:          QTSS_NoErr:     Out of data, haven't hit EOL - EOL yet
	//                  QTSS_RequestArrived: full request has arrived
	//                  E2BIG: ran out of buffer space
	//                  QTSS_RequestFailed: if the client has disconnected
	//                  EINVAL: if we are base64 decoding and the stream is corrupt
	//                  QTSS_OutOfState: 
	QTSS_Error      ReadRequest();

	// Read
	//
	// This function reads data off of the stream, and places it into the buffer provided
	// Returns: QTSS_NoErr, EAGAIN if it will block, or another socket error.
	QTSS_Error      Read(void* ioBuffer, UInt32 inBufLen, UInt32* outLengthRead);

	// Use a different TCPSocket to read request data 
	// this will be used by RTSPSessionInterface::SnarfInputSocket
	void                AttachToSocket(TCPSocket* sock) { fSocket = sock; }

	// Tell the request stream whether or not to decode from base64.
	void                IsBase64Encoded(bool isDataEncoded) { fDecode = isDataEncoded; }

	//GetRequestBuffer
	//This returns a buffer containing the full client request. The length is set to
	//the exact length of the request headers. This will return NULL UNLESS this object
	//is in the proper state (has been initialized, ReadRequest has been called until it returns
		//RequestArrived).
	StrPtrLen*  GetRequestBuffer() { return fRequestPtr; }
	bool      IsDataPacket() { return fIsDataPacket; }
	void        ShowRTSP(bool enable) { fPrintRTSP = enable; }
	void SnarfRetreat(RTSPRequestStream &fromRequest);

private:


	//CONSTANTS:
	enum
	{
		kRequestBufferSizeInBytes = QTSS_MAX_REQUEST_BUFFER_SIZE        //UInt32
	};

	// Base64 decodes into fRequest.Ptr, updates fRequest.Len, and returns the amount
	// of data left undecoded in inSrcData
	QTSS_Error              DecodeIncomingData(char* inSrcData, UInt32 inSrcDataLen);

	TCPSocket*              fSocket;
	UInt32                  fRetreatBytes;
	UInt32                  fRetreatBytesRead; // Used by Read() when it is reading RetreatBytes

	char                    fRequestBuffer[kRequestBufferSizeInBytes];
	UInt32                  fCurOffset; // tracks how much valid data is in the above buffer
	UInt32                  fEncodedBytesRemaining; // If we are decoding, tracks how many encoded bytes are in the buffer

	StrPtrLen               fRequest;
	StrPtrLen*              fRequestPtr;    // pointer to a request header
	bool                  fDecode;        // should we base 64 decode?
	bool                  fIsDataPacket;  // is this a data packet? Like for a record?
	bool                  fPrintRTSP;     // debugging printfs

};

#endif
