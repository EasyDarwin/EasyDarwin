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
#include "HTTPRequest.h"
#include "HTTPProtocol.h"
#include "OSMemory.h"
#include "StringParser.h"
#include "StringTranslator.h"
#include "ResizeableStringFormatter.h"
#include "DateTranslator.h"

StrPtrLen HTTPRequest::sColonSpace(": ", 2);
static Bool16 sFalse = false;
static Bool16 sTrue = true;
static StrPtrLen sCloseString("close", 5);
static StrPtrLen sKeepAliveString("keep-alive", 10);
static StrPtrLen sDefaultRealm("Streaming Server", 19);
UInt8 HTTPRequest::sURLStopConditions[] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //0-9      //'\t' is a stop condition
  1, 0, 0, 1, 0, 0, 0, 0, 0, 0, //10-19    //'\r' & '\n' are stop conditions
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
  0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //30-39    //' '
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40-49
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //60-69
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
  0, 0, 0, 0, 0, 0                         //250-255
};

// Constructor
HTTPRequest::HTTPRequest(StrPtrLen* serverHeader, StrPtrLen* requestPtr)
{
    // Store the pointer to the server header field
    fSvrHeader = *serverHeader;
   
    // Set initial state
    fRequestHeader = *requestPtr;
    fResponseHeader = NULL;
    fResponseFormatter = NULL;
    fMethod = httpIllegalMethod;
    fVersion = httpIllegalVersion;
    fAbsoluteURI = NULL;
    fRelativeURI = NULL;
    fAbsoluteURIScheme = NULL;
    fHostHeader = NULL;
    fRequestPath = NULL;    
    fStatusCode = httpOK;
    fRequestKeepAlive = false; // Default value when there is no version string

	//未解析情况下为httpIllegalType
	fHTTPType = httpIllegalType;
}

// Constructor for creating a response only
HTTPRequest::HTTPRequest(StrPtrLen* serverHeader)
{
    // Store the pointer to the server header field
    fSvrHeader = *serverHeader;
  
    // We do not require any of these:
    fRequestHeader = NULL;
 
    fMethod = httpIllegalMethod;
    fVersion = httpIllegalVersion;
    fRequestLine = NULL;
    fAbsoluteURI = NULL;
    fRelativeURI = NULL;
    fAbsoluteURIScheme = NULL;
    fHostHeader = NULL;
    fRequestPath = NULL;    
    fStatusCode = 0;
    fRequestKeepAlive = false; 
    
    // We require the response  but we allocate memory only when we call
    // CreateResponseHeader
    fResponseHeader = NULL;
    fResponseFormatter = NULL;

	fHTTPType = httpResponseType;
}

// Destructor
 HTTPRequest::~HTTPRequest()
 {
    if (fResponseHeader != NULL)
    {
        if (fResponseHeader->Ptr != NULL)
            delete fResponseHeader->Ptr;
        delete fResponseHeader;
     }
    if (fResponseFormatter != NULL) 
        delete fResponseFormatter;
    if (fRequestPath != NULL)
        delete [] fRequestPath;  
 }
//Parses the request
QTSS_Error HTTPRequest::Parse()
{
    Assert(fRequestHeader.Ptr != NULL);
    StringParser parser(&fRequestHeader);
    
    // Store the request line (used for logging) 
    // (ex: GET /index.html HTTP/1.0)
    StringParser requestLineParser(&fRequestHeader);
    requestLineParser.ConsumeUntil(&fRequestLine, StringParser::sEOLMask);
  
    // Parse request line returns an error if there is an error in the
    // request URI or the formatting of the request line. 
    // If the method or version are not found, they are set
    // to httpIllegalMethod or httpIllegalVersion respectively, 
    // and QTSS_NoErr is returned.
	// 解析第一行
    QTSS_Error err = ParseRequestLine(&parser);
    if (err != QTSS_NoErr)
            return err;

	if(fHTTPType == httpResponseType)
	{
		qtss_printf("recv HTTP Response\n");
	}
  
    // Parse headers and set values of headers into fFieldValues array
    err = ParseHeaders(&parser);
    if (err != QTSS_NoErr)
            return err;
  
    return QTSS_NoErr;
}

