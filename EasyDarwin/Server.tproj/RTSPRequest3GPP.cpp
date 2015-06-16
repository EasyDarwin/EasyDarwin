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
    File:       RTSPRequest3gpp.cpp

    Contains:   Implementation of RTSPRequest3gpp class.

    
    
*/


#include "RTSPRequest3GPP.h"
#include "RTSPProtocol.h"
#include "QTSServerInterface.h"

#include "RTSPSession.h"
#include "RTSPSessionInterface.h"
#include "StringParser.h"
#include "StringTranslator.h"
#include "OS.h"
#include "OSMemory.h"
#include "QTSS.h"
#include "QTSSModuleUtils.h"
#include "base64.h"
#include "OSArrayObjectDeleter.h"
#include "DateTranslator.h"
#include "SocketUtils.h"


void RateAdapationStreamDataFields::SetData(StrPtrLen *streamDataStr)
{

    static StrPtrLen sSize("size");
    static StrPtrLen sTargetTime("target-time");
    static StrPtrLen sURL("url");
    
    StringParser theStreamDataParser(streamDataStr);

    StrPtrLen url;
    theStreamDataParser.GetThru(&url, '=');//consume "url="
    url.TrimWhitespace(); // fix if it is " url =" instead of "url="

    if (false == url.NumEqualIgnoreCase(sURL.Ptr, sURL.Len))
        return; //failed to find url=
    

    theStreamDataParser.GetThru(&url, ';');//consume "url=/asdlfjasdf/id=12;"
    
    { //get the stream id url component 
        
        StringParser theURLParser(&url);
        while (theURLParser.GetThru(&url, '/')); //trim to the last path segment
    }
    
    { // get the id value
        StringParser theIDParser(&url);
        if (false == theIDParser.GetThru(&url, '='))//consume "id=12;"
            return; //failed to find '='
        
        theIDParser.ConsumeWhitespace();
        fTrackID = theIDParser.ConsumeInteger();
    }
    
    
    StrPtrLen theDataStr;
    while ( theStreamDataParser.GetThru(&theDataStr,'=') ) //get the field name
    {
        theDataStr.TrimWhitespace();
        if (theDataStr.NumEqualIgnoreCase(sSize.Ptr, sSize.Len)) // size=
        {
            theStreamDataParser.ConsumeWhitespace();
            fBufferSizeBytes = theStreamDataParser.ConsumeInteger();
        }
        
        if (theDataStr.NumEqualIgnoreCase(sTargetTime.Ptr, sTargetTime.Len)) // target-time =
        {
            theStreamDataParser.ConsumeWhitespace();
            fTargetTimeMilli = theStreamDataParser.ConsumeInteger();
        }
        theStreamDataParser.GetThru(&theDataStr,';'); //skip to next field
    }
 

};



void RateAdapationStreamDataFields::CopyData(RateAdapationStreamDataFields* source)
{
    if (NULL == source)
        return;
        
    fTrackID = source->GetSDPStreamID();
    fBufferSizeBytes = source->GetBufferSizeBytes();
    fTargetTimeMilli = source->GetTargetTimeMilliSec();
 };
   


//-----------
void LinkCharDataFields::ParseData( StrPtrLen &theDataStr, StringParser &theLinkCharDataParser)
{
    static StrPtrLen sGBW("GBW");
    static StrPtrLen sMBW("MBW");
    static StrPtrLen sMTD("MTD");
 
    if (theDataStr.NumEqualIgnoreCase(sGBW.Ptr, sGBW.Len)) // GBW=
    {
        theLinkCharDataParser.ConsumeWhitespace();
        fGuaranteedKBitsPerSec = theLinkCharDataParser.ConsumeInteger();
        return;
    }
    
    if (theDataStr.NumEqualIgnoreCase(sMBW.Ptr, sMBW.Len)) //MBW=
    {
        theLinkCharDataParser.ConsumeWhitespace();
        fMaximumKBitsPerSec = theLinkCharDataParser.ConsumeInteger();
        return;
    }
    
    if (theDataStr.NumEqualIgnoreCase(sMTD.Ptr, sMTD.Len)) //MBW=
    {
        theLinkCharDataParser.ConsumeWhitespace();
        fMaximumTransferDelayMilliSec = theLinkCharDataParser.ConsumeInteger();
        return;
    }
   

}

void LinkCharDataFields::SetData(StrPtrLen *streamDataStr)
{

   static StrPtrLen sURL("url");
    
    StringParser theLinkCharDataParser(streamDataStr);

    StrPtrLen url;
    theLinkCharDataParser.GetThru(&url, '=');//consume "url="
    url.TrimWhitespace(); // fix if it is " url =" instead of "url="

    if (false == url.NumEqualIgnoreCase(sURL.Ptr, sURL.Len))
        return; //failed to find url=
    
    theLinkCharDataParser.ConsumeUntil(&url, ';');//consume "url=/asdlfjasdf;"
    if (!theLinkCharDataParser.Expect(';'))
        return; // no parameters
        
    url.TrimWhitespace(); 
    fURL.Set(url.GetAsCString());
    
    StrPtrLen theDataStr;
    while ( theLinkCharDataParser.GetThru(&theDataStr,'=') ) //get the field name
    {
        theDataStr.TrimWhitespace();
        this->ParseData(theDataStr,theLinkCharDataParser);
        
        theLinkCharDataParser.GetThru(&theDataStr,';'); //skip to next field
    }
 

};



