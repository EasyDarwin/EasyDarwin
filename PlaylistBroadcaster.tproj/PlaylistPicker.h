#ifndef __playlist_picker__
#define __playlist_picker__

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


#include "PLDoubleLinkedList.h"
#include "SimplePlayListElement.h"

#include "OSHeaders.h"

#include "NoRepeat.h"

class PlaylistPicker
{

    public:
        enum { kMaxBuckets = 10 };
        
        PlaylistPicker(bool doLoop);
        PlaylistPicker( UInt32 numBuckets, UInt32 numNoRepeats );
        virtual ~PlaylistPicker();
        void        CleanList(); 
        bool        AddToList( const char *name, int weight );
        bool        AddNode( PLDoubleLinkedListNode<SimplePlayListElement> *node );
        char*       PickOne();
        char*       LastPick() { return fLastPick; }
        UInt32      GetNumBuckets() { return mBuckets; }
/* changed by emil@popwire.com (see relaod.txt for info) */
        UInt32      GetNumMovies() { return mNumToPickFrom; }

        bool        mRemoveFlag;
        bool        mStopFlag;
/* ***************************************************** */
        SInt32*       mPickCounts;
        SInt32        mNumToPickFrom;
        UInt32      mRecentMoviesListSize;
        char*       fLastPick;
        PLDoubleLinkedList<SimplePlayListElement>*  GetBucket( UInt32 myIndex ) { return mElementLists[myIndex]; }
        
        char*       GetFirstFile() { return mFirstElement; }

    protected:
        
        bool        mIsSequentialPicker;    // picker picks sequentially?
        bool        mIsSequentialLooping;   // loop over and over?
        int         mWhichSequentialBucket; // sequential picker picks  from list0 or list1?
        
        UInt32      Random();
        UInt32      mLastResult;
        
        char*       PickFromList( PLDoubleLinkedList<SimplePlayListElement>* list, UInt32 elementIndex );
        
        PLDoubleLinkedList<SimplePlayListElement>* mElementLists[kMaxBuckets];
        
        UInt32                                  mBuckets;
        NoRepeat                                *mUsedElements;
        char*       mFirstElement;
};



#endif
