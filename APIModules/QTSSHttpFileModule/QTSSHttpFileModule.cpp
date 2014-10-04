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
    File:       QTSSHttpFileModule.cpp

    Contains:   Implements the HTTP file module
 */


#ifndef __Win32__
#include <unistd.h>
#include <dirent.h>
#endif

        
#include <stdio.h>      
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>
#include <time.h>

#include "QTSSHttpFileModule.h"
#include "OSArrayObjectDeleter.h"
#include "StringParser.h"
#include "StrPtrLen.h"
#include "OSMemory.h"
#include "OSHeaders.h"
#include "ev.h"
#include "QTFile.h"
#include "QTTrack.h"
#include "QTHintTrack.h"
#include "QTSSModuleUtils.h"
#include "QTSSRollingLog.h"
#include "OSMutex.h"
#include "HTTPProtocol.h"
#include "HTTPRequest.h"

#define HTTP_FILE_ASYNC 1
#define HTTP_FILE_DEBUGGING 0

class QTSSHttpAccessLog;

// STATIC DATA
// For processing the requests
static QTSS_ModulePrefsObject sPrefs = NULL;
static QTSS_PrefsObject sServerPrefs = NULL;
static QTSS_ServerObject sServer = NULL;


static StrPtrLen    sPathSeparator("/");
static StrPtrLen    sRTSPUrlPrefix("rtsp://");
static StrPtrLen    sRefMovieBufPrefix("rtsptext\r");
static StrPtrLen    sRespHeaderPrefix("HTTP/1.0 200 OK\r\nServer: QTSS/2.0\r\nConnection: Close");
static StrPtrLen    sContentLengthHeaderTag("\r\nContent-Length: ");
static StrPtrLen    sContentTypeHeaderTag("\r\nContent-Type: ");
static StrPtrLen    sConnectionKeepAliveTag("Keep-Alive");
static StrPtrLen    sQuickTimeMimeType("video/quicktime");
static StrPtrLen    sUnknownMimeType("application/unknown");
static StrPtrLen    sGifMimeType("image/gif");
static StrPtrLen    sSdpMimeType("application/sdp");
static StrPtrLen    sSmilMimeType("application/smil");
static StrPtrLen    sQTSuffix("qt");
static StrPtrLen    sMovSuffix("mov");
static StrPtrLen    sGifSuffix("gif");
static StrPtrLen    sSdpSuffix("sdp");
static StrPtrLen    sSmiSuffix("smi");
static StrPtrLen    sSmilSuffix("smil");

static const UInt32 kReadingBufferState = 0;
static const UInt32 kWritingBufferState = 1;

// For logging the requests
// Default values for preferences
static Bool16   sDefaultHTTPFileXferEnabled = false; // This module is not enabled by default.
static Bool16   sDefaultLogEnabled      = false;
static char*    sDefaultLogName         = "StreamingServerHttp";
static char*    sDefaultLogDir = NULL;

static UInt32   sDefaultMaxLogBytes     = 50000000;
static UInt32   sDefaultRollInterval    = 7;
static char*    sVoidField              = "-";
// Current values for preferences
static Bool16   sHTTPFileXferEnabled    = false;
static Bool16   sLogEnabled             = false;
static UInt32   sMaxLogBytes            = 50000000;
static UInt32   sRollInterval           = 7;

static OSMutex* sLogMutex = NULL;// Log module isn't reentrant
static QTSSHttpAccessLog* sAccessLog = NULL;

// ATTRIBUTES
// For processing the http requests
static QTSS_AttributeID sTransferTypeAttr   = qtssIllegalAttrID;
static QTSS_AttributeID sEventContextAttr   = qtssIllegalAttrID;
static QTSS_AttributeID sFileAttr           = qtssIllegalAttrID;
static QTSS_AttributeID sStateAttr          = qtssIllegalAttrID;
static QTSS_AttributeID sFileBufferAttr     = qtssIllegalAttrID;
static QTSS_AttributeID sFileBufferLenAttr  = qtssIllegalAttrID;
static QTSS_AttributeID sReadOffsetAttr     = qtssIllegalAttrID;
static QTSS_AttributeID sWriteOffsetAttr    = qtssIllegalAttrID;

// For logging the requests
static QTSS_AttributeID sRequestAttr        = qtssIllegalAttrID;
static QTSS_AttributeID sContentLengthAttr  = qtssIllegalAttrID;

class QTSSHttpAccessLog : public QTSSRollingLog
{
    public:
    
        QTSSHttpAccessLog() : QTSSRollingLog() { this->SetTaskName("QTSSHttpAccessLog"); }
        virtual ~QTSSHttpAccessLog() {}
    
        virtual char* GetLogName() { return QTSSModuleUtils::GetStringAttribute(sPrefs, "http_logfile_name", sDefaultLogName);}
        virtual char* GetLogDir()  { return QTSSModuleUtils::GetStringAttribute(sPrefs, "http_logfile_dir", sDefaultLogDir); }
        virtual UInt32 GetRollIntervalInDays()  { return sRollInterval; }
        virtual UInt32 GetMaxLogBytes()         { return sMaxLogBytes; }
};

// FUNCTIONS
static QTSS_Error   QTSSHttpFileModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error   Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error   FilterRequest(QTSS_Filter_Params* inParams);
static QTSS_Error   CloseRTSPSession(QTSS_RTSPSession_Params* inParams);
// For processing the requests
static UInt32       GetBitRate(char* filePath);
static void*        MakeMoov(void* rmda, UInt32 rmdaLen, UInt32* moovLen);
static void*        MakeRmda(char* url, UInt32 rate, UInt32* rmdaLen);
static StrPtrLen*   GetMimeType(StrPtrLen* fileName);
// For logging the requests
static QTSS_Error   RereadPrefs();
static void         LogRequest(QTSS_RTSPSessionObject inRTSPSession);
static void         CheckHttpAccessLogState(Bool16 forceEnabled);

// FUNCTION IMPLEMENTATIONS

QTSS_Error QTSSHttpFileModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSHttpFileModuleDispatch);
}


