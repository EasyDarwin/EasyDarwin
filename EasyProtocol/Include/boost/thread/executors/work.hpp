//  (C) Copyright 2013,2014 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_THREAD_EXECUTORS_WORK_HPP
#define BOOST_THREAD_EXECUTORS_WORK_HPP

#include <boost/thread/detail/config.hpp>

#if ! defined BOOST_THREAD_EXECUTORS_WORK_ACCEPTS_MOVABLE \
 && ! defined BOOST_THREAD_EXECUTORS_WORK_DONT_ACCEPT_MOVABLE
#define BOOST_THREAD_EXECUTORS_WORK_ACCEPTS_MOVABLE
//#define BOOST_THREAD_EXECUTORS_WORK_DONT_ACCEPT_MOVABLE
#endif

#if defined BOOST_THREAD_EXECUTORS_WORK_ACCEPTS_MOVABLE

#include <boost/thread/detail/nullary_function.hpp>

namespace boost
{
  namespace executors
  {
    typedef detail::nullary_function<void()> work;
  }
} // namespace boost

#else
#include <boost/thread/csbl/functional.hpp>

namespace boost
{
  namespace executors
  {
    typedef csbl::function<void()> work;
  }
} // namespace boost

#endif

#endif //  BOOST_THREAD_EXECUTORS_WORK_HPP
