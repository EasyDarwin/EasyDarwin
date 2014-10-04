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
    File:       StringTranslator.cpp

    Contains:   implements StringTranslator class
                    
    
*/


#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "StringTranslator.h"
#include "MyAssert.h"
#include "SafeStdLib.h"
#include <errno.h>

SInt32 StringTranslator::DecodeURL(const char* inSrc, SInt32 inSrcLen, char* ioDest, SInt32 inDestLen)
{
    // return the number of chars written to ioDest
    // or OS_BadURLFormat in the case of any error.
    
    // inSrcLen must be > inSrcLen and the first character must be a '/'
    if ( inSrcLen <= 0  || *inSrc != '/' )
        return OS_BadURLFormat;
        
    //Assert(*inSrc == '/'); //For the purposes of '..' stripping, we assume first char is a /
    
    SInt32 theLengthWritten = 0;
    int tempChar = 0;
    int numDotChars = 0;
    Bool16 inQuery = false; 
    
    while (inSrcLen > 0)
    {
        if (theLengthWritten == inDestLen)
            return OS_NotEnoughSpace;
            
        if (*inSrc == '?')
            inQuery = true;

        if (*inSrc == '%')
        {
            if (inSrcLen < 3)
                return OS_BadURLFormat;

            //if there is a special character in this URL, extract it
            char tempbuff[3];
            inSrc++;
            if (!isxdigit(*inSrc))
                return OS_BadURLFormat;
            tempbuff[0] = *inSrc;
            inSrc++;
            if (!isxdigit(*inSrc))
                return OS_BadURLFormat;
            tempbuff[1] = *inSrc;
            inSrc++;
            tempbuff[2] = '\0';
            sscanf(tempbuff, "%x", &tempChar);
            Assert(tempChar < 256);
            inSrcLen -= 3;      
        }
        else if (*inSrc == '\0')
            return OS_BadURLFormat;
        else
        {
            // Any normal character just gets copied into the destination buffer
            tempChar = *inSrc;
            inSrcLen--;
            inSrc++;
        }
        
        if (!inQuery)       // don't do seperator parsing or .. parsing in query
        {
            //
            // If we are in a file system that uses a character besides '/' as a
            // path delimiter, we should not allow this character to appear in the URL.
            // In URLs, only '/' has control meaning.
            if ((tempChar == kPathDelimiterChar) && (kPathDelimiterChar != '/'))
                return OS_BadURLFormat;
            
            // Check to see if this character is a path delimiter ('/')
            // If so, we need to further check whether backup is required due to
            // dot chars that need to be stripped
            if ((tempChar == '/') && (numDotChars <= 2) && (numDotChars > 0))
            {
                Assert(theLengthWritten > numDotChars);
                ioDest -= (numDotChars + 1);
                theLengthWritten -= (numDotChars + 1);
            }

            *ioDest = tempChar;
            
            // Note that because we are doing this dotchar check here, we catch dotchars
            // even if they were encoded to begin with.
            
            // If this is a . , check to see if it's one of those cases where we need to track
            // how many '.'s in a row we've gotten, for stripping out later on
            if (*ioDest == '.')
            {
                Assert(theLengthWritten > 0);//first char is always '/', right?
                if ((numDotChars == 0) && (*(ioDest - 1) == '/'))
                    numDotChars++;
                else if ((numDotChars > 0) && (*(ioDest - 1) == '.'))
                    numDotChars++;
            }
            // If this isn't a dot char, we don't care at all, reset this value to 0.
            else
                numDotChars = 0;
        }
        else
            *ioDest = tempChar;

        theLengthWritten++;
        ioDest++;
    }
    
    // Before returning, "strip" any trailing "." or ".." by adjusting "theLengthWritten
    // accordingly
    if (numDotChars <= 2)
        theLengthWritten -= numDotChars;
    return theLengthWritten;
}

