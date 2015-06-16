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
    File:       QTSSDemoSMILModule.cpp

    Contains:   

    
    
 */

/*

<smil>
      <head>
          <layout>
              <root-layout width="240" height="160" background-color="black" />
              <region id="region_1" background-color="black" left="0" top="0" width="240" height="160" />
          </layout>
      </head>
      <body>
          <seq>
              <video src="Blue%20Indigo/George%D5s%20Blues" alt="Cool Jazz" region="region_1" />
              <video src="rtsp://17.333.33.333/catwalk.mov"" alt="Streaming Jazz" region="region_1" begin="2s" />
          </seq>
      </body>
</smil>

*/

#include <unistd.h>     
#include <stdio.h>      
#include <stdlib.h>
#include "SafeStdLib.h"
#include <dirent.h>
#include <string.h>
#include <time.h>

#include "QTSSDemoSMILModule.h"
#include "OSArrayObjectDeleter.h"
#include "StringParser.h"
#include "StrPtrLen.h"
#include "OSFileSource.h"
#include "OSMemory.h"
#include "OSHeaders.h"
#include "ev.h"
#include "QTFile.h"
#include "QTTrack.h"
#include "QTHintTrack.h"
#include "QTSSModuleUtils.h"
#include "OSMutex.h"

#define HTTP_FILE_ASYNC 1
#define HTTP_FILE_DEBUGGING 1

// STATIC DATA
// For processing the requests
static char* sResponseHeader = "HTTP/1.0 200 OK\r\nServer: QTSS/2.0\r\nContent-Type: video/quicktime\r\n\r\n";
static QTSS_PrefsObject sPrefs = NULL;

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


// FUNCTIONS
static QTSS_Error   QTSSDemoSMILDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error   Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error   FilterRequest(QTSS_Filter_Params* inParams);
// For processing the requests


//Demo stuff
struct HitCount {
    char url[512];
    int hitcount;
};

static HitCount gHitcountArray[100] = {};

//protos
void CountHit(char* url);
void GenerateHotHitSMIL(char* buffer);
int HitCountCompare(const void * hitCount1, const void *hitCount2);
QTSS_Error CountRequest( QTSS_RTSPRequestObject inRTSPRequest, QTSS_ClientSessionObject inClientSession,
                                                 QTSS_RTSPSessionObject inRTSPSession, QTSS_CliSesClosingReason *inCloseReasonPtr );
void InitHitCountFromFile();


//funcs

void InitHitCountFromFile()
{
    FILE* hitfile = fopen("hitcount.txt", "r");
    
    if (hitfile == NULL) return;
    
    int c = 0;
    int i = 0;
    while ( (c = fscanf(hitfile, "%s %d", gHitcountArray[i].url, &gHitcountArray[i].hitcount)) == 2 ) 
    {
        qtss_printf("%s %d\n", gHitcountArray[i].url, gHitcountArray[i].hitcount);
        i++;
    }
        
    fclose(hitfile);
}

void WriteHitCountToFile()
{
    FILE* hitfile = fopen("hitcount.txt", "w");
    
    int i=0;
    for (i=0;gHitcountArray[i].url[0] && i<512; i++)
    {
        qtss_fprintf(hitfile, "%s %d\n", gHitcountArray[i].url, gHitcountArray[i].hitcount);
    }
    
    fclose(hitfile);
}

void CountHit(char* url)
{
#if HTTP_FILE_DEBUGGING
            qtss_printf("Counting Hit for \"%s\"\n", url);   
#endif

    if ( url == NULL )
        return;
        
    int i=0;
    for (i=0;gHitcountArray[i].url[0] && i<512; i++)
    {
        if ( strcmp(url, gHitcountArray[i].url ) == 0)
        {
            gHitcountArray[i].hitcount++;
            qsort(&gHitcountArray, i+1, sizeof(HitCount), &HitCountCompare);
            return;
        }
    }
    
    ::strcpy(gHitcountArray[i].url, url);
    gHitcountArray[i].hitcount++;

    qsort(&gHitcountArray, i+1, sizeof(HitCount), &HitCountCompare);
}

int HitCountCompare(const void * hitCount1, const void *hitCount2)
{
    return ((const HitCount*)hitCount2)->hitcount - ((const HitCount*)hitCount1)->hitcount;
}


QTSS_Error ClientSessionClosing(QTSS_ClientSessionClosing_Params* inParams)
{
#if HTTP_FILE_DEBUGGING
    qtss_printf("ClientSessionClosing\n");
#endif
    return CountRequest(NULL, inParams->inClientSession, NULL, &inParams->inReason);
}

QTSS_Error CountRequest( QTSS_RTSPRequestObject inRTSPRequest, QTSS_ClientSessionObject inClientSession,
                         QTSS_RTSPSessionObject inRTSPSession, QTSS_CliSesClosingReason *inCloseReasonPtr )
{

    char urlBuf[256] = { 0 };   
    StrPtrLen url(urlBuf, 256 -1);
    (void)QTSS_GetValue(inClientSession, qtssCliSesFullURL, 0, url.Ptr, &url.Len);
    
    CountHit( url.Ptr );
    WriteHitCountToFile();
    
    return QTSS_NoErr;
}

