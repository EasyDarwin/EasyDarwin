//
// async_read_body.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2012 - 2013  微蔡 <microcai@fedoraproject.org>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_COMPLETION_CONDITION_HPP
#define AVHTTP_COMPLETION_CONDITION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/asio/completion_condition.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace avhttp {

namespace detail {
// match condition!
struct transfer_response_body_t
{
	transfer_response_body_t(boost::int64_t content_length)
		: m_content_length(content_length)
	{
	}

	template <typename Error>
	std::size_t operator()(const Error& err, std::size_t bytes_transferred)
	{
		using boost::asio::detail::default_max_transfer_size;

		if(m_content_length > 0 )
		{
			// just the same boost::asio::transfer_exactly
			return (!!err || static_cast<boost::int64_t>(bytes_transferred) >= m_content_length) ? 0 :
			(m_content_length - bytes_transferred < default_max_transfer_size
				? m_content_length - bytes_transferred : std::size_t(default_max_transfer_size));
		}
		else
		{
			// just the same as boost::asio::transfer_all
			return !!err ? 0 : default_max_transfer_size;
		}
	}

	boost::int64_t m_content_length;
};

}

///Return a completion condition function object that indicates that a read or
// write operation should continue until all http respons body have been
// transferred, or until an error occurs.
//
// This function is used to create an object, of unspecified type, that meets
// CompletionCondition requirements.
//
// @begin example
// Reading until a buffer is full or contains all respons body:
// @code
// boost::array<char, 12800> buf;
// boost::system::error_code ec;
// std::size_t n = boost::asio::read(
//     avhttp_stream, boost::asio::buffer(buf),
//     boost::asio::transfer_response_body(avhttp_stream.content_length()), ec);
// if (ec)
// {
//   // An error occurred.
// }
// else
// {
//   // n == 64
// }
// @end example
# if defined(GENERATING_DOCUMENTATION)
unspecified transfer_response_body(boost::int64_t content_length)
#else
inline detail::transfer_response_body_t transfer_response_body(boost::int64_t content_length)
{
	return detail::transfer_response_body_t(content_length);
}
#endif

}


#include <boost/asio/detail/pop_options.hpp>

#endif // AVHTTP_COMPLETION_CONDITION_HPP
