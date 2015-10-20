//
// http_stream.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __HTTP_STREAM_HPP__
#define __HTTP_STREAM_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>

#include "url.hpp"
#include "settings.hpp"
#include "detail/io.hpp"
#include "detail/parsers.hpp"
#include "detail/error_codec.hpp"
#ifdef AVHTTP_ENABLE_OPENSSL
#include "detail/ssl_stream.hpp"
#endif
#include "detail/socket_type.hpp"

namespace avhttp {

// 一个http流类实现, 用于同步或异步访问一个指定的url上的数据.
// 目前支持http/https协议.
// @备注: 该类http_stream的对象非线程安全!
// 以下是同步方式访问一个url中的数据使用示例.
// @begin example
//  try
//  {
//  	boost::asio::io_service io_service;
//  	avhttp::http_stream h(io_service);
//  	avhttp::request_opts opt;
//
//  	// 设置请求选项.
//  	opt.insert("Connection", "close");
//  	h.request_options(opt);
//  	h.open("http://www.boost.org/LICENSE_1_0.txt");
//
//  	boost::system::error_code ec;
//  	while (!ec)
//  	{
//  		char data[1024];
//  		std::size_t bytes_transferred = h.read_some(boost::asio::buffer(data, 1024), ec);
//			// 如果要读取指定大小的数据, 可以使用boost::asio::read, 如下:
//			// std::size_t bytes_transferred = boost::asio::read(h, boost::asio::buffer(buf), ec);
//  		std::cout.write(data, bytes_transferred);
//  	}
//  }
//  catch (std::exception& e)
//  {
//  	std::cerr << "Exception: " << e.what() << std::endl;
//  }
// @end example
//
// 以下是异步方式访问一个url中的数据使用示例.
// @begin example
//  class downloader
//  {
//  public:
//  	downloader(boost::asio::io_service &io)
//  		: m_io_service(io)
//  		, m_stream(io)
//  	{
//  		// 设置请求选项.
//  		avhttp::request_opts opt;
//  		opt.insert("Connection", "close");
//  		m_stream.request_options(opt);
//			// 发起异步请求.
//  		m_stream.async_open("http://www.boost.org/LICENSE_1_0.txt",
//  			boost::bind(&downloader::handle_open, this, boost::asio::placeholders::error));
//  	}
//  	~downloader()
//  	{}
//  
//  public:
//  	void handle_open(const boost::system::error_code &ec)
//  	{
//  		if (!ec)
//  		{
//  			m_stream.async_read_some(boost::asio::buffer(m_buffer),
//  				boost::bind(&downloader::handle_read, this,
//  				boost::asio::placeholders::bytes_transferred,
//  				boost::asio::placeholders::error));
//				// 在这里也支持使用boost::asio::async_read来读取一定量大小的数据, 用法用boost.asio, 比如:
//				boost::asio::async_read(m_stream, boost::asio::buffer(m_buffer),
// 					boost::bind(&downloader::handle_read, this,
// 					boost::asio::placeholders::bytes_transferred,
// 					boost::asio::placeholders::error));
//  		}
//  	}
//  
//  	void handle_read(int bytes_transferred, const boost::system::error_code &ec)
//  	{
//  		if (!ec)
//  		{
//  			std::cout.write(m_buffer.data(), bytes_transferred);
//  			m_stream.async_read_some(boost::asio::buffer(m_buffer),
//  				boost::bind(&downloader::handle_read, this,
//  				boost::asio::placeholders::bytes_transferred,
//  				boost::asio::placeholders::error));
//  		}
//  	}
//  
//  private:
//  	boost::asio::io_service &m_io_service;
//  	avhttp::http_stream m_stream;
//  	boost::array<char, 1024> m_buffer;
//  };
//
//  int main(int argc, char* argv[])
//  {
//		boost::asio::io_service io;
//		downloader d(io);
//		io.run();
//		return 0;
//  }
// @end example


using boost::asio::ip::tcp;

class http_stream : public boost::noncopyable
{
	// 定义socket_type类型, socket_type是variant_stream的重定义, 它的作用
	// 可以为ssl_socket或nossl_socket, 这样, 在访问socket的时候, 就不需要
	// 区别编写不同的代码.
#ifdef AVHTTP_ENABLE_OPENSSL
	typedef avhttp::detail::ssl_stream<tcp::socket&> ssl_socket;
#endif
	typedef tcp::socket nossl_socket;
	typedef avhttp::detail::variant_stream<
		nossl_socket
#ifdef AVHTTP_ENABLE_OPENSSL
		, ssl_socket
#endif
	> socket_type;

public:
	http_stream(boost::asio::io_service &io)
		: m_io_service(io)
		, m_check_certificate(true)
		, m_sock(io)
		, m_nossl_socket(io)
		, m_keep_alive(false)
		, m_status_code(-1)
		, m_redirects(0)
		, m_content_length(0)
		, m_resolver(io)
	{
		m_proxy.type = proxy_settings::none;
	}

	virtual ~http_stream()
	{}

public:
	// 返回 http_stream 使用的 io_service　引用.
	boost::asio::io_service & get_io_service()
	{
		return m_io_service;
	}

	///打开一个指定的url.
	// 失败将抛出一个boost::system::system_error异常.
	// @param u 将要打开的URL.
	// @begin example
	//   avhttp::http_stream h_stream(io_service);
	//   try
	//   {
	//     h_stream.open("http://www.boost.org");
	//   }
	//   catch (boost::system::system_error& e)
	//   {
	//     std::cerr << e.waht() << std::endl;
	//   }
	// @end example
	void open(const url &u)
	{
		boost::system::error_code ec;
		open(u, ec);
		if (ec)
		{
			boost::throw_exception(boost::system::system_error(ec));
		}
	}

	///打开一个指定的url.
	// @param u 将要打开的URL.
	// 通过ec引用获得执行状态.
	// @begin example
	//   avhttp::http_stream h_stream(io_service);
	//   boost::system::error_code ec;
	//   h_stream.open("http://www.boost.org", ec);
	//   if (ec)
	//   {
	//     std::cerr << e.waht() << std::endl;
	//   }
	// @end example
	void open(const url &u, boost::system::error_code &ec)
	{
		const std::string protocol = u.protocol();

		// 保存url.
		m_url = u;

		// 清空一些选项.
		m_content_type = "";
		m_status_code = 0;
		m_content_length = 0;
		m_content_type = "";
		m_request.consume(m_request.size());
		m_response.consume(m_response.size());
		m_protocol = "";

		// 获得请求的url类型.
		if (protocol == "http")
		{
			m_protocol = "http";
		}
#ifdef AVHTTP_ENABLE_OPENSSL
		else if (protocol == "https")
		{
			m_protocol = "https";
		}
#endif
		else
		{
			ec = boost::asio::error::operation_not_supported;
			return;
		}

		// 构造socket.
		if (!m_sock.instantiated())
		{
			if (protocol == "http")
			{
				m_sock.instantiate<nossl_socket>(m_io_service);
			}
#ifdef AVHTTP_ENABLE_OPENSSL
			else if (protocol == "https")
			{
				m_sock.instantiate<ssl_socket>(m_nossl_socket);
			}
#endif
			else
			{
				ec = boost::asio::error::operation_not_supported;
				return;
			}
		}

		// 开始进行连接.
		if (m_sock.instantiated() && !m_sock.is_open())
		{
			if (m_proxy.type == proxy_settings::none)
			{
				// 开始解析端口和主机名.
				tcp::resolver resolver(m_io_service);
				std::ostringstream port_string;
				port_string << m_url.port();
				tcp::resolver::query query(m_url.host(), port_string.str());
				tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
				tcp::resolver::iterator end;

				// 尝试连接解析出来的服务器地址.
				ec = boost::asio::error::host_not_found;
				while (ec && endpoint_iterator != end)
				{
					m_sock.close(ec);
					m_sock.connect(*endpoint_iterator++, ec);
				}
				if (ec)
				{
					return;
				}
			}
			else if (m_proxy.type == proxy_settings::socks5 ||
				proxy_settings::socks4 || proxy_settings::socks5_pw)	// socks代理.
			{
				if (protocol == "http")
				{
					socks_proxy_connect(m_sock, ec);
					if (ec)
						return;
				}
#ifdef AVHTTP_ENABLE_OPENSSL
				else if (protocol == "https")
				{
					socks_proxy_connect(m_nossl_socket, ec);
					if (ec)
						return;
				}
#endif
				// 和代理服务器连接握手完成.
			}
			else
			{
				// TODO: 暂时未实现其它代理功能.
				ec = boost::asio::error::operation_not_supported;
				return;
			}

			// 禁用Nagle在socket上.
			m_sock.set_option(tcp::no_delay(true), ec);
			if (ec)
			{
				return;
			}

#ifdef AVHTTP_ENABLE_OPENSSL
			if (m_protocol == "https")
			{
				// 认证证书.
				if (m_check_certificate)
				{
					ssl_socket *ssl_sock =  m_sock.get<ssl_socket>();
					if (X509* cert = SSL_get_peer_certificate(ssl_sock->impl()->ssl))
					{
						if (SSL_get_verify_result(ssl_sock->impl()->ssl) == X509_V_OK)
						{
							if (certificate_matches_host(cert, m_url.host()))
								ec = boost::system::error_code();
							else
								ec = make_error_code(boost::system::errc::permission_denied);
						}
						else
							ec = make_error_code(boost::system::errc::permission_denied);
						X509_free(cert);
					}
				}
			}
#endif
		}
		else
		{
			// socket已经打开.
			ec = boost::asio::error::already_open;
			return;
		}

		boost::system::error_code http_code;

		// 发出请求.
		request(m_request_opts, http_code);

		// 判断是否需要跳转.
		if (http_code == avhttp::errc::moved_permanently || http_code == avhttp::errc::found)
		{
			m_sock.close(ec);
			if (++m_redirects <= AVHTTP_MAX_REDIRECTS)
			{
				open(m_location, ec);
				return;
			}
		}

		// 根据http状态码来构造.
		if (http_code)
			ec = http_code;
		else
			ec = boost::system::error_code();	// 打开成功.

		return;
	}