QTSS_Error HTTPRequest::ParseRequestLine(StringParser* parser)
{
    // Get the method - If the method is not one of the defined methods
    // then it doesn't return an error but sets fMethod to httpIllegalMethod
    StrPtrLen theParsedData;
    parser->ConsumeWord(&theParsedData);
    fMethod = HTTPProtocol::GetMethod(&theParsedData);

	//还有可能是HTTP Response类型
	if((fMethod == httpIllegalMethod) && theParsedData.Equal("HTTP"))
	{
		parser->ConsumeUntilWhitespace();//过滤掉HTTP/1.1
		parser->ConsumeUntilDigit(NULL);
		UInt32 statusCode = parser->ConsumeInteger(NULL);
		if( statusCode != 0 )
		{
			fHTTPType = httpResponseType;

			parser->ConsumeWhitespace();
			parser->ConsumeUntilWhitespace(NULL);
			// Go past the end of line
			if (!parser->ExpectEOL())
			{
				fStatusCode = httpBadRequest;
				return QTSS_BadArgument;     // Request line is not properly formatted!
			}
			return QTSS_NoErr;
		}
	} 
    // Consume whitespace
    parser->ConsumeWhitespace();
 

    // Parse the URI - If it fails returns an error after setting 
    // the fStatusCode to the appropriate error code
    QTSS_Error err = ParseURI(parser);
    if (err != QTSS_NoErr)
            return err;
  
    // Consume whitespace
    parser->ConsumeWhitespace();
  
    // If there is a version, consume the version string
    StrPtrLen versionStr;
    parser->ConsumeUntil(&versionStr, StringParser::sEOLMask);
    // Check the version
    if (versionStr.Len > 0)
            fVersion = HTTPProtocol::GetVersion(&versionStr);
  
    // Go past the end of line
    if (!parser->ExpectEOL())
    {
        fStatusCode = httpBadRequest;
        return QTSS_BadArgument;     // Request line is not properly formatted!
    }

    return QTSS_NoErr;
}

QTSS_Error HTTPRequest::ParseURI(StringParser* parser)
{

    // read in the complete URL into fRequestAbsURI
    parser->ConsumeUntil(&fAbsoluteURI, sURLStopConditions);
  
    StringParser urlParser(&fAbsoluteURI);
  
    // we always should have a slash before the URI
    // If not, that indicates this is a full URI
    if (fAbsoluteURI.Ptr[0] != '/')
    {
            //if it is a full URL, store the scheme and host name
            urlParser.ConsumeLength(&fAbsoluteURIScheme, 7); //consume "http://"
            urlParser.ConsumeUntil(&fHostHeader, '/');
    }
  
    // whatever is in this position is the relative URI
    StrPtrLen relativeURI(urlParser.GetCurrentPosition(), urlParser.GetDataReceivedLen() - urlParser.GetDataParsedLen());
    // read this URI into fRequestRelURI
    fRelativeURI = relativeURI;
    
    // Allocate memory for fRequestPath
    UInt32 len = fRelativeURI.Len;
    len++;
    //char* relativeURIDecoded = NEW char[len];
	char relativeURIDecoded[EASYDSS_MAX_URL_LENGTH] = { 0 };

    SInt32 theBytesWritten = StringTranslator::DecodeURL(fRelativeURI.Ptr, fRelativeURI.Len,
                                                       relativeURIDecoded, len);
     
    //if negative, an error occurred, reported as an QTSS_Error
    //we also need to leave room for a terminator.
    if ((theBytesWritten < 0) || ((UInt32)theBytesWritten == len))
    {
        fStatusCode = httpBadRequest;
        return QTSS_BadArgument;
    }

	fRequestPath = NULL;
    ////fRequestPath = NEW char[theBytesWritten + 1];
    ////::memcpy(fRequestPath, relativeURIDecoded + 1, theBytesWritten); 
    //////delete relativeURIDecoded;
    ////fRequestPath[theBytesWritten] = '\0';
    return QTSS_NoErr;
}