QTSS_Error QTSSHttpFileModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RTSPFilter_Role:
            return FilterRequest(&inParams->rtspFilterParams);
        case QTSS_RTSPSessionClosing_Role:
            return CloseRTSPSession(&inParams->rtspSessionClosingParams);
    }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
    sLogMutex = new OSMutex();
    
    // Add attributes for processing the requests
    static char*    sTransferTypeName   = "QTSSHttpFileModuleTransferType";
    static char*    sEventContextName   = "QTSSHttpFileModuleContext";
    static char*    sFileName           = "QTSSHttpFileModuleFile";
    static char*    sStateName          = "QTSSHttpFileModuleState";
    static char*    sFileBufferName     = "QTSSHttpFileModuleFileBuffer";
    static char*    sFileBufferLenName  = "QTSSHttpFileModuleFileBufferLen";
    static char*    sReadOffsetName     = "QTSSHttpFileModuleReadOffset";
    static char*    sWriteOffsetName    = "QTSSHttpFileModuleWriteOffset";
    
    // Add attributes for logging the requests
    static char*    sRequestName        = "QTSSHttpFileModuleRequestName";
    static char*    sContentLengthName  = "QTSSHttpFileModuleContentLengthName";

    
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RTSPFilter_Role);
    (void)QTSS_AddRole(QTSS_RTSPSessionClosing_Role);
    
    // Attributes for processing requests
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sTransferTypeName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sTransferTypeName, &sTransferTypeAttr);
    
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sEventContextName, NULL, qtssAttrDataTypeUnknown);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sEventContextName, &sEventContextAttr);
    
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sFileName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sFileName, &sFileAttr);

    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sStateName, NULL, qtssAttrDataTypeUnknown);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sStateName, &sStateAttr);
    
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sFileBufferName, NULL, qtssAttrDataTypeUnknown);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sFileBufferName, &sFileBufferAttr);

    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sFileBufferLenName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sFileBufferLenName, &sFileBufferLenAttr);
    
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sReadOffsetName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sReadOffsetName, &sReadOffsetAttr);

    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sWriteOffsetName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sWriteOffsetName, &sWriteOffsetAttr);

    // Attributes for logging requests
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sRequestName, NULL, qtssAttrDataTypeUnknown);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sRequestName, &sRequestAttr);
    
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sContentLengthName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sContentLengthName, &sContentLengthAttr);
    
    // Tell the server our name!
    static char* sModuleName = "QTSSHttpFileModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);

    // Get prefs object
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);
    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
    
    RereadPrefs();

    // This creates the log
    CheckHttpAccessLogState(false);
    
    
    return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
    delete [] sDefaultLogDir;
    (void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsErrorLogDir, 0, &sDefaultLogDir);

    // Code from Access Log module
    QTSSModuleUtils::GetAttribute(sPrefs, "http_xfer_enabled",  qtssAttrDataTypeBool16,
                                &sHTTPFileXferEnabled, &sDefaultHTTPFileXferEnabled, sizeof(sHTTPFileXferEnabled));
    QTSSModuleUtils::GetAttribute(sPrefs, "http_logging",   qtssAttrDataTypeBool16,
                                &sLogEnabled, &sDefaultLogEnabled, sizeof(sLogEnabled));
    QTSSModuleUtils::GetAttribute(sPrefs, "http_logfile_size",  qtssAttrDataTypeUInt32,
                                &sMaxLogBytes, &sDefaultMaxLogBytes, sizeof(sMaxLogBytes));
    QTSSModuleUtils::GetAttribute(sPrefs, "http_logfile_interval",  qtssAttrDataTypeUInt32,
                                &sRollInterval, &sDefaultRollInterval, sizeof(sRollInterval));
    return QTSS_NoErr;
}

