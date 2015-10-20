//
// url.hpp
// ~~~~~~~
//
// Copyright (c) 2009 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __URL_HPP__
#define __URL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstring>
#include <string>
#include <cctype>

#include <boost/throw_exception.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace avhttp {

/// The class @c url enables parsing and accessing the components of URLs.
/**
 * @par Example
 * To extract the components of a URL:
 * @code
 * avhttp::url url("http://user:pass@host:1234/dir/page?param=0#anchor");
 * std::cout << "Protocol: " << url.protocol() << std::endl;
 * std::cout << "User Info: " << url.user_info() << std::endl;
 * std::cout << "Host: " << url.host() << std::endl;
 * std::cout << "Port: " << url.port() << std::endl;
 * std::cout << "Path: " << url.path() << std::endl;
 * std::cout << "Query: " << url.query() << std::endl;
 * std::cout << "Fragment: " << url.fragment() << std::endl;
 * @endcode
 * The above code will print:
 * @code
 * Protocol: http
 * User Info: user:pass
 * Host: host
 * Port: 1234
 * Path: /dir/page
 * Query: param=0
 * Fragment: anchor
 * @endcode
 *
 * @par Requirements
 * @e Header: @c <url.hpp> @n
 * @e Namespace: @c avhttp
 */
class url
{
public:
	/// Constructs an object of class @c url.
	/**
	 * @par Remarks
	 * Postconditions: @c protocol(), @c user_info(), @c host(), @c path(),
	 * @c query(), @c fragment() all return an empty string, and @c port() returns
	 * 0.
	 */
	url()
		: ipv6_host_(false)
	{
	}

	/// Constructs an object of class @c url.
	/**
	 * @param s URL string to be parsed into its components.
	 *
	 * @throws boost::system::system_error Thrown when the URL string is invalid.
	 */
	url(const char* s)
		: ipv6_host_(false)
	{
		*this = from_string(s);
	}

	/// Constructs an object of class @c url.
	/**
	 * @param s URL string to be parsed into its components.
	 *
	 * @throws boost::system::system_error Thrown when the URL string is invalid.
	 */
	url(const std::string& s)
		: ipv6_host_(false)
	{
		*this = from_string(s);
	}

	/// Gets the protocol component of the URL.
	/**
	 * @returns A string specifying the protocol of the URL. Examples include
	 * @c http, @c https or @c file.
	 */
	std::string protocol() const
	{
		return protocol_;
	}

	/// Gets the user info component of the URL.
	/**
	 * @returns A string containing the user info of the URL. Typically in the
	 * format <tt>user:password</tt>, but depends on the protocol.
	 */
	std::string user_info() const
	{
		return user_info_;
	}

	/// Gets the host component of the URL.
	/**
	 * @returns A string containing the host name of the URL.
	 */
	std::string host() const
	{
		return host_;
	}

	/// Gets the port component of the URL.
	/**
	 * @returns The port number of the URL.
	 *
	 * @par Remarks
	 * If the URL string did not specify a port, and the protocol is one of @c
	 * http, @c https or @c ftp, an appropriate default port number is returned.
	 */
	unsigned short port() const
	{
		if (!port_.empty())
			return std::atoi(port_.c_str());
		if (protocol_ == "http")
			return 80;
		if (protocol_ == "https")
			return 443;
		if (protocol_ == "ftp")
			return 21;
		return 0;
	}

	/// Gets the path component of the URL.
	/**
	 * @returns A string containing the path of the URL.
	 *
	 * @par Remarks
	 * The path string is unescaped. To obtain the path in escaped form, use
	 * @c to_string(url::path_component).
	 */
	std::string path() const
	{
		std::string tmp_path;
		unescape_path(path_, tmp_path);
		return tmp_path;
	}

	/// Gets the query component of the URL.
	/**
	 * @returns A string containing the query string of the URL.
	 *
	 * @par Remarks
	 * The query string is not unescaped, but is returned in whatever form it
	 * takes in the original URL string.
	 */
	std::string query() const
	{
		return query_;
	}

	/// Gets the fragment component of the URL.
	/**
	 * @returns A string containing the fragment of the URL.
	 */
	std::string fragment() const
	{
		return fragment_;
	}

	/// Components of the URL, used with @c from_string.
	enum components_type
	{
		protocol_component = 1,
		user_info_component = 2,
		host_component = 4,
		port_component = 8,
		path_component = 16,
		query_component = 32,
		fragment_component = 64,
		all_components = protocol_component | user_info_component | host_component
		| port_component | path_component | query_component | fragment_component
	};

