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
    File:       QTSSRefMovieModule.cpp
    Contains:   Implements a module to create RTSP text ref movies
*/

#include "QTSSRefMovieModule.h"
#include "OS.h"
#include "OSMemory.h"
#include "OSArrayObjectDeleter.h"
#include "StringParser.h"
#include "StrPtrLen.h"
#include "OSMemory.h"
#include "OSHeaders.h"
#include "ev.h"
#include "QTFile.h"
#include "QTSSModuleUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//------------------------------------------------------------------------
// STATIC DATA
//------------------------------------------------------------------------

static QTSS_ModulePrefsObject sPrefs = NULL;
static QTSS_PrefsObject sServerPrefs = NULL;
static QTSS_ServerObject sServer = NULL;

// HTTP reply header
static char*  sResponseHeader = "HTTP/1.0 200 OK\r\nServer: QTSS/4.0\r\n"
                                "Connection: Close\r\nContent-Type: video/quicktime\r\n";

static Bool16 sRefMovieXferEnabled = true;
static Bool16 sDefaultRefMovieXferEnabled = true;
static UInt32 sServerIPAddr = 0x74000001;			// 127.0.0.1
static UInt16 sRTSPReplyPort = 0;
static UInt16 sDefaultRTSPReplyPort = 0;

//------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//------------------------------------------------------------------------

QTSS_Error QTSSRefMovieModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error   Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error   RereadPrefs();
static QTSS_Error   Filter(QTSS_Filter_Params* inParams);

static void url_strcpy(char* dest, const char* src);
static QTSS_Error SendTheResponse(QTSS_RTSPSessionObject theSession, QTSS_StreamRef stream, StrPtrLen& movie);
static Bool16 FileExists(StrPtrLen& path, StrPtrLen& movie);
static Bool16 IsHTTPGet(StrPtrLen& theRequest);
static Bool16 IsTunneledRTSP(StrPtrLen& theRequest);
static Bool16 IsAdminURL(StrPtrLen& theUrl);
static Bool16 ParseURL(StrPtrLen& theRequest, char* outURL, UInt16 maxlen);
static Bool16 IsHomeDirURL(StrPtrLen& theUrl);

//------------------------------------------------------------------------
// MODULE FUNCTIONS IMPLEMENTATION
//------------------------------------------------------------------------

QTSS_Error QTSSRefMovieModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSRefMovieModuleDispatch);
}

// Dispatch this module's role call back.
QTSS_Error QTSSRefMovieModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPFilter_Role:
            return Filter(&inParams->rtspFilterParams);
    }
    return QTSS_NoErr;
}

// Handle the QTSS_Register role call back.
QTSS_Error Register(QTSS_Register_Params* inParams)
{
   // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
    (void)QTSS_AddRole(QTSS_RTSPFilter_Role);
    
    // Tell the server our name!
    static char* sModuleName = "QTSSRefMovieModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

// Handle the QTSS_Initialize role call back.
QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    QTSS_Error err = QTSS_NoErr;
	UInt32 ulen = sizeof(sServerIPAddr);
    
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);

    // Get prefs object
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);
    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
    
    // Get the Server's IP address for later use.
	err = QTSS_GetValue(sServer, qtssSvrDefaultIPAddr, 0, &sServerIPAddr, &ulen);
    
    err = RereadPrefs();
    
    return err;
}

// Handle the QTSS_RereadPrefs_Role role call back.
QTSS_Error RereadPrefs()
{
    QTSSModuleUtils::GetAttribute(sPrefs, "refmovie_xfer_enabled",  qtssAttrDataTypeBool16,
                                &sRefMovieXferEnabled, &sDefaultRefMovieXferEnabled, sizeof(sRefMovieXferEnabled));
    QTSSModuleUtils::GetAttribute(sPrefs, "refmovie_rtsp_port",  qtssAttrDataTypeUInt16,
                                &sRTSPReplyPort, &sDefaultRTSPReplyPort, sizeof(sRTSPReplyPort));
    return QTSS_NoErr;
}