QTSS_Error FilterRequest(QTSS_Filter_Params* inParams)
{
    static UInt32 sFileBufSize = 32768; 
    static UInt32 sZero = 0;
    static Bool16 sFalse = false;
    
    UInt32 theLen = 0;
    UInt32 initialState;
    UInt32* theStateP = NULL;
    DIR* theDirectory = NULL;
    QTSS_Object theFile = NULL;
    QTSS_Error theErr = QTSS_NoErr;
    UInt64 theFileLength = 0;
    
    HTTPRequest* httpRequest = NULL;
    
    QTSS_RTSPRequestObject theRequest = inParams->inRTSPRequest;
    QTSS_RTSPSessionObject theSession = inParams->inRTSPSession;
    
    // If this feature is disabled just return
    if (!sHTTPFileXferEnabled)
        return QTSS_NoErr;
    
    StrPtrLen serverHdr;
    (void)QTSS_GetValuePtr(sServer, qtssSvrRTSPServerHeader, 0, (void**)&serverHdr.Ptr, &serverHdr.Len);
    
    (void)QTSS_GetValuePtr(theSession, sStateAttr, 0, (void**)&theStateP, &theLen);
    if ( (theStateP == NULL) || (theLen != sizeof(UInt32)) )
    {
        // Initial state.   
        TransferType type = 0;  // Enum datatype defined in QTSSHttpFileModule.h
        StrPtrLen theFullRequest;
        HTTPMethod reqMethod;
        StrPtrLen reqStr;
        UInt32 index;
        StrPtrLen* responseHeader;
        
        (void)QTSS_GetValuePtr(theRequest, qtssRTSPReqFullRequest, 0, (void**)&theFullRequest.Ptr, &theFullRequest.Len);
        httpRequest = NEW HTTPRequest(&serverHdr, &theFullRequest);
        
        theErr = httpRequest->Parse();
        
        // If the header couldn't be parsed, return from the role
        if(theErr != QTSS_NoErr)
            return QTSS_NoErr;
        
        reqMethod = httpRequest->GetMethod();   
        // If is not a HEAD or a GET request, just return
        if ((reqMethod != httpGetMethod) && (reqMethod != httpHeadMethod)) 
            return QTSS_NoErr;
        
        char *reqPath = httpRequest->GetRequestPath();
        if(::strcmp(reqPath, "") != 0) 
        {
            reqStr.Ptr = reqPath;
            reqStr.Len = ::strlen(reqPath);
        }
        
        // If a filename or directory is not given in the request, just return
        if(reqStr.Ptr == NULL)
            return QTSS_NoErr;    
            
        StrPtrLen* requestLine = httpRequest->GetRequestLine();
        // Store the request line in the request attribute for the purposes of logging
        (void)QTSS_SetValue(theSession, sRequestAttr, 0, requestLine, sizeof(*requestLine));
        
        // Get the movie folder name
        char* movieFolderString = NULL;
        QTSS_Error err = QTSS_GetValueAsString (sServerPrefs, qtssPrefsMovieFolder, 0, &movieFolderString);
        if (err != QTSS_NoErr)
            return QTSS_NoErr;
        
        OSCharArrayDeleter movieFolder(movieFolderString);
        StrPtrLen theMovieFolder(movieFolderString);
        
        // Get the http folder name
        OSCharArrayDeleter httpFolderDtr(QTSSModuleUtils::GetStringAttribute(sPrefs, "http_folder", ""));
        StrPtrLen theHttpFolder(httpFolderDtr.GetObject());
            
        char* thePath = NULL;       
        if ( theMovieFolder.Len != 0 ) 
        {
            // Create the entire path with the movie folder as root directory   
            thePath = NEW char[theMovieFolder.Len + sPathSeparator.Len + reqStr.Len + 1];
            ::memcpy(thePath, theMovieFolder.Ptr, theMovieFolder.Len);
            index = theMovieFolder.Len;
            ::strcpy(thePath + index, sPathSeparator.Ptr);
            index += sPathSeparator.Len;
            ::memcpy(thePath + index, reqStr.Ptr, reqStr.Len);
            index += reqStr.Len;
            thePath[index] = '\0';
            
            theDirectory = ::opendir(thePath);
        
            if ( theDirectory == NULL )
            {                       // If it's not a directory in the movie folder
#if HTTP_FILE_ASYNC
                theErr = QTSS_OpenFileObject(thePath, qtssOpenFileAsync, &theFile);
#else
                theErr = QTSS_OpenFileObject(thePath, qtssOpenFileNoFlags, &theFile);
#endif
                if ((theErr == QTSS_NoErr)) // If it's a file in the movie folder, 
                                            // then it's on-the-fly ref movie generation
                {                           // for a single movie file
#if HTTP_FILE_DEBUGGING
                    qtss_printf("Request for on-the-fly generated ref movie of a hinted QuickTime file\n");
#endif 
                    type = transferRefMovieFile;
                    delete [] thePath;
                }
                else 
                {                   // If it's not a file in the movie folder, 
                                    // then it's not on-the-fly ref movie generation
                    if ( theFile != NULL)
                        (void)QTSS_CloseFileObject(theFile);
                    delete [] thePath;
                    type = 0;
                }                           
            }
            else                    // If it's a directory in the movie folder, then it's on-the-fly ref movie generation
            {                       // for a directory of movie files
#if HTTP_FILE_DEBUGGING
                qtss_printf("Request for on-the-fly generated ref movie for a directory of hinted QuickTime files\n");
#endif 
                type = transferRefMovieFolder;                          
            }    
        }
        
        // If the movie folder doesn't exist or it exists and the request is not for a file or a directory
        // in the movie folder
        if ( type == 0 && theHttpFolder.Len != 0 )
        {
            // Create the entire path with the http folder as root directory    
            thePath = NEW char[theHttpFolder.Len + sPathSeparator.Len + reqStr.Len + 1];
            ::memcpy(thePath, theHttpFolder.Ptr, theHttpFolder.Len);
            index = theHttpFolder.Len;
            ::strcpy(thePath + index, sPathSeparator.Ptr);
            index += sPathSeparator.Len;
            ::memcpy(thePath + index, reqStr.Ptr, reqStr.Len);
            index += reqStr.Len;
            thePath[index] = '\0';
        
#if HTTP_FILE_ASYNC
            theErr = QTSS_OpenFileObject(thePath, qtssOpenFileAsync, &theFile);
#else
            theErr = QTSS_OpenFileObject(thePath, qtssOpenFileNoFlags, &theFile);
#endif
            if( (theErr == QTSS_NoErr) )    // If the file is in the http folder
            {                               // it is a  request for progressive download
#if HTTP_FILE_DEBUGGING
                qtss_printf("Request for the HTTP transfer of a file\n");
#endif 
                type = transferHttpFile;
                delete [] thePath;
            }
            else
            {                                       // If the file is not found in the http folder, just return
                if ( theFile != NULL )
                    (void)QTSS_CloseFileObject(theFile);
                delete [] thePath;
                type = 0;
#if HTTP_FILE_DEBUGGING
                qtss_printf("Request file not found by this module\n");
#endif 
                return QTSS_NoErr;  
            }
        }
        
                
        // If type is still 0, it is neither an HTTP file transfer request nor an on-the-fly ref movie request
        // Just return
        if ( type == 0 )
        {
#if HTTP_FILE_DEBUGGING
            qtss_printf("Request file not found by this module\n");
#endif
            return QTSS_NoErr;
        }
            
        // If it's a "Head" request send the Head response header back and just return
        if ( reqMethod == httpHeadMethod)
        {
            httpRequest->CreateResponseHeader(http10Version, httpOK);
            httpRequest->AppendResponseHeader(httpContentTypeHeader, &sQuickTimeMimeType);
            responseHeader = httpRequest->GetCompleteResponseHeader();
            QTSS_Write(theRequest, responseHeader->Ptr, responseHeader->Len, NULL, 0);  
            if ( theDirectory != NULL )
                   ::closedir(theDirectory);
            else if( theFile != NULL )
                (void)QTSS_CloseFileObject(theFile);
            return QTSS_NoErr;
        }

        // Create a buffer to store data.
        char* theFileBuffer;
        char* contentLength = NEW char[256];
        UInt32 headerLength = sZero;
        UInt32 fileBufferLen = sZero;
        
        char serverIPAddrBuf[20] = { 0 };
        StrPtrLen serverIPAddr(serverIPAddrBuf, 19);      // Server name
        
        // Get the server nameif it's a ref movie transfer
        if ( type == transferRefMovieFile || type == transferRefMovieFolder )
        {
            // Get the server ip address
            (void)QTSS_GetValue(theSession, qtssRTSPSesLocalAddrStr, 0, serverIPAddr.Ptr, &serverIPAddr.Len);
    
            if ( serverIPAddr.Len == 0 )
            // The local IP address for this session is not found; the ref movie cannot be created, so return
                return QTSS_NoErr;
        }
        
        // Before sending any response, set keep alive to off for this connection
        // Regardless of what the client sends, the server always closes the connection after sending the file
        (void)QTSS_SetValue(theRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));
        
        switch (type) 
        {
            case transferHttpFile:      
            {                               // Allocate memory for theFileBuffer
                                            theFileBuffer = NEW char[sFileBufSize];  
                                            httpRequest->CreateResponseHeader(http10Version, httpOK);
                                            httpRequest->AppendConnectionCloseHeader();
                                            theLen = sizeof(UInt64);
                                            theErr = QTSS_GetValue(theFile, qtssFlObjLength, 0, (void*)&theFileLength, &theLen);
                                            Assert(theErr == QTSS_NoErr);
                                            Assert(theLen == sizeof(UInt64));
                                            httpRequest->AppendContentLengthHeader(theFileLength);
                                            httpRequest->AppendResponseHeader(httpContentTypeHeader, GetMimeType(&reqStr));
                                            responseHeader = httpRequest->GetCompleteResponseHeader();
                                            headerLength = responseHeader->Len;
                                            ::memcpy(theFileBuffer, responseHeader->Ptr, responseHeader->Len);
                                            
                                            // We need content length string for logging purposes
                                            qtss_sprintf(contentLength, "%"_64BITARG_"d", theFileLength);
                                            
                                            //Delete the http request object ...no use for it anymore
                                            delete httpRequest;
                                            
                                            // Set the intial file buffer length to be zero as there is nothing in the file buffer (other than the header).
                                            // The read sets this attribute to the length that needs to be written to the stream
                                            fileBufferLen = sZero;
                                            // Set the state to reading so that file contents can be read into the buffer
                                            initialState = kReadingBufferState;
                                            break;
            }
            case transferRefMovieFolder:
            {   
#if HTTP_FILE_DEBUGGING
                                            qtss_printf("Creating a ref movie for the folder\n");
                                            qtss_printf("filenames:\n");     
#endif          
                                            UInt32 fileCount = 0;
                                            struct dirent* entry;
                                            // Find the number of .mov files in the requested directory of the movie folder
                                            while( (entry = ::readdir(theDirectory)) != NULL)
                                            {
                                                if( ::strcmp(entry->d_name + ::strlen(entry->d_name) - 4, ".mov") == 0 )
                                                    fileCount ++;
                                                
                                            }
#if HTTP_FILE_DEBUGGING
                                            qtss_printf("Number of .mov files in the directory: %"_U32BITARG_"\n", fileCount);
#endif          
                                            // Go back to the beginning of the directory
                                            ::rewinddir(theDirectory);
                                            char** fileNames;
                                            fileNames = NEW char*[fileCount];
                                            fileCount = 0;
                                            // Find all the .mov files in the requested directory of the movie folder
                                            while( (entry = ::readdir(theDirectory)) != NULL)
                                            {
                                                char* fileName = NEW char[::strlen(entry->d_name) + 1];
                                                ::strcpy(fileName, entry->d_name);
                                                if( ::strcmp(fileName + strlen(fileName) - 4, ".mov") == 0 ) 
                                                {
                                                    fileNames[fileCount] = fileName;
                                                    fileCount ++;
#if HTTP_FILE_DEBUGGING
                                                    qtss_printf("%"_U32BITARG_" : %s\n", fileCount, fileName);
#endif          
                                                }
                                            }
                                            // Close the directory stream - We no longer have any use for it
                                            ::closedir(theDirectory);
                                            
                                            // Create the base url for each of the files ( ex. rtsp://servername/dirname/ )                                     
                                            OSCharArrayDeleter baseUrl(NEW char[sRTSPUrlPrefix.Len + serverIPAddr.Len + sPathSeparator.Len + reqStr.Len + sPathSeparator.Len + 1]);
                                            ::memcpy(baseUrl.GetObject(), sRTSPUrlPrefix.Ptr, sRTSPUrlPrefix.Len);
                                            index = sRTSPUrlPrefix.Len;
                                            ::memcpy(baseUrl.GetObject() + index, serverIPAddr.Ptr, serverIPAddr.Len);
                                            index += serverIPAddr.Len;
                                            ::strcpy(baseUrl.GetObject() + index, sPathSeparator.Ptr);
                                            index += sPathSeparator.Len;
                                            ::memcpy(baseUrl.GetObject() + index, reqStr.Ptr, reqStr.Len);
                                            index += reqStr.Len;
                                            ::strcpy(baseUrl.GetObject() + index, sPathSeparator.Ptr);
                                            index += sPathSeparator.Len;
                                            baseUrl.GetObject()[index] = '\0';

                                            UInt32 *rmdas = NULL, rmdasLen = 0, *moov = NULL, moovLen = 0, *tempRmda = NULL, tempRmdaLen = 0;
                                            UInt32 *rmdasTemp;
                                            UInt32 arrayIndex;
                                            char* filePath;
                                            char* url;
                                            UInt32 bitRate;

                                            for (arrayIndex = 0; arrayIndex < fileCount; arrayIndex++)
                                            {
                                                // Get the filePath for each .mov file in the directory
                                                filePath = NEW char[::strlen(thePath) + sPathSeparator.Len + ::strlen(fileNames[arrayIndex]) + 1];
                                                ::strcpy(filePath, thePath);
                                                ::strcat(filePath, sPathSeparator.Ptr);
                                                ::strcat(filePath, fileNames[arrayIndex]);
                                                // Create the complete url from the base url for each .mov file in the directory
                                                url = NEW char[index + ::strlen(fileNames[arrayIndex]) + 1];
                                                ::memcpy(url, baseUrl.GetObject(), index);
                                                ::strcpy(url + index, fileNames[arrayIndex]);
                                                //Find the approximate bit rate for each .mov file in the directory
                                                bitRate = GetBitRate(filePath);
#if HTTP_FILE_DEBUGGING
                                                qtss_printf("%"_U32BITARG_"\t: Path = %s\n", arrayIndex + 1, filePath);
                                                qtss_printf("Url = %s\n", url);
                                                qtss_printf("Rate = %"_U32BITARG_"\n", bitRate); 
#endif
                                                if ( bitRate != 0 )
                                                {
                                                    // Create "rmda" for each .mov file in the directory
                                                    tempRmda = (UInt32 *) MakeRmda(url, bitRate, &tempRmdaLen);
                                                }
                                                
                                                // Deallocate the memory for the fileName, filePath and url
                                                delete [] fileNames[arrayIndex];
                                                delete [] filePath;
                                                delete [] url;
                                                
                                                // If tempRmdaLen is NULL, tempRmdaLen is zero: 
                                                // Just move to next file in the folder 
                                                // if( tempRmda == NULL ) // Couldn't create tempRmda. Plan of action: Barf!
                                                // return QTSS_NoErr;
               
                                                // Reallocate the rmdas buffer so as to append the tempRmda
                                                if ( tempRmdaLen != 0 )
                                                {
                                                    rmdasTemp = rmdas;  // Store the rmdas in a temporary pointer and reallocate                                            
                                                    rmdas = NEW UInt32[rmdasLen + tempRmdaLen];
                                                    
                                                    if( rmdas == NULL ) // Couldn't create rmdas. Barf now!
                                                        return QTSS_NoErr;
                                                
                                                    ::memcpy((char *)rmdas, rmdasTemp, rmdasLen); // Copy old contents and append tempRmda
                                                    ::memcpy((char *)rmdas + rmdasLen, tempRmda, tempRmdaLen);
                                                    rmdasLen += tempRmdaLen;
                                                    // Delete the old pointer and the tempRmda pointer
                                                    if (rmdasTemp != NULL)
                                                        delete rmdasTemp;   
                                                    delete tempRmda;
                                                }
                                            }
                                            // Delete the array of filenames
                                            delete [] fileNames;
                                            // Free the path pointer. We are done with it.
                                            delete [] thePath;
                                    
                                            if ( rmdasLen != 0 )
                                                // Create a "moov" for the ref movie
                                                moov = (UInt32 *) MakeMoov(rmdas, rmdasLen, &moovLen);
                                            
                                            if( moov == NULL ) // Couldn't create moov after doing everything else!
                                                return QTSS_NoErr;
                                            
                                            // Create the HTTP response header
                                            httpRequest->CreateResponseHeader(http10Version, httpOK);
                                            httpRequest->AppendConnectionCloseHeader();
                                            httpRequest->AppendContentLengthHeader(moovLen);
                                            httpRequest->AppendResponseHeader(httpContentTypeHeader, &sQuickTimeMimeType);
                                            responseHeader = httpRequest->GetCompleteResponseHeader();
                                            headerLength = responseHeader->Len;
                                                                                        
                                            // Allocate memory for theFileBuffer
                                            theFileBuffer = NEW char[headerLength + moovLen];
                                            
                                            // Write the HTTP response header into the file buffer
                                            ::memcpy(theFileBuffer, responseHeader->Ptr, responseHeader->Len);  
                                            
                                            // We need content length string for logging purposes 
                                            qtss_sprintf(contentLength, "%"_U32BITARG_"", moovLen);
                                            
                                            //Delete the http request object ...no use for it anymore
                                            delete httpRequest;
                                                                                                                                        
                                            // Write the ref movie into the buffer
                                            ::memcpy(theFileBuffer + headerLength, moov, moovLen);
                                            delete moov;    // Delete the moov pointer
                                            // Set the size of the file buffer to that of header length + ref movie length
                                            fileBufferLen = headerLength + moovLen;                         
                                            initialState = kWritingBufferState;
                                            break;
            }
            case transferRefMovieFile:      
            {
#if HTTP_FILE_DEBUGGING
                                            qtss_printf("Creating a ref movie for the hinted file\n");   
#endif
                                            // Create a ref movie buffer for the single file. It is of the form:
                                            //  rtsptext\r
                                            //  rtsp://servername/filepath
                                            OSCharArrayDeleter refMovieBuf(NEW char[sRefMovieBufPrefix.Len + sRTSPUrlPrefix.Len + serverIPAddr.Len + sPathSeparator.Len + reqStr.Len + 1]);
                                            ::memcpy(refMovieBuf.GetObject(), sRefMovieBufPrefix.Ptr, sRefMovieBufPrefix.Len);
                                            index = sRefMovieBufPrefix.Len;
                                            ::memcpy(refMovieBuf.GetObject() + index, sRTSPUrlPrefix.Ptr, sRTSPUrlPrefix.Len);
                                            index += sRTSPUrlPrefix.Len;
                                            ::memcpy(refMovieBuf.GetObject() + index, serverIPAddr.Ptr, serverIPAddr.Len);
                                            index += serverIPAddr.Len;
                                            ::strcpy(refMovieBuf.GetObject() + index, sPathSeparator.Ptr);
                                            index += sPathSeparator.Len;
                                            ::memcpy(refMovieBuf.GetObject() + index, reqStr.Ptr, reqStr.Len);
                                            index += reqStr.Len;    
                                            refMovieBuf.GetObject()[index] = '\0';
                                            
                                            // Create the HTTP response header
                                            httpRequest->CreateResponseHeader(http10Version, httpOK);
                                            httpRequest->AppendConnectionCloseHeader();
                                            httpRequest->AppendContentLengthHeader(index);
                                            httpRequest->AppendResponseHeader(httpContentTypeHeader, &sQuickTimeMimeType);
                                            responseHeader = httpRequest->GetCompleteResponseHeader();
                                            headerLength = responseHeader->Len;
                                            
                                            // Allocate memory for theFileBuffer
                                            theFileBuffer = NEW char[headerLength + index];
                                            // Write the HTTP response header into the file buffer
                                            ::memcpy(theFileBuffer, responseHeader->Ptr, responseHeader->Len);  
                                                                            
                                            // Write the ref movie created above to the file buffer
                                            ::memcpy(theFileBuffer + headerLength, refMovieBuf.GetObject(), index);
                                            
                                            // We need content length string for logging purposes 
                                            qtss_sprintf(contentLength, "%"_U32BITARG_"", index);
                                            
                                            // Write the contents of the file buffer to the request stream and return
                                            QTSS_Write(theRequest, theFileBuffer, (headerLength + index), NULL, 0);
                                            
                                            // Before returning, deallocate theFileBuffer memory
                                            delete [] theFileBuffer;
                                            // Also, delete the file descriptor
                                            
                                            (void)QTSS_CloseFileObject(theFile);
#if HTTP_FILE_DEBUGGING
                                            qtss_printf("Wrote the ref movie to the request stream. Successful!\n"); 
#endif
                                            // Store the content length string for the purposes of logging
                                            // Must be done here as we return a response to the client after this.
                                            (void)QTSS_SetValue(theSession, sContentLengthAttr, 0, &contentLength, sizeof(contentLength));
                                            // Log the request before returning
                                            LogRequest(theSession);
                                            
                                            //Delete the http request object ...no use for it anymore
                                            delete httpRequest;
                                            
                                            return QTSS_NoErr;
                                            
            }
            default:
            {                               // Shouldn't ever happen
#if HTTP_FILE_DEBUGGING
                                            qtss_printf("Illegal transfer type: Not a ref movie file/directory or http file transfer\n");
#endif                                          
                                            return QTSS_NoErr;
            }
        }
        
                                                
        // Setup all the dictionary values we need
        // Store the content length string for the purposes of logging
        (void)QTSS_SetValue(theSession, sContentLengthAttr, 0, &contentLength, sizeof(contentLength));
        // Store our initial state
        (void)QTSS_SetValue(theSession, sStateAttr, 0, &initialState, sizeof(initialState));
        theStateP = &initialState;
        // Set the file buffer pointer and the file buffer length attributes
        (void)QTSS_SetValue(theSession, sFileBufferAttr, 0, &theFileBuffer, sizeof(theFileBuffer));
        (void)QTSS_SetValue(theSession, sFileBufferLenAttr, 0, &fileBufferLen, sizeof(fileBufferLen));
        // Set the initial write offset to zero so that the buffer can be flushed to the stream from the beginning
        (void)QTSS_SetValue(theSession, sWriteOffsetAttr, 0, &sZero, sizeof(sZero));
        // Set the type of transfer so that only the required attributes can be read for reading/writing states
        (void)QTSS_SetValue(theSession, sTransferTypeAttr, 0, &type, sizeof(type));
        
        // For http file transfer type, store the file pointer, response header length, 
        // and the read offset so that the file can be read into the buffer
        // For Async I/O, Create an event context for the file read and store it in the event context attribute
        if ( type == transferHttpFile )
        {
            // Store the file pointer so that we can read from it
            (void)QTSS_SetValue(theSession, sFileAttr, 0, &theFile, sizeof(theFile));

            // Keep track of the initial write into the buffer, so that we can read from file into the buffer at the correct offset
            (void)QTSS_SetValue(theSession, sReadOffsetAttr, 0, &headerLength, sizeof(headerLength));
        }        

    }
       
    // Get our attributes
    char** theFileBufferP = NULL;                   // File buffer pointer
    (void)QTSS_GetValuePtr(theSession, sFileBufferAttr, 0, (void**)&theFileBufferP, &theLen);
    Assert(theFileBufferP != NULL);
    Assert(theLen == sizeof(char**));
    
    UInt32* theFileBufferLenP = NULL;               // File buffer length
    (void)QTSS_GetValuePtr(theSession, sFileBufferLenAttr, 0, (void**)&theFileBufferLenP, &theLen);
    Assert(theFileBufferLenP != NULL);
    Assert(theLen == sizeof(UInt32*));

    UInt32* theWriteOffsetP = NULL;                 // Write offset for the file buffer
    (void)QTSS_GetValuePtr(theSession, sWriteOffsetAttr, 0, (void**)&theWriteOffsetP, &theLen);
    Assert(theWriteOffsetP != NULL);
    Assert(theLen == sizeof(UInt32*));
    
    TransferType* theTransferTypeP = NULL;
    (void)QTSS_GetValuePtr(theSession, sTransferTypeAttr, 0, (void**)&theTransferTypeP, &theLen);
    Assert(theTransferTypeP != NULL);
    Assert(theLen == sizeof(TransferType*));

    UInt32 theState = *theStateP;
    UInt32 theFileBufferLen = *theFileBufferLenP;
    UInt32 theWriteOffset = *theWriteOffsetP;
    TransferType theTransferType = *theTransferTypeP;
    
    QTSS_Object* theFileP = NULL;
    UInt32* theReadOffsetP = NULL;
    UInt32 theReadOffset = 0;
    UInt64 theOffset = 0;
    
    // Only if the type is HTTP file transfer, we need to get the File pointer, the Event Context for the read, and the Read Offset into the buffer.
    if ( theTransferType == transferHttpFile ) 
    {
        // File pointer -- Only for Http File transfer type
        (void)QTSS_GetValuePtr(theSession, sFileAttr, 0, (void**)&theFileP, &theLen);
        Assert(theFileP != NULL);
        Assert(theLen == sizeof(QTSS_Object));
        
        (void)QTSS_GetValuePtr(theSession, sReadOffsetAttr, 0, (void**)&theReadOffsetP, &theLen);
        Assert(theReadOffsetP != NULL);
        Assert(theLen == sizeof(UInt32*));
        theReadOffset = *theReadOffsetP;
    
    
        // Get the length of the file onto the stack
        theLen = sizeof(UInt64);
        theErr = QTSS_GetValue(*theFileP, qtssFlObjLength, 0, (void*)&theFileLength, &theLen);
        Assert(theErr == QTSS_NoErr);
        Assert(theLen == sizeof(UInt64));

        // Get the offset in the file onto the stack
        theLen = sizeof(UInt64);    
        theErr = QTSS_GetValue(*theFileP, qtssFlObjPosition, 0, (void*)&theOffset, &theLen);
        Assert(theErr == QTSS_NoErr);
        Assert(theLen == sizeof(UInt64));
    }
    
    Bool16 isBlocked = false;
    while (!isBlocked)
    {
        UInt32 theBufferSize = 0;
        // The buffer is down adjusted to size of the remaining data in the file for Http file transfer
        if( theTransferType == transferHttpFile )
        {
            // If we have less than the full buffer size left to go in the file,
            // down adjust our buffer size to be the amount of data remaining in the file
            theBufferSize = sFileBufSize - theReadOffset;
            if ((theFileLength - theOffset) < theBufferSize)
            theBufferSize = theFileLength - theOffset;
        }
        
        switch (theState)
        {
            case kReadingBufferState:   // Is valid for the Http File transfer case only.
                                    {
                                        Assert(theTransferType == transferHttpFile);        // Just a check
                                        // Read as much data as possible out of the file
                                        UInt32 theRecvLen = 0;
                                        (void)QTSS_Read(*theFileP, (*theFileBufferP) + theReadOffset, theBufferSize, &theRecvLen);
                                        theReadOffset += theRecvLen;
                                        theOffset += theRecvLen;
#if HTTP_FILE_DEBUGGING
                                        qtss_printf("Got %"_U32BITARG_" bytes back from file read. Now at: %"_64BITARG_"u\n", theRecvLen, theOffset);
#endif
                                        if (theRecvLen < theBufferSize)
                                        {
#if HTTP_FILE_DEBUGGING
                                            qtss_printf("Flow controlled on file. Waiting for read event\n");
#endif
                                            isBlocked = true;
                                            break;
                                        }
                
                                        theFileBufferLen = theReadOffset;
                                        (void)QTSS_SetValue(theSession, sFileBufferLenAttr, 0, &theFileBufferLen, sizeof(theFileBufferLen));
                                        theReadOffset = 0;
                                        Assert(theWriteOffset == 0);
                                        theState = kWritingBufferState;
                                    }

        case kWritingBufferState:   // For both the ref movie directory transfer and the http file transfer
                                    {
                                        UInt32 theWrittenLen = 0;

                                        (void)QTSS_Write(theSession, (*theFileBufferP) + theWriteOffset, theFileBufferLen - theWriteOffset, &theWrittenLen, 0);
                                        theWriteOffset += theWrittenLen;

#if HTTP_FILE_DEBUGGING
                                        qtss_printf("Got %"_U32BITARG_" bytes back from socket write.\n", theWrittenLen);
#endif
                                        if (theWriteOffset < theFileBufferLen)
                                        {
#if HTTP_FILE_DEBUGGING
                                            qtss_printf("Flow controlled on socket.Waiting for write event. \n");
#endif
                                            isBlocked = true;
                                            break;
                                        }
           
                                        // We have flushed the buffer. Check to see if we are done. 
                                        // If it is Http transfer of the file, we need to check if 
                                        // we read the entire file. If we are, delete stuff and return
                                        if ((theTransferType == transferHttpFile) && (theOffset == theFileLength))
                                        {
#if HTTP_FILE_DEBUGGING
                                            qtss_printf("File transfer complete\n");
#endif
                                            // If we've gotten here, we're done sending the file!
                                            // Deletion of resources is handled in another role : RTSPSessionClosing role
                                            // Log the request before returning
                                            LogRequest(theSession);
                                            return QTSS_NoErr;
                                        }
                                        
                                        // In the ref movie case, we are done once we flush the buffer.
                                        if(theTransferType != transferHttpFile)
                                        {                                          
#if HTTP_FILE_DEBUGGING
                                            qtss_printf("Refmovie buffer transfer complete\n");
#endif
                                            // Deletion of resources is handled in another role : RTSPSessionClosing role
                                            theWriteOffset = 0;
                                            theState = 0;
                                            // Log the request before returning
                                            LogRequest(theSession);
                                            return QTSS_NoErr;
                                        }
                                        
                                        // For Http file transfer, we need to set attributes to read the remaining part of the file 
                                        // since we haven't come to the end of the file yet.
                                        if ( theTransferType == transferHttpFile )
                                        {
                                            theWriteOffset = 0;
                                            Assert(theReadOffset == 0);
                                            theState = kReadingBufferState;
                                        }
                                    }
        }

    }
    
    Assert(isBlocked);
    
    // We've reached a blocking condition for some reason.
    // Save our state, request an event, and return.
    (void)QTSS_SetValue(theSession, sWriteOffsetAttr, 0, &theWriteOffset, sizeof(theWriteOffset));
    (void)QTSS_SetValue(theSession, sStateAttr, 0, &theState, sizeof(theState));

    // For http file transfer, we need to save the read offset too.
    if( theTransferType == transferHttpFile )
        (void)QTSS_SetValue(theSession, sReadOffsetAttr, 0, &theReadOffset, sizeof(theReadOffset));

    // If we're reading, wait for the file to become readable (valid only for the http file transfer)
    if (theState == kReadingBufferState)
    {
        Assert(theTransferType == transferHttpFile);    // Just a check
        (void)QTSS_RequestEvent(*theFileP, QTSS_ReadableEvent);
    }
    // If we're writing, wait for the socket to become writable (valid for both ref movie and http file transfer)
    else
        (void)QTSS_RequestEvent(theSession, QTSS_WriteableEvent);

    return QTSS_NoErr;
}

