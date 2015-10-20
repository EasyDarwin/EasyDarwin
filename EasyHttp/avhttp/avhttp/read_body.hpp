//
// async_read_body.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2014 Jack (jack dot wgm at gmail dot com)
// Copyright (C) 2012 - 2014  微蔡 <microcai@fedoraproject.org>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_MISC_HTTP_READBODY_HPP
#define AVHTTP_MISC_HTTP_READBODY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "avhttp/http_stream.hpp"
#include "avhttp/completion_condition.hpp"


namespace avhttp {
///用于http_stream访问url.
// 这个函数用于http_stream访问指定的url, 数据将保存在用户事先提供的buffers中.
// @注意:
//  1. 该函数返回条件为直到读取完整的body或eof或其它错误, 错误信息通过error_code传回.
// @param stream 一个http_stream对象.
// @param url 指定的url.
// @param buffers 一个或多个用于读取数据的缓冲区
// 这个类型必须满足MutableBufferSequence, MutableBufferSequence的定义.
// 具体可参考boost.asio文档中相应的描述.
template<typename AsyncReadStream, typename MutableBufferSequence>
AVHTTP_DECL std::size_t read_body(AsyncReadStream& stream,
	const avhttp::url& url, MutableBufferSequence& buffers, boost::system::error_code &ec)
{
	std::size_t readed = 0;
	stream.open(url, ec);
	if (ec)
		return -1;
	readed = boost::asio::read(stream, buffers, transfer_response_body(stream.content_length()), ec);

	if(ec == boost::asio::error::eof && stream.content_length() == -1)
		ec = boost::system::error_code();
	return readed;
}

} // namespace avhttp

#endif // AVHTTP_MISC_HTTP_READBODY_HPP