SInt32 StringTranslator::EncodeURL(const char* inSrc, SInt32 inSrcLen, char* ioDest, SInt32 inDestLen)
{
    // return the number of chars written to ioDest
    
    SInt32 theLengthWritten = 0;
    
    while (inSrcLen > 0)
    {
        if (theLengthWritten == inDestLen)
            return OS_NotEnoughSpace;
            
        //
        // Always encode 8-bit characters
        if ((unsigned char)*inSrc > 127)
        {
            if (inDestLen - theLengthWritten < 3)
                return OS_NotEnoughSpace;
                
            qtss_sprintf(ioDest,"%%%X",(unsigned char)*inSrc);
            ioDest += 3;
            theLengthWritten += 3;
                        inSrc++;
                        inSrcLen--;
            continue;
        }
        
        //
        // Only encode certain 7-bit characters
        switch (*inSrc)
        {
            // This is the URL RFC list of illegal characters.
            case (' '):
            case ('\r'):
            case ('\n'):
            case ('\t'):
            case ('<'):
            case ('>'):
            case ('#'):
            case ('%'):
            case ('{'):
            case ('}'):
            case ('|'):
            case ('\\'):
            case ('^'):
            case ('~'):
            case ('['):
            case (']'):
            case ('`'):
            case (';'):
//          case ('/'):     // this isn't really an illegal character, it's legitimatly used as a seperator in the url
            case ('?'):
            case ('@'):
            case ('='):
            case ('&'):
            case ('$'):
            case ('"'):
            {
                if ((inDestLen - theLengthWritten) < 3)
                    return OS_NotEnoughSpace;
                    
                qtss_sprintf(ioDest,"%%%X",(int)*inSrc);
                ioDest += 3;
                theLengthWritten += 3;
                break;
            }
            default:
            {
                *ioDest = *inSrc;
                ioDest++;
                theLengthWritten++;
            }
        }
        
        inSrc++;
        inSrcLen--;
    }
    
    return theLengthWritten;
}

void        StringTranslator::DecodePath(char* inSrc, UInt32 inSrcLen)
{
    for (UInt32 x = 0; x < inSrcLen; x++)
        if (inSrc[x] == '/')
            inSrc[x] = kPathDelimiterChar;
}



#if STRINGTRANSLATORTESTING
Bool16 StringTranslator::Test()
{
    //static char* test1 = "/%5D%3f%7eAveryweird%7C/and/long/path/ya/%5d%3F%7eAveryweird%7C/and/long/p%40/ya/%5D%3F%7EAveryweird%7C/and/long/path/ya/%5D%3F%7EAveryweird%7C/and/long/path/ya/%2560%2526a%20strange%3B%23%3D%25filename"
    static char dest[1000];
    static char* test1 = "/Hello%23%20 I want%28don't%29";
    SInt32 err = DecodeURL(test1, strlen(test1), dest, 1000);
    if (err != 22)
        return false;
    if (strcmp(dest, "/Hello#  I want(don't)") != 0)
        return false;
    err = DecodeURL(test1, 15, dest, 1000);
    if (err != 11)
        return false;
    if (strncmp(dest, "/Hello#  I ", 11) != 0)
        return false;
    err = DecodeURL(test1, 50, dest, 1000);
    if (err != OS_BadURLFormat)
        return false;
    if (strncmp(dest, "/Hello#  I want(don't)", 22) != 0)
    if (strcmp(dest, "/Hello#  I want(don't)") != 0)
        return false;
        
    err = DecodeURL(test1, strlen(test1), dest, 20);
    if (err != OS_BadURLFormat)
        return false;
    static char* test2 = "/THis%2h is a bad %28 URL!";
    err = DecodeURL(test2, strlen(test2), dest, 1000);
    if (err != OS_BadURLFormat)
        return false;
        
    static char* test3 = "/...whoa/../is./meeee%3e/./";
    static char* test4 = "/I want/to/sleep/..";
    static char* test5 = "/ve....rr/tire.././../..";
    static char* test6 = "/../beginnings/and/.";
    static char* test7 = "/../begin/%2e./../nin/%2e/gs/an/%2e%2e/fklf/%2e%2e./dfds%2e/%2e%2e/d/.%2e";
    err = DecodeURL(test3, strlen(test3), dest, 1000);
    err = DecodeURL(test4, strlen(test4), dest, 1000);
    err = DecodeURL(test5, strlen(test5), dest, 1000);
    err = DecodeURL(test6, strlen(test6), dest, 1000);
    err = DecodeURL(test7, strlen(test7), dest, 1000);
    return true;
}
#endif  
