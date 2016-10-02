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
 *  SVector.h
 *  
 *  An simple, non-exception safe implementation of vector
 */
 
#ifndef _SVECTOR_H_
#define _SVECTOR_H_

#include"OSHeaders.h"
#include"OSMemory.h"
 
//T must be default and copy constructable; does not have to be assignable
template<class T>
class SVector
{
	public:
		explicit SVector(UInt32 newCapacity = 0)
		:	fCapacity(0), fSize(0), fData(NULL)
		{
			reserve(newCapacity);
		}

		SVector(const SVector &rhs)
		:	fCapacity(0), fSize(0), fData(NULL)
		{
			reserve(rhs.size());
			fSize = rhs.size();
			for(UInt32 i = 0; i < rhs.size(); ++i)
				NEW(fData + i) T(rhs[i]);
		}

		~SVector()
		{
			clear();
			operator delete[](fData);
		}

		SVector &operator=(const SVector &rhs)
		{
			clear();
			reserve(rhs.size());
			fSize = rhs.size();
			for(UInt32 i = 0; i < rhs.size(); ++i)
				NEW(fData + i) T(rhs[i]);
			return *this;
		}

		T &operator[](UInt32 i) const 			{ return fData[i]; }
		T &front() const 							{ return fData[0]; }
		T &back() const 							{ return fData[fSize - 1]; }
		T *begin() const 							{ return fData; }
		T *end() const 							{ return fData + fSize; }
		
		//Returns searchEnd if target is not found; uses == for equality comparison
		UInt32 find(UInt32 searchStart, UInt32 searchEnd, const T &target)			{ return find<EqualOp>(searchStart, searchEnd, target); }
		
		//Allows you to specify an equality comparison functor
		template<class Eq>
		UInt32 find(UInt32 searchStart, UInt32 searchEnd, const T &target, Eq eq = Eq())
		{
            UInt32 i = searchStart;
            for(; i < searchEnd; ++i)
                if (eq(target, fData[i]))
                    break;
            return i;
		}

		//returns size() if the element is not found
		UInt32 find(const T &target)					{ return find<EqualOp>(target); }

		template<class Eq>
		UInt32 find(const T &target, Eq eq = Eq())      { return find(0, size(), target, eq); }

		//Doubles the capacity as needed
		void push_back(const T &newItem)
		{
			reserve(fSize + 1);
			NEW (fData + fSize) T(newItem);
			fSize++;
		}

		void pop_back() 								{ fData[--fSize].~T(); }
		
		void swap(SVector &rhs)
		{
			UInt32 tmpCapacity = fCapacity;
			UInt32 tmpSize = fSize;
			T *tmpData = fData;
			fCapacity = rhs.fCapacity;
			fSize = rhs.fSize;
			fData = rhs.fData;
			rhs.fCapacity = tmpCapacity;
			rhs.fSize = tmpSize;
			rhs.fData = tmpData;
		}
		
		void insert(UInt32 position, const T &newItem)					{ insert(position, 1, newItem); }

		void insert(UInt32 position, UInt32 count, const T &newItem)
		{
			reserve(fSize + count);
			for(UInt32 i = fSize; i > position; --i)
			{
				NEW (fData + i - 1 + count) T(fData[i - 1]);
				fData[i - 1].~T();
			}
			for(UInt32 i = position; i < position + count; ++i)
				NEW (fData + i) T(newItem);
			fSize += count;
		}

		//can accept count of 0 - which results in a NOP
		void erase(UInt32 position, UInt32 count = 1)
		{
			if(count == 0)
				return;
			for(UInt32 i = position; i < position + count; ++i)
				fData[i].~T();
			for(UInt32 i = position + count; i < fSize; ++i)
			{
				NEW (fData + i - count) T(fData[i]);
				fData[i].~T();
			}
			fSize -= count;
		}
		
		//Removes 1 element by swapping it with the last item.
		void swap_erase(UInt32 position)
		{
			fData[position].~T();
			if (position < --fSize)
			{
				NEW(fData + position) T(fData[fSize]);
				fData[fSize].~T();
			}
		}

		bool empty() const 							{ return fSize == 0; }
		UInt32 capacity() const 						{ return fCapacity; }
		UInt32 size() const 							{ return fSize; }
		
		void clear() 									{ resize(0); }

		//unlike clear(), this will free the memories
		void wipe()
		{
			this->~SVector();
			fCapacity = fSize = 0;
			fData = NULL;
		}

		//Doubles the capacity on a reallocation to preserve linear time semantics
		void reserve(UInt32 newCapacity)
		{
			if (newCapacity > fCapacity)
			{
				UInt32 targetCapacity = fCapacity == 0 ? 4 : fCapacity;
				while(targetCapacity < newCapacity)
					targetCapacity *= 2;
				reserveImpl(targetCapacity);
			}
		}

		void resize(UInt32 newSize, const T &newItem = T())
		{
			if (newSize > fSize)
			{
				reserve(newSize);
				for(UInt32 i = fSize; i < newSize; ++i)
					NEW(fData + i) T(newItem);
			}
			else if (newSize < fSize)
			{
				for(UInt32 i = newSize; i < fSize; ++i)
					fData[i].~T();
			}
			fSize = newSize;
		}

	private:
		void reserveImpl(UInt32 newCapacity)
		{
			T *newData = static_cast<T *>(operator new[](sizeof(T) * newCapacity));
			fCapacity = newCapacity;
			for(UInt32 i = 0; i < fSize; ++i)
				NEW (newData + i) T(fData[i]);
			operator delete[](fData);
			fData = newData;
		}

		UInt32 fCapacity;
		UInt32 fSize;
		T *fData;	
		
		struct EqualOp
		{
			bool operator()(const T &left, const T &right)	{ return left == right; }
		};
};

#endif //_SVECTOR_H_

