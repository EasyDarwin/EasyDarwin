//
// upload.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003, Arvid Norberg All rights reserved.
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_UPLOAD_HPP
#define AVHTTP_UPLOAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/noncopyable.hpp>
#include "avhttp/http_stream.hpp"

BOOST_STATIC_ASSERT_MSG(BOOST_VERSION >= 105400, "You must use boost-1.54 or later!!!");

namespace avhttp {

// WebForm文件上传组件.
// 根据RFC1867(http://www.servlets.com/rfcs/rfc1867.txt)实现.
// @begin example
// 	boost::asio::io_service io;
// 	avhttp::file_upload upload(io);
// 	avhttp::file_upload::form_agrs fields;
// 	fields["username"] = "Cai";
// 	boost::system::error_code ec;
// 	upload.open("http://example.upload/upload", "cppStudy.tar.bz2",
// 		"file", fields, ec);
// 	if (ec)
// 	{
// 		// 处理错误.
// 	}
// 	// 开始上传文件数据.
// 	avhttp::default_storge file;
// 	file.open("\\root\\cppStudy.tar.bz2", ec);
// 	if (ec)
// 	{
// 		// 处理错误.
// 	}
//
// 	boost::array<char, 1024> buffer;
// 	while (!file.eof())
// 	{
// 		int readed = file.read(buffer.data(), 1024);
// 		boost::asio::write(upload, boost::asio::buffer(buffer, readed), ec);
// 		if (ec)
// 		{
// 			// 处理错误.
// 		}
// 	}
// 	upload.write_tail(ec);
// 	if (ec)
// 	{
// 		// 处理错误.
// 	}
//  ...
// @end example
class file_upload : public boost::noncopyable
{
public:
	typedef std::map<std::string, std::string> form_args;

	///Constructor.
	// @param io用户指定的io_service对象.
	// @param fake_continue指定启用fake-continue消息.
	// fake continue消息用于在http服务器不支持100的时候,
	// 在async_open/open打开时, 返回一个fake continue.
	AVHTTP_DECL explicit file_upload(boost::asio::io_service& io, bool fake_continue = false);

	/// Destructor.
	AVHTTP_DECL virtual ~file_upload();

