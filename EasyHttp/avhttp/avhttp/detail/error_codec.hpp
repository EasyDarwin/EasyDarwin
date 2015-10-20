//
// http_stream.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __ERROR_CODEC_HPP__
#define __ERROR_CODEC_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>

#ifdef _MSC_VER
#define _attribute_weak_ __declspec(selectany)
#else
#define _attribute_weak_ __attribute__((weak))
#endif

namespace avhttp {

namespace detail {
	class error_category_impl;
}

extern detail::error_category_impl error_category_instance;

inline const boost::system::error_category& error_category()
{
	return reinterpret_cast<const boost::system::error_category&>(error_category_instance);
}

namespace errc {

/// HTTP error codes.
/**
 * The enumerators of type @c errc_t are implicitly convertible to objects of
 * type @c boost::system::error_code.
 *
 * @par Requirements
 * @e Header: @c <error_codec.hpp> @n
 * @e Namespace: @c avhttp::errc
 */
enum errc_t
{
	// Client-generated errors.

	/// The response's status line was malformed.
	malformed_status_line = 1,

	/// The response's headers were malformed.
	malformed_response_headers = 2,

	/// The entry type error.
	invalid_entry_type = 10,

	// Server-generated status codes.

	/// The server-generated status code "100 Continue".
	continue_request = 100,

	/// The server-generated status code "101 Switching Protocols".
	switching_protocols = 101,

	/// The server-generated status code "200 OK".
	ok = 200,

	/// The server-generated status code "201 Created".
	created = 201,

	/// The server-generated status code "202 Accepted".
	accepted = 202,

	/// The server-generated status code "203 Non-Authoritative Information".
	non_authoritative_information = 203,

	/// The server-generated status code "204 No Content".
	no_content = 204,

	/// The server-generated status code "205 Reset Content".
	reset_content = 205,

	/// The server-generated status code "206 Partial Content".
	partial_content = 206,

	/// The server-generated status code "300 Multiple Choices".
	multiple_choices = 300,

	/// The server-generated status code "301 Moved Permanently".
	moved_permanently = 301,

	/// The server-generated status code "302 Found".
	found = 302,

	/// The server-generated status code "303 See Other".
	see_other = 303,

	/// The server-generated status code "304 Not Modified".
	not_modified = 304,

	/// The server-generated status code "305 Use Proxy".
	use_proxy = 305,

	/// The server-generated status code "307 Temporary Redirect".
	temporary_redirect = 307,

	/// The server-generated status code "400 Bad Request".
	bad_request = 400,

	/// The server-generated status code "401 Unauthorized".
	unauthorized = 401,

	/// The server-generated status code "402 Payment Required".
	payment_required = 402,

	/// The server-generated status code "403 Forbidden".
	forbidden = 403,

	/// The server-generated status code "404 Not Found".
	not_found = 404,

	/// The server-generated status code "405 Method Not Allowed".
	method_not_allowed = 405,

	/// The server-generated status code "406 Not Acceptable".
	not_acceptable = 406,

	/// The server-generated status code "407 Proxy Authentication Required".
	proxy_authentication_required = 407,

	/// The server-generated status code "408 Request Time-out".
	request_timeout = 408,

	/// The server-generated status code "409 Conflict".
	conflict = 409,

	/// The server-generated status code "410 Gone".
	gone = 410,

	/// The server-generated status code "411 Length Required".
	length_required = 411,

	/// The server-generated status code "412 Precondition Failed".
	precondition_failed = 412,

	/// The server-generated status code "413 Request Entity Too Large".
	request_entity_too_large = 413,

	/// The server-generated status code "414 Request URI Too Large".
	request_uri_too_large = 414,

	/// The server-generated status code "415 Unsupported Media Type".
	unsupported_media_type = 415,

	/// The server-generated status code "416 Requested Range Not Satisfiable".
	requested_range_not_satisfiable = 416,

	/// The server-generated status code "417 Expectation Failed".
	expectation_failed = 417,

	/// The server-generated status code "500 Internal Server Error".
	internal_server_error = 500,

	/// The server-generated status code "501 Not Implemented".
	not_implemented = 501,

	/// The server-generated status code "502 Bad Gateway".
	bad_gateway = 502,

	/// The server-generated status code "503 Service Unavailable".
	service_unavailable = 503,

	/// The server-generated status code "504 Gateway Timeout".
	gateway_timeout = 504,

	/// The server-generated status code "505 HTTP Version Not Supported".
	version_not_supported = 505,

	/// SOCKS unsupported version.
	unsupported_version = 1000,

	/// SOCKS username required.
	username_required,

	/// SOCKS unsupported authentication version.
	unsupported_authentication_version,

