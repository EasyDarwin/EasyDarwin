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
		OSMutexLocker lock(&mtx_);
		deque_.push_front(val);
		cond_.Signal();
	}

	T PopFront()
	{
		OSMutexLocker lock(&mtx_);

		if (deque_.empty())
		{
			return NULL;
		}

		T val = deque_.front();
		deque_.pop_front();

		return val;
	}

	T PopFrontBlocking(signed long inTimeoutInMilSecs)
	{
		OSMutexLocker lock(&mtx_);

		if (deque_.empty())
		{
			cond_.Wait(&mtx_, inTimeoutInMilSecs);
			return NULL;
		}

		T val = deque_.front();
		deque_.pop_front();

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

	T PopBackBlocking(signed long inTimeoutInMilSecs)
	{
		if (!mtx_.TryLock())
		{
			return NULL;
		}

		if (deque_.empty())
		{
			cond_.Wait(&mtx_, inTimeoutInMilSecs);
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

	OSCond* GetCond()
	{
		return &cond_;
	}

private:
	std::deque<T> deque_;
	OSMutex mtx_;
	OSCond cond_;

};

#endif //_SYNC_DEQUE_