	///异步打开一个指定的URL.
	// @param u 将要打开的URL.
	// @param handler 将被调用在打开完成时. 它必须满足以下条件:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec // 用于返回操作状态.
	//  );
	// @end code
	// @begin example
	//  void open_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // 打开成功!
	//    }
	//  }
	//  ...
	//  avhttp::http_stream h_stream(io_service);
	//  h_stream.async_open("http://www.boost.org", open_handler);
	// @end example
	// @备注: handler也可以使用boost.bind来绑定一个符合规定的函数作
	// 为async_open的参数handler.
	template <typename Handler>
	void async_open(const url &u, Handler handler)
	{
		const std::string protocol = u.protocol();

		// 保存url.
		m_url = u;

		// 清空一些选项.
		m_content_type = "";
		m_status_code = 0;
		m_content_length = 0;
		m_content_type = "";
		m_request.consume(m_request.size());
		m_response.consume(m_response.size());
		m_protocol = "";

		// 获得请求的url类型.
		if (protocol == "http")
			m_protocol = "http";
#ifdef AVHTTP_ENABLE_OPENSSL
		else if (protocol == "https")
			m_protocol = "https";
#endif
		else
		{
			m_io_service.post(boost::asio::detail::bind_handler(
				handler, boost::asio::error::operation_not_supported));
			return;
		}

		// 构造socket.
		if (!m_sock.instantiated())
		{
			if (protocol == "http")
			{
				m_sock.instantiate<nossl_socket>(m_io_service);
			}
#ifdef AVHTTP_ENABLE_OPENSSL
			else if (protocol == "https")
			{
				m_sock.instantiate<ssl_socket>(m_nossl_socket);
			}
#endif
			else
			{
				m_io_service.post(boost::asio::detail::bind_handler(
					handler, boost::asio::error::operation_not_supported));
				return;
			}
		}

		// 判断socket是否打开.
		if (m_sock.instantiated() && m_sock.is_open())
		{
			m_io_service.post(boost::asio::detail::bind_handler(
				handler, boost::asio::error::already_open));
			return;
		}

		// 异步代理功能.
		if (m_proxy.type != proxy_settings::none)
		{
			// HTTP代理协议未实现.
			if (m_proxy.type != proxy_settings::socks4 && m_proxy.type != proxy_settings::socks5
				&& m_proxy.type != proxy_settings::socks5_pw)
			{
				m_io_service.post(boost::asio::detail::bind_handler(
					handler, boost::asio::error::operation_not_supported));
				return;
			}

			if (protocol == "http")
			{
				async_socks_proxy_connect(m_sock, handler);
			}
#ifdef AVHTTP_ENABLE_OPENSSL
			else if (protocol == "https")
			{
				async_socks_proxy_connect(m_nossl_socket, handler);
			}
#endif

			return;
		}

		// 构造异步查询HOST.
		std::ostringstream port_string;
		port_string << m_url.port();
		tcp::resolver::query query(m_url.host(), port_string.str());

		// 开始异步查询HOST信息.
		typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
		m_resolver.async_resolve(query,
			boost::bind(&http_stream::handle_resolve<HandlerWrapper>, this,
			boost::asio::placeholders::error, boost::asio::placeholders::iterator, HandlerWrapper(handler)));
	}

	///从这个http_stream中读取一些数据.
	// @param buffers一个或多个读取数据的缓冲区, 这个类型必须满足MutableBufferSequence,
	// MutableBufferSequence的定义在boost.asio文档中.
	// @函数返回读取到的数据大小.
	// @失败将抛出boost::asio::system_error异常.
	// @备注: 该函数将会阻塞到一直等待有数据或发生错误时才返回.
	// read_some不能读取指定大小的数据.
	// @begin example
	//  try
	//  {
	//    std::size bytes_transferred = s.read_some(boost::asio::buffer(data, size));
	//  } catch (boost::asio::system_error &e)
	//  {
	//    std::cerr << e.what() << std::endl;
	//  }
	//  ...
	// @end example
	template <typename MutableBufferSequence>
	std::size_t read_some(const MutableBufferSequence &buffers)
	{
		boost::system::error_code ec;
		std::size_t bytes_transferred = read_some(buffers, ec);
		if (ec)
		{
			boost::throw_exception(boost::system::system_error(ec));
		}
		return bytes_transferred;
	}

	///从这个http_stream读取一些数据.
	// @param buffers一个或多个用于读取数据的缓冲区, 这个类型必须满足
	// MutableBufferSequence, MutableBufferSequence的定义在boost.asio
	// 文档中.
	// @param ec在发生错误时, 将传回错误信息.
	// @函数返回读取到的数据大小.
	// @备注: 该函数将会阻塞到一直等待有数据或发生错误时才返回.
	// read_some不能读取指定大小的数据.
	// @begin example
	//  boost::system::error_code ec;
	//  std::size bytes_transferred = s.read_some(boost::asio::buffer(data, size), ec);
	//  ...
	// @end example
	// 关于示例中的boost::asio::buffer用法可以参考boost中的文档. 它可以接受一个
	// boost.array或std.vector作为数据容器.
	template <typename MutableBufferSequence>
	std::size_t read_some(const MutableBufferSequence &buffers,
		boost::system::error_code &ec)
	{
		// 如果还有数据在m_response中, 先读取m_response中的数据.
		if (m_response.size() > 0)
		{
			std::size_t bytes_transferred = 0;
			typename MutableBufferSequence::const_iterator iter = buffers.begin();
			typename MutableBufferSequence::const_iterator end = buffers.end();
			for (; iter != end && m_response.size() > 0; ++iter)
			{
				boost::asio::mutable_buffer buffer(*iter);
				size_t length = boost::asio::buffer_size(buffer);
				if (length > 0)
				{
					bytes_transferred += m_response.sgetn(
						boost::asio::buffer_cast<char*>(buffer), length);
				}
			}
			ec = boost::system::error_code();
			return bytes_transferred;
		}

		// 再从socket中读取数据.
		std::size_t bytes_transferred = m_sock.read_some(buffers, ec);
		if (ec == boost::asio::error::shut_down)
			ec = boost::asio::error::eof;
		return bytes_transferred;
	}