	/// SOCKS authentication error.
	authentication_error,

	/// SOCKS general failure.
	general_failure,

	/// SOCKS command not supported.
	command_not_supported,

	/// SOCKS no identd running.
	no_identd,

	/// SOCKS no identd running.
	identd_error
};

/// Converts a value of type @c errc_t to a corresponding object of type
/// @c boost::system::error_code.
/**
 * @par Requirements
 * @e Header: @c <error_codec.hpp> @n
 * @e Namespace: @c avhttp::errc
 */
inline boost::system::error_code make_error_code(errc_t e)
{
	return boost::system::error_code(static_cast<int>(e), avhttp::error_category());
}

} // namespace errc
} // namespace avhttp

namespace boost {
namespace system {

template <>
struct is_error_code_enum<avhttp::errc::errc_t>
{
  static const bool value = true;
};

} // namespace system
} // namespace boost

namespace avhttp {
namespace detail {

class error_category_impl
  : public boost::system::error_category
{
	virtual const char* name() const
	{
		return "HTTP";
	}

	virtual std::string message(int e) const
	{
		switch (e)
		{
		case errc::malformed_status_line:
			return "Malformed status line";
		case errc::malformed_response_headers:
			return "Malformed response headers";
		case errc::invalid_entry_type:
			return "invalid type requested from entry";
		case errc::continue_request:
			return "Continue";
		case errc::switching_protocols:
			return "Switching protocols";
		case errc::ok:
			return "OK";
		case errc::created:
			return "Created";
		case errc::accepted:
			return "Accepted";
		case errc::non_authoritative_information:
			return "Non-authoritative information";
		case errc::no_content:
			return "No content";
		case errc::reset_content:
			return "Reset content";
		case errc::partial_content:
			return "Partial content";
		case errc::multiple_choices:
			return "Multiple choices";
		case errc::moved_permanently:
			return "Moved permanently";
		case errc::found:
			return "Found";
		case errc::see_other:
			return "See other";
		case errc::not_modified:
			return "Not modified";
		case errc::use_proxy:
			return "Use proxy";
		case errc::temporary_redirect:
			return "Temporary redirect";
		case errc::bad_request:
			return "Bad request";
		case errc::unauthorized:
			return "Unauthorized";
		case errc::payment_required:
			return "Payment required";
		case errc::forbidden:
			return "Forbidden";
		case errc::not_found:
			return "Not found";
		case errc::method_not_allowed:
			return "Method not allowed";
		case errc::not_acceptable:
			return "Not acceptable";
		case errc::proxy_authentication_required:
			return "Proxy authentication required";
		case errc::request_timeout:
			return "Request time-out";
		case errc::conflict:
			return "Conflict";
		case errc::gone:
			return "Gone";
		case errc::length_required:
			return "Length required";
		case errc::precondition_failed:
			return "Precondition failed";
		case errc::request_entity_too_large:
			return "Request entity too large";
		case errc::request_uri_too_large:
			return "Request URI too large";
		case errc::unsupported_media_type:
			return "Unsupported media type";
		case errc::requested_range_not_satisfiable:
			return "Requested range not satisfiable";
		case errc::expectation_failed:
			return "Expectation failed";
		case errc::internal_server_error:
			return "Internal server error";
		case errc::not_implemented:
			return "Not implemented";
		case errc::bad_gateway:
			return "Bad gateway";
		case errc::service_unavailable:
			return "Service unavailable";
		case errc::gateway_timeout:
			return "Gateway time-out";
		case errc::version_not_supported:
			return "HTTP version not supported";
		case  errc::unsupported_version:
			return "SOCKS unsupported version";
		case errc::username_required:
			return "SOCKS username required";
		case errc::unsupported_authentication_version:
			return "SOCKS unsupported authentication version";
		case errc::authentication_error:
			return "SOCKS authentication error";
		case errc::general_failure:
			return "SOCKS general failure";
		case errc::command_not_supported:
			return "SOCKS command not supported";
		case errc::no_identd:
			return "SOCKS no identd running";
		case errc::identd_error:
			return "SOCKS no identd running";
		default:
			return "Unknown HTTP error";
		}
	}

	virtual boost::system::error_condition default_error_condition(int e) const
	{
		switch (e)
		{
		case errc::unauthorized:
		case errc::forbidden:
			return boost::system::errc::permission_denied;
		case errc::not_found:
			return boost::system::errc::no_such_file_or_directory;
		default:
			return boost::system::error_condition(e, *this);
		}
	}
};

} // namespace detail

_attribute_weak_ detail::error_category_impl error_category_instance;

} // namespace avhttp

#endif // __ERROR_CODEC_HPP__
