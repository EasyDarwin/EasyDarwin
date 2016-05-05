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
    File:       NetInfoPrefsSource.h

    Contains:   Implements the PrefsSource interface wrt NetInfo database

    Written by: Denis Serenyi

    Change History (most recent first):

    

*/

#ifndef __NETINFOPREFSSOURCE_H__
#define __NETINFOPREFSSOURCE_H__

#include "PrefsSource.h"
#include "OSHeaders.h"

class NetInfoPrefsSource : public PrefsSource
{
    public:
    
        NetInfoPrefsSource();
        virtual ~NetInfoPrefsSource() {}
    
        virtual int     GetValue(const char* inKey, char* ioValue);
        virtual int     GetValueByIndex(const char* inKey, UInt32 inIndex, char* ioValue);


        void    SetValue(char* inKey, char* inValue);
        void    SetValueByIndex(char* inKey, char* inValue, UInt32 inIndex);
};

#endif //__NETINFOPREFSSOURCE_H__