	///从这个http_stream异步读取一些数据.
	// @param buffers一个或多个用于读取数据的缓冲区, 这个类型必须满足MutableBufferSequence,
	//  MutableBufferSequence的定义在boost.asio文档中.
	// http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/reference/MutableBufferSequence.html
	// @param handler在读取操作完成或出现错误时, 将被回调, 它满足以下条件:
	// @begin code
	//  void handler(
	//    int bytes_transferred,				// 返回读取的数据字节数.
	//    const boost::system::error_code& ec	// 用于返回操作状态.
	//  );
	// @end code
	// @begin example
	//   void handler(int bytes_transferred, const boost::system::error_code& ec)
	//   {
	//		// 处理异步回调.
	//   }
	//   http_stream h(io_service);
	//   ...
	//   boost::array<char, 1024> buffer;
	//   boost::asio::async_read(h, boost::asio::buffer(buffer), handler);
	//   ...
	// @end example
	// 关于示例中的boost::asio::buffer用法可以参考boost中的文档. 它可以接受一个
	// boost.array或std.vector作为数据容器.
	template <typename MutableBufferSequence, typename Handler>
	void async_read_some(const MutableBufferSequence& buffers, Handler handler)
	{
		boost::system::error_code ec;
		if (m_response.size() > 0)
		{
			std::size_t bytes_transferred = read_some(buffers, ec);
			m_io_service.post(
				boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
			return;
		}
		// 当缓冲区数据不够, 直接从socket中异步读取.
		m_sock.async_read_some(buffers, handler);
	}

	///向这个http_stream中发送一些数据.
	// @param buffers是一个或多个用于发送数据缓冲. 这个类型必须满足ConstBufferSequence, 参考文档:
	// http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @返回实现发送的数据大小.
	// @备注: 该函数将会阻塞到一直等待数据被发送或发生错误时才返回.
	// write_some不保证发送完所有数据, 用户需要根据返回值来确定已经发送的数据大小.
	// @begin example
	//  try
	//  {
	//    std::size bytes_transferred = s.write_some(boost::asio::buffer(data, size));
	//  }
	//  catch (boost::asio::system_error &e)
	//  {
	//    std::cerr << e.what() << std::endl;
	//  }
	//  ...
	// @end example
	// 关于示例中的boost::asio::buffer用法可以参考boost中的文档. 它可以接受一个
	// boost.array或std.vector作为数据容器.
	template <typename ConstBufferSequence>
	std::size_t write_some(const ConstBufferSequence& buffers)
	{
		boost::system::error_code ec;
		std::size_t bytes_transferred = write_some(buffers, ec);
		if (ec)
		{
			boost::throw_exception(boost::system::system_error(ec));
		}
		return bytes_transferred;
	}

	///向这个http_stream中发送一些数据.
	// @param buffers是一个或多个用于发送数据缓冲. 这个类型必须满足ConstBufferSequence, 参考文档:
	// http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @返回实现发送的数据大小.
	// @备注: 该函数将会阻塞到一直等待数据被发送或发生错误时才返回.
	// write_some不保证发送完所有数据, 用户需要根据返回值来确定已经发送的数据大小.
	// @begin example
	//  boost::system::error_code ec;
	//  std::size bytes_transferred = s.write_some(boost::asio::buffer(data, size), ec);
	//  ...
	// @end example
	// 关于示例中的boost::asio::buffer用法可以参考boost中的文档. 它可以接受一个
	// boost.array或std.vector作为数据容器.
	template <typename ConstBufferSequence>
	std::size_t write_some(const ConstBufferSequence& buffers,
		boost::system::error_code &ec)
	{
		std::size_t bytes_transferred = m_sock.write_some(buffers, ec);
		if (ec == boost::asio::error::shut_down)
			ec = boost::asio::error::eof;
		return bytes_transferred;
	}

	///从这个http_stream异步发送一些数据.
	// @param buffers一个或多个用于读取数据的缓冲区, 这个类型必须满足ConstBufferSequence,
	//  ConstBufferSequence的定义在boost.asio文档中.
	// http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/reference/ConstBufferSequence.html
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
	//   http_stream h(io_service);
	//   ...
	//   h.async_write_some(boost::asio::buffer(data, size), handler);
	//   ...
	// @end example
	// 关于示例中的boost::asio::buffer用法可以参考boost中的文档. 它可以接受一个
	// boost.array或std.vector作为数据容器.
	template <typename ConstBufferSequence, typename Handler>
	void async_write_some(const ConstBufferSequence &buffers, Handler handler)
	{
		m_sock.async_write_some(buffers, handler);
	}

	///向http服务器发起一个请求.
	// @向http服务器发起一个请求, 如果失败抛出异常.
	// @param opt是向服务器发起请求的选项信息.
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  ...
	//  request_opts opt;
	//  opt.insert("cookie", "name=admin;passwd=#@aN@2*242;");
	//  ...
	//  h.request(opt);
	// @end example
	void request(request_opts &opt)
	{
		boost::system::error_code ec;
		request(opt, ec);
		if (ec)
		{
			boost::throw_exception(boost::system::system_error(ec));
		}
	}

	///向http服务器发起一个请求.
	// @param opt是向服务器发起请求的选项信息.
	// @param ec在发生错误时, 将传回错误信息.
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  ...
	//  request_opts opt;
	//  boost::system::error_code ec;
	//  opt.insert("cookie", "name=admin;passwd=#@aN@2*242;");
	//  ...
	//  h.request(opt, ec);
	//  ...
	// @end example
	void request(request_opts &opt, boost::system::error_code &ec)
	{
		// 判断socket是否打开.
		if (!m_sock.is_open())
		{
			ec = boost::asio::error::network_reset;
			return;
		}

		// 保存到一个新的opts中操作.
		request_opts opts = opt;

		// 得到request_method.
		std::string request_method = "GET";
		if (opts.find("_request_method", request_method))
			opts.remove("_request_method");	// 删除处理过的选项.

		// 得到Host信息.
		std::string host = m_url.to_string(url::host_component | url::port_component);
		if (opts.find("Host", host))
			opts.remove("Host");	// 删除处理过的选项.

		// 得到Accept信息.
		std::string accept = "*/*";
		if (opts.find("Accept", accept))
			opts.remove("Accept");	// 删除处理过的选项.


		// 是否带有body选项.
		std::string body;
		if (opts.find("_request_body", body))
			opts.remove("_request_body");	// 删除处理过的选项.

		// 循环构造其它选项.
		std::string other_option_string;
		request_opts::option_item_list &list = opts.option_all();
		for (request_opts::option_item_list::iterator val = list.begin(); val != list.end(); val++)
		{
			other_option_string += (val->first + ": " + val->second + "\r\n");
		}

		// 整合各选项到Http请求字符串中.
		std::string request_string;
		m_request.consume(m_request.size());
		std::ostream request_stream(&m_request);
		request_stream << request_method << " ";
		request_stream << m_url.to_string(url::path_component | url::query_component);
		request_stream << " HTTP/1.0\r\n";
		request_stream << "Host: " << host << "\r\n";
		request_stream << "Accept: " << accept << "\r\n";
		request_stream << other_option_string << "\r\n";
		if (!body.empty())
		{
			request_stream << body;
		}

		// 发送请求.
		boost::asio::write(m_sock, m_request, ec);
		if (ec)
		{
			return;
		}

		// 循环读取.
		for (;;)
		{
			boost::asio::read_until(m_sock, m_response, "\r\n", ec);
			if (ec)
			{
				return;
			}

			// 检查http状态码, version_major和version_minor是http协议的版本号.
			int version_major = 0;
			int version_minor = 0;
			m_status_code = 0;
			if (!detail::parse_http_status_line(
				std::istreambuf_iterator<char>(&m_response),
				std::istreambuf_iterator<char>(),
				version_major, version_minor, m_status_code))
			{
				ec = avhttp::errc::malformed_status_line;
				return;
			}

			// 如果http状态代码不是ok或partial_content, 根据status_code构造一个http_code, 后面
			// 需要判断http_code是不是302等跳转, 如果是, 则将进入跳转逻辑; 如果是http发生了错误
			// , 则直接返回这个状态构造的.
			if (m_status_code != avhttp::errc::ok &&
				m_status_code != avhttp::errc::partial_content)
			{
				ec = make_error_code(static_cast<avhttp::errc::errc_t>(m_status_code));
			}

			// "continue"表示我们需要继续等待接收状态.
			if (m_status_code != avhttp::errc::continue_request)
				break;
		} // end for.

		// 清除原有的返回选项.
		m_response_opts.clear();
		// 添加状态码.
		m_response_opts.insert("_status_code", boost::str(boost::format("%d") % m_status_code));

		// 接收掉所有Http Header.
		boost::system::error_code read_err;
		std::size_t bytes_transferred = boost::asio::read_until(m_sock, m_response, "\r\n\r\n", read_err);
		if (read_err)
		{
			// 说明读到了结束还没有得到Http header, 返回错误的文件头信息而不返回eof.
			if (read_err == boost::asio::error::eof)
				ec = avhttp::errc::malformed_response_headers;
			else
				ec = read_err;
			return;
		}

		std::string header_string;
		header_string.resize(bytes_transferred);
		m_response.sgetn(&header_string[0], bytes_transferred);

		// 解析Http Header.
		if (!detail::parse_http_headers(header_string.begin(), header_string.end(),
			m_content_type, m_content_length, m_location, m_response_opts.option_all()))
		{
			ec = avhttp::errc::malformed_response_headers;
			return;
		}
	}

	///向http服务器发起一个异步请求.
	// @param opt指定的http请求选项.
	// @param handler 将被调用在打开完成时. 它必须满足以下条件:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec	// 用于返回操作状态.
	//  );
	// @end code
	// @begin example
	//  void request_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // 请求成功!
	//    }
	//  }
	//  ...
	//  avhttp::http_stream h(io_service);
	//  ...
	//  request_opts opt;
	//  opt.insert("cookie", "name=admin;passwd=#@aN@2*242;");
	//  h.async_request(opt, boost::bind(&request_handler, boost::asio::placeholders::error));
	// @end example
	template <typename Handler>
	void async_request(request_opts &opt, Handler handler)
	{
		boost::system::error_code ec;

		// 判断socket是否打开.
		if (!m_sock.is_open())
		{
			handler(boost::asio::error::network_reset);
			return;
		}

		// 保存到一个新的opts中操作.
		request_opts opts = opt;

		// 得到request_method.
		std::string request_method = "GET";
		if (opts.find("_request_method", request_method))
			opts.remove("_request_method");	// 删除处理过的选项.

		// 得到Host信息.
		std::string host = m_url.to_string(url::host_component | url::port_component);
		if (opts.find("Host", host))
			opts.remove("Host");	// 删除处理过的选项.

		// 得到Accept信息.
		std::string accept = "*/*";
		if (opts.find("Accept", accept))
			opts.remove("Accept");	// 删除处理过的选项.


		// 是否带有body选项.
		std::string body;
		if (opts.find("_request_body", body))
			opts.remove("_request_body");	// 删除处理过的选项.

		// 循环构造其它选项.
		std::string other_option_string;
		request_opts::option_item_list &list = opts.option_all();
		for (request_opts::option_item_list::iterator val = list.begin(); val != list.end(); val++)
		{
			other_option_string += (val->first + ": " + val->second + "\r\n");
		}

		// 整合各选项到Http请求字符串中.
		std::string request_string;
		m_request.consume(m_request.size());
		std::ostream request_stream(&m_request);
		request_stream << request_method << " ";
		request_stream << m_url.to_string(url::path_component | url::query_component);
		request_stream << " HTTP/1.0\r\n";
		request_stream << "Host: " << host << "\r\n";
		request_stream << "Accept: " << accept << "\r\n";
		request_stream << other_option_string << "\r\n";
		if (!body.empty())
		{
			request_stream << body;
		}

		// 异步发送请求.
		typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
		boost::asio::async_write(m_sock, m_request, boost::asio::transfer_exactly(m_request.size()),
			boost::bind(&http_stream::handle_request<HandlerWrapper>, this,
			HandlerWrapper(handler), boost::asio::placeholders::error));
	}

	///清除读写缓冲区数据.
	// @备注: 非线程安全! 不应在正在进行读写操作时进行该操作!
	void clear()
	{
		m_request.consume(m_request.size());
		m_response.consume(m_response.size());
	}

	///关闭http_stream.
	// @失败抛出asio::system_error异常.
	// @备注: 停止所有正在进行的读写操作, 正在进行的异步调用将回调
	// boost::asio::error::operation_aborted错误.
	void close()
	{
		boost::system::error_code ec;
		close(ec);
		if (ec)
		{
			boost::throw_exception(boost::system::system_error(ec));
		}
	}

	///关闭http_stream.
	// @param ec保存失败信息.
	// @备注: 停止所有正在进行的读写操作, 正在进行的异步调用将回调
	// boost::asio::error::operation_aborted错误.
	void close(boost::system::error_code& ec)
	{
		ec = boost::system::error_code();

		if (is_open())
		{
			// 关闭socket.
			m_sock.close(ec);

			// 清空内部的各种缓冲信息.
			m_request.consume(m_request.size());
			m_response.consume(m_response.size());
			m_content_type.clear();
			m_location.clear();
			m_protocol.clear();
		}
	}

	///判断是否打开.
	// @返回是否打开.
	bool is_open() const
	{
		return m_sock.is_open();
	}

	///设置代理, 通过设置代理访问http服务器.
	// @param s 指定了代理参数.
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  proxy_settings s;
	//  s.type = socks5;
	//  s.hostname = "example.proxy.com";
	//  s.port = 8080;
	//  h.proxy(s);
	//  ...
	// @end example
	void proxy(const proxy_settings &s)
	{
		m_proxy = s;
	}

	///设置请求时的http选项.
	// @param options 为http的选项. 目前有以下几项特定选项:
	//  _request_method, 取值 "GET/POST/HEAD", 默认为"GET".
	//  Host, 取值为http服务器, 默认为http服务器.
	//  Accept, 取值任意, 默认为"*/*".
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  request_opts options;
	//  options.insert("_request_method", "POST"); // 默认为GET方式.
	//  h.request_options(options);
	//  ...
	// @end example
	void request_options(const request_opts& options)
	{
		m_request_opts = options;
	}

	///返回请求时的http选项.
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  request_opts options;
	//  options = h.request_options();
	//  ...
	// @end example
	request_opts request_options(void) const
	{
		return m_request_opts;
	}

	///http服务器回复选项.
	// @返回服务器回复的所有选项信息, key/value形式.
	response_opts response_options(void) const
	{
		return m_response_opts;
	}

	///返回location.
	// @返回location信息, 如果没有则返回空串.
	const std::string& location() const
	{
		return m_location;
	}

	///设置是否认证服务器证书.
	// @param is_check 如果为true表示认证服务器证书, 如果为false表示不认证服务器证书.
	// 默认为认证服务器证书.
	void check_certificate(bool is_check)
	{
#ifdef AVHTTP_ENABLE_OPENSSL
		m_check_certificate = is_check;
#endif
	}

protected:

	// 异步处理模板成员的相关实现.

	template <typename Handler>
	void handle_resolve(const boost::system::error_code &err,
		tcp::resolver::iterator endpoint_iterator, Handler handler)
	{
		if (!err)
		{
			// 发起异步连接.
			// !!!备注: 由于m_sock可能是ssl, 那么连接的握手相关实现被封装到ssl_stream
			// 了, 所以, 如果需要使用boost::asio::async_connect的话, 需要在http_stream
			// 中实现握手操作, 否则将会得到一个错误.
			m_sock.async_connect(tcp::endpoint(*endpoint_iterator),
				boost::bind(&http_stream::handle_connect<Handler>, this,
				handler, endpoint_iterator, boost::asio::placeholders::error));
		}
		else
		{
			// 出错回调.
			handler(err);
		}
	}

	template <typename Handler>
	void handle_connect(Handler handler,
		tcp::resolver::iterator endpoint_iterator, const boost::system::error_code &err)
	{
		if (!err)
		{
#ifdef AVHTTP_ENABLE_OPENSSL
			if (m_protocol == "https")
			{
				// 认证证书.
				if (m_check_certificate)
				{
					boost::system::error_code ec;
					ssl_socket *ssl_sock =  m_sock.get<ssl_socket>();
					if (X509* cert = SSL_get_peer_certificate(ssl_sock->impl()->ssl))
					{
						if (SSL_get_verify_result(ssl_sock->impl()->ssl) == X509_V_OK)
						{
							if (certificate_matches_host(cert, m_url.host()))
								ec = boost::system::error_code();
							else
								ec = make_error_code(boost::system::errc::permission_denied);
						}
						else
							ec = make_error_code(boost::system::errc::permission_denied);
						X509_free(cert);
					}
					
					if (ec)
					{
						handler(ec);
						return;
					}
				}
			}
#endif
			// 发起异步请求.
			async_request(m_request_opts, handler);
		}
		else
		{
			// 检查是否已经尝试了endpoint列表中的所有endpoint.
			if (++endpoint_iterator == tcp::resolver::iterator())
				handler(err);
			else
			{
				// 继续发起异步连接.
				// !!!备注: 由于m_sock可能是ssl, 那么连接的握手相关实现被封装到ssl_stream
				// 了, 所以, 如果需要使用boost::asio::async_connect的话, 需要在http_stream
				// 中实现握手操作, 否则将会得到一个错误.
				m_sock.async_connect(tcp::endpoint(*endpoint_iterator),
					boost::bind(&http_stream::handle_connect<Handler>, this,
					handler, endpoint_iterator, boost::asio::placeholders::error));
			}
		}
	}

	template <typename Handler>
	void handle_request(Handler handler, const boost::system::error_code &err)
	{
		// 发生错误.
		if (err)
		{
			handler(err);
			return;
		}

		// 异步读取Http status.
		boost::asio::async_read_until(m_sock, m_response, "\r\n",
			boost::bind(&http_stream::handle_status<Handler>, this, handler, boost::asio::placeholders::error));
	}

	template <typename Handler>
	void handle_status(Handler handler, const boost::system::error_code &err)
	{
		// 发生错误.
		if (err)
		{
			handler(err);
			return;
		}

		// 检查http状态码, version_major和version_minor是http协议的版本号.
		int version_major = 0;
		int version_minor = 0;
		m_status_code = 0;
		if (!detail::parse_http_status_line(
			std::istreambuf_iterator<char>(&m_response),
			std::istreambuf_iterator<char>(),
			version_major, version_minor, m_status_code))
		{
			handler(avhttp::errc::malformed_status_line);
			return;
		}

		// "continue"表示我们需要继续等待接收状态.
		if (m_status_code == avhttp::errc::continue_request)
		{
			boost::asio::async_read_until(m_sock, m_response, "\r\n",
				boost::bind(&http_stream::handle_status<Handler>, this, handler, boost::asio::placeholders::error));
		}
		else
		{
			// 清除原有的返回选项.
			m_response_opts.clear();
			// 添加状态码.
			m_response_opts.insert("_status_code", boost::str(boost::format("%d") % m_status_code));

			// 异步读取所有Http header部分.
			boost::asio::async_read_until(m_sock, m_response, "\r\n\r\n",
				boost::bind(&http_stream::handle_header<Handler>, this, handler,
				boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error));
		}
	}

	template <typename Handler>
	void handle_header(Handler handler, int bytes_transferred, const boost::system::error_code &err)
	{
		if (err)
		{
			handler(err);
			return;
		}

		std::string header_string;
		header_string.resize(bytes_transferred);
		m_response.sgetn(&header_string[0], bytes_transferred);

		// 解析Http Header.
		if (!detail::parse_http_headers(header_string.begin(), header_string.end(),
			m_content_type, m_content_length, m_location, m_response_opts.option_all()))
		{
			handler(avhttp::errc::malformed_response_headers);
			return;
		}
		boost::system::error_code ec;
		// 判断是否需要跳转.
		if (m_status_code == avhttp::errc::moved_permanently || m_status_code == avhttp::errc::found)
		{
			m_sock.close(ec);
			if (++m_redirects <= AVHTTP_MAX_REDIRECTS)
			{
				async_open(m_location, handler);
				return;
			}
		}
		if (m_status_code != avhttp::errc::ok && m_status_code != avhttp::errc::partial_content)
			ec = make_error_code(static_cast<avhttp::errc::errc_t>(m_status_code));
		// 回调通知.
		handler(ec);
	}


protected:

	// 同步相关的其它实现.

	// 连接到socks代理, 在这一步中完成和socks的信息交换过程, 出错信息在ec中.
	template <typename Stream>
	void socks_proxy_connect(Stream &sock, boost::system::error_code &ec)
	{
		using namespace avhttp::detail;

		const proxy_settings &s = m_proxy;

		// 开始解析代理的端口和主机名.
		tcp::resolver resolver(m_io_service);
		std::ostringstream port_string;
		port_string << s.port;
		tcp::resolver::query query(s.hostname.c_str(), port_string.str());
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, ec);
		tcp::resolver::iterator end;

		// 如果解析失败, 则返回.
		if (ec)
			return;

		// 尝试连接解析出来的服务器地址.
		ec = boost::asio::error::host_not_found;
		while (ec && endpoint_iterator != end)
		{
			sock.close(ec);
			sock.connect(*endpoint_iterator++, ec);
		}
		if (ec)
		{
			return;
		}

		if (s.type == proxy_settings::socks5 || s.type == proxy_settings::socks5_pw)
		{
			// 发送版本信息.
			{
				m_request.consume(m_request.size());

				std::size_t bytes_to_write = s.username.empty() ? 3 : 4;
				boost::asio::mutable_buffer b = m_request.prepare(bytes_to_write);
				char *p = boost::asio::buffer_cast<char*>(b);
				write_uint8(5, p); // SOCKS VERSION 5.
				if (s.username.empty())
				{
					write_uint8(1, p); // 1 authentication method (no auth)
					write_uint8(0, p); // no authentication
				}
				else
				{
					write_uint8(2, p); // 2 authentication methods
					write_uint8(0, p); // no authentication
					write_uint8(2, p); // username/password
				}
				m_request.commit(bytes_to_write);
				boost::asio::write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write), ec);
				if (ec)
					return;
			}

