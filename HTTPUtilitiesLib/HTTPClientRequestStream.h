/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
	File:       HTTPClientRequestStream.h

	Contains:   Provides a stream abstraction for HTTP. Given a socket, this object
				can read in data until an entire HTTP request header is available.
				(do this by calling ReadRequest). It handles HTTP pipelining (request
				headers are produced serially even if multiple headers arrive simultaneously),
				& HTTP request data.
*/

#ifndef __HTTPClient_REQUEST_STREAM_H__
#define __HTTPClient_REQUEST_STREAM_H__


//INCLUDES
#include "StrPtrLen.h"
#include "ClientSocket.h"
#include "QTSS.h"

class HTTPClientRequestStream
{
public:

	//CONSTRUCTOR / DESTRUCTOR
	HTTPClientRequestStream(ClientSocket* sock);

	// We may have to delete this memory if it was allocated due to base64 decoding
	~HTTPClientRequestStream() { if (fRequest.Ptr != &fRequestBuffer[0]) delete[] fRequest.Ptr; }

	//ReadRequest
	//This function will not block.
	//Attempts to read data into the stream, stopping when we hit the EOL - EOL that
	//ends an HTTP header.
	//
	//Returns:          QTSS_NoErr:     Out of data, haven't hit EOL - EOL yet
	//                  QTSS_RequestArrived: full request has arrived
	//                  E2BIG: ran out of buffer space
	//                  QTSS_RequestFailed: if the client has disconnected
	//                  EINVAL: if we are base64 decoding and the stream is corrupt
	//                  QTSS_OutOfState: 
	QTSS_Error			ReadRequest();

	// Read
	//
	// This function reads data off of the stream, and places it into the buffer provided
	// Returns: QTSS_NoErr, EAGAIN if it will block, or another socket error.
	QTSS_Error			Read(void* ioBuffer, UInt32 inBufLen, UInt32* outLengthRead);

	// Use a different TCPSocket to read request data 
	// this will be used by RTSPSessionInterface::SnarfInputSocket
	void                AttachToSocket(ClientSocket* sock) { ResetRequestBuffer(); fSocket = sock; }

	void				ResetRequestBuffer();

	// Tell the request stream whether or not to decode from base64.
	void                IsBase64Encoded(bool isDataEncoded) { fDecode = isDataEncoded; }

	//GetRequestBuffer
	//This returns a buffer containing the full client request. The length is set to
	//the exact length of the request headers. This will return NULL UNLESS this object
	//is in the proper state (has been initialized, ReadRequest has been called until it returns
	//RequestArrived).
	StrPtrLen*			GetRequestBuffer() { return fRequestPtr; }
	bool				IsDataPacket() { return fIsDataPacket; }
	void				ShowMsg(bool enable) { fPrintMsg = enable; }
	void				SnarfRetreat(HTTPClientRequestStream &fromRequest);

private:
	//CONSTANTS:
	// Base64 decodes into fRequest.Ptr, updates fRequest.Len, and returns the amount
	// of data left undecoded in inSrcData
	QTSS_Error              DecodeIncomingData(char* inSrcData, UInt32 inSrcDataLen);

	ClientSocket*			fSocket;
	UInt32                  fRetreatBytes;
	UInt32                  fRetreatBytesRead; // Used by Read() when it is reading RetreatBytes

	char                    fRequestBuffer[QTSS_MAX_REQUEST_BUFFER_SIZE];
	UInt32                  fCurOffset; // tracks how much valid data is in the above buffer
	UInt32                  fEncodedBytesRemaining; // If we are decoding, tracks how many encoded bytes are in the buffer

	StrPtrLen               fRequest;
	StrPtrLen*              fRequestPtr;    // pointer to a request header
	bool                  fDecode;        // should we base 64 decode?
	bool                  fIsDataPacket;  // is this a data packet? Like for a record?
	bool                  fPrintMsg;     // debugging printfs

};

#endif
