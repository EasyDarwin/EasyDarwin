///////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2014. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_PLACEMENT_NEW_HPP
#define BOOST_CONTAINER_DETAIL_PLACEMENT_NEW_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

struct boost_container_new_t{};

//avoid including <new>
inline void *operator new(std::size_t, void *p, boost_container_new_t)
{  return p;  }

inline void operator delete(void *, void *, boost_container_new_t)
{}

#endif   //BOOST_CONTAINER_DETAIL_PLACEMENT_NEW_HPP