			// 读取版本信息.
			m_response.consume(m_response.size());
			boost::asio::read(sock, m_response, boost::asio::transfer_exactly(2), ec);
			if (ec)
				return;

			int version, method;
			{
				boost::asio::const_buffer b = m_response.data();
				const char *p = boost::asio::buffer_cast<const char*>(b);
				version = read_uint8(p);
				method = read_uint8(p);
				if (version != 5)	// 版本不等于5, 不支持socks5.
				{
					ec = make_error_code(errc::unsupported_version);
					return;
				}
			}
			if (method == 2)
			{
				if (s.username.empty())
				{
					ec = make_error_code(errc::username_required);
					return;
				}

				// start sub-negotiation.
				m_request.consume(m_request.size());
				std::size_t bytes_to_write = s.username.size() + s.password.size() + 3;
				boost::asio::mutable_buffer b = m_request.prepare(bytes_to_write);
				char *p = boost::asio::buffer_cast<char*>(b);
				write_uint8(1, p);
				write_uint8(s.username.size(), p);
				write_string(s.username, p);
				write_uint8(s.password.size(), p);
				write_string(s.password, p);
				m_request.commit(bytes_to_write);

				// 发送用户密码信息.
				boost::asio::write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write), ec);
				if (ec)
					return;

				// 读取状态.
				m_response.consume(m_response.size());
				boost::asio::read(sock, m_response, boost::asio::transfer_exactly(2), ec);
				if (ec)
					return;
			}
			else if (method == 0)
			{
				socks_proxy_handshake(sock, ec);
				return;
			}

			{
				// 读取版本状态.
				boost::asio::const_buffer b = m_response.data();
				const char *p = boost::asio::buffer_cast<const char*>(b);

				int version = read_uint8(p);
				int status = read_uint8(p);

				// 不支持的认证版本.
				if (version != 1)
				{
					ec = make_error_code(errc::unsupported_authentication_version);
					return;
				}

				// 认证错误.
				if (status != 0)
				{
					ec = make_error_code(errc::authentication_error);
					return;
				}

				socks_proxy_handshake(sock, ec);
			}
		}
		else if (s.type == proxy_settings::socks4)
		{
			socks_proxy_handshake(sock, ec);
		}
	}

	template <typename Stream>
	void socks_proxy_handshake(Stream &sock, boost::system::error_code & ec)
	{
		using namespace avhttp::detail;

		const url &u = m_url;
		const proxy_settings &s = m_proxy;

		m_request.consume(m_request.size());
		std::string host = u.host();
		std::size_t bytes_to_write = 7 + host.size();
		if (s.type == proxy_settings::socks4)
			bytes_to_write = 9 + s.username.size();
		boost::asio::mutable_buffer mb = m_request.prepare(bytes_to_write);
		char *wp = boost::asio::buffer_cast<char*>(mb);

		if (s.type == proxy_settings::socks5 || s.type == proxy_settings::socks5_pw)
		{
			// 发送socks5连接命令.
			write_uint8(5, wp); // SOCKS VERSION 5.
			write_uint8(1, wp); // CONNECT command.
			write_uint8(0, wp); // reserved.
			write_uint8(3, wp); // address type.
			BOOST_ASSERT(host.size() <= 255);
			write_uint8(host.size(), wp);				// domainname size.
			std::copy(host.begin(), host.end(),wp);		// domainname.
			wp += host.size();
			write_uint16(u.port(), wp);					// port.
		}
		else if (s.type == proxy_settings::socks4)
		{
			write_uint8(4, wp); // SOCKS VERSION 4.
			write_uint8(1, wp); // CONNECT command.
			// socks4协议只接受ip地址, 不支持域名.
			tcp::resolver resolver(m_io_service);
			std::ostringstream port_string;
			port_string << u.port();
			tcp::resolver::query query(host.c_str(), port_string.str());
			// 解析出域名中的ip地址.
			unsigned long ip = resolver.resolve(query, ec)->endpoint().address().to_v4().to_ulong();
			write_uint16(u.port(), wp);	// port.
			write_uint32(ip, wp);		// ip address.
			// username.
			if (!s.username.empty())
			{
				std::copy(s.username.begin(), s.username.end(), wp);
				wp += s.username.size();
			}
			// NULL terminator.
			write_uint8(0, wp);
		}
		else
		{
			ec = make_error_code(errc::unsupported_version);
			return;
		}

		// 发送.
		m_request.commit(bytes_to_write);
		boost::asio::write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write), ec);
		if (ec)
			return;

		// 接收socks服务器返回.
		std::size_t bytes_to_read;
		if (s.type == proxy_settings::socks5 || s.type == proxy_settings::socks5_pw)
			bytes_to_read = 10;
		else if (s.type == proxy_settings::socks4)
			bytes_to_read = 8;

		m_response.consume(m_response.size());
		boost::asio::read(sock, m_response,
			boost::asio::transfer_exactly(bytes_to_read), ec);

		// 分析服务器返回.
		boost::asio::const_buffer cb = m_response.data();
		const char *rp = boost::asio::buffer_cast<const char*>(cb);
		int version = read_uint8(rp);
		int response = read_uint8(rp);

		if (version == 5)
		{
			if (s.type != proxy_settings::socks5 && s.type != proxy_settings::socks5_pw)
			{
				// 请求的socks协议不是sock5.
				ec = make_error_code(errc::unsupported_version);
				return;
			}

			if (response != 0)
			{
				ec = make_error_code(errc::general_failure);
				// 得到更详细的错误信息.
				switch (response)
				{
				case 2: ec = boost::asio::error::no_permission; break;
				case 3: ec = boost::asio::error::network_unreachable; break;
				case 4: ec = boost::asio::error::host_unreachable; break;
				case 5: ec = boost::asio::error::connection_refused; break;
				case 6: ec = boost::asio::error::timed_out; break;
				case 7: ec = make_error_code(errc::command_not_supported); break;
				case 8: ec = boost::asio::error::address_family_not_supported; break;
				}
				return;
			}

			rp++;	// skip reserved.
			int atyp = read_uint8(rp);	// atyp.

			if (atyp == 1)		// address / port 形式返回.
			{
				m_response.consume(m_response.size());
				ec = boost::system::error_code();	// 没有发生错误, 返回.
				return;
			}
			else if (atyp == 3)	// domainname 返回.
			{
				int len = read_uint8(rp);	// 读取domainname长度.
				bytes_to_read = len - 3;
				// 继续读取.
				m_response.commit(boost::asio::read(sock,
					m_response.prepare(bytes_to_read), boost::asio::transfer_exactly(bytes_to_read), ec));
				// if (ec)
				//	return;
				//
				// 得到domainname.
				// std::string domain;
				// domain.resize(len);
				// std::copy(rp, rp + len, domain.begin());
				m_response.consume(m_response.size());
				ec = boost::system::error_code();
				return;
			}
			// else if (atyp == 4)	// ipv6 返回, 暂无实现!
			// {
			//	ec = boost::asio::error::address_family_not_supported;
			//	return;
			// }
			else
			{
				ec = boost::asio::error::address_family_not_supported;
				return;
			}
		}
		else if (version == 4)
		{
			// 90: request granted.
			// 91: request rejected or failed.
			// 92: request rejected becasue SOCKS server cannot connect to identd on the client.
			// 93: request rejected because the client program and identd report different user-ids.
			if (response == 90)	// access granted.
			{
				m_response.consume(m_response.size());
				ec = boost::system::error_code();
				return;
			}
			else
			{
				ec = errc::general_failure;
				switch (response)
				{
				case 91: ec = errc::authentication_error; break;
				case 92: ec = errc::no_identd; break;
				case 93: ec = errc::identd_error; break;
				}
				return;
			}
		}
		else
		{
			ec = errc::general_failure;
			return;
		}
	}

	// socks处理流程状态.
	enum socks_status
	{
		socks_proxy_resolve,	// 查询proxy主机的IP.
		socks_connect_proxy,	// 连接proxy主机.
		socks_send_version,		// 发送socks版本号.
		socks4_resolve_host,	// 用于socks4查询连接的主机IP端口信息.
		socks4_response,		// socks4服务器返回请求.
		socks5_response_version,// socks5返回版本信息.
		socks5_send_userinfo,	// 发送用户密码信息.
		socks5_connect_request,	// 发送连接请求.
		socks5_connect_response,// 服务器返回连接请求.
		socks5_auth_status,		// 认证状态.
		socks5_result,			// 最终结局.
		socks5_read_domainname,	// 读取域名信息.
	};

	// socks代理进行异步连接.
	template <typename Stream, typename Handler>
	void async_socks_proxy_connect(Stream &sock, Handler handler)
	{
		// 构造异步查询proxy主机信息.
		std::ostringstream port_string;
		port_string << m_proxy.port;
		tcp::resolver::query query(m_proxy.hostname, port_string.str());

		m_proxy_status = socks_proxy_resolve;

		// 开始异步解析代理的端口和主机名.
		typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
		m_resolver.async_resolve(query,
			boost::bind(&http_stream::async_socks_proxy_resolve<Stream, HandlerWrapper>, this,
			boost::asio::placeholders::error, boost::asio::placeholders::iterator,
			boost::ref(sock), HandlerWrapper(handler)));
	}

	// 异步代理查询回调.
	template <typename Stream, typename Handler>
	void async_socks_proxy_resolve(const boost::system::error_code &err,
		tcp::resolver::iterator endpoint_iterator, Stream &sock, Handler handler)
	{
		if (err)
		{
			handler(err);
			return;
		}

		if (m_proxy_status == socks_proxy_resolve)
		{
			m_proxy_status = socks_connect_proxy;
			// 开始异步连接代理.
			boost::asio::async_connect(sock.lowest_layer(), endpoint_iterator,
				boost::bind(&http_stream::handle_connect_socks<Stream, Handler>,
				this, boost::ref(sock), handler,
				endpoint_iterator, boost::asio::placeholders::error));

			return;
		}

		if (m_proxy_status == socks4_resolve_host)
		{
			// 保存IP和PORT信息.
			m_remote_endp = *endpoint_iterator;
			m_remote_endp.port(m_url.port());

			// 进入状态.
			handle_socks_process(sock, handler, 0, err);
		}
	}

	template <typename Stream, typename Handler>
	void handle_connect_socks(Stream &sock, Handler handler,
		tcp::resolver::iterator endpoint_iterator, const boost::system::error_code &err)
	{
		using namespace avhttp::detail;

		if (err)
		{
			tcp::resolver::iterator end;
			if (endpoint_iterator == end)
			{
				handler(err);
				return;
			}

			// 继续尝试连接下一个IP.
			endpoint_iterator++;
			boost::asio::async_connect(sock.lowest_layer(), endpoint_iterator,
				boost::bind(&http_stream::handle_connect_socks<Stream, Handler>,
				this, boost::ref(sock), handler,
				endpoint_iterator, boost::asio::placeholders::error));

			return;
		}

		// 连接成功, 发送协议版本号.
		if (m_proxy.type == proxy_settings::socks5 || m_proxy.type == proxy_settings::socks5_pw)
		{
			// 发送版本信息.
			m_proxy_status = socks_send_version;

			m_request.consume(m_request.size());

			std::size_t bytes_to_write = m_proxy.username.empty() ? 3 : 4;
			boost::asio::mutable_buffer b = m_request.prepare(bytes_to_write);
			char *p = boost::asio::buffer_cast<char*>(b);
			write_uint8(5, p);		// SOCKS VERSION 5.
			if (m_proxy.username.empty())
			{
				write_uint8(1, p); // 1 authentication method (no auth)
				write_uint8(0, p); // no authentication
			}
			else
			{
				write_uint8(2, p); // 2 authentication methods
				write_uint8(0, p); // no authentication
				write_uint8(2, p); // username/password
			}

			m_request.commit(bytes_to_write);

			typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
			boost::asio::async_write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write),
				boost::bind(&http_stream::handle_socks_process<Stream, HandlerWrapper>, this,
				boost::ref(sock), HandlerWrapper(handler),
				boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error)
			);

			return;
		}

		if (m_proxy.type == proxy_settings::socks4)
		{
			m_proxy_status = socks4_resolve_host;

			// 构造异步查询远程主机的HOST.
			std::ostringstream port_string;
			port_string << m_url.port();
			tcp::resolver::query query(m_url.host(), port_string.str());

			// 开始异步解析代理的端口和主机名.
			typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
			m_resolver.async_resolve(query,
				boost::bind(&http_stream::async_socks_proxy_resolve<Stream, HandlerWrapper>, this,
				boost::asio::placeholders::error, boost::asio::placeholders::iterator,
				boost::ref(sock), HandlerWrapper(handler)));
		}
	}

	template <typename Stream, typename Handler>
	void handle_socks_process(Stream &sock, Handler handler, int bytes_transferred, const boost::system::error_code &err)
	{
		using namespace avhttp::detail;

		if (err)
		{
			handler(err);
			return;
		}

		switch (m_proxy_status)
		{
		case socks_send_version:	// 完成版本号发送.
			{
				// 接收socks服务器返回.
				std::size_t bytes_to_read;
				if (m_proxy.type == proxy_settings::socks5 || m_proxy.type == proxy_settings::socks5_pw)
					bytes_to_read = 10;
				else if (m_proxy.type == proxy_settings::socks4)
					bytes_to_read = 8;

				if (m_proxy.type == proxy_settings::socks4)
				{
					// 修改状态.
					m_proxy_status = socks4_response;

					m_response.consume(m_response.size());
					boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(bytes_to_read),
						boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
						boost::ref(sock), handler,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error));

					return;
				}

				if (m_proxy.type == proxy_settings::socks5 || m_proxy.type == proxy_settings::socks5_pw)
				{
					m_proxy_status = socks5_response_version;

					// 读取版本信息.
					m_response.consume(m_response.size());
					boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(2),
						boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
						boost::ref(sock), handler,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error));

					return;
				}
			}
			break;
		case socks4_resolve_host:	// socks4协议, IP/PORT已经得到, 开始发送版本信息.
			{
				m_proxy_status = socks_send_version;

				m_request.consume(m_request.size());
				std::size_t bytes_to_write = 9 + m_proxy.username.size();
				boost::asio::mutable_buffer mb = m_request.prepare(bytes_to_write);
				char *wp = boost::asio::buffer_cast<char*>(mb);

				write_uint8(4, wp); // SOCKS VERSION 4.
				write_uint8(1, wp); // CONNECT command.

				// socks4协议只接受ip地址, 不支持域名.
				unsigned long ip = m_remote_endp.address().to_v4().to_ulong();
				write_uint16(m_remote_endp.port(), wp);	// port.
				write_uint32(ip, wp);					// ip address.

				// username.
				if (!m_proxy.username.empty())
				{
					std::copy(m_proxy.username.begin(), m_proxy.username.end(), wp);
					wp += m_proxy.username.size();
				}
				// NULL terminator.
				write_uint8(0, wp);

				m_request.commit(bytes_to_write);

				boost::asio::async_write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write),
					boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
					boost::ref(sock), handler,
					boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error)
					);

				return;
			}
			break;
		case socks5_send_userinfo:
			{
				m_proxy_status = socks5_auth_status;
				// 读取认证状态.
				m_response.consume(m_response.size());
				boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(2),
					boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
					boost::ref(sock), handler,
					boost::asio::placeholders::bytes_transferred,
					boost::asio::placeholders::error));
				return;
			}
			break;
		case socks5_connect_request:
			{
				m_proxy_status = socks5_connect_response;

				// 接收状态信息.
				m_request.consume(m_request.size());
				std::string host = m_url.host();
				std::size_t bytes_to_write = 7 + host.size();
				boost::asio::mutable_buffer mb = m_request.prepare(bytes_to_write);
				char *wp = boost::asio::buffer_cast<char*>(mb);
				// 发送socks5连接命令.
				write_uint8(5, wp); // SOCKS VERSION 5.
				write_uint8(1, wp); // CONNECT command.
				write_uint8(0, wp); // reserved.
				write_uint8(3, wp); // address type.
				BOOST_ASSERT(host.size() <= 255);
				write_uint8(host.size(), wp);				// domainname size.
				std::copy(host.begin(), host.end(),wp);		// domainname.
				wp += host.size();
				write_uint16(m_url.port(), wp);				// port.
				m_request.commit(bytes_to_write);
				boost::asio::async_write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write),
					boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
					boost::ref(sock), handler,
					boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error)
					);

				return;
			}
			break;
		case socks5_connect_response:
			{
				m_proxy_status = socks5_result;
				std::size_t bytes_to_read = 10;
				m_response.consume(m_response.size());
				boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(bytes_to_read),
					boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
					boost::ref(sock), handler,
					boost::asio::placeholders::bytes_transferred,
					boost::asio::placeholders::error));
			}
			break;
		case socks4_response:	// socks4服务器返回请求.
			{
				// 分析服务器返回.
				boost::asio::const_buffer cb = m_response.data();
				const char *rp = boost::asio::buffer_cast<const char*>(cb);
				int version = read_uint8(rp);
				int response = read_uint8(rp);

				// 90: request granted.
				// 91: request rejected or failed.
				// 92: request rejected becasue SOCKS server cannot connect to identd on the client.
				// 93: request rejected because the client program and identd report different user-ids.
				if (response == 90)	// access granted.
				{
					m_response.consume(m_response.size());	// 没有发生错误, 开始异步发送请求.
					async_request(m_request_opts, handler);
					return;
				}
				else
				{
					boost::system::error_code ec = errc::general_failure;
					switch (response)
					{
					case 91: ec = errc::authentication_error; break;
					case 92: ec = errc::no_identd; break;
					case 93: ec = errc::identd_error; break;
					}
					handler(ec);
					return;
				}
			}
			break;
		case socks5_response_version:
			{
				boost::asio::const_buffer cb = m_response.data();
				const char *rp = boost::asio::buffer_cast<const char*>(cb);
				int version = read_uint8(rp);
				int method = read_uint8(rp);
				if (version != 5)	// 版本不等于5, 不支持socks5.
				{
					boost::system::error_code ec = make_error_code(errc::unsupported_version);
					handler(ec);
					return;
				}

				const proxy_settings &s = m_proxy;

				if (method == 2)
				{
					if (s.username.empty())
					{
						boost::system::error_code ec = make_error_code(errc::username_required);
						handler(ec);
						return;
					}

					// start sub-negotiation.
					m_request.consume(m_request.size());
					std::size_t bytes_to_write = m_proxy.username.size() + m_proxy.password.size() + 3;
					boost::asio::mutable_buffer mb = m_request.prepare(bytes_to_write);
					char *wp = boost::asio::buffer_cast<char*>(mb);
					write_uint8(1, wp);
					write_uint8(s.username.size(), wp);
					write_string(s.username, wp);
					write_uint8(s.password.size(), wp);
					write_string(s.password, wp);
					m_request.commit(bytes_to_write);

					// 修改状态.
					m_proxy_status = socks5_send_userinfo;

					// 发送用户密码信息.
					boost::asio::async_write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write),
						boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
						boost::ref(sock), handler,
						boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error));

					return;
				}

				if (method == 0)
				{
					m_proxy_status = socks5_connect_request;
					handle_socks_process(sock, handler, 0, err);
					return;
				}
			}
			break;
		case socks5_auth_status:
			{
				boost::asio::const_buffer cb = m_response.data();
				const char *rp = boost::asio::buffer_cast<const char*>(cb);

				int version = read_uint8(rp);
				int status = read_uint8(rp);

				if (version != 1)	// 不支持的版本.
				{
					boost::system::error_code ec = make_error_code(errc::unsupported_authentication_version);
					handler(ec);
					return;
				}

				if (status != 0)	// 认证错误.
				{
					boost::system::error_code ec = make_error_code(errc::authentication_error);
					handler(ec);
					return;
				}

				// 发送请求连接命令.
				m_proxy_status = socks5_connect_request;
				handle_socks_process(sock, handler, 0, err);
			}
			break;
		case socks5_result:
			{
				// 分析服务器返回.
				boost::asio::const_buffer cb = m_response.data();
				const char *rp = boost::asio::buffer_cast<const char*>(cb);
				int version = read_uint8(rp);
				int response = read_uint8(rp);

				if (version != 5)
				{
					boost::system::error_code ec = make_error_code(errc::general_failure);
					handler(ec);
					return;
				}

				if (response != 0)
				{
					boost::system::error_code ec = make_error_code(errc::general_failure);
					// 得到更详细的错误信息.
					switch (response)
					{
					case 2: ec = boost::asio::error::no_permission; break;
					case 3: ec = boost::asio::error::network_unreachable; break;
					case 4: ec = boost::asio::error::host_unreachable; break;
					case 5: ec = boost::asio::error::connection_refused; break;
					case 6: ec = boost::asio::error::timed_out; break;
					case 7: ec = make_error_code(errc::command_not_supported); break;
					case 8: ec = boost::asio::error::address_family_not_supported; break;
					}
					handler(ec);
					return;
				}

				rp++;	// skip reserved.
				int atyp = read_uint8(rp);	// atyp.

				if (atyp == 1)		// address / port 形式返回.
				{
					m_response.consume(m_response.size());

					// 没有发生错误, 开始异步发送请求.
					async_request(m_request_opts, handler);

					return;
				}
				else if (atyp == 3)				// domainname 返回.
				{
					int len = read_uint8(rp);	// 读取domainname长度.
					std::size_t bytes_to_read = len - 3;

					m_proxy_status = socks5_read_domainname;

					m_response.consume(m_response.size());
					boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(bytes_to_read),
						boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
						boost::ref(sock), handler,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error));

					return;
				}
				// else if (atyp == 4)	// ipv6 返回, 暂无实现!
				// {
				//	ec = boost::asio::error::address_family_not_supported;
				//	return;
				// }
				else
				{
					boost::system::error_code ec = boost::asio::error::address_family_not_supported;
					handler(ec);
					return;
				}
			}
			break;
		case socks5_read_domainname:
			{
				m_response.consume(m_response.size());

				// 没有发生错误, 开始异步发送请求.
				async_request(m_request_opts, handler);

				return;
			}
			break;
		}
	}

