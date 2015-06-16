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
    File:       RTPSession3GPP.h

    Contains:   RTPSession3GPP represents the 3gpp specific support for an RTP session. The server creates
                one of these when a new client connects.
                
    Change History (most recent first):

    
    

*/


#ifndef _RTPSESSION3GPP_H_
#define _RTPSESSION3GPP_H_

#include "QTSS.h"
#include "QTSSDictionary.h"
#include "RTSPRequest3GPP.h"


class RTPSession3GPP : public QTSSDictionary
{
    public:
    
        //Initialize
        //Call initialize before instantiating this class: see QTSServer.cpp.
        static void         Initialize();

        RTPSession3GPP(Bool16 enabled);
        virtual ~RTPSession3GPP();
        
        void SetLinkCharData(LinkCharDataFields* linkCharDataPtr);
		void SetGKbits(UInt32 gKBits) { fLinkCharGuarntdKBitsPerSec = gKBits; }
        void SetMaxKbits(UInt32 maxKbits) { fLinkCharMaxKBitsPerSec = maxKbits; }
        void SetMaxDelayMilliSecs(UInt32 maxDelayMilliSecs) { fLinkCharMaxTransferDelayMilliSec = maxDelayMilliSecs; }
        void SetURLCopy(StrPtrLen* urlPtr) { if (urlPtr) (void) this->SetValue(qtss3GPPCliSesLinkCharURL, 0, urlPtr->Ptr , urlPtr->Len, QTSSDictionary::kDontObeyReadOnly);}
        UInt32 GetLinkCharMaxKBits() { return fLinkCharMaxKBitsPerSec; }
        
    private:
        
        Bool16      fEnabled;
        char        fURL[128]; //will be resized as needed
        UInt32      fLinkCharGuarntdKBitsPerSec;
        UInt32      fLinkCharMaxKBitsPerSec;
        UInt32      fLinkCharMaxTransferDelayMilliSec;
       
        
    //Dictionary support
    static QTSSAttrInfoDict::AttrInfo   sAttributes[];
 
};

#endif //_RTPSESSION_H_