	///异步打开文件上传.
	template <typename Handler>
	void async_open(const std::string& url, const std::string& filename,
		const std::string& file_of_form, const form_args& args, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///打开文件上传.
	// @param url指定上传文件的url.
	// @param filename指定的上传文件名.
	// @param file_of_form在web form中的上传文件名字段.
	// @param args其它各上传文件字段.
	// @param ec错误信息.
	// @begin example
	//  avhttp::file_upload f(io_service);
	//  file_upload::form_agrs fields;
	//  fields["username"] = "Cai";
	//  boost::system::error_code ec;
	//  f.open("http://example.upload/upload.cgi", "cppStudy.tar.bz2",
	//    "file", fields, ec);
	// @end example
	AVHTTP_DECL void open(const std::string& url, const std::string& filename,
		const std::string& file_of_form, const form_args& args, boost::system::error_code& ec);

	///打开文件上传.
	// @param url指定上传文件的url.
	// @param filename指定的上传文件名.
	// @param file_of_form在web form中的上传文件名字段.
	// @param args其它各上传文件字段.
	// 失败将抛出一个boost::system::system_error异常.
	// @begin example
	//  avhttp::file_upload f(io_service);
	//  file_upload::form_agrs fields;
	//  fields["username"] = "Cai";
	//  boost::system::error_code ec;
	//  f.open("http://example.upload/upload.cgi", "cppStudy.tar.bz2",
	//    "file", fields, ec);
	// @end example
	AVHTTP_DECL void open(const std::string& url, const std::string& filename,
		const std::string& file_of_form, const form_args& agrs);

	///发送一些上传的文件数据.
	// @param buffers是一个或多个用于发送数据缓冲. 这个类型必须满足ConstBufferSequence, 参考文档:
	// http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @返回实现发送的数据大小.
	// @备注: 该函数将会阻塞到一直等待数据被发送或发生错误时才返回.
	// write_some不保证发送完所有数据, 用户需要根据返回值来确定已经发送的数据大小.
	// @begin example
	//  try
	//  {
	//    std::size bytes_transferred = f.write_some(boost::asio::buffer(data, size));
	//  }
	//  catch (boost::asio::system_error& e)
	//  {
	//    std::cerr << e.what() << std::endl;
	//  }
	//  ...
	// @end example
	// 关于示例中的boost::asio::buffer用法可以参考boost中的文档. 它可以接受一个
	// boost.array或std.vector作为数据容器.
	template <typename ConstBufferSequence>
	std::size_t write_some(const ConstBufferSequence& buffers);

	///发送一些上传的文件数据.
	// @param buffers是一个或多个用于发送数据缓冲. 这个类型必须满足ConstBufferSequence, 参考文档:
	// http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @返回实现发送的数据大小.
	// @备注: 该函数将会阻塞到一直等待数据被发送或发生错误时才返回.
	// write_some不保证发送完所有数据, 用户需要根据返回值来确定已经发送的数据大小.
	// @begin example
	//  boost::system::error_code ec;
	//  std::size bytes_transferred = f.write_some(boost::asio::buffer(data, size), ec);
	//  ...
	// @end example
	// 关于示例中的boost::asio::buffer用法可以参考boost中的文档. 它可以接受一个
	// boost.array或std.vector作为数据容器.
	template <typename ConstBufferSequence>
	std::size_t write_some(const ConstBufferSequence& buffers,
		boost::system::error_code& ec);

	///异步上传一些文件数据.
	// @param buffers一个或多个用于读取数据的缓冲区, 这个类型必须满足ConstBufferSequence,
	//  ConstBufferSequence的定义在boost.asio文档中.
	// http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @param handler在发送操作完成或出现错误时, 将被回调, 它满足以下条件:
	// @begin code
	//  void handler(
	//    int bytes_transferred,				// 返回发送的数据字节数.
	//    const boost::system::error_code& ec	// 用于返回操作状态.
	//  );
	// @end code
	// @begin example
	//   void handler(int bytes_transferred, const boost::system::error_code& ec)
	//   {
	//		// 处理异步回调.
	//   }
	//   file_upload f(io_service);
	//   ...
	//   f.async_write_some(boost::asio::buffer(data, size), handler);
	//   ...
	// @end example
	// 关于示例中的boost::asio::buffer用法可以参考boost中的文档. 它可以接受一个
	// boost.array或std.vector作为数据容器.
	template <typename ConstBufferSequence, typename Handler>
	void async_write_some(const ConstBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///发送结尾行.
	// @param ec错误信息.
	AVHTTP_DECL void write_tail(boost::system::error_code& ec);

	///发送结尾行.
	// 失败将抛出一个boost::system::system_error异常.
	AVHTTP_DECL void write_tail();

	///异步发送结尾行.
	// @param handler在发送操作完成或出现错误时, 将被回调, 它满足以下条件:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec // 用于返回操作状态.
	//  );
	// @end code
	// @begin example
	//  void tail_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // 发送成功!
	//    }
	//  }
	//  ...
	//  avhttp::file_upload f(io_service);
	//  ...
	//  f.async_write_tail(handler);
	// @end example
	// @备注: handler也可以使用boost.bind来绑定一个符合规定的函数作
	// 为async_open的参数handler.
	template <typename Handler>
	void async_write_tail(BOOST_ASIO_MOVE_ARG(Handler) handler);

	///设置http header选项.
	AVHTTP_DECL void request_option(request_opts& opts);

	///返回http_stream对象的引用.
	AVHTTP_DECL http_stream& get_http_stream();

	///反回当前file_upload所使用的io_service的引用.
	AVHTTP_DECL boost::asio::io_service& get_io_service();

private:

	template <typename Handler>
	struct open_coro;

	template <typename Handler>
	struct tail_coro;

	///辅助函数，用于创建协程并进行 Templet type deduction.
	template <typename Handler>
	open_coro<Handler> make_open_coro(const std::string& url, const std::string& filename,
		const std::string& file_of_form, const form_args& args, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///辅助函数，用于创建协程并进行 Templet type deduction.
	template <typename Handler>
	tail_coro<Handler> make_tail_coro(BOOST_ASIO_MOVE_ARG(Handler) handler);

private:

	// io_service引用.
	boost::asio::io_service& m_io_service;

	// http_stream对象.
	http_stream m_http_stream;

	// 边界符.
	std::string m_boundary;
	std::string m_base_boundary;

	// 表单参数.
	form_args m_form_args;

	// 是否启用fake-continue消息.
	// fake continue消息用于在http服
	// 务器不支持100的时候, 在
	// async_open/open打开时, 返回一个
	// fake continue.
	bool m_fake_continue;
};

} // namespace avhttp

#if defined(AVHTTP_HEADER_ONLY)
#	include "avhttp/impl/file_upload.ipp"
#endif

#endif // AVHTTP_UPLOAD_HPP
