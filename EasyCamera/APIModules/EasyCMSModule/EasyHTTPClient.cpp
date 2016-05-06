/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#include "EasyHTTPClient.h"
#include "OSMemory.h"
#include "MyAssert.h"
#include "HTTPRequest.h"
#include "QTSS.h"

#ifndef _WIN32
#define Sleep(x) usleep(x*1000)
#endif

static char* sEmptyString = "";

EasyHTTPClient::EasyHTTPClient(ClientSocket* inSocket, Bool16 verbosePrinting)
:	fSocket(inSocket),
	fVerboseLevel(verbosePrinting ? 1 : 0),
	fCSeq(1),
	fStatus(0),
	fSessionID(sEmptyString),
	fContentLength(0),
	fRecvContentBuffer(NULL),
	fContentRecvLen(0),
	fHeaderRecvLen(0),
	fHeaderLen(0),
	fSetupTrackID(0),
	fState(kInitial)
{
	::memset(fSendBuffer, 0,EASY_REQUEST_BUFFER_SIZE_LEN + 1);
	::memset(fRecvHeaderBuffer, 0,EASY_REQUEST_BUFFER_SIZE_LEN + 1);
}

EasyHTTPClient::~EasyHTTPClient()
{
	delete [] fRecvContentBuffer;
	delete [] fURL.Ptr;
	if (fSessionID.Ptr != sEmptyString)
		delete [] fSessionID.Ptr;
}

void EasyHTTPClient::Set(const StrPtrLen& inURL)
{
	delete [] fURL.Ptr;
	fURL.Ptr = NEW char[inURL.Len + 2];
	fURL.Len = inURL.Len;
	char* destPtr = fURL.Ptr;
    
	// add a leading '/' to the url if it isn't a full URL and doesn't have a leading '/'
	if ( !inURL.NumEqualIgnoreCase("http://", strlen("http://")) && inURL.Ptr[0] != '/')
	{
		*destPtr = '/';
		destPtr++;
		fURL.Len++;
	}
	::memcpy(destPtr, inURL.Ptr, inURL.Len);
	fURL.Ptr[fURL.Len] = '\0';
	
}

OS_Error EasyHTTPClient::SendHttpPacket(char *respXML)
{
	if (!IsTransactionInProgress())
	{
		qtss_sprintf(fMethod,"%s","POST");

		UInt32 contentLen = strlen(respXML);

		StringFormatter fmt(fSendBuffer, EASY_REQUEST_BUFFER_SIZE_LEN);
		fmt.PutFmtStr(
				"POST %s HTTP/1.1\r\n"
				"CSeq: %d\r\n"
				"Content-Length: %d\r\n",
				fURL.Ptr == NULL? "/":fURL.Ptr, fCSeq, contentLen);
		
		fmt.PutFmtStr("\r\n");

		fmt.PutFmtStr(respXML);

		fmt.PutTerminator();
	}
	return this->DoTransaction();
}

OS_Error EasyHTTPClient::SendPacket(char *msg)
{
	OS_Error theErr = OS_NoErr;
	
	qtss_sprintf(fMethod,"%s","POST");

	UInt32 contentLen = strlen(msg);

	StringFormatter fmt(fSendBuffer, EASY_REQUEST_BUFFER_SIZE_LEN);
	fmt.PutFmtStr(
			"POST %s HTTP/1.1\r\n"
			"CSeq: %d\r\n"
			"Content-Length: %d\r\n",
			fURL.Ptr == NULL? "www.easydarwin.org/EasyCMS":fURL.Ptr, fCSeq, contentLen);
		
	fmt.PutFmtStr("\r\n");
	fmt.PutFmtStr(msg);
	fmt.PutTerminator();

	StrPtrLen theRequest(fSendBuffer, ::strlen(fSendBuffer));

	theErr = fSocket->Send(theRequest.Ptr, theRequest.Len);

	return theErr;
}