#ifdef AVHTTP_ENABLE_OPENSSL
	inline bool certificate_matches_host(X509* cert, const std::string& host)
	{
		// Try converting host name to an address. If it is an address then we need
		// to look for an IP address in the certificate rather than a host name.
		boost::system::error_code ec;
		boost::asio::ip::address address
			= boost::asio::ip::address::from_string(host, ec);
		bool is_address = !ec;

		// Go through the alternate names in the certificate looking for DNS or IPADD
		// entries.
		GENERAL_NAMES* gens = static_cast<GENERAL_NAMES*>(
			X509_get_ext_d2i(cert, NID_subject_alt_name, 0, 0));
		for (int i = 0; i < sk_GENERAL_NAME_num(gens); ++i)
		{
			GENERAL_NAME* gen = sk_GENERAL_NAME_value(gens, i);
			if (gen->type == GEN_DNS && !is_address)
			{
				ASN1_IA5STRING* domain = gen->d.dNSName;
				if (domain->type == V_ASN1_IA5STRING
					&& domain->data && domain->length)
				{
					const char* cert_host = reinterpret_cast<const char*>(domain->data);
					int j;
					for (j = 0; host[j] && cert_host[j]; ++j)
						if (std::tolower(host[j]) != std::tolower(cert_host[j]))
							break;
					if (host[j] == 0 && cert_host[j] == 0)
						return true;
				}
			}
			else if (gen->type == GEN_IPADD && is_address)
			{
				ASN1_OCTET_STRING* ip_address = gen->d.iPAddress;
				if (ip_address->type == V_ASN1_OCTET_STRING && ip_address->data)
				{
					if (address.is_v4() && ip_address->length == 4)
					{
						boost::asio::ip::address_v4::bytes_type address_bytes
							= address.to_v4().to_bytes();
						if (std::memcmp(address_bytes.data(), ip_address->data, 4) == 0)
							return true;
					}
					else if (address.is_v6() && ip_address->length == 16)
					{
						boost::asio::ip::address_v6::bytes_type address_bytes
							= address.to_v6().to_bytes();
						if (std::memcmp(address_bytes.data(), ip_address->data, 16) == 0)
							return true;
					}
				}
			}
		}

		// No match in the alternate names, so try the common names.
		X509_NAME* name = X509_get_subject_name(cert);
		int i = -1;
		while ((i = X509_NAME_get_index_by_NID(name, NID_commonName, i)) >= 0)
		{
			X509_NAME_ENTRY* name_entry = X509_NAME_get_entry(name, i);
			ASN1_STRING* domain = X509_NAME_ENTRY_get_data(name_entry);
			if (domain->data && domain->length)
			{
				const char* cert_host = reinterpret_cast<const char*>(domain->data);
				int j;
				for (j = 0; host[j] && cert_host[j]; ++j)
					if (std::tolower(host[j]) != std::tolower(cert_host[j]))
						break;
				if (host[j] == 0 && cert_host[j] == 0)
					return true;
			}
		}

		return false;
	}
