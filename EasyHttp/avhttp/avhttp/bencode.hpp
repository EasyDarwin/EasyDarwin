//
// bencode.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003, Arvid Norberg All rights reserved.
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __BENCODE_HPP__
#define __BENCODE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdlib>
#include <boost/static_assert.hpp>
#include "entry.hpp"
#include "detail/escape_string.hpp"

namespace avhttp {

namespace detail {

	template <class OutIt>
	int write_string(OutIt& out, const std::string& val)
	{
		for (std::string::const_iterator i = val.begin()
			, end(val.end()); i != end; ++i)
			*out++ = *i;
		return val.length();
	}

	inline char const* integer_to_str(char* buf, int size, entry::integer_type val)
	{
		int sign = 0;
		if (val < 0)
		{
			sign = 1;
			val = -val;
		}
		buf[--size] = '\0';
		if (val == 0) buf[--size] = '0';
		for (; size > sign && val != 0;)
		{
			buf[--size] = '0' + char(val % 10);
			val /= 10;
		}
		if (sign) buf[--size] = '-';
		return buf + size;
	}

	template <class OutIt>
	int write_integer(OutIt& out, entry::integer_type val)
	{
		// the stack allocated buffer for keeping the
		// decimal representation of the number can
		// not hold number bigger than this:
		BOOST_STATIC_ASSERT(sizeof(entry::integer_type) <= 8);
		char buf[21];
		int ret = 0;
		for (char const* str = integer_to_str(buf, 21, val);
			*str != 0; ++str)
		{
			*out = *str;
			++out;
			++ret;
		}
		return ret;
	}

	template <class OutIt>
	void write_char(OutIt& out, char c)
	{
		*out = c;
		++out;
	}

	template <class InIt>
	std::string read_until(InIt& in, InIt end, char end_token, bool& err)
	{
		std::string ret;
		if (in == end)
		{
			err = true;
			return ret;
		}
		while (*in != end_token)
		{
			ret += *in;
			++in;
			if (in == end)
			{
				err = true;
				return ret;
			}
		}
		return ret;
	}

	template<class InIt>
	void read_string(InIt& in, InIt end, int len, std::string& str, bool& err)
	{
		BOOST_ASSERT(len >= 0);
		for (int i = 0; i < len; ++i)
		{
			if (in == end)
			{
				err = true;
				return;
			}
			str += *in;
			++in;
		}
	}

	// returns the number of bytes written
	template<class OutIt>
	int bencode_recursive(OutIt& out, const entry& e)
	{
		int ret = 0;
		switch(e.type())
		{
		case entry::int_t:
			write_char(out, 'i');
			ret += write_integer(out, e.integer());
			write_char(out, 'e');
			ret += 2;
			break;
		case entry::string_t:
			ret += write_integer(out, e.string().length());
			write_char(out, ':');
			ret += write_string(out, e.string());
			ret += 1;
			break;
		case entry::list_t:
			write_char(out, 'l');
			for (entry::list_type::const_iterator i = e.list().begin(); i != e.list().end(); ++i)
				ret += bencode_recursive(out, *i);
			write_char(out, 'e');
			ret += 2;
			break;
		case entry::dictionary_t:
			write_char(out, 'd');
			for (entry::dictionary_type::const_iterator i = e.dict().begin();
				i != e.dict().end(); ++i)
			{
				// write key
				ret += write_integer(out, i->first.length());
				write_char(out, ':');
				ret += write_string(out, i->first);
				// write value
				ret += bencode_recursive(out, i->second);
				ret += 1;
			}
			write_char(out, 'e');
			ret += 2;
			break;
		default:
			// do nothing
			break;
		}
		return ret;
	}

	template<class InIt>
	void bdecode_recursive(InIt& in, InIt end, entry& ret, bool& err, int depth)
	{
		if (depth >= 100)
		{
			err = true;
			return;
		}

		if (in == end)
		{
			err = true;
			return;
		}
		switch (*in)
		{

		// ----------------------------------------------
		// integer
		case 'i':
			{
			++in; // 'i' 
			std::string val = read_until(in, end, 'e', err);
			if (err) return;
			BOOST_ASSERT(*in == 'e');
			++in; // 'e' 
			ret = entry(entry::int_t);
			char* end_pointer;
#if defined WIN32 && !defined _MINGW
			ret.integer() = _strtoi64(val.c_str(), &end_pointer, 10);
#else
			ret.integer() = strtoll(val.c_str(), &end_pointer, 10);
#endif
			if (end_pointer == val.c_str())
			{
				err = true;
				return;
			}
			} break;

		// ----------------------------------------------
		// list
		case 'l':
			{
			ret = entry(entry::list_t);
			++in; // 'l'
			while (*in != 'e')
			{
				ret.list().push_back(entry());
				entry& e = ret.list().back();
				bdecode_recursive(in, end, e, err, depth + 1);
				if (err)
				{
					return;
				}
				if (in == end)
				{
					err = true;
					return;
				}
			}
			BOOST_ASSERT(*in == 'e');
			++in; // 'e'
			} break;

		// ----------------------------------------------
		// dictionary
		case 'd':
			{
			ret = entry(entry::dictionary_t);
			++in; // 'd'
			while (*in != 'e')
			{
				entry key;
				bdecode_recursive(in, end, key, err, depth + 1);
				if (err || key.type() != entry::string_t)
				{
					return;
				}
				entry& e = ret[key.string()];
				bdecode_recursive(in, end, e, err, depth + 1);
				if (err)
				{
					return;
				}
				if (in == end)
				{
					err = true;
					return;
				}
			}
			BOOST_ASSERT(*in == 'e');
			++in; // 'e'
			} break;

		// ----------------------------------------------
		// string
		default:
			if (is_digit((unsigned char)*in))
			{
				std::string len_s = read_until(in, end, ':', err);
				if (err)
				{
					return;
				}
				BOOST_ASSERT(*in == ':');
				++in; // ':'
				int len = std::atoi(len_s.c_str());
				ret = entry(entry::string_t);
				read_string(in, end, len, ret.string(), err);
				if (err)
				{
					return;
				}
			}
			else
			{
				err = true;
				return;
			}
		}
	}
} // namespace detail

template<class OutIt>
int bencode(OutIt out, const entry& e)
{
	return detail::bencode_recursive(out, e);
}

template<class InIt>
entry bdecode(InIt start, InIt end)
{
	entry e;
	bool err = false;
	detail::bdecode_recursive(start, end, e, err, 0);
	if (err) return entry();
	return e;
}

template<class InIt>
entry bdecode(InIt start, InIt end, int& len)
{
	entry e;
	bool err = false;
	InIt s = start;
	detail::bdecode_recursive(start, end, e, err, 0);
	len = std::distance(s, start);
	BOOST_ASSERT(len >= 0);
	if (err) return entry();
	return e;
}

}

#endif // __BENCODE_HPP__
