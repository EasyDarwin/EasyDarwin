//
// socket_type.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
// Copyright (c) 2003, Arvid Norberg
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __SOCKET_TYPE_HPP__
#define __SOCKET_TYPE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/variant.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/void.hpp>
#include <boost/mpl/remove.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/size.hpp>

#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>

#include <boost/type_traits/add_pointer.hpp>
#include <boost/noncopyable.hpp>

#include <boost/asio/io_service.hpp>

#define NETWORK_VARIANT_STREAM_LIMIT 5

namespace avhttp {
namespace detail {

namespace aux
{

	struct delete_visitor
		: boost::static_visitor<>
	{
		template <class T>
		void operator()(T* p) const
		{
			delete p;
		}

		void operator()(boost::blank) const
		{}
	};

	// -------------- io_control -----------

	template<class IO_Control_Command>
	struct io_control_visitor_ec: boost::static_visitor<>
	{
		io_control_visitor_ec(IO_Control_Command& io, boost::system::error_code& e)
			: ioc(io), ec(e) {}

		template <class T>
		void operator()(T* p) const
		{
			p->io_control(ioc, ec);
		}

		void operator()(boost::blank) const
		{}

		IO_Control_Command& ioc;
		boost::system::error_code& ec;
	};

	template<class IO_Control_Command>
	struct io_control_visitor
		: boost::static_visitor<>
	{
		io_control_visitor(IO_Control_Command& io)
			: ioc(io) {}

		template <class T>
		void operator()(T* p) const
		{
			p->io_control(ioc);
		}

		void operator()(boost::blank) const
		{}

		IO_Control_Command& ioc;
	};

	// -------------- connect -----------

#ifndef BOOST_NO_EXCEPTIONS
	template <class EndpointType>
	struct connect_visitor
		: public boost::static_visitor<>
	{
		connect_visitor(EndpointType const &ep)
			: endpoint(ep)
		{}

		template <class T>
		void operator()(T* p) const
		{
			p->connect(endpoint);
		}

		void operator()(boost::blank) const
		{}

		EndpointType const& endpoint;
	};
#endif

	template <class EndpointType>
	struct connect_visitor_ec
		: public boost::static_visitor<>
	{
		connect_visitor_ec(EndpointType const &ep, boost::system::error_code& ec_)
			: endpoint(ep)
			, ec(ec_)
		{}

		template <class T>
		void operator()(T* p) const
		{
			p->connect(endpoint, ec);
		}

		void operator()(boost::blank) const
		{}

		EndpointType const& endpoint;
		boost::system::error_code& ec;
	};

	// -------------- async_connect -----------

	template <class EndpointType, class Handler>
	struct async_connect_visitor
		: boost::static_visitor<>
	{
		async_connect_visitor(EndpointType &ep, Handler &h)
			: endpoint(ep)
			, handler(h)
		{}

		template <class T>
		void operator()(T* p) const
		{
			p->async_connect(endpoint, handler);
		}

		void operator()(boost::blank) const	{}

		EndpointType &endpoint;
		Handler &handler;
	};

	// -------------- bind -----------

	template <class EndpointType>
	struct bind_visitor_ec
		: boost::static_visitor<>
	{
		bind_visitor_ec(EndpointType const& ep, boost::system::error_code& ec_)
			: endpoint(ep)
			, ec(ec_)
		{}

		template <class T>
		void operator()(T* p) const
		{ p->bind(endpoint, ec); }

		void operator()(boost::blank) const {}

		EndpointType const& endpoint;
		boost::system::error_code& ec;
	};

	template <class EndpointType>
	struct bind_visitor
		: boost::static_visitor<>
	{
		bind_visitor(EndpointType const& ep)
			: endpoint(ep)
		{}

		template <class T>
		void operator()(T* p) const
		{ p->bind(endpoint); }

		void operator()(boost::blank) const {}

		EndpointType const& endpoint;
	};

	// -------------- open -----------

	template <class Protocol>
	struct open_visitor_ec
		: boost::static_visitor<>
	{
		open_visitor_ec(Protocol const& p, boost::system::error_code& ec_)
			: proto(p)
			, ec(ec_)
		{}

		template <class T>
		void operator()(T* p) const
		{ p->open(proto, ec); }

		void operator()(boost::blank) const {}

		Protocol const& proto;
		boost::system::error_code& ec;
	};