	/// Converts an object of class @c url to a string representation.
	/**
	 * @param components A bitmask specifying which components of the URL should
	 * be included in the string. See the @c url::components_type enumeration for
	 * possible values.
	 *
	 * @returns A string representation of the URL.
	 *
	 * @par Examples
	 * To convert the entire URL to a string:
	 * @code
	 * std::string s = url.to_string();
	 * @endcode
	 * To convert only the host and port number into a string:
	 * @code
	 * std::string s = url.to_string(
	 *     urdl::url::host_component
	 *     | urdl::url::port_component);
	 * @endcode
	 */
	std::string to_string(int components = all_components) const
	{
		std::string s;

		if ((components & protocol_component) != 0 && !protocol_.empty())
		{
			s = protocol_;
			s += "://";
		}

		if ((components & user_info_component) != 0 && !user_info_.empty())
		{
			s += user_info_;
			s += "@";
		}

		if ((components & host_component) != 0)
		{
			if (ipv6_host_)
				s += "[";
			s += host_;
			if (ipv6_host_)
				s += "]";
		}

		if ((components & port_component) != 0 && !port_.empty())
		{
			s += ":";
			s += port_;
		}

		if ((components & path_component) != 0 && !path_.empty())
		{
			s += path_;
		}

		if ((components & query_component) != 0 && !query_.empty())
		{
			s += "?";
			s += query_;
		}

		if ((components & fragment_component) != 0 && !fragment_.empty())
		{
			s += "#";
			s += fragment_;
		}

		return s;
	}

	/// Converts a string representation of a URL into an object of class @c url.
	/**
	 * @param s URL string to be parsed into its components.
	 *
	 * @returns A @c url object corresponding to the specified string.
	 *
	 * @throws boost::system::system_error Thrown when the URL string is invalid.
	 */
	static url from_string(const char* s)
	{
		boost::system::error_code ec;
		url new_url(from_string(s, ec));
		if (ec)
		{
			boost::system::system_error ex(ec);
			boost::throw_exception(ex);
		}
		return new_url;
	}

	/// Converts a string representation of a URL into an object of class @c url.
	/**
	 * @param s URL string to be parsed into its components.
	 *
	 * @param ec Error code set to indicate the reason for failure, if any.
	 *
	 * @returns A @c url object corresponding to the specified string.
	 */
	static url from_string(const char* s, boost::system::error_code& ec)
	{
		url new_url;

		// Protocol.
		std::size_t length = std::strcspn(s, ":");
		new_url.protocol_.assign(s, s + length);
		for (std::size_t i = 0; i < new_url.protocol_.length(); ++i)
			new_url.protocol_[i] = std::tolower(new_url.protocol_[i]);
		s += length;

		// "://".
		if (*s++ != ':')
		{
			ec = make_error_code(boost::system::errc::invalid_argument);
			return url();
		}
		if (*s++ != '/')
		{
			ec = make_error_code(boost::system::errc::invalid_argument);
			return url();
		}
		if (*s++ != '/')
		{
			ec = make_error_code(boost::system::errc::invalid_argument);
			return url();
		}

		// UserInfo.
		length = std::strcspn(s, "@:[/?#");
		if (s[length] == '@')
		{
			new_url.user_info_.assign(s, s + length);
			s += length + 1;
		}
		else if (s[length] == ':')
		{
			std::size_t length2 = std::strcspn(s + length, "@/?#");
			if (s[length + length2] == '@')
			{
				new_url.user_info_.assign(s, s + length + length2);
				s += length + length2 + 1;
			}
		}

		// Host.
		if (*s == '[')
		{
			length = std::strcspn(++s, "]");
			if (s[length] != ']')
			{
				ec = make_error_code(boost::system::errc::invalid_argument);
				return url();
			}
			new_url.host_.assign(s, s + length);
			new_url.ipv6_host_ = true;
			s += length + 1;
			if (std::strcspn(s, ":/?#") != 0)
			{
				ec = make_error_code(boost::system::errc::invalid_argument);
				return url();
			}
		}
		else
		{
			length = std::strcspn(s, ":/?#");
			new_url.host_.assign(s, s + length);
			s += length;
		}

		// Port.
		if (*s == ':')
		{
			length = std::strcspn(++s, "/?#");
			if (length == 0)
			{
				ec = make_error_code(boost::system::errc::invalid_argument);
				return url();
			}
			new_url.port_.assign(s, s + length);
			for (std::size_t i = 0; i < new_url.port_.length(); ++i)
			{
				if (!std::isdigit(new_url.port_[i]))
				{
					ec = make_error_code(boost::system::errc::invalid_argument);
					return url();
				}
			}
			s += length;
		}

		// Path.
		if (*s == '/')
		{
			length = std::strcspn(s, "?#");
			new_url.path_.assign(s, s + length);
			std::string tmp_path;
			if (!unescape_path(new_url.path_, tmp_path))
			{
				ec = make_error_code(boost::system::errc::invalid_argument);
				return url();
			}
			s += length;
		}
		else
			new_url.path_ = "/";

		// Query.
		if (*s == '?')
		{
			length = std::strcspn(++s, "#");
			new_url.query_.assign(s, s + length);
			s += length;
		}

		// Fragment.
		if (*s == '#')
			new_url.fragment_.assign(++s);

		ec = boost::system::error_code();
		return new_url;
	}

