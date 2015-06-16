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
    File:       QTSSSpamDefanseModule.h

    Contains:   Protects the server against denial-of-service attacks by
                only allowing X number of RTSP connections from a certain
                IP address

    
    

*/


#ifndef __QTSSSPAMDEFENSEMODULE_H__
#define __QTSSSPAMDEFENSEMODULE_H__

#include "QTSS.h"

extern "C"
{
    EXPORT QTSS_Error QTSSSpamDefenseModule_Main(void* inPrivateArgs);
}

#endif // __QTSSSPAMDEFENSEMODULE_H__

