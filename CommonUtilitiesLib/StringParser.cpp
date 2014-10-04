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
    File:       StringParser.cpp

    Contains:   Implementation of StringParser class.  
                    
    
    
*/

#include "StringParser.h"

UInt8 StringParser::sNonWordMask[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0-9 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //10-19 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //20-29
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //30-39 
    1, 1, 1, 1, 1, 0, 1, 1, 1, 1, //40-49 - is a word
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //50-59
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //60-69 //stop on every character except a letter
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
    0, 1, 1, 1, 1, 0, 1, 0, 0, 0, //90-99 _ is a word
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, //120-129
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //130-139
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //140-149
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //150-159
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //160-169
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //170-179
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //180-189
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //190-199
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //200-209
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //210-219
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //220-229
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //230-239
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //240-249
    1, 1, 1, 1, 1, 1             //250-255
};

UInt8 StringParser::sWordMask[] =
{
    // Inverse of the above
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0-9 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //10-19 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //30-39 
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, //40-49 - is a word
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, //60-69 //stop on every character except a letter
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //70-79
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //80-89
    1, 0, 0, 0, 0, 1, 0, 1, 1, 1, //90-99 _ is a word
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //100-109
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //110-119
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, //120-129
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
    0, 0, 0, 0, 0, 0             //250-255
};

UInt8 StringParser::sDigitMask[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0-9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //10-19 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //30-39
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, //40-49 //stop on every character except a number
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, //50-59
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //60-69 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
    0, 0, 0, 0, 0, 0             //250-255
};

UInt8 StringParser::sEOLMask[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0-9   
    1, 0, 0, 1, 0, 0, 0, 0, 0, 0, //10-19    //'\r' & '\n' are stop conditions
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //30-39 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40-49
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //60-69  
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
    0, 0, 0, 0, 0, 0             //250-255
};

UInt8 StringParser::sWhitespaceMask[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, //0-9      // stop on '\t'
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, //10-19    // '\r', \v', '\f' & '\n'
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //20-29
    1, 1, 0, 1, 1, 1, 1, 1, 1, 1, //30-39   //  ' '
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //40-49
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //50-59
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //60-69
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //70-79
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //80-89
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //90-99
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //100-109
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //110-119
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //120-129
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //130-139
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //140-149
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //150-159
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //160-169
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //170-179
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //180-189
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //190-199
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //200-209
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //210-219
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //220-229
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //230-239
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //240-249
    1, 1, 1, 1, 1, 1             //250-255
};

UInt8 StringParser::sEOLWhitespaceMask[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //0-9     // \t is a stop
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, //10-19    //'\r' & '\n' are stop conditions
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //30-39   ' '  is a stop
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40-49
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //60-69  
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
    0, 0, 0, 0, 0, 0             //250-255
};


UInt8 StringParser::sEOLWhitespaceQueryMask[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //0-9     // \t is a stop
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, //10-19    //'\r' & '\n' are stop conditions
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //30-39   ' '  is a stop
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40-49
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, //60-69  ? is a stop
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
    0, 0, 0, 0, 0, 0             //250-255
};

void StringParser::ConsumeUntil(StrPtrLen* outString, char inStop)
{
    if (this->ParserIsEmpty(outString))
        return;

    char *originalStartGet = fStartGet;

    while ((fStartGet < fEndGet) && (*fStartGet != inStop))
        AdvanceMark();
        
    if (outString != NULL)
    {
        outString->Ptr = originalStartGet;
        outString->Len = fStartGet - originalStartGet;
    }
}

void StringParser::ConsumeUntil(StrPtrLen* outString, UInt8 *inMask)
{
    if (this->ParserIsEmpty(outString))
        return;
        
    char *originalStartGet = fStartGet;

    while ((fStartGet < fEndGet) && (!inMask[(unsigned char) (*fStartGet)]))//make sure inMask is indexed with an unsigned char
        AdvanceMark();

    if (outString != NULL)
    {
        outString->Ptr = originalStartGet;
        outString->Len = fStartGet - originalStartGet;
    }
}

void StringParser::ConsumeLength(StrPtrLen* spl, SInt32 inLength)
{
    if (this->ParserIsEmpty(spl))
        return;

    //sanity check to make sure we aren't being told to run off the end of the
    //buffer
    if ((fEndGet - fStartGet) < inLength)
        inLength = fEndGet - fStartGet;
    
    if (spl != NULL)
    {
        spl->Ptr = fStartGet;
        spl->Len = inLength;
    }
    if (inLength > 0)
    {
        for (short i=0; i<inLength; i++)
            AdvanceMark();
    }
    else
        fStartGet += inLength;  // ***may mess up line number if we back up too much
}


UInt32 StringParser::ConsumeInteger(StrPtrLen* outString)
{
    if (this->ParserIsEmpty(outString))
        return 0;

    UInt32 theValue = 0;
    char *originalStartGet = fStartGet;
    
    while ((fStartGet < fEndGet) && (*fStartGet >= '0') && (*fStartGet <= '9'))
    {
        theValue = (theValue * 10) + (*fStartGet - '0');
        AdvanceMark();
    }

    if (outString != NULL)
    {
        outString->Ptr = originalStartGet;
        outString->Len = fStartGet - originalStartGet;
    }
    return theValue;
}

