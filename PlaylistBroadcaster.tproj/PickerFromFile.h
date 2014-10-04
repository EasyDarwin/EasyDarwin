#ifndef __picker_from_file__
#define __picker_from_file__

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



#include "PlaylistPicker.h"
#include "PLDoubleLinkedList.h"
#include <string.h>

class LoopDetectionListElement {

    public:
        LoopDetectionListElement( const char * name )
        {
            mPathName = new char[ strlen(name) + 1 ];
            
            Assert( mPathName );
            if( mPathName )
                ::strcpy( mPathName, name );
            
        }       
        
        virtual ~LoopDetectionListElement() 
        { 
            if ( mPathName )  
                delete [] mPathName;
        }
        
        char    *mPathName;

};


typedef PLDoubleLinkedList<LoopDetectionListElement> LoopDetectionList;
typedef PLDoubleLinkedListNode<LoopDetectionListElement> LoopDetectionNode;

enum PickerPopulationErrors {

    kPickerPopulateLoopDetected = 1000
    , kPickerPopulateBadFormat
    , kPickerPopulateFileError
    , kPickerPopulateNoMem
    
    , kPickerPopulateNoErr = 0

};

int PopulatePickerFromFile( PlaylistPicker* picker, char* fname, const char* basePath, LoopDetectionList *ldList );
int PopulatePickerFromDir( PlaylistPicker* picker, char* dirPath, int weight = 10 );


#endif
