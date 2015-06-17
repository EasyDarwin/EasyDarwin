/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       RTSPProtocol.h

    Contains:   A grouping of static utilities that abstract keyword strings
                in the RTSP protocol. This should be maintained as new versions
                of the RTSP protoocl appear & as the server evolves to take
                advantage of new RTSP features.
*/


#ifndef __RTSPPROTOCOL_H__
#define __RTSPPROTOCOL_H__

#include "QTSSRTSPProtocol.h"
#include "StrPtrLen.h"

class RTSPProtocol
{
    public:

        //METHODS
        
        //  Method enumerated type definition in QTSS_RTSPProtocol.h
            
        //The lookup function. Very simple
        static UInt32   GetMethod(const StrPtrLen &inMethodStr);
        
        static StrPtrLen&   GetMethodString(QTSS_RTSPMethod inMethod)
            { return sMethods[inMethod]; }
        
        //HEADERS

        //  Header enumerated type definitions in QTSS_RTSPProtocol.h
        
        //The lookup function. Very simple
        static UInt32 GetRequestHeader(const StrPtrLen& inHeaderStr);
        
        //The lookup function. Very simple.
        static StrPtrLen& GetHeaderString(UInt32 inHeader)
            { return sHeaders[inHeader]; }
        
        
        //STATUS CODES

        //returns name of this error
        static StrPtrLen&       GetStatusCodeString(QTSS_RTSPStatusCode inStat)
            { return sStatusCodeStrings[inStat]; }
        //returns error number for this error
        static SInt32           GetStatusCode(QTSS_RTSPStatusCode inStat)
            { return sStatusCodes[inStat]; }
        //returns error number as a string
        static StrPtrLen&       GetStatusCodeAsString(QTSS_RTSPStatusCode inStat)
            { return sStatusCodeAsStrings[inStat]; }
        
        // VERSIONS
        enum RTSPVersion
        {
            k10Version = 0,
            kIllegalVersion = 1
        };
        
        // NAMES OF THINGS
        static StrPtrLen&       GetRetransmitProtocolName() { return sRetrProtName; }
        
        //accepts strings that look like "RTSP/1.0" etc...
        static RTSPVersion      GetVersion(StrPtrLen &versionStr);
        static StrPtrLen&       GetVersionString(RTSPVersion version)
            { return sVersionString[version]; }
        
    private:

        //for other lookups
        static StrPtrLen            sMethods[];
        static StrPtrLen            sHeaders[];
        static StrPtrLen            sStatusCodeStrings[];
        static StrPtrLen            sStatusCodeAsStrings[];
        static SInt32               sStatusCodes[];
        static StrPtrLen            sVersionString[];
        
        static StrPtrLen            sRetrProtName;

};
#endif // __RTSPPROTOCOL_H__