// url_strcpy - works like strcpy except that it handles URL escape
// conversions as it copies the string.
void url_strcpy(char* dest, const char* src)
{
    int c1, c2;
    while(*src)
    {
        if (*src == '%')
        {
            src++;
            c1 = *src++;
            if (c1 >= '0' && c1 <= '9')
                c1 -= '0';
            else if (c1 >= 'A' && c1 <= 'F')
                c1 -= 'A' + 10;
            else if (c1 >= 'a' && c1 <= 'f')
                c1 -= 'a' + 10;
            else
                c1 = 0;
            c2 = *src;
            if (c2 >= '0' && c2 <= '9')
                c2 -= '0';
            else if (c2 >= 'A' && c2 <= 'F')
                c2 -= 'A' + 10;
            else if (c2 >= 'a' && c2 <= 'f')
                c2 -= 'a' + 10;
            else
                c2 = 32;
            *dest = (c1 * 16) + c2;
        }
        else
        {
            *dest = *src;
        }
        dest++;
        src++;
    }
    *dest = '\0';
}

// This sends the HTTP response to the server that contains the RTSPtext Ref movie
QTSS_Error SendTheResponse(QTSS_RTSPSessionObject theSession, QTSS_StreamRef stream, StrPtrLen& movie)
{
    QTSS_Error err = QTSS_NoErr;
    char theMovieFile[512];
    theMovieFile[sizeof(theMovieFile) -1] = 0;
    
    char tmp[600];	
    tmp[sizeof(tmp) -1] = 0;
    
    char tmp2[80];
    tmp2[sizeof(tmp2) -1] = 0;
    
    UInt8 x1, x2, x3, x4;

    // send the HTTP reply header to the client
    err= QTSS_Write(stream, sResponseHeader, ::strlen(sResponseHeader), NULL, qtssWriteFlagsBufferData);
    if (err != QTSS_NoErr)
        return QTSS_NoErr;
   
    UInt32 ip4address = 0;
    UInt32 len = sizeof(ip4address);
    err = QTSS_GetValue(theSession, qtssRTSPSesLocalAddr, 0, &ip4address, &len);
        
	// Format the server IP address for building the RTSP address in the reply.
    x1 = (UInt8)((ip4address >> 24) & 0xff);
    x2 = (UInt8)((ip4address >> 16) & 0xff);
    x3 = (UInt8)((ip4address >> 8) & 0xff);
    x4 = (UInt8)((ip4address) & 0xff);
    
    if (movie.Len > sizeof(theMovieFile)  -1 )
        movie.Len = sizeof(theMovieFile) -1;
        
    ::memcpy(theMovieFile, movie.Ptr, movie.Len);
	theMovieFile[movie.Len] = '\0';
	
    UInt16 port = sRTSPReplyPort;
    if (0 == port)
    {
        len = sizeof(port);
        err = QTSS_GetValue(theSession, qtssRTSPSesLocalPort, 0, &port, &len);
    }
            
	// construct the RTSP address reply string for the client.
	qtss_snprintf(tmp,sizeof(tmp) -1, "rtsptext\r\nrtsp://%d.%d.%d.%d:%d%s\r\n", x1,x2,x3,x4, port, theMovieFile);
    
    // send the 'Content-Length:' part of the HTTP reply
    qtss_snprintf(tmp2, sizeof(tmp2) -1, "Content-Length: %d\r\n\r\n", (int) ::strlen(tmp));
    err = QTSS_Write(stream, tmp2, ::strlen(tmp2), NULL, qtssWriteFlagsBufferData);
    if (err != QTSS_NoErr)
        return QTSS_NoErr;
        
    // send the formatted RTSP reference part of the reply
    err = QTSS_Write(stream, tmp, ::strlen(tmp), NULL, qtssWriteFlagsBufferData);
    if (err != QTSS_NoErr)
        return QTSS_NoErr;
        
    // flush the pending write to the client.
    err = QTSS_Flush(stream);
    return err;
}

// This determines if the specified movie file
// exists at the designated path.
Bool16 FileExists(StrPtrLen& path, StrPtrLen& movie)
{
    struct stat sb;
    char fullpath[1024];
    
    // if the movie path ends in a '/' then there is no movie file to be found.
    // (This is probably a user error in typing the URL.)
    if(movie.Ptr[movie.Len-1] == '/')
        return false;
        
    // copy path to our local buffer
    ::memcpy(fullpath, path.Ptr, path.Len);
    
    // remove any URL escape characters when we contruct the full path.
    ::url_strcpy(fullpath+path.Len, (char*) movie.Ptr);
    fullpath[path.Len+movie.Len] = '\0';
    // check for file existance with the POSIX stat() function.
    if (::stat(fullpath, &sb) != 0)
        return false;
    else
        return true;
}

