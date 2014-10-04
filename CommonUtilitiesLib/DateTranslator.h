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
    File:       DateTranslator.h

    Contains:   Efficient routines & data structures for converting from
                RFC 1123 compliant date strings to local file system dates & vice versa.

    

*/

#ifndef _DATE_TRANSLATOR_H_
#define _DATE_TRANSLATOR_H_

#include <ctype.h>
#include <time.h>
#include "StrPtrLen.h"

class DateBuffer;

class DateTranslator
{
    public:

        // this updates the DateBuffer to be the current date / time.
        // If you wish to set the DateBuffer to a particular date, pass in that date.
        // Dates should be in the OS.h compliant format
        static void UpdateDateBuffer(DateBuffer* inDateBuffer, const SInt64& inDate, time_t gmtoffset = 0);
        
        //Given an HTTP/1.1 compliant date string (in one of the three 1.1 formats)
        //this returns an OS.h compliant date/time value.
        static SInt64   ParseDate(StrPtrLen* inDateString);
        
    private:

        static UInt32 ConvertCharToMonthTableIndex(int inChar)
        {
            return (UInt32)(toupper(inChar) - 'A'); // Convert to a value between 0 - 25
        }
};

class DateBuffer
{
public:

    // This class provides no protection against being accessed from multiple threads
    // simultaneously. Update & InexactUpdate rewrite the date buffer, so care should
    // be taken to protect against simultaneous access.

    DateBuffer() : fLastDateUpdate(0) { fDateBuffer[0] = 0; }
    ~DateBuffer() {}
    
    //SEE RFC 1123 for details on the date string format
    //ex: Mon, 04 Nov 1996 21:42:17 GMT
    
    //RFC 1123 date strings are always of this length
    enum
    {
        kDateBufferLen = 29
    };
    
    // Updates this date buffer to reflect the current time.
    // If a date is provided, this updates the DateBuffer to be that date.
    void Update(const SInt64& inDate)           { DateTranslator::UpdateDateBuffer(this, inDate); }
    
    // Updates this date buffer to reflect the current time, with a certain degree
    // of inexactitude (the range of error is defined by the kUpdateInterval value)
    void InexactUpdate();
    
    //returns a NULL terminated C-string always of kHTTPDateLen length.
    char *GetDateBuffer()   { return fDateBuffer; }

private:

    enum
    {
        kUpdateInterval = 60000 // Update every minute
    };

    //+1 for terminator +1 for padding
    char    fDateBuffer[kDateBufferLen + 2];
    SInt64  fLastDateUpdate;
    
    friend class DateTranslator;
};


#endif


