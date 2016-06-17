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
    File:       RTSPRequest3GPP.h

    Contains:   This parses 3gpp headers from a request
                    
    

*/

#ifndef __RTSPREQUEST3GPP_H__
#define __RTSPREQUEST3GPP_H__

#include "QTSS.h"
#include "QTSSDictionary.h"
#include "StringParser.h"

/*

5.3.2.2 The 3GPP-Adaptation header 
To enable PSS clients to set bit-rate adaptation parameters, a new RTSP request and response header is defined. The 
header can be used in the methods SETUP, PLAY, OPTIONS, and SET_PARAMETER.  The header defined in ABNF 
[53] has the following syntax: 
3GPP-adaptation-def = "3GPP-Adaptation" ":" adaptation-spec 0*("," adaptation-spec) 
adaptation-spec = url-def *adapt-params 
adapt-params = ";" buffer-size-def 
     / ";" target-time-def 
url-def  = "url" "=" <"> url <"> 
buffer-size-def  = "size" "=" 1*9DIGIT ; bytes 
target-time-def  = "target-time" "=" 1*9DIGIT; ms 
url              = ( absoluteURI / relativeURI ) 
absoluteURI and relativeURI are defined in RFC 2396 [60] and updated in RFC 2732 [61]. The base URI for any 
relative URI is the RTSP request URI. 
The "3GPP-Adaptation" header shall be sent in responses to requests containing this header. The PSS server shall not 
change the values in the response header. The presence of the header in the response indicates to the client that the 
server acknowledges the request. 
The buffer size signalled in the "3GPP-Adaptation" header shall correspond to reception, de-jittering, and, if used, de- 
interleaving buffer(s) that have this given amount of space for complete application data units (ADU), including the 
following RTP header and RTP payload header fields: RTP timestamp, and sequence numbers or decoding order 
numbers. The specified buffer size shall also include any Annex G pre-decoder buffer space used for this media, as the 
two buffers cannot be separated. 
The target protection time signalled in the value of the "target-time" parameter is the targeted minimum buffer level or, 
in other words, the client desired amount of playback time in milliseconds to guarantee interrupt-free playback and 
allow the server to adjust the transmission rate, if needed. 


Telnet test data
DESCRIBE rtsp://127.0.0.1/sample_50kbit.3gp RTSP/1.0
CSeq: 1
Accept: application/sdp
User-Agent: telnet manual blah blah (03.05) Profile/MIDP-1.0 Configuration/CLDC-1.0


SETUP rtsp://127.0.0.1/sample_50kbit.3gp/trackID=3 RTSP/1.0
CSeq: 2
Content-Length: 0
User-Agent: telnet 
Transport: RTP/AVP;unicast;client_port=1566-1567
3GPP-Adaptation: url="rtsp:/127.0.0.1/sample_50kbit.3gp/streamID=0";target-time=7000,url="rtsp:/127.0.0.1/sample_50kbit.3gp/streamID=1";target-time=7000

*/


class RateAdapationStreamDataFields 
{
    public:
        RateAdapationStreamDataFields() :   fTrackID (0), fBufferSizeBytes(0), fTargetTimeMilli(0) {}
        void SetData(StrPtrLen *streamDataStr);
        void CopyData(RateAdapationStreamDataFields* source);
        ~RateAdapationStreamDataFields() {}
        UInt32 GetSDPStreamID() { return fTrackID; }
        UInt32 GetBufferSizeBytes()  { return fBufferSizeBytes; }
        UInt32 GetTargetTimeMilliSec()  { return fTargetTimeMilli; }
        void PrintData(StrPtrLen *streamDataStr = NULL) {if (streamDataStr != NULL) this->SetData(streamDataStr); qtss_printf("RateAdapationStreamDataFields::PrintData trackID=%" _U32BITARG_ " bufferSize=%" _U32BITARG_ " targetTime=%" _U32BITARG_ "\n", GetSDPStreamID(), GetBufferSizeBytes(),GetTargetTimeMilliSec() ); }

 
    private:
    
