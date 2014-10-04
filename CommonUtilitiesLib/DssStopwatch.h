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
#ifndef __DSS_STOPWATCH__
#define __DSS_STOPWATCH__
//#include "DssStopwatch.h"

#include "OS.h"


class DssEggtimer {
    public:
        enum { kDurationNeverExpire = -1 };
        
        DssEggtimer( SInt64 inMilliseconds ) { fTimerDuration = inMilliseconds; Reset(); }
        
        void OneShotSetTo( SInt64 inMilliseconds )
        {
            // set the egg timer to this time for one cycle.
            // there after Reset will use fTimerDuration
            fExpirationMilliseconds =  OS::Milliseconds() + inMilliseconds;
        }
        void Reset() 
        { 
            //if ( fTimerDuration != (SInt64)kDurationNeverExpire ) 
            fExpirationMilliseconds =  OS::Milliseconds() + fTimerDuration; 
        }
        
        void ResetTo(SInt64 inMilliseconds) 
        { 
            fTimerDuration = inMilliseconds;
            this->Reset();
        }
        
        Bool16 Expired() 
        { 
            //if (fTimerDuration == (SInt64)kDurationNeverExpire  )
            //  return false;
            
            return fExpirationMilliseconds <= OS::Milliseconds(); 
        }
        SInt64  MaxDuration() { return fTimerDuration; }
        
    private:
        SInt64  fTimerDuration; 
        SInt64  fExpirationMilliseconds;

};

class DssMillisecondStopwatch {

    public:
        DssMillisecondStopwatch() :
        fIsStarted(false)
        , fTimerDuration(-1)
        {}
        ;
        void Start() { fStartedAt = OS::Milliseconds(); fIsStarted = true; }
        void Stop()  { fTimerDuration = OS::Milliseconds() - fStartedAt; }
        
        SInt64  Duration() { return fTimerDuration; }

    private:
        Bool16  fIsStarted;
        SInt64  fTimerDuration; 
        SInt64  fStartedAt; 
};

class DssDurationTimer {

    public:
        DssDurationTimer() { fStartedAtMsec = OS::Milliseconds(); }
        void Reset() { fStartedAtMsec = OS::Milliseconds(); }
        void ResetToDuration( SInt64 inDurationInMsec ) { fStartedAtMsec = OS::Milliseconds() - inDurationInMsec; }
        SInt64 DurationInMilliseconds() { return OS::Milliseconds() - fStartedAtMsec; }
        SInt64 DurationInSeconds() { return (OS::Milliseconds() - fStartedAtMsec) / (SInt64)1000; }
        

    private:
        SInt64  fStartedAtMsec; 
};


#endif