// This determines if an incoming request is an HTTP GET
// request.
Bool16 IsHTTPGet(StrPtrLen& theRequest)
{
    StrPtrLen token = theRequest;
    token.Len = 3;
    return token.EqualIgnoreCase(StrPtrLen("GET"));
}

// This determines if an incoming request is actually
// an RTSP request tunneled in a HTTP request.
Bool16 IsTunneledRTSP(StrPtrLen& theRequest)
{
    if (::strstr((char*)theRequest.Ptr, "x-rtsp-tunneled") != 0)
        return true;
    return false;
}

// This determines if a URL in an HTTP request is actually
// a server admin request.
Bool16 IsAdminURL(StrPtrLen& theUrl)
{
    StrPtrLen token = theUrl;
    token.Len = 15;
    return token.EqualIgnoreCase(StrPtrLen("/modules/admin/"));
}


Bool16 IsHomeDirURL(StrPtrLen& theUrl)
{
    StrPtrLen token = theUrl;
    token.Len = 2;
    return token.EqualIgnoreCase(StrPtrLen("/~"));
}

// Parse out the URL from the HTTP GET line.
Bool16 ParseURL(StrPtrLen& theRequest, char* outURL, UInt16 maxlen)
{
    StringParser reqParse(&theRequest);
    StrPtrLen strPtr;
    
    ::memset(outURL, 0, maxlen);
    reqParse.ConsumeWord(&strPtr);

    if ( !strPtr.Equal(StrPtrLen("GET")) )
    {
        return false;
    }
    reqParse.ConsumeWhitespace();
    reqParse.ConsumeUntilWhitespace(&strPtr);
    if (strPtr.Len == 0)
        return false;
    else if ((UInt16)strPtr.Len > maxlen-1)
        strPtr.Len = maxlen-1;
    ::memcpy(outURL, strPtr.Ptr, strPtr.Len);

    return true;
}

// Handle the QTSS_RTSPFilter_Role role call back.
QTSS_Error Filter(QTSS_Filter_Params* inParams)
{
    QTSS_Error err = QTSS_NoErr;
    QTSS_RTSPRequestObject theRequest = inParams->inRTSPRequest;
    QTSS_RTSPSessionObject theSession = inParams->inRTSPSession;
    char theURL[512];
    
    // If this module is disabled do nothing but return.
    if (!sRefMovieXferEnabled)
        return QTSS_NoErr;
            
   // Get the full RTSP request from the server's attribute.
    StrPtrLen theFullRequest;
    err = QTSS_GetValuePtr(theRequest, qtssRTSPReqFullRequest, 0, (void**)&theFullRequest.Ptr, &theFullRequest.Len);
    
    if (err != QTSS_NoErr) 
    {
        return QTSS_NoErr;
    }
    
    // if this is not an HTTP GET then ignore it.
    if (!IsHTTPGet(theFullRequest))
        return QTSS_NoErr;
    
    // If this is a tunneled RTSP request we ignore it.
    if (IsTunneledRTSP(theFullRequest))
    {
        return QTSS_NoErr;
    }
        
    // if we can't parse out the URL then we just ignore this request.
    if (!ParseURL(theFullRequest, theURL, 512))
    {
        return QTSS_NoErr;
    }
    
    // Make sure that this is not an admin request before
    // we go any further.
    StrPtrLen movie(theURL);
    if (IsAdminURL(movie))
    {
        // The file path in the URL is actually an admin request.
        // Just ignore it and let the admin module handle it.
        return QTSS_NoErr;
    }
    
    Bool16 isHomeDir = IsHomeDirURL(movie);
    
    // Get the server's movie folder location.
    char* movieFolderString = NULL;
    err = QTSS_GetValueAsString (sServerPrefs, qtssPrefsMovieFolder, 0, &movieFolderString);
    if (err != QTSS_NoErr)
        return QTSS_NoErr;
        
    OSCharArrayDeleter movieFolder(movieFolderString);
    StrPtrLen theMovieFolder(movieFolderString);
    
    if (!isHomeDir && !FileExists(theMovieFolder, movie))
    {
        // we couldn't find a file at the specified location
        // so we will ignore this HTTP request and let some other module
        // deal with the issue.
        return QTSS_NoErr;
    }
    else
    {
        // Eureka!!! We found a file at the specified location.
        // We assume that it is a valid movie file and we send
        // the client an RTSP text reference movie in the HTTP reply.
        err = SendTheResponse(theSession,theRequest, movie);
    }
        
    return QTSS_NoErr;
}