#endif // AVHTTP_ENABLE_OPENSSL

protected:
	boost::asio::io_service &m_io_service;			// io_service引用.
	tcp::resolver m_resolver;						// 解析HOST.
	socket_type m_sock;								// socket.
	nossl_socket m_nossl_socket;					// 非ssl socket, 只用于https的proxy实现.
	bool m_check_certificate;						// 是否认证服务端证书.
	request_opts m_request_opts;					// 向http服务器请求的头信息.
	response_opts m_response_opts;					// http服务器返回的http头信息.
	proxy_settings m_proxy;							// 代理设置.
	int m_proxy_status;								// 异步中代理状态.
	tcp::endpoint m_remote_endp;					// 用于socks4代理中.
	std::string m_protocol;							// 协议类型(http/https).
	url m_url;										// 保存当前请求的url.
	bool m_keep_alive;								// 获得connection选项, 同时受m_response_opts影响.
	int m_status_code;								// http返回状态码.
	std::size_t m_redirects;						// 重定向次数计数.
	std::string m_content_type;						// 数据类型.
	std::size_t m_content_length;					// 数据内容长度.
	std::string m_location;							// 重定向的地址.
	boost::asio::streambuf m_request;				// 请求缓冲.
	boost::asio::streambuf m_response;				// 回复缓冲.
};

}

#endif // __HTTP_STREAM_HPP__
