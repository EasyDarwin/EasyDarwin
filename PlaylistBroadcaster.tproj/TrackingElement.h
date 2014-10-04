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
#ifndef __tracking_element__
#define __tracking_element__

#include <unistd.h>
#include <string.h>
#include "OSHeaders.h"
#include "MyAssert.h"

class TrackingElement {

    public:
        TrackingElement( pid_t pid, const char * name )
        {
            mName = new char[ strlen(name) + 1 ];
            
            Assert( mName );
            if( mName )
                strcpy( mName, name );
            
            mPID = pid;
            
        }       
        
        virtual ~TrackingElement() 
        { 
            if ( mName )  
                delete [] mName;
        }
        
        char    *mName;
        pid_t   mPID;

};

#endif
