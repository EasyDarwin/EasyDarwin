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

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>

#include "playlist_SimpleParse.h"


SimpleString::SimpleString(char *theString)
{   
    fTheString = theString;
    if (theString == NULL)
        fLen = 0;
    else
        fLen = strlen(theString);
}

void SimpleString::Init()
{
    fTheString = NULL;
    fLen = 0;

}

void SimpleString::SetString(char *theString, SInt32 len)
{   
    fTheString = theString;
    fLen = len;
}

SInt32 SimpleString::GetString(char *theString, SInt32 len)
{
    SInt32 copyLen = fLen + 1;
    if (len < copyLen ) copyLen = len;
    if (copyLen > 0)
    {   memcpy(theString,fTheString,copyLen);
        theString[copyLen -1] = 0;
    }
    
    return copyLen;
}

SInt32 SimpleString::GetInt()
{
    SInt32 result = 0;

    if (fTheString == NULL|| fLen == 0 || fLen > 32)
        return result;

    char* buff = new char[fLen +1];
    memcpy (buff, fTheString,fLen);
    buff[fLen] = 0;

    result = strtol(buff, NULL, 0);
    delete [] buff;
   
    return result;
}

void SimpleString::Print()
{
    char* buff = new char[fLen +1];
    memcpy (buff, fTheString,fLen);
    buff[fLen] = 0;
    printf("SimpleString( len=%"_S32BITARG_" str=>%s< )\n",fLen,buff);
    delete [] buff;
}


char SimpleParser::sWordDelimeters[] = "=:/\t \r\n";
char SimpleParser::sLineDelimeters[] = "\r\n";

bool SimpleParser::Compare(SimpleString *str1Ptr, SimpleString *str2Ptr, bool caseSensitive)
{
	bool result = false;
	
	do 
	{	
		if (NULL == str1Ptr) break;
		if (NULL == str2Ptr) break;
	
		if (NULL == str1Ptr->fTheString) break;
		if (NULL == str2Ptr->fTheString) break;

		if (str1Ptr->fLen != str2Ptr->fLen)
			break;
		
        int test = 0;
        if (caseSensitive)
            test = strncmp(str1Ptr->fTheString, str2Ptr->fTheString,str1Ptr->fLen);
        else
#if __Win32__

           test = _strnicmp(str1Ptr->fTheString, str2Ptr->fTheString,str1Ptr->fLen);
#else

           test = strncasecmp(str1Ptr->fTheString, str2Ptr->fTheString,str1Ptr->fLen);

#endif


		if (test != 0) break;
		
		result = true;
	} while (false);
	
	
	return result;
}

bool SimpleParser::FindString( SimpleString *sourcePtr,  SimpleString *findPtr, SimpleString *resultStringPtr)
{
    bool result = false;
    
    do
    {
        if (NULL == sourcePtr) break;
        if (NULL == findPtr) break;
                
        if (NULL == sourcePtr->fTheString) break;
        if (NULL == findPtr->fTheString) break;

        if (findPtr->fLen > sourcePtr->fLen)
            break;
                    
        char *start = strstr(sourcePtr->fTheString, findPtr->fTheString);       
        if (start == NULL) break;
            
        if (NULL != resultStringPtr)
        {   
            SInt32 len = (PointerSizedInt) start - (PointerSizedInt) sourcePtr->fTheString;
            if (len > sourcePtr->fLen) break;
            
            resultStringPtr->SetString(start, findPtr->fLen);
        }
        
        result = true;
    } while (false);

    return result;
}

bool SimpleParser::FindNextString( SimpleString *sourcePtr,  SimpleString *currentPtr,  SimpleString *findPtr, SimpleString *resultStringPtr)
{
    bool result = false;
    SInt32 length = 0;

    
    do 
    {
        if (NULL == sourcePtr) break;
        if (NULL == currentPtr) break;
        if (NULL == findPtr) break;
        if (NULL == sourcePtr->fTheString) break;
        if (NULL == currentPtr->fTheString) break;
        
        SimpleString tempSource(NULL);  
        

        length =  currentPtr->fTheString - sourcePtr->fTheString;
        if (length < 0) break;
        if (length > sourcePtr->fLen) break;

        length = sourcePtr->fLen - (length + currentPtr->fLen); // the remaining length to search
        tempSource.SetString(&currentPtr->fTheString[currentPtr->fLen], length); // step past the end of current with remaining length
        
        result = FindString(&tempSource, findPtr, resultStringPtr);
    
    } while (false);

    return result;

}


bool SimpleParser::GetString( SimpleString *sourcePtr,  SimpleString *findPtr, SimpleString *resultStringPtr)
{
    bool result = false;
    
    result = FindString(sourcePtr, findPtr, resultStringPtr);
    
    if (result)
    {   resultStringPtr->fLen = (PointerSizedInt) resultStringPtr->fTheString -(PointerSizedInt) sourcePtr->fTheString;
        resultStringPtr->fTheString = sourcePtr->fTheString;
    }
    
    return result;
}

