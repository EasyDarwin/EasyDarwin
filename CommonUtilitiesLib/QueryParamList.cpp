
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

#include "QueryParamList.h"

#include "StringParser.h"
#include "OSMemory.h"

#include <string.h>
#include <stdlib.h>
#include "SafeStdLib.h"
QueryParamList::QueryParamList( StrPtrLen* querySPL )
{
    // ctor from StrPtrLen
    fNameValueQueryParamlist = NEW PLDoubleLinkedList<QueryParamListElement>;

    this->BulidList( querySPL );    
}


QueryParamList::QueryParamList( char* queryString )
{
    // ctor from char*
    StrPtrLen       querySPL( queryString );

    fNameValueQueryParamlist = NEW PLDoubleLinkedList<QueryParamListElement>;
        
    this->BulidList( &querySPL );
}


void QueryParamList::BulidList( StrPtrLen* querySPL )
{
    // parse the string and build the name/value list from the tokens.
    // the string is a 'form' encoded query string ( see rfc - 1808 )
    
    StringParser    queryParser( querySPL );
	char *stopCharPtr = NULL;
    
    while  ( queryParser.GetDataRemaining() > 0 )
    {
        StrPtrLen       theCGIParamName;
        StrPtrLen       theCGIParamValue;
        
        queryParser.ConsumeUntil(&theCGIParamName, '=');        // leaves "=..." in cgiParser, puts item keywd in theCGIParamName
        
        //if ( queryParser.GetDataRemaining() > 1  )
		if ( queryParser.GetDataRemaining() >= 1  )//change,邵帅，20160614,对于"xxxxx="这种会陷入死循环，阻塞任务线程。
        {
            queryParser.ConsumeLength(&theCGIParamValue, 1 );   // the '='

			stopCharPtr = queryParser.GetCurrentPosition();
			if (*stopCharPtr == '"') // if quote read to next quote
			{
				queryParser.ConsumeLength(NULL, 1);
				queryParser.ConsumeUntil(&theCGIParamValue, '"');
				queryParser.ConsumeLength(NULL, 1);
				queryParser.ConsumeUntil(NULL, '&');   // our value will end by here...
			}
			else
				queryParser.ConsumeUntil(&theCGIParamValue, '&');   // our value will end by here...
            
            AddNameValuePairToList( theCGIParamName.GetAsCString(), theCGIParamValue.GetAsCString() );
            
            queryParser.ConsumeLength(&theCGIParamValue, 1 );   // the '='
            
        }
    }
}


static void  PrintNameAndValue( PLDoubleLinkedListNode<QueryParamListElement> *node,  void *userData )
{
    // used by QueryParamList::PrintAll
    QueryParamListElement*  nvPair = node->fElement;
    
    qtss_printf( "qpl: %s, name %s, val %s\n", (char*)userData, nvPair->mName, nvPair->mValue );
}


void QueryParamList::PrintAll( char *idString )
{
    // print name and value of each item in the list, print each pair preceded by "idString"
    fNameValueQueryParamlist->ForEach( PrintNameAndValue, idString );
}


static bool  CompareStrToName( PLDoubleLinkedListNode<QueryParamListElement> *node,  void *userData )
{
    /*
        make a case insenstitive comparison between "node" name and the userData
        
        used by QueryParamList::DoFindCGIValueForParam
    */
    
    QueryParamListElement*  nvPair = node->fElement;
    StrPtrLen               name( nvPair->mName );
    
    if ( name.EqualIgnoreCase( (char*)userData, strlen( (char*)userData ) )  )
        return true;
    
    return false;
}


const char *QueryParamList::DoFindCGIValueForParam( char *name )
{
    /*
        return the first value where the paramter name matches "name"
        use case insenstitive comparison
    
    */
    PLDoubleLinkedListNode<QueryParamListElement>*  node;

    node = fNameValueQueryParamlist->ForEachUntil( CompareStrToName, name );
    
    if ( node != NULL )
    {   
        QueryParamListElement*  nvPair = (QueryParamListElement*)node->fElement;
        
        return  nvPair->mValue;
    }
    
    return NULL;
    
}
    
    
void QueryParamList::AddNameValuePairToList( char* name, char* value  )
{
    // add the name/value pair to the by creating the holder struct
    // then adding that as the element in the linked list
    
    PLDoubleLinkedListNode<QueryParamListElement>*      nvNode;
    QueryParamListElement*      nvPair;
    
    this->DecodeArg( name );
    this->DecodeArg( value );
    
    nvPair = NEW  QueryParamListElement( name, value );
    
    
    // create a node to hold the pair
    nvNode = NEW PLDoubleLinkedListNode<QueryParamListElement> ( nvPair );
    
    // add it to the list
    fNameValueQueryParamlist->AddNode( nvNode );
}



void QueryParamList::DecodeArg( char *ioCodedPtr )
{
    // perform In-Place  &hex and + to space decoding of the parameter
    //  on input, ioCodedPtr mau contain encoded text, on exit ioCodedPtr will be plain text
    // on % decoding errors, the 
    
    if ( !ioCodedPtr ) 
        return;

    char*   destPtr;
    char*   curChar;
    short   lineState = kLastWasText;
    char    hexBuff[32];

    destPtr = curChar = ioCodedPtr;
    
    while( *curChar )
    {
        switch( lineState )
        {
            case kRcvHexDigitOne:
                if ( IsHex( *curChar ) )
                {   
                    hexBuff[3] = *curChar;
                    hexBuff[4] = 0;
                    
                    *destPtr++ = (char)::strtoul( hexBuff, NULL, 0 );
                }
                else
                {   // not a valid encoding
                    *destPtr++ = '%';           // put back the pct sign
                    *destPtr++ = hexBuff[2];    // put back the first digit too.
                    *destPtr++ = *curChar;      // and this one!
                    
                }
                lineState = kLastWasText;
                break;

            case kLastWasText:
                if ( *curChar == '%' )
                    lineState = kPctEscape;
                else
                {
                    if (  *curChar == '+' )
                        *destPtr++ = ' ';
                    else
                        *destPtr++ = *curChar;
                }
                break;

            case kPctEscape:
                if ( *curChar == '%' )
                {
                    *destPtr++ = '%';
                    lineState = kLastWasText;
                }
                else
                {
                    if ( IsHex( *curChar ) )
                    {   
                        hexBuff[0] = '0';
                        hexBuff[1] = 'x';
                        hexBuff[2] = *curChar;
                        lineState = kRcvHexDigitOne;
                    }
                    else
                    {   
                        *destPtr++ = '%';       // put back the pct sign
                        *destPtr++ = *curChar;  // and this one!
                        lineState = kLastWasText;
                    }
                }
                break;
                
                    
        }   
        
        curChar++;
    }

    *destPtr = *curChar;
}

Bool16 QueryParamList::IsHex( char c )
{
    // return true if char c is a valid hexidecimal digit
    // false otherwise.
    
    if ( c >= '0' && c <= '9' )
        return true;

    if ( c >= 'A' && c <= 'F' )
        return true;

    if ( c >= 'a' && c <= 'f' )
        return true;

    return false;
}