OS_Error EasyHTTPClient::DoTransaction()
{
		OS_Error theErr = OS_NoErr;
		StrPtrLen theRequest(fSendBuffer, ::strlen(fSendBuffer));
		StrPtrLen theMethod(fMethod);
    
		for(;;)
		{
			switch(fState)
			{
				//Initial state: getting ready to send the request; the authenticator is initialized if it exists.
				//This is the only state where a new request can be made.
				case kInitial:
				
        			fCSeq++;	//this assumes that the sequence number will not be read again until the next transaction

					fState = kRequestSending;
					break;

				//Request Sending state: keep on calling Send while Send returns EAGAIN or EINPROGRESS
				case kRequestSending:
        			theErr = fSocket->Send(theRequest.Ptr, theRequest.Len);

					if (theErr == EAGAIN || theErr == EINPROGRESS)
					{
	/*					Sleep(100);
						continue;*/
						printf("Err:%d   ", theErr);
					}
        
        			if (theErr != OS_NoErr)
					{
						return theErr;
					}

					printf("Send HTTP:\n%s",theRequest.Ptr);

					//Done sending request; moving onto the response
        			fContentRecvLen = 0;
        			fHeaderRecvLen = 0;
        			fHeaderLen = 0;
        			::memset(fRecvHeaderBuffer, 0, EASY_REQUEST_BUFFER_SIZE_LEN+1);

            		fState = kResponseReceiving;
					break;

				//Response Receiving state: keep on calling ReceiveResponse while it returns EAGAIN or EINPROGRESS
				case kResponseReceiving:
				//Header Received state: the response header has been received(and parsed), but the entity(response content) has not been completely received
				case kHeaderReceived:
        			theErr = this->ReceiveResponse();  //note that this function can change the fState
    
        			if (theErr != OS_NoErr)
            			return theErr;
					printf("Recv HTTP:\n%s%s",fRecvHeaderBuffer, fRecvContentBuffer);
					return OS_NoErr;
					break;
			}
		}
		Assert(false);  //not reached
		return 0;
}

void EasyHTTPClient::ResetRecvBuf()
{
	//Done sending request; moving onto the response
	fContentRecvLen = 0;
	fHeaderRecvLen = 0;
	fHeaderLen = 0;
	::memset(fRecvHeaderBuffer, 0, EASY_REQUEST_BUFFER_SIZE_LEN+1);
}

//This implementation cannot parse interleaved headers with entity content.
OS_Error EasyHTTPClient::ReceiveResponse()
{
		Assert(fState == kResponseReceiving | fState == kHeaderReceived);
		OS_Error theErr = OS_NoErr;

		while (fState == kResponseReceiving)
		{
			UInt32 theRecvLen = 0;
			//fRecvHeaderBuffer[0] = 0;
			theErr = fSocket->Read(&fRecvHeaderBuffer[fHeaderRecvLen], EASY_REQUEST_BUFFER_SIZE_LEN - fHeaderRecvLen, &theRecvLen);

			if (theErr != OS_NoErr)
				return theErr;
        
			fHeaderRecvLen += theRecvLen;
			fRecvHeaderBuffer[fHeaderRecvLen] = 0;

			//fRecvHeaderBuffer[fHeaderRecvLen] = '\0';
			// Check to see if we've gotten a complete header, and if the header has even started       
			// The response may not start with the response if we are interleaving media data,
			// in which case there may be leftover media data in the stream. If we encounter any
			// of this cruft, we can just strip it off.
			char* theHeaderStart = ::strstr(fRecvHeaderBuffer, "HTTP");
			if (theHeaderStart == NULL)
			{
				fHeaderRecvLen = 0;
				continue;
			}
			else if (theHeaderStart != fRecvHeaderBuffer)
			{
				//strip off everything before the RTSP
				fHeaderRecvLen -= theHeaderStart - fRecvHeaderBuffer;
				::memmove(fRecvHeaderBuffer, theHeaderStart, fHeaderRecvLen);
				//fRecvHeaderBuffer[fHeaderRecvLen] = '\0';
			}

			char* theResponseData = ::strstr(fRecvHeaderBuffer, "\r\n\r\n");    

			if (theResponseData != NULL)
			{               
				// skip past the \r\n\r\n
				theResponseData += 4;
            
				// We've got a new response
				fState = kHeaderReceived;
            
				// Figure out how much of the content body we've already received
				// in the header buffer. If we are interleaving, this may also be packet data
				fHeaderLen = theResponseData - &fRecvHeaderBuffer[0];
				fContentRecvLen = fHeaderRecvLen - fHeaderLen;

				// Zero out fields that will change with every RTSP response
				fStatus = 0;
				fContentLength = 0;
        
				// Parse the response.
				StrPtrLen theData(fRecvHeaderBuffer, fHeaderLen);
				StringParser theParser(&theData);
            
				theParser.ConsumeLength(NULL, 9); //skip past RTSP/1.0
				fStatus = theParser.ConsumeInteger(NULL);
            
				StrPtrLen theLastHeader;
				while (theParser.GetDataRemaining() > 0)
				{
					static StrPtrLen sSessionHeader("Session");
					static StrPtrLen sContentLenHeader("Content-length");
					static StrPtrLen sSameAsLastHeader(" ,");
                
					StrPtrLen theKey;
					theParser.GetThruEOL(&theKey);
                
					if (theKey.NumEqualIgnoreCase(sSessionHeader.Ptr, sSessionHeader.Len))
					{
						if (fSessionID.Len == 0)
						{
							// Copy the session ID and store it.
							// First figure out how big the session ID is. We copy
							// everything up until the first ';' returned from the server
							UInt32 keyLen = 0;
							while ((theKey.Ptr[keyLen] != ';') && (theKey.Ptr[keyLen] != '\r') && (theKey.Ptr[keyLen] != '\n'))
								keyLen++;
                        
							// Append an EOL so we can stick this thing transparently into the SETUP request
                        
							fSessionID.Ptr = NEW char[keyLen + 3];
							fSessionID.Len = keyLen + 2;
							::memcpy(fSessionID.Ptr, theKey.Ptr, keyLen);
							::memcpy(fSessionID.Ptr + keyLen, "\r\n", 2);//Append a EOL
							fSessionID.Ptr[keyLen + 2] = '\0';
						}
					}
					else if (theKey.NumEqualIgnoreCase(sContentLenHeader.Ptr, sContentLenHeader.Len))
					{
						//exclusive with interleaved
						StringParser theCLengthParser(&theKey);
						theCLengthParser.ConsumeUntil(NULL, StringParser::sDigitMask);
						fContentLength = theCLengthParser.ConsumeInteger(NULL);
                    
						delete [] fRecvContentBuffer;
						fRecvContentBuffer = NEW char[fContentLength + 1];
						::memset(fRecvContentBuffer, '\0', fContentLength + 1);
                    
						// Immediately copy the bit of the content body that we've already
						// read off of the socket.
						//Assert(fContentRecvLen <= fContentLength)

						//如果fContentRecvLen(读取到的Content数据)大于fContentLength(Content-Length指定长度)
						//按照Content-Length指定长度来
						if(fContentRecvLen > fContentLength)
							memcpy(fRecvContentBuffer, theResponseData, fContentLength);
						else
							memcpy(fRecvContentBuffer, theResponseData, fContentRecvLen);

					}
                
					theLastHeader = theKey;
				}
            
				//// Check to see if there is any packet data in the header buffer; everything that is left should be packet data
				//if (fContentRecvLen > fContentLength)
				//{
				//    fPacketDataInHeaderBuffer = theResponseData + fContentLength;
				//    fPacketDataInHeaderBufferLen = fContentRecvLen - fContentLength;
				//}
			}
			
			if (fHeaderRecvLen == EASY_REQUEST_BUFFER_SIZE_LEN) //the "\r\n" is not found --> read more data
				return ENOBUFS; // This response is too big for us to handle!
		}
    
		//the advertised data length is less than what has been received...need to read more data
		while (fContentLength > fContentRecvLen)
		{
			UInt32 theContentRecvLen = 0;
			theErr = fSocket->Read(&fRecvContentBuffer[fContentRecvLen], fContentLength - fContentRecvLen, &theContentRecvLen);
			if (theErr != OS_NoErr)
			{
				//fEventMask = EV_RE;???
				return theErr;
			}
			fContentRecvLen += theContentRecvLen;       
		}

		return OS_NoErr;
}

