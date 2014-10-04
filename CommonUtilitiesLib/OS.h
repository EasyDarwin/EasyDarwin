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
    File:       OS.h

    Contains:   OS utility functions. Memory allocation, time, etc.



*/

#ifndef _OS_H_
#define _OS_H_


#include "OSHeaders.h"
#include "OSMutex.h"
#include <string.h>

class OS
{
    public:
    
        //call this before calling anything else
        static void Initialize();

        static SInt32 Min(SInt32 a, SInt32 b)   { if (a < b) return a; return b; }
        
        //
        // Milliseconds always returns milliseconds since Jan 1, 1970 GMT.
        // This basically makes it the same as a POSIX time_t value, except
        // in msec, not seconds. To convert to a time_t, divide by 1000.
        static SInt64   Milliseconds();

        static SInt64   Microseconds();
        
        // Some processors (MIPS, Sparc) cannot handle non word aligned memory
        // accesses. So, we need to provide functions to safely get at non-word
        // aligned memory.
        static inline UInt32    GetUInt32FromMemory(UInt32* inP);

        //because the OS doesn't seem to have these functions
        static SInt64   HostToNetworkSInt64(SInt64 hostOrdered);
        static SInt64   NetworkToHostSInt64(SInt64 networkOrdered);
                            
		static SInt64	TimeMilli_To_Fixed64Secs(SInt64 inMilliseconds); //new CISCO provided implementation
        //disable: calculates integer value only                { return (SInt64) ( (Float64) inMilliseconds / 1000) * ((SInt64) 1 << 32 ) ; }
		static SInt64	Fixed64Secs_To_TimeMilli(SInt64 inFixed64Secs)
						{ UInt64 value = (UInt64) inFixed64Secs; return (value >> 32) * 1000 + (((value % ((UInt64) 1 << 32)) * 1000) >> 32); }
		
		//This converts the local time (from OS::Milliseconds) to NTP time.
		static SInt64	TimeMilli_To_1900Fixed64Secs(SInt64 inMilliseconds)
						{ return TimeMilli_To_Fixed64Secs(sMsecSince1900) + TimeMilli_To_Fixed64Secs(inMilliseconds); }

		static SInt64	TimeMilli_To_UnixTimeMilli(SInt64 inMilliseconds)
						{ return inMilliseconds; }

		static time_t	TimeMilli_To_UnixTimeSecs(SInt64 inMilliseconds)
						{ return (time_t)  ( (SInt64) TimeMilli_To_UnixTimeMilli(inMilliseconds) / (SInt64) 1000); }
		
		static time_t 	UnixTime_Secs(void) // Seconds since 1970
						{ return TimeMilli_To_UnixTimeSecs(Milliseconds()); }

        static time_t   Time1900Fixed64Secs_To_UnixTimeSecs(SInt64 in1900Fixed64Secs)
                        { return (time_t)( (SInt64)  ((SInt64)  ( in1900Fixed64Secs - TimeMilli_To_Fixed64Secs(sMsecSince1900) ) /  ((SInt64) 1 << 32)  ) ); }
                            
        static SInt64   Time1900Fixed64Secs_To_TimeMilli(SInt64 in1900Fixed64Secs)
                        { return   ( (SInt64) ( (Float64) ((SInt64) in1900Fixed64Secs - (SInt64) TimeMilli_To_Fixed64Secs(sMsecSince1900) ) / (Float64)  ((SInt64) 1 << 32) ) * 1000) ; }
 
        // Returns the offset in hours between local time and GMT (or UTC) time.
        static SInt32   GetGMTOffset();
                            
        //Both these functions return QTSS_NoErr, QTSS_FileExists, or POSIX errorcode
        //Makes whatever directories in this path that don't exist yet 
        static OS_Error RecursiveMakeDir(char *inPath);
        //Makes the directory at the end of this path
        static OS_Error MakeDir(char *inPath);
        
        // Discovery of how many processors are on this machine
        static UInt32   GetNumProcessors();
        
        // CPU Load
        static Float32  GetCurrentCPULoadPercent();
        
        // Mutex for StdLib calls
         static OSMutex* GetStdLibMutex()  { return &sStdLibOSMutex; }

        static SInt64   InitialMSec()       { return sInitialMsec; }
        static Float32  StartTimeMilli_Float() { return (Float32) ( (Float64) ( (SInt64) OS::Milliseconds() - (SInt64) OS::InitialMSec()) / (Float64) 1000.0 ); }
        static SInt64   StartTimeMilli_Int()      { return (OS::Milliseconds() - OS::InitialMSec()); }

		static Bool16 	ThreadSafe();

   private:
    
        static double sDivisor;
        static double sMicroDivisor;
        static SInt64 sMsecSince1900;
        static SInt64 sMsecSince1970;
        static SInt64 sInitialMsec;
        static SInt32 sMemoryErr;
        static void SetDivisor();
        static SInt64 sWrapTime;
        static SInt64 sCompareWrap;
        static SInt64 sLastTimeMilli;
        static OSMutex sStdLibOSMutex;
};

inline UInt32   OS::GetUInt32FromMemory(UInt32* inP)
{
#if ALLOW_NON_WORD_ALIGN_ACCESS
    return *inP;
#else
    char* tempPtr = (char*)inP;
    UInt32 temp = 0;
    ::memcpy(&temp, tempPtr, sizeof(UInt32));
    return temp;
#endif
}


#endif