void LinkCharDataFields::CopyData(LinkCharDataFields* source)
{
    if (NULL == source)
        return;
        
    fURL.Set(source->GetURL()->GetAsCString());
    fGuaranteedKBitsPerSec = source->GetGKbits();
    fMaximumKBitsPerSec = source->GetMaxKBits();
    fMaximumTransferDelayMilliSec = source->GetMaxDelayMilliSecs();
};
   


//--------
QTSSAttrInfoDict::AttrInfo  RTSPRequest3GPP::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0 */ { "qtss3GPPRequestEnabled",                         NULL,  qtssAttrDataTypeBool16,  qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe },
    /* 1 */ { "qtss3GPPRequestRateAdaptationStreamData",        NULL,  qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe }
 

};

void  RTSPRequest3GPP::Initialize()
{
    for (UInt32 x = 0; x < qtss3GPPRequestNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::k3GPPRequestDictIndex)->
            SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
                            sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);

}


RTSPRequest3GPP::RTSPRequest3GPP(Bool16 enabled)
:   QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::k3GPPRequestDictIndex)),
    fEnabled (enabled),fIs3GPP(false), fHasRateAdaptation(false), fHasLinkChar(false)
{
    this->SetVal(qtss3GPPRequestEnabled, &fEnabled, sizeof(fEnabled));
 
}

QTSS_Error RTSPRequest3GPP::ParseAdpationHeader(QTSSDictionary* headerDictionaryPtr)
{

    if (NULL == headerDictionaryPtr)
    {   Assert(0);
        return -1;
    }   

    if (!fEnabled)
        return QTSS_NoErr;
        
    StringParser theRateAdaptHeaderParser(headerDictionaryPtr->GetValue(qtss3GPPAdaptationHeader));
    
    if (theRateAdaptHeaderParser.GetDataRemaining()  == 0)
        return -1;
        
    fIs3GPP = true;
    fHasRateAdaptation = true;

    StrPtrLen theStreamData;
    UInt32 numValueIndex = 0;
    while (theRateAdaptHeaderParser.GetDataRemaining() != 0) 
    {
        theRateAdaptHeaderParser.GetThru(&theStreamData, ',');
        theStreamData.TrimWhitespace();
       (void) this->SetValuePtr(qtss3GPPRequestRateAdaptationStreamData,theStreamData.Ptr, theStreamData.Len, QTSSDictionary::kDontObeyReadOnly);
        numValueIndex++;
    }   
    
    //this->ParseAdpationHeaderTest();

   return QTSS_NoErr;
}

void RTSPRequest3GPP::ParseAdpationHeaderTest()
{
    StrPtrLen dataStr;
    UInt32 numValues = this->GetNumValues(qtss3GPPRequestRateAdaptationStreamData);
    qtss_printf("RTSPRequest3GPP::ParseAdpationHeaderTest numValues =%lu\n", numValues);
    
    for (;numValues > 0; numValues --)
    {
    
        if(0 != this->GetValuePtr(qtss3GPPRequestRateAdaptationStreamData, numValues - 1, (void**) &dataStr.Ptr, &dataStr.Len, true))
            qtss_printf("RTSPRequest3GPP::ParseAdpationHeaderTest err GetValuePtr(qtss3GPPRequestRateAdaptationStreamData\n");
        
        dataStr.PrintStr("RTSPRequest3GPP::ParseAdpationHeaderTest qtss3GPPRequestRateAdaptationStreamData=[","]\n");
        RateAdapationStreamDataFields fieldsParser;
        fieldsParser.SetData(&dataStr);
        fieldsParser.PrintData(NULL);
    }
}        



QTSS_Error RTSPRequest3GPP::ParseLinkCharHeader(QTSSDictionary* headerDictionaryPtr)
{

    if (NULL == headerDictionaryPtr)
    {   Assert(0);
        return -1;
    }   

    StrPtrLen* theLinkCharStr = headerDictionaryPtr->GetValue(qtss3GPPLinkCharHeader);
    if (theLinkCharStr == NULL)
        return -1;
     
    if (theLinkCharStr->Len == 0)
        return -1;
        
    fIs3GPP = true;
    fHasLinkChar = true;

    //this->ParseLinkCharHeaderTest(headerDictionaryPtr);

   return QTSS_NoErr;
}


void RTSPRequest3GPP::ParseLinkCharHeaderTest(QTSSDictionary* headerDictionaryPtr)
{
    StrPtrLen dataStr;
    UInt32 numValues = headerDictionaryPtr->GetNumValues(qtss3GPPLinkCharHeader);
    qtss_printf("RTSPRequest3GPP::ParseLinkCharHeaderTest qtss3GPPLinkCharHeader numValues =%lu\n", numValues); //should be 1
    
    for (;numValues > 0; numValues --)
    {
    
        if(0 != headerDictionaryPtr->GetValuePtr(qtss3GPPLinkCharHeader, numValues - 1, (void**) &dataStr.Ptr, &dataStr.Len, true))
            qtss_printf("RTSPRequest3GPP::ParseLinkCharHeaderTest err GetValuePtr(qtss3GPPLinkCharHeader\n");
        
        dataStr.PrintStr("RTSPRequest3GPP::ParseLinkCharHeaderTest qtss3GPPLinkCharHeader=[","]\n");
        LinkCharDataFields fieldsParser;
        fieldsParser.SetData(&dataStr);
        fieldsParser.PrintData(NULL);
    }
}        