QTSS_Error CloseRTSPSession(QTSS_RTSPSession_Params* inParams)
{
    // In this role, the allocated resources are deleted before closing the RTSP session
    QTSS_RTSPSessionObject theSession = inParams->inRTSPSession;
    UInt32 theLen = 0;
    
    // If this feature is disabled just return
    if (!sHTTPFileXferEnabled)
        return QTSS_NoErr;
    
    // Delete the content length string
    char** theContentLengthP = NULL;
    (void)QTSS_GetValuePtr(theSession, sContentLengthAttr, 0, (void**)&theContentLengthP, &theLen);
    if ( theContentLengthP != NULL )
        delete [] *theContentLengthP;
    
    TransferType* theTransferTypeP = NULL;          // Transfer type
    (void)QTSS_GetValuePtr(theSession, sTransferTypeAttr, 0, (void**)&theTransferTypeP, &theLen);
    if (theTransferTypeP != NULL)
    {
          TransferType theTransferType = *theTransferTypeP;
      
         // Get our file buffer pointer and delete it
         char** theFileBufferP = NULL;                  // File buffer pointer
        (void)QTSS_GetValuePtr(theSession, sFileBufferAttr, 0, (void**)&theFileBufferP, &theLen);
        if (theFileBufferP != NULL) 
              delete [] *theFileBufferP;
    
        QTSS_Object* theFileP = NULL;
    
       // Only if the type is HTTP file transfer, we need to delete the File pointer and destroy the Event Context
       if ( theTransferType == transferHttpFile ) 
       {
        // Get our file pointer and delete it
        (void)QTSS_GetValuePtr(theSession, sFileAttr, 0, (void**)&theFileP, &theLen);
        Assert(theFileP != NULL);
        Assert(theLen == sizeof(QTSS_Object));
        (void)QTSS_CloseFileObject(*theFileP);
       }
    }
    return QTSS_NoErr;
}