        UInt32                  fTrackID;
        UInt32                  fBufferSizeBytes;
        UInt32                  fTargetTimeMilli;
        
};


#if 0
/*
5.3.2.1 The 3GPP-Link-Char header 
To enable PSS clients to report the link characteristics of the radio interface to the PSS server, the "3GPP-Link-Char" 
RTSP header is defined. The header takes one or more arguments. The reported information should be taken from a 
QoS reservation (i.e. the QoS profile as defined in [56]). Note that this information is only valid for the wireless link 
and does not apply end-to-end. However, the parameters do provide constraints that can be used.  

Three parameters are defined that can be included in the header, and future extensions are possible to define. Any 
unknown parameter shall be ignored. The three parameters are: 
- "GBW": the link's guaranteed bit-rate in kilobits per second as defined by [56]; 
- "MBW": the link's maximum bit-rate in kilobits per second as defined by [56]; 
- "MTD": the link's maximum transfer delay, as defined by [56] in milliseconds. 
The "3GPP-Link-Char" header syntax is defined below using ABNF [53]: 
3gpplinkheader  = "3GPP-Link-Char" ":" link-char-spec *("," 0*1SP link-char-spec) CRLF 
link-char-spec  = char-link-url *(";" 0*1SP link-parameters)  
char-link-url  = "url" "=" <">url<"> 
link-parameters  = Guaranteed-BW / Max-BW / Max-Transfer-delay / extension-type 
Guaranteed-BW  = "GBW" "=" 1*DIGIT  ; bps 
Max-BW  = "MBW" "=" 1*DIGIT ; bps 
Max-Transfer-delay  = "MTD" "=" 1*DIGIT ; ms 
extension-type   = token "=" (token / quoted-string) 
DIGIT  = as defined in RFC 2326 [5] 
token  = as defined in RFC 2326 [5] 
quoted-string  = as defined in RFC 2326 [5] 
url   = as defined in RFC 2326 [5] 
The "3GPP-Link-Char" header can be included in a request using any of the following RTSP methods: SETUP, PLAY, 
OPTIONS, and SET_PARAMETER. The header shall not be included in any response. The header can contain one or 
more characteristics specifications. Each specification contains a URI that can either be an absolute or a relative, any 
relative URI use the RTSP request URI as base. The URI points out the media component that the given parameters 
apply to. This can either be an individual media stream or a session aggregate.  
If a QoS reservation (PDP context) is shared by several media components in a session the 3GPP-Link-Char header 
shall not be sent prior to the RTSP PLAY request. In this case the URI to use is the aggregated RTSP URI. If the QoS 
reservation is not shared (one PDP context per media) the media stream URI must be used in the 3GPP-Link-Char 
specification. If one QoS reservation (PDP context) per media component is used, the specification parameters shall be 
sent per media component. 
The "3GPP-Link-Char" header should be included in a SETUP or PLAY request by the client, to give the initial values 
for the link characteristics. A SET_PARAMETER or OPTIONS request can be used to update the 3GPP-Link-Char 
values in a session currently playing. It is strongly recommended that SET_PARAMETER is used, as this has the 
correct semantics for the operation and also requires less overhead both in bandwidth and server processing. When 
performing updates of the parameters, all of the previous signalled values are undefined and only the given ones in the 
update are defined. This means that even if a parameter has not changed, it must be included in the update.  

Example:  
3GPP-LinkChar: url="rtsp://server.example.com/media.3gp"; GBW=32; MBW=128; MTD=2000 

In the above example the header tells the server that its radio link has a QoS setting with a guaranteed bit-rate of 32 
kbps, a maximum bit-rate of 128 kbps, and a maximum transfer delay of 2.0 seconds. These parameters are valid for the 
aggregate of all media components, as the URI is an aggregated RTSP URI. 

Test protocol 
DESCRIBE rtsp://127.0.0.1/sample_50kbit.3gp RTSP/1.0
CSeq: 1
Accept: application/sdp
User-Agent: telnet manual blah blah (03.05) Profile/MIDP-1.0 Configuration/CLDC-1.0


SETUP rtsp://127.0.0.1/sample_50kbit.3gp/trackID=3 RTSP/1.0
CSeq: 2
Content-Length: 0
User-Agent: telnet 
Transport: RTP/AVP;unicast;client_port=1566-1567
3GPP-Adaptation: url="rtsp:/127.0.0.1/sample_50kbit.3gp/streamID=0";target-time=7000,url="rtsp:/127.0.0.1/sample_50kbit.3gp/streamID=1";target-time=7000
3GPP-Link-Char: url="rtsp://server.example.com/media.3gp"; GBW=32; MBW=128; MTD=2000 


*/
#endif

