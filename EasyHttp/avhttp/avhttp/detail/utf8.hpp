//
// utf8.hpp
// ~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_UTF8_HPP
#define AVHTTP_UTF8_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <stdlib.h> // for mbtowc, wctomb.
#include <string>

#include <boost/locale.hpp>
#include <boost/locale/utf.hpp>

namespace avhttp {
namespace detail {

// 字符集编码转换接口声明.

// wide string to utf8 string.
inline std::string wide_utf8(const std::wstring& source);

// utf8 string to wide string.
inline std::wstring utf8_wide(std::string const& source);

// ansi string to utf8 string.
inline std::string ansi_utf8(std::string const& source);
inline std::string ansi_utf8(
	std::string const& source, const std::string& characters);

// utf8 string to ansi string.
inline std::string utf8_ansi(std::string const& source);
inline std::string utf8_ansi(
	std::string const& source, const std::string& characters);

// wide string to ansi string.
inline std::string wide_ansi(const std::wstring& source);
inline std::string wide_ansi(
	std::wstring const& source, const std::string& characters);

// ansi string to wide string.
inline std::wstring ansi_wide(const std::string& source);
inline std::wstring ansi_wide(
	const std::string& source, const std::string& characters);

// 字符集编码转换接口实现, 使用boost实现.
inline std::wstring ansi_wide(
	const std::string& source, const std::string& characters)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(ansi_utf8(source, characters));
}

inline std::wstring ansi_wide(const std::string& source)
{
	std::wstring wide;
	wchar_t dest;
	std::size_t max = source.size();

	// reset mbtowc.
	mbtowc(NULL, NULL, 0);

	// convert source to wide.
	for (std::string::const_iterator i = source.begin();
		i != source.end(); )
	{
		int length = mbtowc(&dest, &(*i), max);
		if (length < 1)
			break;
		max -= length;
		while (length--) i++;
		wide.push_back(dest);
	}

	return wide;
}

inline std::string ansi_utf8(
	std::string const& source, const std::string& characters)
{
	return boost::locale::conv::between(source, "UTF-8", characters);
}

inline std::string ansi_utf8(std::string const& source)
{
	std::wstring wide = ansi_wide(source);
	return wide_utf8(wide);
}

inline std::string wide_utf8(const std::wstring& source)
{
	return boost::locale::conv::utf_to_utf<char>(source);
}

inline std::wstring utf8_wide(std::string const& source)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(source);
}

inline std::string utf8_ansi(
	std::string const& source, const std::string& characters)
{
	return boost::locale::conv::between(source, characters, "UTF-8");
}

inline std::string utf8_ansi(std::string const& source)
{
	return wide_ansi(utf8_wide(source));
}

inline std::string wide_ansi(
	std::wstring const& source, const std::string& characters)
{
	return utf8_ansi(wide_utf8(source), characters);
}

inline std::string wide_ansi(const std::wstring& source)
{
	std::size_t buffer_size = MB_CUR_MAX;
	std::vector<char> buffer(buffer_size, 0);
	std::string ansi;

	// reset wctomb.
	wctomb(NULL, 0);

	// convert source to wide.
	for (std::wstring::const_iterator i = source.begin();
		i != source.end(); i++)
	{
		int length = wctomb(&(*buffer.begin()), *i);
		if (length < 1)
			break;
		for (int j = 0; j < length; j++)
			ansi.push_back(buffer[j]);
	}

	return ansi;
}

} // namespace detail
} // namespace avhttp

#endif // AVHTTP_UTF8_HPP