UInt32 GetBitRate(char* filePath) 
{
    UInt32 actualRate = 0, rate = 0;
    QTFile file(false, false);
    QTTrack* track = NULL;
    UInt64 totalRTPBytes = 0;

    QTFile::ErrorCode err = file.Open(filePath);
    if(err != QTFile::errNoError)
        return 0;

    while( file.NextTrack(&track, track) )
    {
        QTHintTrack* hintTrack;
        if( track->Initialize() != QTTrack::errNoError )
        {
            qtss_printf("!!! Failed to initialize track !!!\n");
            continue;
        }
         
        if( file.IsHintTrack(track) )
        {
            hintTrack = (QTHintTrack*) track;
            totalRTPBytes += hintTrack->GetTotalRTPBytes();
        }
    }

    Float64 duration = file.GetDurationInSeconds();

    if(duration > 0)
        actualRate = (UInt32) ((Float64) (totalRTPBytes * 8) / duration);
       
    if(actualRate <= 14000) 
        rate = 14000;
    else if(actualRate <= 28000)
        rate = 28000;
    else if(actualRate <= 56000)
        rate = 56000;
    else if(actualRate <= 112000)
        rate = 112000;
    else
        rate = 150000;

#if HTTP_FILE_DEBUGGING
    qtss_printf("Actual rate: %"_U32BITARG_"\n", actualRate);
#endif

    return rate;
}
       
