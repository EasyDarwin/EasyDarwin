#ifndef _SYNC_DEQUE_
#define _SYNC_DEQUE_

#include <deque>
#include "OSMutex.h"

template <class T>
class SyncDeque
{
public:
	//LIFO
	void PushFront(const T& val)
	{
		OSMutexLocker lock(mtx_);
		deque_.push_front(val);
	}

	T PopFront()
	{
		mtx_.Lock();

		if (deque_.empty()) 
		{
			mtx_.Unlock();
			return NULL;
		}

		T val = deque_.front();
		deque_.pop_front();

		mtx_.Unlock();

		return val;
	}

	//for steal FIFO
	T PopBack()
	{
		if (!mtx_.TryLock())
		{
			return NULL;
		}

		if (deque_.empty()) 
		{
			mtx_.Unlock();
			return NULL;
		}

		T val = deque_.back();
		deque_.pop_back();

		mtx_.Unlock();

		return val;
	}

	bool Empty()
	{
		return deque_.empty();
	}

private:
	std::deque<T> deque_;
	OSMutex mtx_;

};

#endif //_SYNC_DEQUE_