	/// Converts a string representation of a URL into an object of class @c url.
	/**
	 * @param s URL string to be parsed into its components.
	 *
	 * @returns A @c url object corresponding to the specified string.
	 *
	 * @throws boost::system::system_error Thrown when the URL string is invalid.
	 */
	static url from_string(const std::string& s)
	{
		return from_string(s.c_str());
	}

	/// Converts a string representation of a URL into an object of class @c url.
	/**
	 * @param s URL string to be parsed into its components.
	 *
	 * @param ec Error code set to indicate the reason for failure, if any.
	 *
	 * @returns A @c url object corresponding to the specified string.
	 */
	static url from_string(const std::string& s, boost::system::error_code& ec)
	{
		return from_string(s.c_str(), ec);
	}

	/// Compares two @c url objects for equality.
	friend bool operator==(const url& a, const url& b)
	{
		return a.protocol_ == b.protocol_
			&& a.user_info_ == b.user_info_
			&& a.host_ == b.host_
			&& a.port_ == b.port_
			&& a.path_ == b.path_
			&& a.query_ == b.query_
			&& a.fragment_ == b.fragment_;
	}

	/// Compares two @c url objects for inequality.
	friend bool operator!=(const url& a, const url& b)
	{
		return !(a == b);
	}

	/// Compares two @c url objects for ordering.
	friend bool operator<(const url& a, const url& b)
	{
		if (a.protocol_ < b.protocol_)
			return true;
		if (b.protocol_ < a.protocol_)
			return false;

		if (a.user_info_ < b.user_info_)
			return true;
		if (b.user_info_ < a.user_info_)
			return false;

		if (a.host_ < b.host_)
			return true;
		if (b.host_ < a.host_)
			return false;

		if (a.port_ < b.port_)
			return true;
		if (b.port_ < a.port_)
			return false;

		if (a.path_ < b.path_)
			return true;
		if (b.path_ < a.path_)
			return false;

		if (a.query_ < b.query_)
			return true;
		if (b.query_ < a.query_)
			return false;

		return a.fragment_ < b.fragment_;
	}

private:
	static bool unescape_path(const std::string& in, std::string& out)
	{
		out.clear();
		out.reserve(in.size());
		for (std::size_t i = 0; i < in.size(); ++i)
		{
			switch (in[i])
			{
			case '%':
				if (i + 3 <= in.size())
				{
					unsigned int value = 0;
					for (std::size_t j = i + 1; j < i + 3; ++j)
					{
						switch (in[j])
						{
						case '0': case '1': case '2': case '3': case '4':
						case '5': case '6': case '7': case '8': case '9':
							value += in[j] - '0';
							break;
						case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
							value += in[j] - 'a' + 10;
							break;
						case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
							value += in[j] - 'A' + 10;
							break;
						default:
							return false;
						}
						if (j == i + 1)
							value <<= 4;
					}
					out += static_cast<char>(value);
					i += 2;
				}
				else
					return false;
				break;
			case '-': case '_': case '.': case '!': case '~': case '*':
			case '\'': case '(': case ')': case ':': case '@': case '&':
			case '=': case '+': case '$': case ',': case '/': case ';':
				out += in[i];
				break;
			default:
				if (!std::isalnum(in[i]))
					return false;
				out += in[i];
				break;
			}
		}
		return true;
	}

	std::string protocol_;
	std::string user_info_;
	std::string host_;
	std::string port_;
	std::string path_;
	std::string query_;
	std::string fragment_;
	bool ipv6_host_;
};

} // namespace avhttp

#endif // __URL_HPP__
