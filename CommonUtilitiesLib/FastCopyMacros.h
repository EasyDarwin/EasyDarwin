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
#ifndef __FastCopyMacros__
#define __FastCopyMacros__

#define     COPY_BYTE( dest, src ) ( *((char*)(dest)) = *((char*)(src)) )
#define     COPY_WORD( dest, src ) ( *((SInt16*)(dest)) =  *((SInt16*)(src)) )
#define     COPY_LONG_WORD( dest, src ) ( *((SInt32*)(dest)) =  *((SInt32*)(src)) )
#define     COPY_LONG_LONG_WORD( dest, src ) ( *((SInt64*)(dest)) =  *((SInt64**)(src)) )

#define     MOVE_BYTE( dest, src ) ( dest = *((char*)(src)) )
#define     MOVE_WORD( dest, src ) ( dest =  *((SInt16*)(src)) )
#define     MOVE_LONG_WORD( dest, src ) ( dest =  *((SInt32*)(src)) )
#define     MOVE_LONG_LONG_WORD( dest, src ) ( dest =  *((SInt64**)(src)) )


#endif

