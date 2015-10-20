//
// abi_suffix.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#if defined(_MSC_VER)
# pragma warning (pop)
#endif // defined(_MSC_VER)

// No include guard.

#if defined(__GNUC__)
# if (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  pragma GCC visibility pop
# endif // (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#endif // defined(__GNUC__)
