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
    File:       OSQueue.h

    Contains:   implements OSQueue class
                    
    
*/

#ifndef _OSQUEUE_H_
#define _OSQUEUE_H_

#ifndef __Win32__
	#include <unistd.h>
#endif

#include "MyAssert.h"
#include "OSHeaders.h"
#include "OSMutex.h"
#include "OSCond.h"
#include "OSThread.h"

#define OSQUEUETESTING 0
static int queueId = 0;
class OSQueue;

class OSQueueElem {
    public:
        OSQueueElem(void* enclosingObject = NULL) : fNext(NULL), fPrev(NULL), fQueue(NULL),
                                                    fEnclosingObject(enclosingObject) {}
        virtual ~OSQueueElem() { Assert(fQueue == NULL); }

        Bool16 IsMember(const OSQueue& queue) { return (&queue == fQueue); }
        Bool16 IsMemberOfAnyQueue()     { return fQueue != NULL; }
        void* GetEnclosingObject()  { return fEnclosingObject; }
        void SetEnclosingObject(void* obj) { fEnclosingObject = obj; }

        OSQueueElem* Next() { return fNext; }
        OSQueueElem* Prev() { return fPrev; }
        OSQueue* InQueue()  { return fQueue; }
        inline void Remove();

    private:

        OSQueueElem*    fNext;
        OSQueueElem*    fPrev;
        OSQueue *       fQueue;
        void*           fEnclosingObject;

        friend class    OSQueue;
};

class OSQueue {
    public:
        OSQueue();
        ~OSQueue() {}

        void            EnQueue(OSQueueElem* object);
        OSQueueElem*    DeQueue();

        OSQueueElem*    GetHead() { if (fLength > 0) return fSentinel.fPrev; return NULL; }
        OSQueueElem*    GetTail() { if (fLength > 0) return fSentinel.fNext; return NULL; }
        UInt32          GetLength() { return fLength; }
        
        void            Remove(OSQueueElem* object);

#if OSQUEUETESTING
        static Bool16       Test();
#endif

    protected:

        OSQueueElem     fSentinel;
        UInt32          fLength;
};

class CyclicQueue
{//keep the read and write pointer never at the same place,do not need any Mutex
private:
	#define MAX_QUEUE_ELEMS	64
	typedef struct CyclicElem_tag
	{
	       OSQueueElem *elem;
	       bool valid;
	}CyclicElem;
	
	int iread_pos;
	int iwrite_pos;
	int ivalid_elems;
	int curId;
	CyclicElem elems[MAX_QUEUE_ELEMS];
public:
	int GetLength() 
	{
		return ivalid_elems; 
	}

	
	CyclicQueue()
	{
		iread_pos = 0;
		iwrite_pos = 0;
		ivalid_elems = 0;
		
		memset(elems,0x0,sizeof(CyclicElem)*MAX_QUEUE_ELEMS);
	}
	~CyclicQueue()
	{

	}

	int  EnQueue(OSQueueElem* object)
    {
		if(ivalid_elems == MAX_QUEUE_ELEMS || object == NULL)
		{
			//EnQueue failed full
//			printf("EnQueue failed reason 1\n");
			return -1;
		}
//		printf("iwrite_pos== %d iread_pos == %d\n",iwrite_pos,iread_pos);
		if(iwrite_pos < 0 || iwrite_pos >= MAX_QUEUE_ELEMS || iwrite_pos + 1 == iread_pos)
		{
//			printf("EnQueue failed reason 2\n");		
			return -1;
		}

		elems[iwrite_pos].elem = object;
		elems[iwrite_pos].valid = true;
		iwrite_pos ++;
		if(iwrite_pos == MAX_QUEUE_ELEMS)
		{
			iwrite_pos = 0;
		}
		ivalid_elems ++;
		return 0;
//printf("insert iwrite_pos == %d ivalid_elems[%d] %d\n",iwrite_pos,ivalid_elems,curId);
	}

	OSQueueElem* DeQueue()
	{
		OSQueueElem *object = NULL;

		if(ivalid_elems == 0)
		{
			return NULL;
		}
			
		if(iread_pos < 0 || iread_pos >= MAX_QUEUE_ELEMS)
		{
			return NULL;
		}

		if(elems[iread_pos].valid == true)
		{
			object = elems[iread_pos].elem;
		}

		elems[iread_pos].valid = false;
		elems[iread_pos].elem = NULL;
		iread_pos ++;
		if(iread_pos == MAX_QUEUE_ELEMS)
		{
			iread_pos = 0;
		}
		ivalid_elems --;
//printf("got iread_pos == %d ivalid_elems[%d] %d\n",iread_pos,ivalid_elems,curId);
		return object;
	}
};

class OSQueueIter//自定义迭代器
{
    public:
        OSQueueIter(OSQueue* inQueue) : fQueueP(inQueue), fCurrentElemP(inQueue->GetHead()) {}
        OSQueueIter(OSQueue* inQueue, OSQueueElem* startElemP ) : fQueueP(inQueue)
            {
                if ( startElemP )
                {   Assert( startElemP->IsMember(*inQueue ) );
                    fCurrentElemP = startElemP;
                
                }
                else
                    fCurrentElemP = NULL;
            }
        ~OSQueueIter() {}
        
        void            Reset() { fCurrentElemP = fQueueP->GetHead(); }
        
        OSQueueElem*    GetCurrent() { return fCurrentElemP; }
        void            Next();
        
        Bool16          IsDone() { return fCurrentElemP == NULL; }
        
    private:
    
        OSQueue*        fQueueP;
        OSQueueElem*    fCurrentElemP;
};

class OSQueue_Blocking
{
    public:
        OSQueue_Blocking() {}
        ~OSQueue_Blocking() {}
        
        OSQueueElem*    DeQueueBlocking(OSThread* inCurThread, SInt32 inTimeoutInMilSecs);
        OSQueueElem*    DeQueue();//will not block
        void            EnQueue(OSQueueElem* obj);
        
        OSCond*         GetCond()   { return &fCond; }
//        OSQueue*        GetQueue()  { return &fQueue; }
  		CyclicQueue*	GetQueue()  { return &fQueue; } 	      
    private:

        OSCond              fCond;
        OSMutex             fMutex;
//        OSQueue             fQueue;
		CyclicQueue			fQueue;
};


void    OSQueueElem::Remove()
{
    if (fQueue != NULL)
        fQueue->Remove(this);
}
#endif //_OSQUEUE_H_
