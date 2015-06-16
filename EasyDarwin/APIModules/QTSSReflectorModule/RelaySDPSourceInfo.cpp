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
    File:       RelaySDPSourceInfo.cpp

    Contains:   Implementation of object defined in .h file

    

*/

#include "RelaySDPSourceInfo.h"
#include "SDPSourceInfo.h"

#include "MyAssert.h"
#include "StringParser.h"
#include "OSMemory.h"

#ifndef __Win32__
#include <netinet/in.h>
#endif

RelaySDPSourceInfo::~RelaySDPSourceInfo()
{
    // Not necessary anymore as the destructor of the base class will take care
    // of deleting all allocated memory for fOutputArray and fStreamArray
    /*
    if (fOutputArray != NULL)
    {
        for (UInt32 x = 0; x < fNumOutputs; x++)
        {
            delete [] fOutputArray[x].fPortArray;
            fOutputArray[x].fNumPorts = 0;
        }
            
        char* theOutputArray = (char*)fOutputArray;
        delete [] theOutputArray;
    }
    if (fStreamArray != NULL)
    {
        char* theStreamArray = (char*)fStreamArray;
        delete [] theStreamArray;
    }
    */
}


void    RelaySDPSourceInfo::Parse(StrPtrLen* inSDPData)
{
    // These are the lines of the SDP file that we are interested in
    static StrPtrLen sRelayAddr("a=x-qt-relay-addr");
    static StrPtrLen sRelayPort("a=x-qt-relay-port");
    
    Assert(fOutputArray == NULL);
    Assert(fStreamArray == NULL);
    
    StrPtrLen sdpLine;
    StrPtrLen outputAddrs;
    
    StringParser trackCounter(inSDPData);

    UInt32 theDestIPAddr = 0;
    UInt16 theDestTtl = 0;

    //
    // FIRST WALK THROUGH SDP
    // The first walk is to count up the number of StreamInfo & OutputInfo
    // objects that we will need.
    while (true)
    {
        // grab a line
        trackCounter.ConsumeUntil(&sdpLine, StringParser::sEOLMask);
        
        if (sdpLine.NumEqualIgnoreCase(sRelayAddr.Ptr, sRelayAddr.Len))
        {
            // there is a x-qt-relay-addr line, look for all IP addrs
            StringParser relayAddrParser(&sdpLine);
            relayAddrParser.ConsumeUntil(NULL, StringParser::sDigitMask);
            
            // The first IP addr on this line is the destination IP addr of the
            // source broadcast.
            theDestIPAddr = SDPSourceInfo::GetIPAddr(&relayAddrParser, ' ');
            relayAddrParser.ConsumeWhitespace();
            
            // Store this position so we can later go back to it
            outputAddrs.Ptr = relayAddrParser.GetCurrentPosition();
            outputAddrs.Len = relayAddrParser.GetDataRemaining();
            
            StrPtrLen theTtl;
            while (relayAddrParser.GetDataRemaining() > 0)
            {
                relayAddrParser.ConsumeUntil(&theTtl, ' '); 
                relayAddrParser.ConsumeWhitespace();
                fNumOutputs++;
            }
            fNumOutputs--;// Don't count the ttl as an output!
            
            StringParser ttlParser(&theTtl);
            theDestTtl = (UInt16) ttlParser.ConsumeInteger(NULL);
        }
        // Each x=qt-relay-port line corresponds to one source stream.
        else if (sdpLine.NumEqualIgnoreCase(sRelayPort.Ptr, sRelayPort.Len))
            fNumStreams++;

        //stop when we reach an empty line.
        if (!trackCounter.ExpectEOL())
            break;
    }
    
    // No relay info in this file!
    if ((fNumStreams == 0) || (fNumOutputs == 0))
        return;
        
    // x-qt-relay-port lines should always be in pairs (RTP & RTCP)
    if ((fNumStreams & 1) != 0)
        return;
    fNumStreams /= 2;

    //
    // CONSTRUCT fStreamInfo AND fOutputInfo ARRAYS
    fStreamArray = NEW StreamInfo[fNumStreams];
    fOutputArray = NEW OutputInfo[fNumOutputs];
    
    //
    // FILL IN ARRAYS
    
    // Filling in the output addresses is easy because the outputAddrs
    // StrPtrLen points right at the data we want
    StringParser theOutputAddrParser(&outputAddrs);
    for (UInt32 x = 0; x < fNumOutputs; x++)
    {
        fOutputArray[x].fDestAddr = SDPSourceInfo::GetIPAddr(&theOutputAddrParser, ' ');
        fOutputArray[x].fLocalAddr = INADDR_ANY;
        fOutputArray[x].fTimeToLive = theDestTtl;
        fOutputArray[x].fPortArray = NEW UInt16[fNumStreams];//Each output has one port per stream
        fOutputArray[x].fNumPorts = fNumStreams;
        ::memset(fOutputArray[x].fPortArray, 0, fNumStreams * sizeof(UInt16));
        fOutputArray[x].fAlreadySetup = false;
        theOutputAddrParser.ConsumeWhitespace();
        Assert(fOutputArray[x].fDestAddr > 0);
    }
    
    StringParser sdpParser(inSDPData);
    
    // Now go through and find all the port information on all the x-qt-relay-port lines
    for (UInt32 theStreamIndex = 0; theStreamIndex < fNumStreams; )
    {
        sdpParser.ConsumeUntil(&sdpLine, StringParser::sEOLMask);
        
        // parse through all the x-qt-relay-port lines
        if (sdpLine.NumEqualIgnoreCase(sRelayPort.Ptr, sRelayPort.Len))
        {
            // Begin parsing... find the first port on the line
            StringParser relayAddrParser(&sdpLine);
            relayAddrParser.ConsumeUntil(NULL, StringParser::sDigitMask);
        
            // The first port is the source port for this stream
            fStreamArray[theStreamIndex].fPort = (UInt16) relayAddrParser.ConsumeInteger(NULL);
            if (fStreamArray[theStreamIndex].fPort & 1)
                continue; //we only care about RTP ports
            
            // Fill in all the fields we can for this stream
            fStreamArray[theStreamIndex].fDestIPAddr = theDestIPAddr;
            fStreamArray[theStreamIndex].fTimeToLive = theDestTtl;
            fStreamArray[theStreamIndex].fTrackID = theStreamIndex + 1;
            
            // Now fill in all the output ports for this stream
            for (UInt32 x = 0; x < fNumOutputs; x++)
            {
                relayAddrParser.ConsumeWhitespace();
                fOutputArray[x].fPortArray[theStreamIndex] = (UInt16) relayAddrParser.ConsumeInteger(NULL);
            }
            
            theStreamIndex++;
        }
        //stop when we reach an empty line.
        if (!sdpParser.ExpectEOL())
            break;
    }
}
