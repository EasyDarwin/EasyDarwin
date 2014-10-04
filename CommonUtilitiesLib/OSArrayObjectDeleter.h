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
    File:       OSArrayObjectDeleter.h

    Contains:   Auto object for deleting arrays.



*/

#ifndef __OS_ARRAY_OBJECT_DELETER_H__
#define __OS_ARRAY_OBJECT_DELETER_H__

#include "MyAssert.h"

template <class T>
class OSArrayObjectDeleter
{
    public:
		OSArrayObjectDeleter() : fT(NULL) {}
        OSArrayObjectDeleter(T* victim) : fT(victim)  {}
        ~OSArrayObjectDeleter() { delete [] fT; }
        
        void ClearObject() { fT = NULL; }

        void SetObject(T* victim) 
        {
            Assert(fT == NULL);
            fT = victim; 
        }
        T* GetObject() { return fT; }
        
        operator T*() { return fT; }
    
    private:
    
        T* fT;
};


template <class T>
class OSPtrDeleter
{
    public:
		OSPtrDeleter() : fT(NULL) {}
        OSPtrDeleter(T* victim) : fT(victim)  {}
        ~OSPtrDeleter() { delete fT; }
        
        void ClearObject() { fT = NULL; }

        void SetObject(T* victim) 
        {   Assert(fT == NULL);
            fT = victim; 
        }
            
    private:
    
        T* fT;
};


typedef OSArrayObjectDeleter<char*> OSCharPointerArrayDeleter;
typedef OSArrayObjectDeleter<char> OSCharArrayDeleter;

#endif //__OS_OBJECT_ARRAY_DELETER_H__