	template <class Protocol>
	struct open_visitor
		: boost::static_visitor<>
	{
		open_visitor(Protocol const& p)
			: proto(p)
		{}

		template <class T>
		void operator()(T* p) const
		{ p->open(proto); }

		void operator()(boost::blank) const {}

		Protocol const& proto;
	};

	// -------------- is_open -----------

	struct is_open_visitor
		: boost::static_visitor<bool>
	{
		is_open_visitor() {}

		template <class T>
		bool operator()(T const* p) const
		{ return p->is_open(); }

		bool operator()(boost::blank) const { return false; }
	};

	// -------------- close -----------

	struct close_visitor_ec
		: boost::static_visitor<>
	{
		close_visitor_ec(boost::system::error_code& ec_)
			: ec(ec_)
		{}

		template <class T>
		void operator()(T* p) const
		{ p->close(ec); }

		void operator()(boost::blank) const {}

		boost::system::error_code& ec;
	};

#ifndef BOOST_NO_EXCEPTIONS
	struct close_visitor
		: boost::static_visitor<>
	{
		template <class T>
		void operator()(T* p) const
		{ p->close(); }

		void operator()(boost::blank) const {}
	};
#endif

	// -------------- remote_endpoint -----------

	template <class EndpointType>
	struct remote_endpoint_visitor_ec
		: boost::static_visitor<EndpointType>
	{
		remote_endpoint_visitor_ec(boost::system::error_code& ec_)
			: ec(ec_)
		{}

		template <class T>
		EndpointType operator()(T const* p) const
		{ return p->remote_endpoint(ec); }

		EndpointType operator()(boost::blank) const
		{ return EndpointType(); }

		boost::system::error_code& ec;
	};

#ifndef BOOST_NO_EXCEPTIONS
	template <class EndpointType>
	struct remote_endpoint_visitor
		: boost::static_visitor<EndpointType>
	{
		template <class T>
		EndpointType operator()(T const* p) const
		{ return p->remote_endpoint(); }

		EndpointType operator()(boost::blank) const
		{ return EndpointType(); }
	};
#endif

	// -------------- set_option -----------

#ifndef BOOST_NO_EXCEPTIONS
	template <class SettableSocketOption>
	struct set_option_visitor
		: boost::static_visitor<>
	{
		set_option_visitor(SettableSocketOption const& opt)
			: opt_(opt)
		{}

		template <class T>
		void operator()(T* p) const
		{ p->set_option(opt_); }

		void operator()(boost::blank) const {}

		SettableSocketOption const& opt_;
	};
#endif

	template <class SettableSocketOption>
	struct set_option_visitor_ec
		: boost::static_visitor<boost::system::error_code>
	{
		set_option_visitor_ec(SettableSocketOption const& opt, boost::system::error_code& ec)
			: opt_(opt)
			, ec_(ec)
		{}

		template <class T>
		boost::system::error_code operator()(T* p) const
		{ return p->set_option(opt_, ec_); }

		boost::system::error_code operator()(boost::blank) const
		{ return ec_; }

		SettableSocketOption const& opt_;
		boost::system::error_code& ec_;
	};

	// -------------- local_endpoint -----------

	template <class EndpointType>
	struct local_endpoint_visitor_ec
		: boost::static_visitor<EndpointType>
	{
		local_endpoint_visitor_ec(boost::system::error_code& ec_)
			: ec(ec_)
		{}

		template <class T>
		EndpointType operator()(T const* p) const
		{
			return p->local_endpoint(ec);
		}

		EndpointType operator()(boost::blank) const
		{
			return EndpointType();
		}

		boost::system::error_code& ec;
	};

#ifndef BOOST_NO_EXCEPTIONS
	template <class EndpointType>
	struct local_endpoint_visitor
		: boost::static_visitor<EndpointType>
	{
		template <class T>
		EndpointType operator()(T const* p) const
		{
			return p->local_endpoint();
		}

		EndpointType operator()(boost::blank) const
		{
			return EndpointType();
		}
	};
#endif

	// -------------- async_read_some -----------