// Parses the Connection header and makes sure that request is properly terminated
QTSS_Error HTTPRequest::ParseHeaders(StringParser* parser)
{
    StrPtrLen theKeyWord;
    Bool16 isStreamOK;
  
    //Repeat until we get a \r\n\r\n, which signals the end of the headers
    while ((parser->PeekFast() != '\r') && (parser->PeekFast() != '\n'))
    {
        //First get the header identifier
    
        isStreamOK = parser->GetThru(&theKeyWord, ':');
        if (!isStreamOK)
        {       // No colon after header!
            fStatusCode = httpBadRequest;
            return QTSS_BadArgument;
        }
    
        if (parser->PeekFast() == ' ') 
        {        // handle space, if any
            isStreamOK = parser->Expect(' ');
            Assert(isStreamOK);
        }
     
        //Look up the proper header enumeration based on the header string.
        HTTPHeader theHeader = HTTPProtocol::GetHeader(&theKeyWord);
      
        StrPtrLen theHeaderVal;
        isStreamOK = parser->GetThruEOL(&theHeaderVal);
      
        if (!isStreamOK)
        {       // No EOL after header!
            fStatusCode = httpBadRequest;
            return QTSS_BadArgument;
        }
      
        // If this is the connection header
        if ( theHeader == httpConnectionHeader )
        { // Set the keep alive boolean based on the connection header value
            SetKeepAlive(&theHeaderVal);
        }
      
        // Have the header field and the value; Add value to the array
        // If the field is invalid (or unrecognized) just skip over gracefully
        if ( theHeader != httpIllegalHeader )
            fFieldValues[theHeader] = theHeaderVal;
            
    }
  
    isStreamOK = parser->ExpectEOL();
    Assert(isStreamOK);
  
    return QTSS_NoErr;
}

void HTTPRequest::SetKeepAlive(StrPtrLen *keepAliveValue)
{
    if ( sCloseString.EqualIgnoreCase(keepAliveValue->Ptr, keepAliveValue->Len) )
            fRequestKeepAlive = sFalse;
    else
        {
            Assert( sKeepAliveString.EqualIgnoreCase(keepAliveValue->Ptr, keepAliveValue->Len) );
            fRequestKeepAlive = sTrue;
        }
}

void HTTPRequest::PutStatusLine(StringFormatter* putStream, HTTPStatusCode status,
                                HTTPVersion version)
{
    putStream->Put(*(HTTPProtocol::GetVersionString(version)));
    putStream->PutSpace();
    putStream->Put(*(HTTPProtocol::GetStatusCodeAsString(status)));
    putStream->PutSpace();
    putStream->Put(*(HTTPProtocol::GetStatusCodeString(status)));
    putStream->PutEOL();
}

StrPtrLen* HTTPRequest::GetHeaderValue(HTTPHeader inHeader)
{
    if ( inHeader !=  httpIllegalHeader )
        return &fFieldValues[inHeader];
    return NULL;
}

