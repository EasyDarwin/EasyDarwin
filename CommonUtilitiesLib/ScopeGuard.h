#ifndef __COMMON_UTILITIES_LIB_SCOPE_GUARD_H__
#define __COMMON_UTILITIES_LIB_SCOPE_GUARD_H__

#include <type_traits>

template<typename F>
class ScopeGuard
{
public:
	explicit ScopeGuard(const F& f)
		: f_(f)
	{
	}

	explicit ScopeGuard(F&& f)
		: f_(std::move(f))
	{
	}

	ScopeGuard(ScopeGuard&& rhs) noexcept
		: f_(std::move(rhs.f_))
		, dismissed_(rhs.dismissed_)
	{
		rhs.Release();
	}

	~ScopeGuard()
	{
		if (!dismissed_)
		{
			f_();
		}
	}

	void Release()
	{
		dismissed_ = true;
	}

private:
	ScopeGuard() = delete;
	ScopeGuard(const ScopeGuard&) = delete;
	ScopeGuard& operator =(const ScopeGuard&) = delete;

	F f_;
	bool dismissed_ = false;

};

template<typename F>
ScopeGuard<typename std::decay<F>::type> MakeGuard(F&& f)
{
	return ScopeGuard<typename std::decay<F>::type>(std::forward<F>(f));
}

#endif //__COMMON_UTILITIES_LIB_SCOPE_GUARD_H__