void GenerateHotHitSMIL(char* buffer)
{
    char smilTemplate[8192]  =  {};
    char* templateCursor = smilTemplate;
    char* bufferCursor = buffer;

    FILE* smilTemplateFile = fopen("template.smil", "r");
    if (smilTemplateFile != NULL)
    {
        int len = fread(smilTemplate, sizeof(char), sizeof(smilTemplate), smilTemplateFile);
        
        smilTemplate[len] = '\0';

        fclose(smilTemplateFile);
    }
    else
    {
        strcpy(smilTemplate, "<smil>\n"
                            "      <head>\n"
                            "         <layout>\n"
                            "         </layout>\n"
                            "      </head>\n"
                            "      <body>\n"
                            "          <seq>\n"
                            "              <video src=\"%s\"  />\n"
                            "          </seq>\n"
                            "      </body>\n"
                            "</smil>\n");
    }
    
    int hitNum = 0;
    char* p = NULL;
    while ( (p = strstr(templateCursor, "%s")) != 0 )
    {
        char saveCh = p[2];
        p[2] = '\0';
        int len = qtss_sprintf(bufferCursor, templateCursor, gHitcountArray[hitNum++].url);
        p[2] = saveCh;
        bufferCursor += len;
        templateCursor = &p[2];
    }
    
    strcat(bufferCursor, templateCursor);
    
#if HTTP_FILE_DEBUGGING
            qtss_printf("smil generated:\n%s\n", buffer);    
#endif

}






// FUNCTION IMPLEMENTATIONS

QTSS_Error QTSSDemoSMILModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSDemoSMILDispatch);
}


QTSS_Error QTSSDemoSMILDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
#if HTTP_FILE_DEBUGGING
    qtss_printf("QTSSDemoSMILDispatch\n");
#endif

    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RTSPFilter_Role:
            return FilterRequest(&inParams->rtspFilterParams);
        case QTSS_ClientSessionClosing_Role:
            return ClientSessionClosing(&inParams->clientSessionClosingParams);
    }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
#if HTTP_FILE_DEBUGGING
    qtss_printf("Register\n");
#endif
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RTSPFilter_Role);
    (void)QTSS_AddRole(QTSS_ClientSessionClosing_Role);
    
    // Tell the server our name!
    static char* sModuleName = "QTSSDemoSMILModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
#if HTTP_FILE_DEBUGGING
    qtss_printf("Initialize\n");
#endif
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);

    // Get prefs object
    sPrefs = inParams->inPrefs;
    
    InitHitCountFromFile();
    
    return QTSS_NoErr;
}


QTSS_Error FilterRequest(QTSS_Filter_Params* inParams)
{
#if HTTP_FILE_DEBUGGING
    qtss_printf("FilterRequest\n");
#endif

    static Bool16 sFalse = false;

    QTSS_RTSPRequestObject theRequest = inParams->inRTSPRequest;
    
    // Initial state.   
    StrPtrLen theFullRequest;
    StrPtrLen reqMethod;
    StrPtrLen reqStr;
    StrPtrLen httpVersion;
    
    (void)QTSS_GetValuePtr(theRequest, qtssRTSPReqFullRequest, 0, (void**)&theFullRequest.Ptr, &theFullRequest.Len);
    StringParser fullRequest(&theFullRequest);

    // Parsing the HTTP request
    fullRequest.ConsumeWord(&reqMethod);
    if ( !(reqMethod.Equal(StrPtrLen("GET")) || reqMethod.Equal(StrPtrLen("HEAD"))) )
        // It's not a "Get" or a "Head" request 
        return QTSS_NoErr;
     
    fullRequest.ConsumeWhitespace();
    if ( !fullRequest.Expect('/') )
        // Improperly formed request
        return QTSS_NoErr;
            
    fullRequest.ConsumeUntil(&reqStr, StringParser::sEOLWhitespaceMask);
    if( reqStr.Len == 0 )
        //if a file or directory name is not given, return
        return QTSS_NoErr;
        
    if ( !reqStr.Equal(StrPtrLen("Popular.smil")) )
        return QTSS_NoErr;
    
    // If it's a "Head" request send the Head response header back and just return
    if ( reqMethod.Equal(StrPtrLen("HEAD")) )
    {
        QTSS_Write(theRequest, sResponseHeader, ::strlen(sResponseHeader), NULL, 0);    
        return QTSS_NoErr;
    }

    // Create a buffer to store data.
    char theFileBuffer[8192];
    char contentLength[256];
        
    // Before sending any response, set keep alive to off for this connection
    // Regardless of what the client sends, the server always closes the connection after sending the file
    (void)QTSS_SetValue(theRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));
    
#if HTTP_FILE_DEBUGGING
    qtss_printf("Creating a smil file\n");   
#endif
    // Create a ref movie buffer for the single file. It is of the form:
    //  rtsptext\r
    //  rtsp://servername/filepath
    char smilFileBuf[8192] = {0};

    GenerateHotHitSMIL(smilFileBuf);
                
    qtss_sprintf(contentLength, "%"_U32BITARG_"", (UInt32) ::strlen(smilFileBuf));
    // Allocate memory for theFileBuffer
    // Write the HTTP header prefix into the buffer
    ::strcpy(theFileBuffer, sRespHeaderPrefix.Ptr);
    ::strcat(theFileBuffer, sContentLengthHeaderTag.Ptr);
    // Write the remaining part of the HTTP header into the file buffer
    ::strcat(theFileBuffer, contentLength);
    ::strcat(theFileBuffer, sContentTypeHeaderTag.Ptr);
    ::strcat(theFileBuffer, sSmilMimeType.Ptr);
    ::strcat(theFileBuffer, "\r\n\r\n");
    
    // Write the smil file created above to the file buffer
    ::strcat(theFileBuffer, smilFileBuf);
    
    // Write the contents of the file buffer to the request stream and return
    QTSS_Write(theRequest, theFileBuffer, strlen(theFileBuffer), NULL, 0);
    
#if HTTP_FILE_DEBUGGING
    qtss_printf("Wrote the smil file to the request stream. Successful!\n"); 
#endif

    return QTSS_NoErr;
}