	template <class Mutable_Buffers, class Handler>
	struct async_read_some_visitor
		: boost::static_visitor<>
	{
		async_read_some_visitor(Mutable_Buffers const& bufs, Handler const& h)
			: buffers(bufs)
			, handler(h)
		{}

		template <class T>
		void operator()(T* p) const
		{
			p->async_read_some(buffers, handler);
		}
		void operator()(boost::blank) const
		{}

		Mutable_Buffers const& buffers;
		Handler const& handler;
	};

	// -------------- read_some -----------

#ifndef BOOST_NO_EXCEPTIONS
	template <class Mutable_Buffers>
	struct read_some_visitor
		: boost::static_visitor<std::size_t>
	{
		read_some_visitor(Mutable_Buffers const& bufs)
			: buffers(bufs)
		{}

		template <class T>
		std::size_t operator()(T* p) const
		{ return p->read_some(buffers); }

		std::size_t operator()(boost::blank) const
		{ return 0; }

		Mutable_Buffers const& buffers;
	};
#endif

	template <class Mutable_Buffers>
	struct read_some_visitor_ec
		: boost::static_visitor<std::size_t>
	{
		read_some_visitor_ec(Mutable_Buffers const& bufs, boost::system::error_code& e)
			: buffers(bufs)
			, ec(e)
		{}

		template <class T>
		std::size_t operator()(T* p) const
		{ return p->read_some(buffers, ec); }

		std::size_t operator()(boost::blank) const
		{ return 0; }

		Mutable_Buffers const& buffers;
		boost::system::error_code& ec;
	};

	// -------------- async_write_some -----------

	template <class Const_Buffers, class Handler>
	struct async_write_some_visitor
		: boost::static_visitor<>
	{
		async_write_some_visitor(Const_Buffers const& bufs, Handler const& h)
			: buffers(bufs)
			, handler(h)
		{}

		template <class T>
		void operator()(T* p) const
		{
			p->async_write_some(buffers, handler);
		}

		void operator()(boost::blank) const
		{}

		Const_Buffers const& buffers;
		Handler const& handler;
	};

	// -------------- write_some -----------

#ifndef BOOST_NO_EXCEPTIONS
	template <class ConstBufferSequence>
	struct write_some_visitor
		: boost::static_visitor<std::size_t>
	{
		write_some_visitor(ConstBufferSequence const& bufs)
			: buffers(bufs)
		{}

		template <class T>
		std::size_t operator()(T* p) const
		{ return p->write_some(buffers); }

		std::size_t operator()(boost::blank) const
		{ return 0; }

		ConstBufferSequence const& buffers;
	};
#endif

	template <class ConstBufferSequence>
	struct write_some_visitor_ec
		: boost::static_visitor<std::size_t>
	{
		write_some_visitor_ec(ConstBufferSequence const& bufs, boost::system::error_code& e)
			: buffers(bufs)
			, ec(e)
		{}

		template <class T>
		std::size_t operator()(T* p) const
		{ return p->write_some(buffers, ec); }

		std::size_t operator()(boost::blank) const
		{ return 0; }

		ConstBufferSequence const& buffers;
		boost::system::error_code& ec;
	};

	// -------------- in_avail -----------

	struct in_avail_visitor_ec
		: boost::static_visitor<std::size_t>
	{
		in_avail_visitor_ec(boost::system::error_code& ec_)
			: ec(ec_)
		{}

		template <class T>
		std::size_t operator()(T const* p) const
		{
			return p->in_avail(ec);
		}

		std::size_t operator()(boost::blank) const
		{
			return 0;
		}

		boost::system::error_code& ec;
	};

#ifndef BOOST_NO_EXCEPTIONS
	struct in_avail_visitor
		: boost::static_visitor<std::size_t>
	{
		template <class T>
		std::size_t operator()(T const* p) const
		{
			return p->in_avail();
		}

		void operator()(boost::blank) const
		{}
	};
#endif

	// -------------- io_service -----------

	template <class IOService>
	struct io_service_visitor
		: boost::static_visitor<IOService&>
	{
		template <class T>
		IOService& operator()(T* p) const
		{
			return p->get_io_service();
		}

		IOService& operator()(boost::blank) const
		{
			return *(IOService*)0;
		}
	};