bool SimpleParser::FindDelimeter( SimpleString *sourcePtr, char *findChars, SimpleString *resultStringPtr)
{
    bool result = false;
    
    do
    {
        if (NULL == sourcePtr) break;
        if (NULL == findChars) break;
        if (NULL == resultStringPtr) break;
                    
        SInt32  charCount = 0;
        char*   charOffset = sourcePtr->fTheString;
        char*   foundChar = NULL;
        
        if ( (NULL == sourcePtr->fTheString) || (0 == sourcePtr->fLen) )
        {   //qtss_printf("NULL string in FindDelimeter \n");
            break;
        }
        
        // skip past any delimeters
        while ( (*charOffset != 0) && (charCount <= sourcePtr->fLen) )  
        {   foundChar = strchr(findChars, *charOffset);
            if (NULL == foundChar) break; // found non delimeter char
            charOffset ++; charCount ++;
        }
        
        char *theChar = charOffset; // start past delimeters
        
        while ( (*theChar != 0) && (charCount <= sourcePtr->fLen) )  
        {
            foundChar = strchr(findChars, *theChar);
            if (NULL != foundChar) break; // found delimeter
            charCount++;
            theChar++;
        }           
        if (NULL == foundChar) break; // we didn't find a delimeter;
            
        
        if (NULL != resultStringPtr)
        {   UInt32 theLen = ((PointerSizedInt) theChar - (PointerSizedInt)charOffset); 
            resultStringPtr->SetString(charOffset, theLen); // start is charOffset
            if (theLen == 0) break;
        }
    
        result = true;
    } while (false);
    
    if (!result) 
        resultStringPtr->SetString(NULL, 0); // start is charOffset

    return result;
}


int SimpleParser::CountDelimeters( SimpleString *sourcePtr, char *delimeters)
{
    short count = 0;
    if (sourcePtr && delimeters)
    {
        SimpleString currentString = *sourcePtr;
        currentString.fLen = 0;
        
        while (GetNextThing(sourcePtr,&currentString, delimeters, &currentString))
        {   count ++;
        } 
    }
    return count;
}


        
bool SimpleParser::GetWord( SimpleString *sourcePtr, SimpleString *resultStringPtr)
{
    bool result = false;
    char delimeter[] = " :/"; // space or colon or /
    result = FindDelimeter(sourcePtr, delimeter,resultStringPtr);
    return result;
}

bool SimpleParser::GetLine(SimpleString *sourcePtr, SimpleString *resultStringPtr)
{
    bool result = false;
    char delimeter[] = "\r\n"; 
    result = FindDelimeter(sourcePtr, delimeter,resultStringPtr);
    return result;
}

bool SimpleParser::GetNextThing(SimpleString *sourcePtr, SimpleString *currentPtr, char *findChars, SimpleString *resultStringPtr)
{
    bool result = false;

    do 
    {
        if (NULL == sourcePtr) break;
        if (NULL == currentPtr) break;
        if (NULL == findChars) break;
        
        if ( (PointerSizedInt) currentPtr->fTheString < (PointerSizedInt) sourcePtr->fTheString) 
            break;
            
        if (NULL ==  sourcePtr->fTheString) 
        {   //qtss_printf("NULL sourcePtr->fTheString in GetNextThing \n");
            break;
        }
        if (NULL ==  currentPtr->fTheString) 
        {   //qtss_printf("NULL currentPtr->fTheString in GetNextThing \n");
            break;
        }
        
        PointerSizedInt endSource =  (PointerSizedInt) &sourcePtr->fTheString[sourcePtr->fLen];
        PointerSizedInt endCurrent = (PointerSizedInt) &currentPtr->fTheString[currentPtr->fLen];

        if (endCurrent > endSource) break;
        
        SimpleString tempSource(NULL);  
        
        
        UInt32 searchLen = endSource - endCurrent;
        
        tempSource.SetString(&currentPtr->fTheString[currentPtr->fLen], searchLen); // step past the end of current with remaining length
        
        result = FindDelimeter(&tempSource, findChars,resultStringPtr);
    
    } while (false);

    return result;
}

bool SimpleParser::GetNextWord( SimpleString *sourcePtr, SimpleString *currentWord, SimpleString *resultStringPtr)
{
    bool result = false;
    char *findChars = sWordDelimeters;// 
    result = GetNextThing(sourcePtr,currentWord, findChars, resultStringPtr);
    return result;
}

bool SimpleParser::GetNextLine( SimpleString *sourcePtr, SimpleString *currentLine, SimpleString *resultStringPtr)
{
    bool result = false;
    char *findChars = sLineDelimeters;//"\r\n"; 
    result = GetNextThing(sourcePtr,currentLine, findChars, resultStringPtr);
    return result;
}
