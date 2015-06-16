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
	File:       DSAccessChecker.h

	Contains:   Class definition for access checking via Open Directory

	Created By: Dan Sinema

	Created: Jan 14, 2005
*/

#ifndef _QTSSACCESSCHECKER_H_
#define _QTSSACCESSCHECKER_H_

// STL Headers
#include <cstdio>	// for struct FILE
#include <string>
#include "QTAccessFile.h"


class DSAccessChecker
{
/*
    Access check logic:
    
    If "modAccess_enabled" == "enabled,
                 
*/

public:
        static const char* kDefaultAccessFileName;

        DSAccessChecker();
        virtual ~DSAccessChecker();

        Bool16 CheckPassword(const char* inUsername, const char* inPassword);
        Bool16 CheckDigest(const char* inUsername, const char* inServerChallenge, const char* inClientResponse, const char* inMethod);


protected:
        Bool16 CheckGroupMembership(const char* inUsername, const char* inGroupName);
};
 



#endif //_QTSSACCESSCHECKER_H_
