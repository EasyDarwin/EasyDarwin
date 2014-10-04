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
    File:       SourceInfo.cpp

    Contains:   Implements object defined in .h file.
                    

*/

#include "SourceInfo.h"
#include "SocketUtils.h"
#include "SDPSourceInfo.h"
#include "OSMemory.h"
#include "StringParser.h"

SourceInfo::SourceInfo(const SourceInfo& copy)
:   fStreamArray(NULL), fNumStreams(copy.fNumStreams), 
    fOutputArray(NULL), fNumOutputs(copy.fNumOutputs),
    fTimeSet(copy.fTimeSet),fStartTimeUnixSecs(copy.fStartTimeUnixSecs),
    fEndTimeUnixSecs(copy.fEndTimeUnixSecs), fSessionControlType(copy.fSessionControlType),
    fHasValidTime(false)
{   
    
    if(copy.fStreamArray != NULL && fNumStreams != 0)
    {
        fStreamArray = NEW StreamInfo[fNumStreams];
        for (UInt32 index=0; index < fNumStreams; index++)
            fStreamArray[index].Copy(copy.fStreamArray[index]);
    }
    
    if(copy.fOutputArray != NULL && fNumOutputs != 0)
    {
        fOutputArray = NEW OutputInfo[fNumOutputs];
        for (UInt32 index2=0; index2 < fNumOutputs; index2++)
            fOutputArray[index2].Copy(copy.fOutputArray[index2]);
    }
    
}

SourceInfo::~SourceInfo()
{
    if(fStreamArray != NULL)
        delete [] fStreamArray;

    if(fOutputArray != NULL)
        delete [] fOutputArray;
        
}

Bool16  SourceInfo::IsReflectable()
{
    if (fStreamArray == NULL)
        return false;
    if (fNumStreams == 0)
        return false;
        
    //each stream's info must meet certain criteria
    for (UInt32 x = 0; x < fNumStreams; x++)
    {
        if (fStreamArray[x].fIsTCP)
            continue;
            
        if ((!this->IsReflectableIPAddr(fStreamArray[x].fDestIPAddr)) ||
            (fStreamArray[x].fTimeToLive == 0))
            return false;
    }
    return true;
}

Bool16  SourceInfo::IsReflectableIPAddr(UInt32 inIPAddr)
{
    if (SocketUtils::IsMulticastIPAddr(inIPAddr) || SocketUtils::IsLocalIPAddr(inIPAddr))
        return true;
    return false;
}

Bool16  SourceInfo::HasTCPStreams()
{   
    //each stream's info must meet certain criteria
    for (UInt32 x = 0; x < fNumStreams; x++)
    {
        if (fStreamArray[x].fIsTCP)
            return true;
    }
    return false;
}

Bool16  SourceInfo::HasIncomingBroacast()
{   
    //each stream's info must meet certain criteria
    for (UInt32 x = 0; x < fNumStreams; x++)
    {
        if (fStreamArray[x].fSetupToReceive)
            return true;
    }
    return false;
}
SourceInfo::StreamInfo* SourceInfo::GetStreamInfo(UInt32 inIndex)
{
    Assert(inIndex < fNumStreams);
    if (fStreamArray == NULL)
        return NULL;
    if (inIndex < fNumStreams)
        return &fStreamArray[inIndex];
    else
        return NULL;
}

SourceInfo::StreamInfo* SourceInfo::GetStreamInfoByTrackID(UInt32 inTrackID)
{
    if (fStreamArray == NULL)
        return NULL;
    for (UInt32 x = 0; x < fNumStreams; x++)
    {
        if (fStreamArray[x].fTrackID == inTrackID)
            return &fStreamArray[x];
    }
    return NULL;
}

SourceInfo::OutputInfo* SourceInfo::GetOutputInfo(UInt32 inIndex)
{
    Assert(inIndex < fNumOutputs);
    if (fOutputArray == NULL)
        return NULL;
    if (inIndex < fNumOutputs)
        return &fOutputArray[inIndex];
    else
        return NULL;
}

UInt32 SourceInfo::GetNumNewOutputs()
{
    UInt32 theNumNewOutputs = 0;
    for (UInt32 x = 0; x < fNumOutputs; x++)
    {
        if (!fOutputArray[x].fAlreadySetup)
            theNumNewOutputs++;
    }
    return theNumNewOutputs;
}

Bool16  SourceInfo::SetActiveNTPTimes(UInt32 startTimeNTP,UInt32 endTimeNTP)
{   // right now only handles earliest start and latest end time.

    //qtss_printf("SourceInfo::SetActiveNTPTimes start=%"_U32BITARG_" end=%"_U32BITARG_"\n",startTimeNTP,endTimeNTP);
    Bool16 accepted = false;
    do 
    {
        if ((startTimeNTP > 0) && (endTimeNTP > 0) && (endTimeNTP < startTimeNTP)) break; // not valid NTP time
        
        UInt32 startTimeUnixSecs = 0; 
        UInt32 endTimeUnixSecs  = 0; 
        
        if (startTimeNTP != 0 && IsValidNTPSecs(startTimeNTP)) // allow anything less than 1970 
            startTimeUnixSecs = NTPSecs_to_UnixSecs(startTimeNTP);// convert to 1970 time
        
        if (endTimeNTP != 0 && !IsValidNTPSecs(endTimeNTP)) // don't allow anything less than 1970
            break;
            
        if (endTimeNTP != 0) // convert to 1970 time
            endTimeUnixSecs = NTPSecs_to_UnixSecs(endTimeNTP);

        fStartTimeUnixSecs = startTimeUnixSecs;
        fEndTimeUnixSecs = endTimeUnixSecs; 
        accepted = true;
        
    }  while(0);
    
    //char buffer[kTimeStrSize];
    //qtss_printf("SourceInfo::SetActiveNTPTimes fStartTimeUnixSecs=%"_U32BITARG_" fEndTimeUnixSecs=%"_U32BITARG_"\n",fStartTimeUnixSecs,fEndTimeUnixSecs);
    //qtss_printf("SourceInfo::SetActiveNTPTimes start time = %s",qtss_ctime(&fStartTimeUnixSecs, buffer, sizeof(buffer)) );
    //qtss_printf("SourceInfo::SetActiveNTPTimes end time = %s",qtss_ctime(&fEndTimeUnixSecs, buffer, sizeof(buffer)) );
    fHasValidTime = accepted;
    return accepted;
}

