//
// async_read_body.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
// Copyright (C) 2012 - 2013  微蔡 <microcai@fedoraproject.org>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_MISC_HTTP_ASYNC_READBODY_HPP
#define AVHTTP_MISC_HTTP_ASYNC_READBODY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

BOOST_STATIC_ASSERT_MSG(BOOST_VERSION >= 105400, "You must use boost-1.54 or later!!!");

#include "avhttp/http_stream.hpp"
#include "avhttp/completion_condition.hpp"

namespace avhttp {
namespace detail {

template <typename AsyncReadStream, typename MutableBufferSequence, typename Handler>
class read_body_op : boost::asio::coroutine
{
public:
	read_body_op(AsyncReadStream& stream, const avhttp::url& url,
		MutableBufferSequence& buffers, Handler handler)
		: m_stream(stream)
		, m_buffers(buffers)
		, m_handler(BOOST_ASIO_MOVE_CAST(Handler)(handler))
	{
		m_stream.async_open(url, *this);
	}

	void operator()(const boost::system::error_code& ec, std::size_t bytes_transferred = 0)
	{
		BOOST_ASIO_CORO_REENTER(this)
		{
			if(!ec)
			{
				BOOST_ASIO_CORO_YIELD boost::asio::async_read(
					m_stream, m_buffers, transfer_response_body(m_stream.content_length()), *this);
			}
			else
			{
				m_handler(ec, bytes_transferred);
				return;
			}

			if(ec == boost::asio::error::eof && m_stream.content_length() == -1)
			{
				m_handler(boost::system::error_code(), bytes_transferred);
			}
			else
			{
				m_handler(ec, bytes_transferred);
			}
		}
	}

// private:
	AsyncReadStream& m_stream;
	MutableBufferSequence& m_buffers;
	Handler m_handler;
};

template <typename AsyncReadStream, typename MutableBufferSequence, typename Handler>
read_body_op<AsyncReadStream, MutableBufferSequence, Handler>
	make_read_body_op(AsyncReadStream& stream,
	const avhttp::url& url, MutableBufferSequence& buffers, Handler handler)
{
	return read_body_op<AsyncReadStream, MutableBufferSequence, Handler>(
		stream, url, buffers, handler);
}

} // namespace detail


///用于http_stream异步访问url.
// 这个函数用于http_stream异步访问指定的url, 并通过handler回调通知用户, 数据将
// 保存在用户事先提供的buffers中.
// @注意:
//  1. 该函数回调条件为直到读取完整的body或eof或其它错误, 错误信息通过error_code传回.
//  2. 在完成整个过程中, 应该保持 stream 和 buffers的生命期.
// @param stream 一个http_stream对象.
// @param url 指定的url.
// @param buffers 一个或多个用于读取数据的缓冲区
// 这个类型必须满足MutableBufferSequence, MutableBufferSequence的定义.
// 具体可参考boost.asio文档中相应的描述.
// @param handler在读取操作完成或出现错误时, 将被回调, 它满足以下条件:
// @begin code
//  void handler(
//    const boost::system::error_code& ec,	// 用于返回操作状态.
//    std::size_t bytes_transferred			// 返回读取的数据字节数.
//  );
// @end code
// @begin example
//  void handler(const boost::system::error_code& ec, std::size_t bytes_transferred)
//  {
//      // 处理异步回调.
//  }
//  ...
//  avhttp::http_stream h(io);
//  async_read_body(h, "http://www.boost.org/LICENSE_1_0.txt",
//      boost::asio::buffer(data, 1024), handler);
//  io.run();
//  ...
// @end example
template<typename AsyncReadStream, typename MutableBufferSequence, typename Handler>
AVHTTP_DECL void async_read_body(AsyncReadStream& stream,
	const avhttp::url& url, MutableBufferSequence& buffers, Handler handler)
{
	detail::make_read_body_op(stream, url, buffers, handler);
}

} // namespace avhttp

#endif // AVHTTP_MISC_HTTP_ASYNC_READBODY_HPP
