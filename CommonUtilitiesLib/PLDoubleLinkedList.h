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
 
#ifndef __pldoublelinkedlist__
#define __pldoublelinkedlist__

#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>

#include "OSHeaders.h"
#include "MyAssert.h"

#ifndef __PLDoubleLinkedListDEBUG__
#define __PLDoubleLinkedListDEBUG__ 0
#endif

template <class T> class PLDoubleLinkedList;

template <class S> class PLDoubleLinkedListNode
{
    friend class PLDoubleLinkedList <S>;

    public:
        PLDoubleLinkedListNode( S* element )
        {
                // the node takes ownership of "element"
                fElement = element;
                fNext = NULL;
                fPrev = NULL;
        }
        virtual ~PLDoubleLinkedListNode()
                {
                    #if __PLDoubleLinkedListDEBUG__
                    Assert( fPrev == NULL && fNext == NULL );
                    #endif
                    
                    delete  fElement;
                }
        
        S* fElement;

    protected:
        PLDoubleLinkedListNode *fNext;
        PLDoubleLinkedListNode *fPrev;
};



template <class T> class PLDoubleLinkedList
{
    
    public:
        PLDoubleLinkedList()
        {
            fHead = NULL;
            fTail = NULL;
            fNumNodes = 0;
        }
        
        virtual ~PLDoubleLinkedList()
                {
                    ClearList();
                }
        
        #if __PLDoubleLinkedListDEBUG__
        
        void                    ValidateLinks()
                                {
                                    PLDoubleLinkedListNode<T> *nextNode;
                                    
                                    Assert( fHead == NULL || fHead->fPrev == NULL );
                                    Assert( fTail == NULL || fTail->fNext == NULL );
                                    
                                    
                                    if ( fTail == fHead && fTail != NULL )
                                    {
                                        Assert(  fTail->fPrev == NULL && fTail->fNext == NULL );
                                    }
                                    
                                    if ( fHead  )
                                    {
                                        Assert(  fTail != NULL  );
                                    }

                                    if ( fTail  )
                                    {
                                        Assert(  fHead != NULL  );
                                    }

                                    
                                    if ( fTail && fTail->fPrev )
                                        Assert( fTail->fPrev->fNext == fTail  );

                                    if ( fHead && fHead->fNext )
                                        Assert( fHead->fNext->fPrev == fHead  );
                                    
                                    
                                        
                                    nextNode = fHead;
                                    
                                    while ( nextNode )
                                    {   
                                        Assert( fHead == nextNode || nextNode->fPrev->fNext == nextNode );
                                        Assert( fTail == nextNode || nextNode->fNext->fPrev == nextNode );
                                        
                                        if ( !nextNode->fNext )
                                            Assert( fTail == nextNode );
                                            
                                        nextNode = nextNode->fNext;
                                    }

                                    nextNode = fTail;
                                    
                                    while ( nextNode )
                                    {   
                                        Assert( fHead == nextNode || nextNode->fPrev->fNext == nextNode );
                                        Assert( fTail == nextNode || nextNode->fNext->fPrev == nextNode );
                                        
                                        if ( !nextNode->fPrev )
                                            Assert( fHead == nextNode );
                                            
                                        nextNode = nextNode->fPrev;
                                    }
                                }
        #endif // __PLDoubleLinkedListDEBUG__
        
        PLDoubleLinkedListNode<T>   * GetFirst() { return fHead; };
        
        void                    AddNodeToTail(PLDoubleLinkedListNode<T> *node)
                                {
                                    
                                    
                                    #if __PLDoubleLinkedListDEBUG__
                                    // must not be associated with another list
                                    Assert( node->fPrev == NULL && node->fNext == NULL );
                                    #endif
                                    
                                    
                                    if ( fTail )
                                        fTail->fNext = node;

                                    
                                    node->fPrev = fTail;
                                    node->fNext = NULL;
                                    
                                    fTail = node;
                                        
                                    if ( !fHead )
                                        fHead = node;

                                    fNumNodes++;
                                    
                                    #if __PLDoubleLinkedListDEBUG__
                                    ValidateLinks();
                                    #endif
                                }       
        
        void                    AddNode(PLDoubleLinkedListNode<T> *node )
                                {
                                    #if __PLDoubleLinkedListDEBUG__
                                    // must not be associated with another list
                                    Assert( node->fPrev == NULL && node->fNext == NULL );
                                    #endif
                                    
                                    if ( fHead )
                                        fHead->fPrev = node;

                                    
                                    node->fPrev = NULL;
                                    node->fNext = fHead;
                                    
                                    fHead = node;
                                        
                                    if ( !fTail )
                                        fTail = node;

                                    fNumNodes++;
                                    
                                    #if __PLDoubleLinkedListDEBUG__
                                    ValidateLinks();
                                    #endif
                                    
                                }

        void                    RemoveNode(PLDoubleLinkedListNode<T> *node)
                                {
                                    
                                    #if __PLDoubleLinkedListDEBUG__
                                    // must be associated with this list    
                                    Assert( fHead == node || node->fPrev->fNext == node );
                                    
                                    // must be associated with this list
                                    Assert( fTail == node || node->fNext->fPrev == node );
                                    #endif
                                    
                                    
                                    if ( fHead == node)
                                        fHead = node->fNext;
                                    else
                                        node->fPrev->fNext = node->fNext;

                                    if ( fTail == node)
                                        fTail = node->fPrev;
                                    else
                                        node->fNext->fPrev = node->fPrev;

                                    
                                    node->fPrev = NULL;
                                    node->fNext = NULL;

                                    
                                    
                                    fNumNodes--;
                                    
                                    #if __PLDoubleLinkedListDEBUG__
                                    ValidateLinks();
                                    #endif
                                    
                                }
                                
        PLDoubleLinkedListNode<T>   *ForEachUntil( bool (*doFunc)( PLDoubleLinkedListNode<T> *node,  void *userData), void *userData )
                                    {
                                        PLDoubleLinkedListNode<T> *nextElement, *curElement;
                                        bool                    stopIteration = false;
                                        
                                        curElement = fHead;
                                        
                                        while ( curElement && !stopIteration )
                                        {
                                            nextElement = curElement->fNext;
                                        
                                            stopIteration = (*doFunc)( curElement, userData);

                                            if ( !stopIteration )
                                                curElement = nextElement;
                                        }

                                        return curElement;
                                    }

        void                    ForEach( void (*doFunc)( PLDoubleLinkedListNode<T> *node,  void *userData), void *userData )
                                {
                                    PLDoubleLinkedListNode<T> *nextElement, *curElement;
                                    
                                    curElement = fHead;
                                    
                                    while ( curElement )
                                    {
                                        nextElement = curElement->fNext;    
                                    
                                        (*doFunc)( curElement, userData);
                                        
                                        curElement = nextElement;
                                    }

                                }


        void                    ClearList()
                                {
                                    ForEach( DoClearList, this );
                                }


        PLDoubleLinkedListNode<T>   *GetNthNode( int nodeIndex )

                                    {
                                        return ForEachUntil( CompareIndexToZero, &nodeIndex );
                                    }
        UInt32                  GetNumNodes() { return fNumNodes; }
        
    protected:
        static bool     CompareIndexToZero( PLDoubleLinkedListNode<T> *, void * nodeIndex ) // (node, nodeIndex)
                        {
                            int     val = *(int*)nodeIndex;
                            
                            if ( val == 0  )
                                return true;
                            
                            *(int*)nodeIndex = val -1;
                            
                            return false;
                        }       
                        
        static void     DoClearList( PLDoubleLinkedListNode<T> *node, void * listPtr )
                        {
                            PLDoubleLinkedList<T> *list = (PLDoubleLinkedList<T> *)listPtr;
                            
                            list->RemoveNode( node );
                            
                            delete node;
                            
                        }
        PLDoubleLinkedListNode<T> *fHead;
        PLDoubleLinkedListNode<T> *fTail;
        UInt32                  fNumNodes;
        
    

};

#endif




