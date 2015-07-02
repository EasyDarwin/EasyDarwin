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
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       RTSPRequestStream.cpp

    Contains:   Implementation of RTSPRequestStream class. 
*/


#include "HttpRequestStream.h"
#include "StringParser.h"
#include "OSMemory.h"
#include "base64.h"
#include "OSArrayObjectDeleter.h"
#include "OS.h"

#include <errno.h>

#define READ_DEBUGGING 0

HttpRequestStream::HttpRequestStream(TCPSocket* sock)
:   fSocket(sock),
    fRetreatBytes(0), 
    fRetreatBytesRead(0),
    fCurOffset(0),
    fRequest(fRequestBuffer, 0),
    fRequestPtr(NULL),
    fPrintMSG(false)
{}


/*
	读取网络报文进行处理，区分报文头部、Content、以及数据包
*/
QTSS_Error HttpRequestStream::ReadRequest()
{
	while (true)
	{
		UInt32 newOffset = 0;

		//If this is the case, we already HAVE a request on this session, and we now are done
		//with the request and want to move onto the next one. The first thing we should do
		//is check whether there is any lingering(逗留的) data in the stream. If there is, the parent
		//session believes that is part of a new request

		//进行request  切换时的操作
		if (fRequestPtr != NULL)
		{
			//清0
			fRequestPtr = NULL;//flag that we no longer have a complete request

			//把遗留下来的数据copy 进入新的request
			// Take all the retreated leftover data and move it to the beginning of the buffer
			if ((fRetreatBytes > 0) && (fRequest.Len > 0))
				::memmove(fRequest.Ptr, fRequest.Ptr + fRequest.Len + fRetreatBytesRead, fRetreatBytes);

			fCurOffset = fRetreatBytes;

			newOffset = fRequest.Len = fRetreatBytes;
			fRetreatBytes = fRetreatBytesRead = 0;
		}

	        // We don't have any new data, so try and get some
	        if (newOffset == 0)
	        {
			// We don't have any new data, get some from the socket...
			QTSS_Error sockErr = fSocket->Read(&fRequestBuffer[fCurOffset], 
			                    (QTSS_MAX_HTTP_REQUEST_LENGTH - fCurOffset) - 1, &newOffset);
			//assume the client is dead if we get an error back
			if (sockErr == EAGAIN)
				return QTSS_NoErr;
			if (sockErr != QTSS_NoErr)
			{
				Assert(!fSocket->IsConnected());
				return sockErr;
			}
			
			fRequest.Len += newOffset;
		       	Assert(fRequest.Len < QTSS_MAX_HTTP_REQUEST_LENGTH);
			fCurOffset += newOffset;
		}
		Assert(newOffset > 0);

	        if (fPrintMSG)
	        {
			DateBuffer theDate;
			DateTranslator::UpdateDateBuffer(&theDate, 0); // get the current GMT date and time
			qtss_printf("\n\n#C->S:\n#time: ms=%"_U32BITARG_" date=%s\n", (UInt32) OS::StartTimeMilli_Int(), theDate.GetDateBuffer());
			if (fSocket != NULL)    
			{
				UInt16 serverPort = fSocket->GetLocalPort();
				UInt16 clientPort = fSocket->GetRemotePort();    
				StrPtrLen* theLocalAddrStr = fSocket->GetLocalAddrStr();
				StrPtrLen* theRemoteAddrStr = fSocket->GetRemoteAddrStr();
				if (theLocalAddrStr != NULL)
				{
					qtss_printf("#server: ip="); theLocalAddrStr->PrintStr(); qtss_printf(" port=%u\n" , serverPort );
				}
				else
				{
					qtss_printf("#server: ip=NULL port=%u\n" , serverPort );
				}

				if (theRemoteAddrStr != NULL)
				{
					qtss_printf("#client: ip="); theRemoteAddrStr->PrintStr(); qtss_printf(" port=%u\n" , clientPort );
				}
				else
				{
					qtss_printf("#client: ip=NULL port=%u\n" , clientPort );
				}
			}
			StrPtrLen str(fRequest);
			str.PrintStrEOL("\n\r\n", "\n");// print the request but stop on \n\r\n and add a \n afterwards.
		}
	        
		//use a StringParser object to search for a double EOL, which signifies the end of
		//the header.
		Bool16 weAreDone = false;
		StringParser headerParser(&fRequest);
		while (headerParser.GetThruEOL(NULL))
		{
			if (headerParser.ExpectEOL())
			{
				//The legal end-of-header sequences are \r\r, \r\n\r\n, & \n\n. NOT \r\n\r!
				//If the packets arrive just a certain way, we could get here with the latter
				//combo, and not wait for a final \n.
				if ((headerParser.GetDataParsedLen() > 2) &&
				(memcmp(headerParser.GetCurrentPosition() - 3, "\r\n\r", 3) == 0))//排除\r\n\r 的情况
					continue;
				weAreDone = true;
				break;
			}
		}
	        //weAreDone means we have gotten a full request
	        if (weAreDone)
	        {
	            //put back any data that is not part of the header
	            fRequest.Len -= headerParser.GetDataRemaining();
	            fRetreatBytes += headerParser.GetDataRemaining();
	            fRequestPtr = &fRequest;
	            return QTSS_RequestArrived;
	        }
	        
	        //check for a full buffer
	        if (fCurOffset == QTSS_MAX_HTTP_REQUEST_LENGTH - 1)
	        {
	            fRequestPtr = &fRequest;
	            return E2BIG;
	        }
	}
}

QTSS_Error HttpRequestStream::Read(void* ioBuffer, UInt32 inBufLen, UInt32* outLengthRead)
{
    UInt32 theLengthRead = 0;
    UInt8* theIoBuffer = (UInt8*)ioBuffer;
    
    //
    // If there are retreat bytes available, read them first.
    if (fRetreatBytes > 0)
    {
        theLengthRead = fRetreatBytes;
        if (inBufLen < theLengthRead)
            theLengthRead = inBufLen;
        ::memcpy(theIoBuffer, fRequest.Ptr + fRequest.Len + fRetreatBytesRead, theLengthRead);
        
        //
        // We should not update fRequest.Len even though we've read some of the retreat bytes.
        // fRequest.Len always refers to the length of the request header. Instead, we
        // have a separate variable, fRetreatBytesRead
        fRetreatBytes -= theLengthRead;
        fRetreatBytesRead += theLengthRead;
#if READ_DEBUGGING
        qtss_printf("In HttpRequestStream::Read: Got %d Retreat Bytes\n",theLengthRead);
#endif  
    }

    //
    // If there is still space available in ioBuffer, continue. Otherwise, we can return now
    if (theLengthRead == inBufLen)
    {
        if (outLengthRead != NULL)
            *outLengthRead = theLengthRead;
        return QTSS_NoErr;
    }
    
    //
    // Read data directly from the socket and place it in our buffer
    UInt32 theNewOffset = 0;
    QTSS_Error theErr = fSocket->Read(&theIoBuffer[theLengthRead], inBufLen - theLengthRead, &theNewOffset);
#if READ_DEBUGGING
    qtss_printf("In HttpRequestStream::Read: Got %d bytes off Socket\n",theNewOffset);
#endif  
    if (outLengthRead != NULL)
        *outLengthRead = theNewOffset + theLengthRead;
        
    return theErr;
}

