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
#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include "HTTPProtocol.h"
#include "StrPtrLen.h"
#include "StringParser.h"
#include "ResizeableStringFormatter.h"
#include "OSHeaders.h"
#include "QTSS.h"

class HTTPRequest
{
public:

    //HTTP请求构造函数
    HTTPRequest(StrPtrLen* serverHeader, StrPtrLen* requestPtr);
    
    //HTTP响应构造函数
    HTTPRequest(StrPtrLen* serverHeader); 
    
    // Destructor
    virtual ~HTTPRequest();
  
    // Should be called before accessing anything in the request header
    // Calls ParseRequestLine and ParseHeaders
    QTSS_Error              Parse();

    // Basic access methods for the HTTP method, the absolute request URI,
    // the host name from URI, the relative request URI, the request file path,
    // the HTTP version, the Status code, the keep-alive tag.
    HTTPMethod              GetMethod(){ return fMethod; }
	HTTPType				GetHTTPType() { return fHTTPType; }

    StrPtrLen*              GetRequestLine(){ return &fRequestLine; }
    StrPtrLen*              GetRequestAbsoluteURI(){ return &fAbsoluteURI; }
    StrPtrLen*              GetSchemefromAbsoluteURI(){ return &fAbsoluteURIScheme; }
    StrPtrLen*              GetHostfromAbsoluteURI(){ return &fHostHeader; }
    StrPtrLen*              GetRequestRelativeURI(){ return &fRelativeURI; }
    char*                   GetRequestPath(){ return fRequestPath; }
	char*					GetQueryString(){ return fQueryString; }
    HTTPVersion             GetVersion(){ return fVersion; }
    HTTPStatusCode          GetStatusCode(){ return fStatusCode; }
    Bool16                  IsRequestKeepAlive(){ return fRequestKeepAlive; }
  
    // If header field exists in the request, it will be found in the dictionary
    // and the value returned. Otherwise, NULL is returned.
    StrPtrLen*              GetHeaderValue(HTTPHeader inHeader);
  
    // Creates a header with the corresponding version and status code
    void                    CreateResponseHeader(HTTPStatusCode statusCode = httpOK, HTTPVersion version = http11Version);
  
    // To append response header fields as appropriate
    void                    AppendResponseHeader(HTTPHeader inHeader, StrPtrLen* inValue);
    void                    AppendDateAndExpiresFields();
    void                    AppendDateField();
    void                    AppendConnectionCloseHeader();
    void                    AppendConnectionKeepAliveHeader();
    void                    AppendContentLengthHeader(UInt64 length_64bit);
    void                    AppendContentLengthHeader(UInt32 length_32bit);

    // Returns the completed response header by appending CRLF to the end of the header
    // fields buffer
    StrPtrLen*              GetCompleteResponseHeader();
    
    // Parse if-modified-since header
    time_t                  ParseIfModSinceHeader();
  
private:
    enum { kMinHeaderSizeInBytes = 512 };
  
    // Gets the method, version and calls ParseURI
    QTSS_Error              ParseRequestLine(StringParser* parser);
    // Parses the URI to get absolute and relative URIs, the host name and the file path
    QTSS_Error              ParseURI(StringParser* parser);
    // Parses the headers and adds them into a dictionary
    // Also calls SetKeepAlive with the Connection header field's value if it exists
    QTSS_Error              ParseHeaders(StringParser* parser);
  
    // Sets fRequestKeepAlive
    void                    SetKeepAlive(StrPtrLen* keepAliveValue);
    // Used in initialize and CreateResponseHeader
    void                    PutStatusLine(StringFormatter* putStream, HTTPStatusCode status, HTTPVersion version);
    //For writing into the premade headers
    StrPtrLen*              GetServerHeader(){ return &fSvrHeader; }
  
    // Complete request and response headers
    StrPtrLen                       fRequestHeader;
    ResizeableStringFormatter*      fResponseFormatter;
    StrPtrLen*                      fResponseHeader;
  
    // Private members
    HTTPMethod          fMethod;
    HTTPVersion         fVersion;

	HTTPType			fHTTPType;
    
    StrPtrLen           fRequestLine;
  
    // For the URI (fAbsoluteURI and fRelativeURI are the same if the URI is of the form "/path")
    StrPtrLen           fAbsoluteURI;       // If it is of the form "http://foo.bar.com/path"
    StrPtrLen           fRelativeURI;       // If it is of the form "/path"
                                            
                                            // If it is an absolute URI, these fields will be filled in
                                            // "http://foo.bar.com/path" => fAbsoluteURIScheme = "http", fHostHeader = "foo.bar.com",
                                            // fRequestPath = "path"
    StrPtrLen           fAbsoluteURIScheme;
    StrPtrLen           fHostHeader;        // If the full url is given in the request line
    char*               fRequestPath;       // Also contains the query string
	char*				fQueryString;
      
    HTTPStatusCode      fStatusCode;
    Bool16              fRequestKeepAlive;              // Keep-alive information in the client request
    StrPtrLen           fFieldValues[httpNumHeaders];   // Array of header field values parsed from the request
    StrPtrLen           fSvrHeader;                     // Server header set up at initialization
    static StrPtrLen    sColonSpace;
    static UInt8        sURLStopConditions[]; 
};

#endif // __HTTPREQUEST_H__
