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
    File:       PrefsSource.h

    Contains:   Defines an abstract common interface for extracting prefs
                from some data source. Very general, low level

    Written by: Denis Serenyi

    Change History (most recent first):


    

*/

#ifndef __PREFSSOURCE_H__
#define __PREFSSOURCE_H__

#include "OSHeaders.h"

class PrefsSource
{
    public:
    
        virtual int     GetValue(const char* inKey, char* ioValue) = 0;
        virtual int     GetValueByIndex(const char* inKey, UInt32 inIndex, char* ioValue) = 0;
		virtual ~PrefsSource(){};
};

#endif