class LinkCharDataFields 
{
    public:
        LinkCharDataFields() :   fGuaranteedKBitsPerSec (0), fMaximumKBitsPerSec(0), fMaximumTransferDelayMilliSec(0), fURL() {}
        void SetData(StrPtrLen *streamDataStr);
        void CopyData(LinkCharDataFields* source);
        ~LinkCharDataFields() {}
        UInt32 GetGKbits() { return fGuaranteedKBitsPerSec; }
        UInt32 GetMaxKBits()  { return fMaximumKBitsPerSec; }
        UInt32 GetMaxDelayMilliSecs()  { return fMaximumTransferDelayMilliSec; }
        StrPtrLen* GetURL() { return &fURL; }
        void PrintData(StrPtrLen *streamDataStr = NULL) 
        {if (streamDataStr != NULL) this->SetData(streamDataStr); qtss_printf("LinkCharDataFields::PrintData fURL=%s fGuaranteedKBitsPerSec=%" _U32BITARG_ " fMaximumKBitsPerSec=%" _U32BITARG_ " fMaximumTransferDelayMilliSec=%" _U32BITARG_ "\n",GetURL()->Ptr, GetGKbits(), GetMaxKBits(),GetMaxDelayMilliSecs() ); }

 
    private:
        void ParseData( StrPtrLen &theDataStr, StringParser &theLinkCharDataParser);

        UInt32                  fGuaranteedKBitsPerSec;
        UInt32                  fMaximumKBitsPerSec;
        UInt32                  fMaximumTransferDelayMilliSec;
        StrPtrLenDel            fURL;
        
};




//3GPPRequest utility class definition
class RTSPRequest3GPP : public QTSSDictionary
{
    public:
        //Initialize
        static void         Initialize();

        //CONSTRUCTOR / DESTRUCTOR
        //these do very little. Just initialize / delete some member data.
        //
        //Arguments:        enable the object
        RTSPRequest3GPP(Bool16 enabled);
        ~RTSPRequest3GPP() {}
    
        //Parses the request. Returns an error if there was an error encountered
        //in parsing.
         QTSS_Error ParseAdpationHeader(QTSSDictionary* headerDictionaryPtr); 
         
        //Parses the request. Returns an error if there was an error encountered
        //in parsing.
         QTSS_Error ParseLinkCharHeader(QTSSDictionary* headerDictionaryPtr);
         
         
        Bool16                  Is3GPP() {return fIs3GPP;} 
        Bool16                  HasRateAdaptation()  {return fHasRateAdaptation;} 
        Bool16                  HasLinkChar()   {return fHasLinkChar;}
        
    private:
        void ParseAdpationHeaderTest();
        void ParseLinkCharHeaderTest(QTSSDictionary* headerDictionaryPtr);


        Bool16                  fEnabled;     
        Bool16                  fIs3GPP;
        Bool16                  fHasRateAdaptation;
        Bool16                  fHasLinkChar;
        
    
       
    //Dictionary support
    static QTSSAttrInfoDict::AttrInfo   sAttributes[];

};
#endif // __RTSPREQUEST3GPP_H__