Float32 StringParser::ConsumeFloat()
{
    if (this->ParserIsEmpty(NULL))
        return 0.0;

    Float32 theFloat = 0;
    while ((fStartGet < fEndGet) && (*fStartGet >= '0') && (*fStartGet <= '9'))
    {
        theFloat = (theFloat * 10) + (*fStartGet - '0');
        AdvanceMark();
    }
    if ((fStartGet < fEndGet) && (*fStartGet == '.'))
        AdvanceMark();
    Float32 multiplier = (Float32) .1;
    while ((fStartGet < fEndGet) && (*fStartGet >= '0') && (*fStartGet <= '9'))
    {
        theFloat += (multiplier * (*fStartGet - '0'));
        multiplier *= (Float32).1;

        AdvanceMark();
    }
    return theFloat;
}

Float32 StringParser::ConsumeNPT()
{
    if (this->ParserIsEmpty(NULL))
	    return 0.0;

    Float32 valArray[4] = {0, 0, 0, 0};
    Float32 divArray[4] = {1, 1, 1, 1};
    UInt32 valType = 0; // 0 == npt-sec, 1 == npt-hhmmss
    UInt32 index;
    
    for (index = 0; index < 4; index ++)
    {
        while ((fStartGet < fEndGet) && (*fStartGet >= '0') && (*fStartGet <= '9'))
        {
            valArray[index] = (valArray[index] * 10) + (*fStartGet - '0');
            divArray[index] *= 10;
            AdvanceMark();
        }
        
        if (fStartGet >= fEndGet || valType == 0 && index >= 1)
            break;
            
        if (*fStartGet == '.' && valType == 0 && index == 0)
            ;
        else if (*fStartGet == ':' && index < 2)
            valType = 1;
        else if (*fStartGet == '.' && index == 2)
            ;
        else
            break;
        AdvanceMark();
    }
    
    if (valType == 0)
        return valArray[0] + (valArray[1] / divArray[1]);
    else
        return (valArray[0] * 3600) + (valArray[1] * 60) + valArray[2] + (valArray[3] / divArray[3]);
}


Bool16  StringParser::Expect(char stopChar)
{
    if (this->ParserIsEmpty(NULL))
        return false;

    if (fStartGet >= fEndGet)
        return false;
    if(*fStartGet != stopChar)
        return false;
    else
    {
        AdvanceMark();
        return true;
    }
}
Bool16 StringParser::ExpectEOL()
{
    if (this->ParserIsEmpty(NULL))
        return false;

    //This function processes all legal forms of HTTP / RTSP eols.
    //They are: \r (alone), \n (alone), \r\n
    Bool16 retVal = false;
    if ((fStartGet < fEndGet) && ((*fStartGet == '\r') || (*fStartGet == '\n')))
    {
        retVal = true;
        AdvanceMark();
        //check for a \r\n, which is the most common EOL sequence.
        if ((fStartGet < fEndGet) && ((*(fStartGet - 1) == '\r') && (*fStartGet == '\n')))
            AdvanceMark();
    }
    return retVal;
}

void StringParser::ConsumeEOL(StrPtrLen* outString)
{
    if (this->ParserIsEmpty(outString))
        return;

	//This function processes all legal forms of HTTP / RTSP eols.
	//They are: \r (alone), \n (alone), \r\n
	char *originalStartGet = fStartGet;
	
	if ((fStartGet < fEndGet) && ((*fStartGet == '\r') || (*fStartGet == '\n')))
	{
		AdvanceMark();
		//check for a \r\n, which is the most common EOL sequence.
		if ((fStartGet < fEndGet) && ((*(fStartGet - 1) == '\r') && (*fStartGet == '\n')))
			AdvanceMark();
	}

	if (outString != NULL)
	{
		outString->Ptr = originalStartGet;
		outString->Len = fStartGet - originalStartGet;
	}
}

void StringParser::UnQuote(StrPtrLen* outString)
{
    // If a string is contained within double or single quotes 
    // then UnQuote() will remove them. - [sfu]
    
    // sanity check
    if (outString->Ptr == NULL || outString->Len < 2)
        return;
        
    // remove begining quote if it's there.
    if (outString->Ptr[0] == '"' || outString->Ptr[0] == '\'')
    {
        outString->Ptr++; outString->Len--;
    }
    // remove ending quote if it's there.
    if ( outString->Ptr[outString->Len-1] == '"' || 
         outString->Ptr[outString->Len-1] == '\'' )
    {
        outString->Len--;
    }
}

void StringParser::AdvanceMark()
{
     if (this->ParserIsEmpty(NULL))
        return;

   if ((*fStartGet == '\n') || ((*fStartGet == '\r') && (fStartGet[1] != '\n')))
    {
        // we are progressing beyond a line boundary (don't count \r\n twice)
        fCurLineNumber++;
    }
    fStartGet++;
}

#if STRINGPARSERTESTING
Bool16 StringParser::Test()
{
    static char* string1 = "RTSP 200 OK\r\nContent-Type: MeowMix\r\n\t   \n3450";
    
    StrPtrLen theString(string1, strlen(string1));
    
    StringParser victim(&theString);
    
    StrPtrLen rtsp;
    SInt32 theInt = victim.ConsumeInteger();
    if (theInt != 0)
        return false;
    victim.ConsumeWord(&rtsp);
    if ((rtsp.len != 4) && (strncmp(rtsp.Ptr, "RTSP", 4) != 0))
        return false;
        
    victim.ConsumeWhiteSpace();
    theInt = victim.ConsumeInteger();
    if (theInt != 200)
        return false;
        
    return true;
}
#endif
