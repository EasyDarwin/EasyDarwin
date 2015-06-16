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
    File:       QTSSExpirationDate.h

    Contains:   Routine that checks to see if software is expired.

    Written by: Denis Serenyi

    Copyright:  © 1998 by Apple Computer, Inc., all rights reserved.


*/

#ifndef __QTSS_EXPIRATION_DATE_H__
#define __QTSS_EXPIRATION_DATE_H__

#include "OSHeaders.h"

class QTSSExpirationDate
{
    public:
    
        //checks current time vs. hard coded time constant.
        static Bool16   WillSoftwareExpire(){return sIsExpirationEnabled;}
        static Bool16   IsSoftwareExpired();
        static void PrintExpirationDate();
        static void sPrintExpirationDate(char* ioExpireMessage);
        
    private:
    
        static Bool16 sIsExpirationEnabled;
        static char*    sExpirationDate;
        
};

#endif //__QTSS_EXPIRATION_DATE_H__
