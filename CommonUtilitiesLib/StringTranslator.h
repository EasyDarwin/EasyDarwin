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
    Contains:   Static utilities for translating strings from one encoding scheme to
                another. For example, routines for encoding and decoding URLs
                

    
*/

#ifndef __STRINGTRANSLATOR_H__
#define __STRINGTRANSLATOR_H__

#include "OSHeaders.h"

#define STRINGTRANSLATORTESTING 0

class StringTranslator
{
    public:
    
        //DecodeURL:
        //
        // This function does 2 things: Decodes % encoded characters in URLs, and strips out
        // any ".." or "." complete filenames from the URL. Writes the result into ioDest.
        //
        //If successful, returns the length of the destination string.
        //If failure, returns an OS errorcode: OS_BadURLFormat, OS_NotEnoughSpace

        static SInt32   DecodeURL(const char* inSrc, SInt32 inSrcLen, char* ioDest, SInt32 inDestLen);

        //EncodeURL:
        //
        // This function takes a character string and % encodes any special URL characters.
        // In general, the output buffer will be longer than the input buffer, so caller should
        // be aware of that.
        //
        //If successful, returns the length of the destination string.
        //If failure, returns an QTSS errorcode: OS_NotEnoughSpace
        //
        // If function returns E2BIG, ioDest will be valid, but will contain
        // only the portion of the URL that fit.
        static SInt32   EncodeURL(const char* inSrc, SInt32 inSrcLen, char* ioDest, SInt32 inDestLen);
        
        // DecodePath:
        //
        // This function converts "network" or "URL" path delimiters (the '/' char) to
        // the path delimiter of the local file system. It does this conversion in place,
        // so the old data will be overwritten
        static void     DecodePath(char* inSrc, UInt32 inSrcLen);
        
#if STRINGTRANSLATORTESTING
        static Bool16       Test();
#endif  
};
#endif // __STRINGTRANSLATOR_H__

