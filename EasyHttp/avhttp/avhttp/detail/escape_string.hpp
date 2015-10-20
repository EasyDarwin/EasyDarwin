//
// escape_string.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __ESCAPE_STRING_HPP__
#define __ESCAPE_STRING_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

namespace avhttp {
namespace detail {

static const char hex_chars[] = "0123456789abcdef";

inline bool is_char(int c)
{
	return c >= 0 && c <= 127;
}

inline bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

inline bool is_ctl(int c)
{
	return (c >= 0 && c <= 31) || c == 127;
}

inline bool is_tspecial(int c)
{
	switch (c)
	{
	case '(': case ')': case '<': case '>': case '@':
	case ',': case ';': case ':': case '\\': case '"':
	case '/': case '[': case ']': case '?': case '=':
	case '{': case '}': case ' ': case '\t':
		return true;
	default:
		return false;
	}
}

inline std::string to_hex(std::string const& s)
{
	std::string ret;
	for (std::string::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		ret += hex_chars[((unsigned char)*i) >> 4];
		ret += hex_chars[((unsigned char)*i) & 0xf];
	}
	return ret;
}

inline void to_hex(char const *in, int len, char* out)
{
	for (char const* end = in + len; in < end; ++in)
	{
		*out++ = hex_chars[((unsigned char)*in) >> 4];
		*out++ = hex_chars[((unsigned char)*in) & 0xf];
	}
	*out = '\0';
}

inline bool is_print(char c)
{
	return c >= 32 && c < 127;
}

inline bool tolower_compare(char a, char b)
{
	return std::tolower(a) == std::tolower(b);
}

#if 0
inline long long int strtoll(const char *nptr, char **endptr, int base)
{
	long long i_value = 0;
	int sign = 1, newbase = base ? base : 10;

	nptr += strspn(nptr, "\t ");
	if (*nptr == '-')
	{
		sign = -1;
		nptr++;
	}

	/* Try to detect base */
	if (*nptr == '0')
	{
		newbase = 8;
		nptr++;

		if (*nptr == 'x')
		{
			newbase = 16;
			nptr++;
		}
	}

	if (base && newbase != base)
	{
		if (endptr) *endptr = (char *)nptr;
		return i_value;
	}

	switch (newbase)
	{
	case 10:
		while (*nptr >= '0' && *nptr <= '9')
		{
			i_value *= 10;
			i_value += ( *nptr++ - '0' );
		}
		if (endptr) *endptr = (char *)nptr;
		break;

	case 16:
		while ((*nptr >= '0' && *nptr <= '9') ||
			(*nptr >= 'a' && *nptr <= 'f') ||
			(*nptr >= 'A' && *nptr <= 'F'))
		{
			int i_valc = 0;
			if (*nptr >= '0' && *nptr <= '9') i_valc = *nptr - '0';
			else if (*nptr >= 'a' && *nptr <= 'f') i_valc = *nptr - 'a' +10;
			else if (*nptr >= 'A' && *nptr <= 'F') i_valc = *nptr - 'A' +10;
			i_value *= 16;
			i_value += i_valc;
			nptr++;
		}
		if (endptr) *endptr = (char *)nptr;
		break;

	default:
		i_value = strtol(nptr, endptr, newbase);
		break;
	}

	return i_value * sign;
}
#endif

}
}

#endif // __ESCAPE_STRING_HPP__
