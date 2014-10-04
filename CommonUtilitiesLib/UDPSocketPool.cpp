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
    File:       UDPSocketPool.cpp

    Contains:   Object that creates & maintains UDP socket pairs in a pool.

    
*/

#include "UDPSocketPool.h"

UDPSocketPair* UDPSocketPool::GetUDPSocketPair(UInt32 inIPAddr, UInt16 inPort,
                                                UInt32 inSrcIPAddr, UInt16 inSrcPort)
{
    OSMutexLocker locker(&fMutex);
    if ((inSrcIPAddr != 0) || (inSrcPort != 0))
    {
        for (OSQueueIter qIter(&fUDPQueue); !qIter.IsDone(); qIter.Next())
        {
            //If we find a pair that is a) on the right IP address, and b) doesn't
            //have this source IP & port in the demuxer already, we can return this pair
            UDPSocketPair* theElem = (UDPSocketPair*)qIter.GetCurrent()->GetEnclosingObject();
            if ((theElem->fSocketA->GetLocalAddr() == inIPAddr) &&
                ((inPort == 0) || (theElem->fSocketA->GetLocalPort() == inPort)))
            {
                //check to make sure this source IP & port is not already in the demuxer.
                //If not, we can return this socket pair.
                if ((theElem->fSocketB->GetDemuxer() == NULL) ||
                    ((!theElem->fSocketB->GetDemuxer()->AddrInMap(0, 0)) &&
                    (!theElem->fSocketB->GetDemuxer()->AddrInMap(inSrcIPAddr, inSrcPort))))
                {
                    theElem->fRefCount++;
                    return theElem;
                }
                //If port is specified, there is NO WAY a socket pair can exist that matches
                //the criteria (because caller wants a specific ip & port combination)
                else if (inPort != 0)
                    return NULL;
            }
        }
    }
    //if we get here, there is no pair already in the pool that matches the specified
    //criteria, so we have to create a new pair.
    return this->CreateUDPSocketPair(inIPAddr, inPort);
}

void UDPSocketPool::ReleaseUDPSocketPair(UDPSocketPair* inPair)
{
    OSMutexLocker locker(&fMutex);
    inPair->fRefCount--;
    if (inPair->fRefCount == 0)
    {
        fUDPQueue.Remove(&inPair->fElem);
        this->DestructUDPSocketPair(inPair);
    }
}

UDPSocketPair*  UDPSocketPool::CreateUDPSocketPair(UInt32 inAddr, UInt16 inPort)
{
   //try to find an open pair of ports to bind these suckers tooo
    OSMutexLocker locker(&fMutex);
    UDPSocketPair* theElem = NULL;
    Bool16 foundPair = false;
    UInt16 curPort = kLowestUDPPort;
    UInt16 stopPort = kHighestUDPPort -1; // prevent roll over when iterating over port nums
    UInt16 socketBPort = kLowestUDPPort + 1;
    
    //If port is 0, then the caller doesn't care what port # we bind this socket to.
    //Otherwise, ONLY attempt to bind this socket to the specified port
     if (inPort != 0)
        curPort = inPort;
     if (inPort != 0)
        stopPort = inPort;
        

    while ((!foundPair) && (curPort < kHighestUDPPort))
    {
        socketBPort = curPort +1; // make socket pairs adjacent to one another
        
        theElem = ConstructUDPSocketPair();
        Assert(theElem != NULL);
        if (theElem->fSocketA->Open() != OS_NoErr)
        {
            this->DestructUDPSocketPair(theElem);
            return NULL;
        }
        if (theElem->fSocketB->Open() != OS_NoErr)
        {
            this->DestructUDPSocketPair(theElem);
            return NULL;
        }
            
        // Set socket options on these new sockets
        this->SetUDPSocketOptions(theElem);
        
        OS_Error theErr = theElem->fSocketA->Bind(inAddr, curPort);
        if (theErr == OS_NoErr)
        {   //qtss_printf("fSocketA->Bind ok on port%u\n", curPort);
            theErr = theElem->fSocketB->Bind(inAddr, socketBPort);
            if (theErr == OS_NoErr)
            {   //qtss_printf("fSocketB->Bind ok on port%u\n", socketBPort);
                foundPair = true;
                fUDPQueue.EnQueue(&theElem->fElem);
                theElem->fRefCount++;
                return theElem;
            }
            //else qtss_printf("fSocketB->Bind failed on port%u\n", socketBPort);
         }
         //else qtss_printf("fSocketA->Bind failed on port%u\n", curPort);
 
        //If we are looking to bind to a specific port set, and we couldn't then
        //just break here.
        if (inPort != 0)
            break;
            
        if (curPort >= stopPort) //test for stop condition
            break;
            
        curPort += 2; //try a higher port pair
        
        this->DestructUDPSocketPair(theElem); //a bind failure
        theElem = NULL;
    }
    //if we couldn't find a pair of sockets, make sure to clean up our mess
    if (theElem != NULL)
        this->DestructUDPSocketPair(theElem); 
       
    return NULL;
}
