//
// ssl_stream.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
// Copyright (c) 2003, Arvid Norberg
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __SSL_STREAM_HPP__
#define __SSL_STREAM_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/ssl.hpp>
#include <openssl/x509v3.h>

// openssl seems to believe it owns this name in every single scope.
#undef set_key

namespace avhttp {
namespace detail {

template <class Stream>
class ssl_stream
{
public:

	explicit ssl_stream(boost::asio::io_service &io_service)
		: m_context(io_service, boost::asio::ssl::context::sslv23_client)
		, m_sock(io_service, m_context)
	{
		boost::system::error_code ec;
		m_context.set_verify_mode(boost::asio::ssl::context::verify_none, ec);
	}

	template <typename Arg>
	explicit ssl_stream(Arg &arg, boost::asio::io_service &io_service)
		: m_context(io_service, boost::asio::ssl::context::sslv23_client)
		, m_sock(arg, m_context)
	{
		boost::system::error_code ec;
		m_context.set_verify_mode(boost::asio::ssl::context::verify_none, ec);
	}

	~ssl_stream() {}

	typedef typename boost::remove_reference<Stream>::type next_layer_type;
	typedef typename next_layer_type::lowest_layer_type lowest_layer_type;
	typedef typename lowest_layer_type::endpoint_type endpoint_type;
	typedef typename lowest_layer_type::protocol_type protocol_type;
	typedef typename boost::asio::ssl::stream<Stream> sock_type;
	typedef typename boost::asio::ssl::stream<Stream>::impl_type impl_type;

	typedef boost::function<void(boost::system::error_code const&)> handler_type;


	void connect(endpoint_type const &endpoint)
	{
		// 1. connect to peer
		// 2. perform SSL client handshake

		m_sock.next_layer().connect(endpoint);
		m_sock.handshake(boost::asio::ssl::stream_base::client);
	}

	void connect(endpoint_type const &endpoint, boost::system::error_code &ec)
	{
		// 1. connect to peer
		// 2. perform SSL client handshake

		m_sock.next_layer().connect(endpoint, ec);
		if (ec)
			return;
		m_sock.handshake(boost::asio::ssl::stream_base::client, ec);
	}

	template <class Handler>
	void async_connect(endpoint_type &endpoint, Handler &handler)
	{
		// the connect is split up in the following steps:
		// 1. connect to peer
		// 2. perform SSL client handshake

		// to avoid unnecessary copying of the handler,
		// store it in a shared_ptr
		boost::shared_ptr<handler_type> h(new handler_type(handler));

		m_sock.next_layer().async_connect(endpoint
			, boost::bind(&ssl_stream::connected, this, _1, h));
	}

	template <class Mutable_Buffers, class Handler>
	void async_read_some(Mutable_Buffers const& buffers, Handler const& handler)
	{
		m_sock.async_read_some(buffers, handler);
	}

	template <class Mutable_Buffers>
	std::size_t read_some(Mutable_Buffers const& buffers, boost::system::error_code& ec)
	{
		return m_sock.read_some(buffers, ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <class Mutable_Buffers>
	std::size_t read_some(Mutable_Buffers const& buffers)
	{
		return m_sock.read_some(buffers);
	}

	template <class IO_Control_Command>
	void io_control(IO_Control_Command& ioc)
	{
		m_sock.next_layer().io_control(ioc);
	}
#endif

	template <class IO_Control_Command>
	void io_control(IO_Control_Command& ioc, boost::system::error_code& ec)
	{
		m_sock.next_layer().io_control(ioc, ec);
	}

	template <class Const_Buffers, class Handler>
	void async_write_some(Const_Buffers const &buffers, Handler const& handler)
	{
		m_sock.async_write_some(buffers, handler);
	}

	template <class Const_Buffers>
	std::size_t write_some(Const_Buffers const &buffers, boost::system::error_code &ec)
	{
		return m_sock.write_some(buffers, ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <class Const_Buffers>
	std::size_t write_some(Const_Buffers const &buffers)
	{
		return m_sock.write_some(buffers);
	}

	void bind(endpoint_type const& endpoint)
	{
		m_sock.next_layer().bind(endpoint);
	}
#endif

	void bind(endpoint_type const& endpoint, boost::system::error_code& ec)
	{
		m_sock.next_layer().bind(endpoint, ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	void open(protocol_type const& p)
	{
		m_sock.next_layer().open(p);
	}
#endif

	void open(protocol_type const& p, boost::system::error_code& ec)
	{
		m_sock.next_layer().open(p, ec);
	}

	bool is_open() const
	{
		return const_cast<sock_type&>(m_sock).next_layer().is_open();
	}

#ifndef BOOST_NO_EXCEPTIONS
	void close()
	{
		m_sock.next_layer().close();
	}
#endif

	void close(boost::system::error_code& ec)
	{
		m_sock.next_layer().close(ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	endpoint_type remote_endpoint() const
	{
		return const_cast<sock_type&>(m_sock).next_layer().remote_endpoint();
	}
#endif

	endpoint_type remote_endpoint(boost::system::error_code& ec) const
	{
		return const_cast<sock_type&>(m_sock).next_layer().remote_endpoint(ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	endpoint_type local_endpoint() const
	{
		return const_cast<sock_type&>(m_sock).next_layer().local_endpoint();
	}
#endif

	endpoint_type local_endpoint(boost::system::error_code& ec) const
	{
		return const_cast<sock_type&>(m_sock).next_layer().local_endpoint(ec);
	}

	boost::asio::io_service& get_io_service()
	{
		return m_sock.get_io_service();
	}

	lowest_layer_type& lowest_layer()
	{
		return m_sock.lowest_layer();
	}

	next_layer_type& next_layer()
	{
		return m_sock.next_layer();
	}

	impl_type impl()
	{
		return m_sock.impl();
	}

	template <typename SettableSocketOption>
	boost::system::error_code set_option(const SettableSocketOption& option,
		boost::system::error_code& ec)
	{
		return m_sock.next_layer().set_option(option, ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <typename SettableSerialPortOption>
	void set_option(const SettableSerialPortOption& option)
	{
		m_sock.next_layer().set_option(option);
	}
#endif

private:

	void connected(boost::system::error_code const& e, boost::shared_ptr<handler_type> h)
	{
		if (e)
		{
			(*h)(e);
			return;
		}

		m_sock.async_handshake(boost::asio::ssl::stream_base::client
			, boost::bind(&ssl_stream::handshake, this, _1, h));
	}

	void handshake(boost::system::error_code const& e, boost::shared_ptr<handler_type> h)
	{
		(*h)(e);
	}

	boost::asio::ssl::context m_context;
	boost::asio::ssl::stream<Stream> m_sock;
};

}
}

#endif // __SSL_STREAM_HPP__
