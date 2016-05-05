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
/* revision.h -- define the version number
 
*/

// Use no http/rtsp tspecial chars in kVersionString and kBuildString defines
// tspecials =  ()<>@,;:\/"[]?=

#define kVersionString "1.0.1"
#define kBuildString "16.0515"

// Use kCommentString for seed or other release info 
// Do not use '(' or ')' in the kCommentString
// form = token1/info; token2/info;
// example "Release/public seed 1; Event/Big Event; state/half-baked"

#define kCommentString "Release/EasyCamera; State/Development; " 