Bool16  SourceInfo::IsActiveTime(time_t unixTimeSecs)
{ 
    // order of tests are important here
    // we do it this way because of the special case time value of 0 for end time
    // start - 0 = unbounded 
    // 0 - 0 = permanent
    if (false == fHasValidTime)
        return false;
        
    if (unixTimeSecs < 0) //check valid value
        return false;
        
    if (IsPermanentSource()) //check for 0 0
        return true;
    
    if (unixTimeSecs < fStartTimeUnixSecs)
        return false; //too early

    if (fEndTimeUnixSecs == 0)  
        return true;// accept any time after start

    if (unixTimeSecs > fEndTimeUnixSecs)
        return false; // too late

    return true; // ok start <= time <= end

}


UInt32 SourceInfo::GetDurationSecs() 
{    
    
    if (fEndTimeUnixSecs == 0) // unbounded time
        return (UInt32) ~0; // max time
    
    time_t timeNow = OS::UnixTime_Secs();
    if (fEndTimeUnixSecs <= timeNow) // the active time has past or duration is 0 so return the minimum duration
        return (UInt32) 0; 
            
    if (fStartTimeUnixSecs == 0) // relative duration = from "now" to end time
        return fEndTimeUnixSecs - timeNow;
    
    return fEndTimeUnixSecs - fStartTimeUnixSecs; // this must be a duration because of test for endtime above

}

Bool16 SourceInfo::Equal(SourceInfo* inInfo)
{
    // Check to make sure the # of streams matches up
    if (this->GetNumStreams() != inInfo->GetNumStreams())
        return false;
    
    // Check the src & dest addr, and port of each stream. 
    for (UInt32 x = 0; x < this->GetNumStreams(); x++)
    {
        if (GetStreamInfo(x)->fDestIPAddr != inInfo->GetStreamInfo(x)->fDestIPAddr)
            return false;
        if (GetStreamInfo(x)->fSrcIPAddr != inInfo->GetStreamInfo(x)->fSrcIPAddr)
            return false;
        
        // If either one of the comparators is 0 (the "wildcard" port), then we know at this point
        // they are equivalent
        if ((GetStreamInfo(x)->fPort == 0) || (inInfo->GetStreamInfo(x)->fPort == 0))
            return true;
            
        // Neither one is the wildcard port, so they must be the same
        if (GetStreamInfo(x)->fPort != inInfo->GetStreamInfo(x)->fPort)
            return false;
    }
    return true;
}

void SourceInfo::StreamInfo::Copy(const StreamInfo& copy)
{
    fSrcIPAddr = copy.fSrcIPAddr;
    fDestIPAddr = copy.fDestIPAddr;
    fPort = copy.fPort;
    fTimeToLive = copy.fTimeToLive;
    fPayloadType = copy.fPayloadType;
    if ((copy.fPayloadName).Ptr != NULL)
        fPayloadName.Set((copy.fPayloadName).GetAsCString(), (copy.fPayloadName).Len);
    fTrackID = copy.fTrackID;
    fBufferDelay = copy.fBufferDelay;
    fIsTCP = copy.fIsTCP;
    fSetupToReceive = copy.fSetupToReceive;
    fTimeScale = copy.fTimeScale;    
}

SourceInfo::StreamInfo::~StreamInfo()
{
    if (fPayloadName.Ptr != NULL)
        delete fPayloadName.Ptr;
    fPayloadName.Len = 0;
}

void SourceInfo::OutputInfo::Copy(const OutputInfo& copy)
{
    fDestAddr = copy.fDestAddr;
    fLocalAddr = copy.fLocalAddr;
    fTimeToLive = copy.fTimeToLive;
    fNumPorts = copy.fNumPorts;
    if(fNumPorts != 0)
    {
        fPortArray = NEW UInt16[fNumPorts];
        ::memcpy(fPortArray, copy.fPortArray, fNumPorts * sizeof(UInt16));
    }
    fBasePort = copy.fBasePort;
    fAlreadySetup = copy.fAlreadySetup;
}

SourceInfo::OutputInfo::~OutputInfo()
{
    if (fPortArray != NULL)
        delete [] fPortArray;
}

Bool16 SourceInfo::OutputInfo::Equal(const OutputInfo& info)
{
    if ((fDestAddr == info.fDestAddr) && (fLocalAddr == info.fLocalAddr) && (fTimeToLive == info.fTimeToLive))
    {
        if ((fBasePort != 0) && (fBasePort == info.fBasePort))
            return true;
        else if ((fNumPorts == 0) || ((fNumPorts == info.fNumPorts) && (fPortArray[0] == info.fPortArray[0])))
            return true;
    }
    return false;
}


