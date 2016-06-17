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
    File:       QTSSExpirationDate.cpp

    Contains:   Implementation of class defined in QTSSExpirationDate.h

    Written by: Denis Serenyi

    Copyright:  ?1998 by Apple Computer, Inc., all rights reserved.


    

*/

#include "QTSSExpirationDate.h"

#include "MyAssert.h"
#include "OSHeaders.h"
#include "SafeStdLib.h"
#include <time.h>


Bool16  QTSSExpirationDate::sIsExpirationEnabled = false;
//must be in "5/12/1998" format, "m/d/4digityear"
char*   QTSSExpirationDate::sExpirationDate = "3/15/2002";

void QTSSExpirationDate::PrintExpirationDate()
{
    if (sIsExpirationEnabled)
        qtss_printf("Software expires on: %s\n", sExpirationDate);
}

void QTSSExpirationDate::sPrintExpirationDate(char* ioExpireMessage)
{
    if (sIsExpirationEnabled)
        qtss_sprintf(ioExpireMessage, "Software expires on: %s\n", sExpirationDate);
}


Bool16 QTSSExpirationDate::IsSoftwareExpired()
{
    if (!sIsExpirationEnabled)
        return false;
        
    SInt32 expMonth, expDay, expYear;
    if (EOF == ::sscanf(sExpirationDate, "%" _S32BITARG_ "/%" _S32BITARG_ "/%" _S32BITARG_ "", &expMonth, &expDay, &expYear))
    {
        Assert(false);
        return true;
    }
    
    //sanity checks
    Assert((expMonth > 0) && (expMonth <= 12));
    if ((expMonth <= 0) || (expMonth > 12))
        return true;
    
    Assert((expDay > 0) && (expDay <= 31));
    if ((expDay <= 0) || (expDay > 31))
        return true;
        
    Assert(expYear >= 1998);
    if (expYear < 1998)
        return true;
    
    time_t theCurrentTime = ::time(NULL);
    Assert(theCurrentTime != -1);
    if (theCurrentTime == -1)
        return true;
        
    struct tm  timeResult;
    struct tm* theLocalTime = qtss_localtime(&theCurrentTime, &timeResult);
    Assert(theLocalTime != NULL);
    if (theLocalTime == NULL)
        return true;
        
    if (expYear > (theLocalTime->tm_year + 1900))
        return false;//ok
    if (expYear < (theLocalTime->tm_year + 1900))
        return true;//expired

    if (expMonth > (theLocalTime->tm_mon + 1))
        return false;//ok
    if (expMonth < (theLocalTime->tm_mon + 1))
        return true;//expired

    if (expDay > theLocalTime->tm_mday)
        return false;//ok
    else
        return true;//expired
}