void* MakeMoov(void* rmda, UInt32 rmdaLen, UInt32* moovLen)
{
       UInt32 *rmra, rmraLen, *moov;

       // Make the RMRA
       rmraLen = rmdaLen + 8;
       rmra = NEW UInt32[rmraLen];
       if( rmra == NULL )
         return NULL;

       rmraLen = htonl(rmraLen);

       ::memcpy(&rmra[0], &rmraLen, 4);
       ::memcpy(&rmra[1], "rmra", 4);
       ::memcpy((char *)rmra + 8, rmda, rmdaLen);

       // Make the MOOV

       *moovLen = ntohl(rmraLen) + 8;
       moov = NEW UInt32[*moovLen];
       if( moov == NULL )
         return NULL;

       *moovLen = htonl(*moovLen);

       ::memcpy(&moov[0], moovLen, 4);
       ::memcpy(&moov[1], "moov", 4);
       ::memcpy((char *)moov + 8, rmra, ntohl(rmraLen));

       delete rmra;

       *moovLen = ntohl(*moovLen);

       // moov needs to be deleted by the calling function
       return moov;
}

void* MakeRmda(char* url, UInt32 rate, UInt32* rmdaLen) 
{
      UInt32 *rdrf, rdrfLen, *rmdr, rmdrLen, *rmda, zero, size;

      zero = htonl(0); // Okay, this is silly ???
      rate = htonl(rate);

      // Make the RDRF
      size = ::strlen(url) + 1;
      rdrfLen = 20 + size;
      rdrf = NEW UInt32[rdrfLen];
      if( rdrf == NULL )
         return NULL;

      rdrfLen = htonl(rdrfLen);
      size = htonl(size);

      ::memcpy(&rdrf[0], &rdrfLen, 4);
      ::memcpy(&rdrf[1], "rdrf", 4);
      ::memcpy(&rdrf[2], &zero, 4);
      ::memcpy(&rdrf[3], "url ", 4);
      ::memcpy(&rdrf[4], &size, 4);
      ::strcpy((char *)&rdrf[5], url);

      // Make the RMDR
      rmdrLen = 16;
      rmdr = NEW UInt32[rmdrLen];
      if( rmdr == NULL )
         return NULL;

      rmdrLen = htonl(rmdrLen);
      
      ::memcpy(&rmdr[0], &rmdrLen, 4);
      ::memcpy(&rmdr[1], "rmdr", 4);
      ::memcpy(&rmdr[2], &zero, 4);
      ::memcpy(&rmdr[3], &rate, 4);

      // Make the RMDA

      *rmdaLen = ntohl(rdrfLen) + ntohl(rmdrLen) + 8;
      rmda = NEW UInt32[*rmdaLen];
      if( rmda == NULL )
         return NULL;

      *rmdaLen = htonl(*rmdaLen);

      ::memcpy(&rmda[0], rmdaLen, 4);
      ::memcpy(&rmda[1], "rmda", 4);
      ::memcpy((char *)rmda + 8, rmdr, ntohl(rmdrLen));
      ::memcpy((char *)rmda + 8 + ntohl(rmdrLen), rdrf, ntohl(rdrfLen));

      delete rdrf;
      delete rmdr;

      *rmdaLen = ntohl(*rmdaLen);

      // rmda needs to be deleted by the calling function
      return rmda;
}