/*
	功能：读取完整网络数据，注意报文分两个部分，一个Header，一个Content
*/
OS_Error EasyHTTPClient::ReceivePacket()
{
		OS_Error theErr = OS_NoErr;

#if 1
		ResetRecvBuf();
#endif

		//fContentRecvLen = 0;
		//fHeaderRecvLen = 0;
		//fHeaderLen = 0;
		////每次在接收报文之前，先清空报文接收缓冲，注意，还有一个Content的缓冲
		//::memset(fRecvHeaderBuffer, 0, kReqBufSize+1);

		while (1)
		{
			//读取报文到Header缓冲
			UInt32 theRecvLen = 0;
			theErr = fSocket->Read(&fRecvHeaderBuffer[fHeaderRecvLen], EASY_REQUEST_BUFFER_SIZE_LEN - fHeaderRecvLen, &theRecvLen);

			if (theErr != OS_NoErr)
				return theErr;
        
			fHeaderRecvLen += theRecvLen;
			fRecvHeaderBuffer[fHeaderRecvLen] = 0;

			//fRecvHeaderBuffer[fHeaderRecvLen] = '\0';
			// Check to see if we've gotten a complete header, and if the header has even started       
			// The response may not start with the response if we are interleaving media data,
			// in which case there may be leftover media data in the stream. If we encounter any
			// of this cruft, we can just strip it off.
			char* theHeaderStart = ::strstr(fRecvHeaderBuffer, "HTTP");
			if (theHeaderStart == NULL)
			{
				fHeaderRecvLen = 0;
				continue;
			}
			else if (theHeaderStart != fRecvHeaderBuffer)
			{
				//strip off everything before the HTTP
				fHeaderRecvLen -= theHeaderStart - fRecvHeaderBuffer;
				::memmove(fRecvHeaderBuffer, theHeaderStart, fHeaderRecvLen);
				//fRecvHeaderBuffer[fHeaderRecvLen] = '\0';
			}

			char* theResponseData = ::strstr(fRecvHeaderBuffer, "\r\n\r\n");    

			if (theResponseData != NULL)
			{               
				// skip past the \r\n\r\n
				theResponseData += 4;
            
            
				// Figure out how much of the content body we've already received
				// in the header buffer. If we are interleaving, this may also be packet data
				fHeaderLen = theResponseData - &fRecvHeaderBuffer[0];
				fContentRecvLen = fHeaderRecvLen - fHeaderLen;

				// Zero out fields that will change with every http response
				fStatus = 0;
				fContentLength = 0;
        
				// Parse the response.
				StrPtrLen theData(fRecvHeaderBuffer, fHeaderLen);
				StringParser theParser(&theData);
            
				theParser.ConsumeLength(NULL, 9); //skip past HTTP/1.0
				fStatus = theParser.ConsumeInteger(NULL);
				theParser.GetThruEOL(NULL);

				StrPtrLen theLastHeader;
				while (theParser.GetDataRemaining() > 0)
				{
					static StrPtrLen sSessionHeader("Session");
					static StrPtrLen sContentLenHeader("Content-length");
					static StrPtrLen sSameAsLastHeader(" ,");
                
					StrPtrLen theKey;
					theParser.GetThruEOL(&theKey);
                
					if (theKey.NumEqualIgnoreCase(sSessionHeader.Ptr, sSessionHeader.Len))
					{
						if (fSessionID.Len == 0)
						{
							// Copy the session ID and store it.
							// First figure out how big the session ID is. We copy
							// everything up until the first ';' returned from the server
							UInt32 keyLen = 0;
							while ((theKey.Ptr[keyLen] != ';') && (theKey.Ptr[keyLen] != '\r') && (theKey.Ptr[keyLen] != '\n'))
								keyLen++;
                        
							// Append an EOL so we can stick this thing transparently into the SETUP request
                        
							fSessionID.Ptr = NEW char[keyLen + 3];
							fSessionID.Len = keyLen + 2;
							::memcpy(fSessionID.Ptr, theKey.Ptr, keyLen);
							::memcpy(fSessionID.Ptr + keyLen, "\r\n", 2);//Append a EOL
							fSessionID.Ptr[keyLen + 2] = '\0';
						}
					}
					else if (theKey.NumEqualIgnoreCase(sContentLenHeader.Ptr, sContentLenHeader.Len))
					{
						//exclusive with interleaved
						StringParser theCLengthParser(&theKey);
						theCLengthParser.ConsumeUntil(NULL, StringParser::sDigitMask);
						fContentLength = theCLengthParser.ConsumeInteger(NULL);
                    
						delete [] fRecvContentBuffer;
						fRecvContentBuffer = NEW char[fContentLength + 1];
						::memset(fRecvContentBuffer, '\0', fContentLength + 1);
                    
						// Immediately copy the bit of the content body that we've already
						// read off of the socket.
	/*					Assert(fContentRecvLen <= fContentLength)*/
						
						//如果fContentRecvLen(读取到的Content数据)大于fContentLength(Content-Length指定长度)
						//按照Content-Length指定长度来
						if(fContentRecvLen > fContentLength)
							memcpy(fRecvContentBuffer, theResponseData, fContentLength);
						else
							memcpy(fRecvContentBuffer, theResponseData, fContentRecvLen);
					}
                
					theLastHeader = theKey;
				}
				break;
				//// Check to see if there is any packet data in the header buffer; everything that is left should be packet data
				//if (fContentRecvLen > fContentLength)
				//{
				//    fPacketDataInHeaderBuffer = theResponseData + fContentLength;
				//    fPacketDataInHeaderBufferLen = fContentRecvLen - fContentLength;
				//}
			}
			else if (fHeaderRecvLen == EASY_REQUEST_BUFFER_SIZE_LEN) //the "\r\n" is not found --> read more data
				return ENOBUFS; // This response is too big for us to handle!
		}
    
		//the advertised data length is less than what has been received...need to read more data
		while (fContentLength > fContentRecvLen)
		{
			UInt32 theContentRecvLen = 0;
			theErr = fSocket->Read(&fRecvContentBuffer[fContentRecvLen], fContentLength - fContentRecvLen, &theContentRecvLen);
			if (theErr != OS_NoErr)
			{
				//fEventMask = EV_RE;
				return theErr;
			}
			fContentRecvLen += theContentRecvLen;       
		}
		return OS_NoErr;
}


OS_Error EasyHTTPClient::SendBytes(char* bytes, unsigned int len)
{
	if(len == 0)
		return -10;
	OS_Error theErr = fSocket->Send((char*)bytes, len);
	
	return theErr;
}