	// -------------- lowest_layer -----------

	template <class LowestLayer>
	struct lowest_layer_visitor
		: boost::static_visitor<LowestLayer&>
	{
		template <class T>
		LowestLayer& operator()(T* p) const
		{
			return p->lowest_layer();
		}

		LowestLayer& operator()(boost::blank) const
		{
			return *(LowestLayer*)0;
		}
	};

} // namespace aux

template <
	BOOST_PP_ENUM_BINARY_PARAMS(NETWORK_VARIANT_STREAM_LIMIT, class S, = boost::mpl::void_ BOOST_PP_INTERCEPT)
>
class variant_stream : boost::noncopyable
{
public:
	typedef BOOST_PP_CAT(boost::mpl::vector, NETWORK_VARIANT_STREAM_LIMIT)<
		BOOST_PP_ENUM_PARAMS(NETWORK_VARIANT_STREAM_LIMIT, S)
	> types0;

	typedef typename boost::mpl::remove<types0, boost::mpl::void_>::type types;

	typedef typename boost::make_variant_over<
		typename boost::mpl::push_back<
		typename boost::mpl::transform<
		types
		, boost::add_pointer<boost::mpl::_>
		>::type
		, boost::blank
		>::type
	>::type variant_type;

	typedef typename boost::remove_reference<S0>::type P0;
	typedef typename P0::lowest_layer_type lowest_layer_type;
	typedef typename lowest_layer_type::endpoint_type endpoint_type;
	typedef typename lowest_layer_type::protocol_type protocol_type;

	explicit variant_stream(boost::asio::io_service& ios)
		: m_io_service(ios), m_variant(boost::blank()) {}

	template <class S>
	void instantiate(boost::asio::io_service &ios)
	{
		BOOST_ASSERT(&ios == &m_io_service);
		std::auto_ptr<S> owned(new S(ios));
		boost::apply_visitor(aux::delete_visitor(), m_variant);
		m_variant = owned.get();
		owned.release();
	}

	template <class S>
	void instantiate(boost::asio::ip::tcp::socket &socket)
	{
		BOOST_ASSERT(&socket.get_io_service() == &m_io_service);
		std::auto_ptr<S> owned(new S(socket, m_io_service));
		boost::apply_visitor(aux::delete_visitor(), m_variant);
		m_variant = owned.get();
		owned.release();
	}

	template <class S>
	S* get()
	{
		S** ret = boost::get<S*>(&m_variant);
		if (!ret) return 0;
		return *ret;
	}

	bool instantiated() const
	{
		return m_variant.which() != boost::mpl::size<types>::value;
	}

	~variant_stream()
	{
		boost::apply_visitor(aux::delete_visitor(), m_variant);
	}

#ifndef BOOST_NO_EXCEPTIONS
	void connect(endpoint_type const& endpoint)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(
			aux::connect_visitor<endpoint_type>(endpoint)
			, m_variant
			);
	}
#endif

	void connect(endpoint_type const& endpoint, boost::system::error_code &ec)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(
			aux::connect_visitor_ec<endpoint_type>(endpoint, ec)
			, m_variant
			);
	}

	template <class Mutable_Buffers>
	std::size_t read_some(Mutable_Buffers const& buffers, boost::system::error_code &ec)
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(
			aux::read_some_visitor_ec<Mutable_Buffers>(buffers, ec)
			, m_variant
			);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <class Mutable_Buffers>
	std::size_t read_some(Mutable_Buffers const& buffers)
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(
			aux::read_some_visitor<Mutable_Buffers>(buffers)
			, m_variant
			);
	}
#endif

	template <typename ConstBufferSequence>
	std::size_t write_some(const ConstBufferSequence& buffers,
		boost::system::error_code& ec)
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(
			aux::write_some_visitor_ec<ConstBufferSequence>(buffers, ec)
			, m_variant
			);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <class ConstBufferSequence>
	std::size_t write_some(ConstBufferSequence const& buffers)
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(
			aux::write_some_visitor<ConstBufferSequence>(buffers)
			, m_variant
			);
	}