void HTTPRequest:: CreateResponseHeader(HTTPStatusCode statusCode, HTTPVersion version)
{
    // If we are creating a second response for the same request, make sure and
    // deallocate memory for old response and allocate fresh memory
    if (fResponseFormatter != NULL) 
    {
        if(fResponseHeader->Ptr != NULL)
            delete fResponseHeader->Ptr;
        delete fResponseHeader;
        delete fResponseFormatter;
    }
    
    // Allocate memory for the response when you first create it
    char* responseString = NEW char[kMinHeaderSizeInBytes];
    fResponseHeader = NEW StrPtrLen(responseString, kMinHeaderSizeInBytes);
    fResponseFormatter = NEW ResizeableStringFormatter(fResponseHeader->Ptr, fResponseHeader->Len);
    
    //make a partial header for the given version and status code
    PutStatusLine(fResponseFormatter, statusCode, version);
    Assert(fSvrHeader.Ptr != NULL);
    //fResponseFormatter->Put(fSvrHeader);
    //fResponseFormatter->PutEOL();
	//AppendResponseHeader(httpServerHeader,&fSvrHeader);
    fResponseHeader->Len = fResponseFormatter->GetCurrentOffset();
}

StrPtrLen* HTTPRequest::GetCompleteResponseHeader()
{
    fResponseFormatter->PutEOL();
    fResponseHeader->Len = fResponseFormatter->GetCurrentOffset();
    return fResponseHeader;
}

void HTTPRequest::AppendResponseHeader(HTTPHeader inHeader, StrPtrLen* inValue)
{
    fResponseFormatter->Put(*(HTTPProtocol::GetHeaderString(inHeader)));
    fResponseFormatter->Put(sColonSpace);
    fResponseFormatter->Put(*inValue);
    fResponseFormatter->PutEOL();
    fResponseHeader->Len = fResponseFormatter->GetCurrentOffset();
}

void HTTPRequest::AppendContentLengthHeader(UInt64 length_64bit)
{
    //char* contentLength = NEW char[256];
	char contentLength[256] = { 0 };
    qtss_sprintf(contentLength, "%"_64BITARG_"d", length_64bit);
    StrPtrLen contentLengthPtr(contentLength);
    AppendResponseHeader(httpContentLengthHeader, &contentLengthPtr);
}

void HTTPRequest::AppendContentLengthHeader(UInt32 length_32bit)
{
    //char* contentLength = NEW char[256];
	char contentLength[256] = { 0 };
    qtss_sprintf(contentLength, "%"_U32BITARG_"", length_32bit);
    StrPtrLen contentLengthPtr(contentLength);
    AppendResponseHeader(httpContentLengthHeader, &contentLengthPtr);
}

void HTTPRequest::AppendConnectionCloseHeader()
{
    AppendResponseHeader(httpConnectionHeader, &sCloseString);
}

void HTTPRequest::AppendConnectionKeepAliveHeader()
{
    AppendResponseHeader(httpConnectionHeader, &sKeepAliveString);
}

void HTTPRequest::AppendDateAndExpiresFields()
{
    Assert(OSThread::GetCurrent() != NULL);
    DateBuffer* theDateBuffer = OSThread::GetCurrent()->GetDateBuffer();
    theDateBuffer->InexactUpdate(); // Update the date buffer to the current date & time
    StrPtrLen theDate(theDateBuffer->GetDateBuffer(), DateBuffer::kDateBufferLen);
  
    // Append dates, and have this response expire immediately
    this->AppendResponseHeader(httpDateHeader, &theDate);
    this->AppendResponseHeader(httpExpiresHeader, &theDate);
}

void HTTPRequest::AppendDateField()
{
    Assert(OSThread::GetCurrent() != NULL);
    DateBuffer* theDateBuffer = OSThread::GetCurrent()->GetDateBuffer();
    theDateBuffer->InexactUpdate(); // Update the date buffer to the current date & time
    StrPtrLen theDate(theDateBuffer->GetDateBuffer(), DateBuffer::kDateBufferLen);
  
    // Append date
    this->AppendResponseHeader(httpDateHeader, &theDate);
}

time_t HTTPRequest::ParseIfModSinceHeader()
{
    time_t theIfModSinceDate = (time_t) DateTranslator::ParseDate(&fFieldValues[httpIfModifiedSinceHeader]);
    return theIfModSinceDate;
}