StrPtrLen* GetMimeType(StrPtrLen* fileName)
{

    StringParser fileNameParser(fileName);
    StrPtrLen str;
    UInt32 suffixSize;
    
    // If the filename doesn't have a suffix, return unknown mime type
    if ( !fileNameParser.GetThru(&str, '.') )
        return &sUnknownMimeType;
                  
    suffixSize = fileName->Len - fileNameParser.GetDataParsedLen();
    StrPtrLen suffix(fileNameParser.GetCurrentPosition(), suffixSize);

    if ( suffix.Equal(sMovSuffix) || suffix.Equal(sQTSuffix) )
        return &sQuickTimeMimeType;

    if( suffix.Equal(sGifSuffix) )
        return &sGifMimeType;

    if( suffix.Equal(sSdpSuffix) )
        return &sSdpMimeType;
         
    if( suffix.Equal(sSmiSuffix) || suffix.Equal(sSmilSuffix) )
        return &sSmilMimeType;
                
    return &sUnknownMimeType;
}

void LogRequest(QTSS_RTSPSessionObject inRTSPSession)
{
    static StrPtrLen sUnknownStr(sVoidField);
    static char* sStatus = "200";
    UInt32 theLen = 0;
    
    OSMutexLocker locker(sLogMutex);
    CheckHttpAccessLogState(false);
    if (sAccessLog == NULL)             // Http logging is disabled; just return
        return;
        
    // If http logging is on, then log the request... first construct a timestamp
    char theDateBuffer[QTSSRollingLog::kMaxDateBufferSizeInBytes];
    Bool16 result = QTSSRollingLog::FormatDate(theDateBuffer, true);         // date field
    
    //for now, just ignore the error.
    if (!result)
        theDateBuffer[0] = '\0';
    
    // Get info to log. Each attribute must be copied out to ensure that it is NULL terminated.
    // To ensure NULL termination, just memset the buffers to 0, and make sure that
    // the last byte of each array is untouched.
    
    // Get the remost host field
    char remoteAddrBuf[20]; 
    StrPtrLen remoteAddr(remoteAddrBuf, 19);    
    if (inRTSPSession != NULL)
        (void)QTSS_GetValue(inRTSPSession, qtssRTSPSesRemoteAddrStr, 0, remoteAddr.Ptr, &remoteAddr.Len);
    
    // Get the request field
    StrPtrLen* theRequestP = NULL;
    (void)QTSS_GetValuePtr(inRTSPSession, sRequestAttr, 0, (void**)&theRequestP, &theLen);
    Assert(theRequestP != NULL);
    Assert(theLen == sizeof(StrPtrLen));
    OSCharArrayDeleter request(NEW char[(*theRequestP).Len + 1]);
    ::memcpy(request.GetObject(), (*theRequestP).Ptr, (*theRequestP).Len);
    request.GetObject()[(*theRequestP).Len] = '\0';
    
    // Get the bytes (content length of the document transferred) field
    char** theContentLengthP = NULL;
    char* contentLengthStr = NULL;
    (void)QTSS_GetValuePtr(inRTSPSession, sContentLengthAttr, 0, (void**)&theContentLengthP, &theLen);
    Assert(theContentLengthP != NULL);
    Assert(theLen == sizeof(char*));
    contentLengthStr = *theContentLengthP;
    
    char tempLogBuffer[2048];
    qtss_sprintf(tempLogBuffer, "%s %s %s %s \"%s\" %s %s\n",  
                                    (remoteAddr.Ptr[0] == '\0') ? sVoidField : remoteAddr.Ptr,
                                    sVoidField,
                                    sVoidField, 
                                    (theDateBuffer[0] == '\0') ? sVoidField : theDateBuffer,    
                                    (request.GetObject()[0] == '\0') ? sVoidField : request.GetObject(),
                                    sStatus,
                                    (contentLengthStr[0] == '\0' ) ? sVoidField : contentLengthStr
                                    );


    Assert(::strlen(tempLogBuffer) < 2048);
    
    //Write the log message
    sAccessLog->WriteToLog(tempLogBuffer, kAllowLogToRoll);
}

void CheckHttpAccessLogState(Bool16 forceEnabled)
{
    // Code from Access Log module
    // This function makes sure the logging state is in sync with the preferences.
    // extern variable declared in QTSSPreferences.h
    if ((NULL == sAccessLog) && (forceEnabled || sLogEnabled))
    {
        sAccessLog = NEW QTSSHttpAccessLog();
        sAccessLog->EnableLog();
    }

    if ((NULL != sAccessLog) && ((!forceEnabled) && (!sLogEnabled)))
    {
        delete sAccessLog;
        sAccessLog = NULL;
    }
}   

/*
The Common Logfile Format is used for logging all the http requests

format: remotehost rfc931 authuser [date] "request" status bytes

Field Name      Value
remotehost      Remote hostname (or IP number if DNS hostname is not available, or if DNSLookup is Off. 
rfc931          The remote logname of the user. 
authuser        The username as which the user has authenticated himself. 
[date]          Date and time of the request. 
"request"       The request line exactly as it came from the client. 
status          The HTTP status code returned to the client. 
bytes           The content-length of the document transferred. 

*/      