#endif

	template <class Mutable_Buffers, class Handler>
	void async_read_some(Mutable_Buffers const& buffers, Handler const& handler)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(
			aux::async_read_some_visitor<Mutable_Buffers, Handler>(buffers, handler)
			, m_variant
			);
	}

	template <class Const_Buffers, class Handler>
	void async_write_some(Const_Buffers const& buffers, Handler const& handler)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(
			aux::async_write_some_visitor<Const_Buffers, Handler>(buffers, handler)
			, m_variant
			);
	}

	template <class Handler>
	void async_connect(endpoint_type endpoint, Handler handler)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(
			aux::async_connect_visitor<endpoint_type, Handler>(endpoint, handler), m_variant
			);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <class IO_Control_Command>
	void io_control(IO_Control_Command& ioc)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(
			aux::io_control_visitor<IO_Control_Command>(ioc), m_variant
			);
	}
#endif

	template <class IO_Control_Command>
	void io_control(IO_Control_Command& ioc, boost::system::error_code& ec)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(
			aux::io_control_visitor_ec<IO_Control_Command>(ioc, ec)
			, m_variant
			);
	}

#ifndef BOOST_NO_EXCEPTIONS
	void bind(endpoint_type const& endpoint)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(aux::bind_visitor<endpoint_type>(endpoint), m_variant);
	}
#endif

	void bind(endpoint_type const& endpoint, boost::system::error_code& ec)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(
			aux::bind_visitor_ec<endpoint_type>(endpoint, ec), m_variant
			);
	}

#ifndef BOOST_NO_EXCEPTIONS
	void open(protocol_type const& p)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(aux::open_visitor<protocol_type>(p), m_variant);
	}
#endif

	void open(protocol_type const& p, boost::system::error_code& ec)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(
			aux::open_visitor_ec<protocol_type>(p, ec), m_variant
			);
	}

	bool is_open() const
	{
		return boost::apply_visitor(aux::is_open_visitor(), m_variant);
	}

#ifndef BOOST_NO_EXCEPTIONS
	void close()
	{
		if (!instantiated()) return;
		boost::apply_visitor(aux::close_visitor(), m_variant);
	}
#endif

	void close(boost::system::error_code& ec)
	{
		if (!instantiated()) return;
		boost::apply_visitor(
			aux::close_visitor_ec(ec), m_variant
			);
	}

#ifndef BOOST_NO_EXCEPTIONS
	std::size_t in_avail() const
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(aux::in_avail_visitor(), m_variant);
	}
#endif

	std::size_t in_avail(boost::system::error_code& ec) const
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(
			aux::in_avail_visitor_ec(ec), m_variant
			);
	}

#ifndef BOOST_NO_EXCEPTIONS
	endpoint_type remote_endpoint() const
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(aux::remote_endpoint_visitor<endpoint_type>(), m_variant);
	}
#endif

	endpoint_type remote_endpoint(boost::system::error_code& ec) const
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(
			aux::remote_endpoint_visitor_ec<endpoint_type>(ec), m_variant
			);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <class SettableSocketOption>
	void set_option(SettableSocketOption const& opt)
	{
		BOOST_ASSERT(instantiated());
		boost::apply_visitor(aux::set_option_visitor<SettableSocketOption>(opt)
			, m_variant);
	}
#endif

	template <class SettableSocketOption>
	boost::system::error_code set_option(SettableSocketOption const& opt, boost::system::error_code& ec)
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(aux::set_option_visitor_ec<SettableSocketOption>(opt, ec)
			, m_variant);
	}

#ifndef BOOST_NO_EXCEPTIONS
	endpoint_type local_endpoint() const
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(aux::local_endpoint_visitor<endpoint_type>(), m_variant);
	}
#endif

	endpoint_type local_endpoint(boost::system::error_code& ec) const
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(
			aux::local_endpoint_visitor_ec<endpoint_type>(ec), m_variant
			);
	}

	boost::asio::io_service& get_io_service()
	{
		return m_io_service;
	}

	lowest_layer_type& lowest_layer()
	{
		BOOST_ASSERT(instantiated());
		return boost::apply_visitor(
			aux::lowest_layer_visitor<lowest_layer_type>(), m_variant
			);
	}

private:
	boost::asio::io_service& m_io_service;
	variant_type m_variant;
};

}
}

#endif // __SOCKET_TYPE_HPP__
