//
// detail/handler_type_requirements.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
// Copyright (c) 2003, Arvid Norberg
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_HANDLER_TYPE_REQUIREMENTS_HPP
#define AVHTTP_HANDLER_TYPE_REQUIREMENTS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/handler_type_requirements.hpp>

namespace boost {
namespace asio {
namespace detail {

#if defined(BOOST_ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS)

#define AVHTTP_READ_HANDLER_CHECK( \
    handler_type, handler) \
  \
  BOOST_ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(boost::asio::detail::two_arg_handler_test( \
          handler, \
          static_cast<const boost::system::error_code*>(0), \
          static_cast<const std::size_t*>(0))) == 1, \
      "ReadHandler type requirements not met") \
  \
  typedef boost::asio::detail::handler_type_requirements< \
      sizeof( \
        boost::asio::detail::argbyv( \
          boost::asio::detail::clvref(handler))) + \
      sizeof( \
        boost::asio::detail::lvref(handler)( \
          boost::asio::detail::lvref<const boost::system::error_code>(), \
          boost::asio::detail::lvref<const std::size_t>()), \
        char(0))>

#define AVHTTP_WRITE_HANDLER_CHECK( \
    handler_type, handler) \
  \
  BOOST_ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(boost::asio::detail::two_arg_handler_test( \
          handler, \
          static_cast<const boost::system::error_code*>(0), \
          static_cast<const std::size_t*>(0))) == 1, \
      "WriteHandler type requirements not met") \
  \
  typedef boost::asio::detail::handler_type_requirements< \
      sizeof( \
        boost::asio::detail::argbyv( \
          boost::asio::detail::clvref(handler))) + \
      sizeof( \
        boost::asio::detail::lvref(handler)( \
          boost::asio::detail::lvref<const boost::system::error_code>(), \
          boost::asio::detail::lvref<const std::size_t>()), \
        char(0))>

#define AVHTTP_OPEN_HANDLER_CHECK( \
    handler_type, handler) \
  \
  BOOST_ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(boost::asio::detail::one_arg_handler_test( \
          handler, \
          static_cast<const boost::system::error_code*>(0))) == 1, \
      "OpenHandler type requirements not met") \
  \
  typedef boost::asio::detail::handler_type_requirements< \
      sizeof( \
        boost::asio::detail::argbyv( \
          boost::asio::detail::clvref(handler))) + \
      sizeof( \
        boost::asio::detail::lvref(handler)( \
          boost::asio::detail::lvref<const boost::system::error_code>()), \
        char(0))>

#define AVHTTP_REQUEST_HANDLER_CHECK( \
    handler_type, handler) \
  \
  BOOST_ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(boost::asio::detail::one_arg_handler_test( \
          handler, \
          static_cast<const boost::system::error_code*>(0))) == 1, \
      "RequestHandler type requirements not met") \
  \
  typedef boost::asio::detail::handler_type_requirements< \
      sizeof( \
        boost::asio::detail::argbyv( \
          boost::asio::detail::clvref(handler))) + \
      sizeof( \
        boost::asio::detail::lvref(handler)( \
          boost::asio::detail::lvref<const boost::system::error_code>()), \
        char(0))>

#define AVHTTP_RECEIVE_HEADER_CHECK( \
    handler_type, handler) \
  \
  BOOST_ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(boost::asio::detail::one_arg_handler_test( \
          handler, \
          static_cast<const boost::system::error_code*>(0))) == 1, \
      "ReceiveHandler type requirements not met") \
  \
  typedef boost::asio::detail::handler_type_requirements< \
      sizeof( \
        boost::asio::detail::argbyv( \
          boost::asio::detail::clvref(handler))) + \
      sizeof( \
        boost::asio::detail::lvref(handler)( \
          boost::asio::detail::lvref<const boost::system::error_code>()), \
        char(0))>

#else

#define AVHTTP_READ_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int

#define AVHTTP_WRITE_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int

#define AVHTTP_OPEN_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int

#define AVHTTP_REQUEST_HANDLER_CHECK( \
	handler_type, handler) \
  typedef int

#define AVHTTP_RECEIVE_HEADER_CHECK( \
	handler_type, handler) \
  typedef int

#endif

} // namespace detail
} // namespace asio
} // namespace boost

#endif // AVHTTP_HANDLER_TYPE_REQUIREMENTS_HPP
