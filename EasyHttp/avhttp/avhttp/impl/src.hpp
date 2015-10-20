//
// impl/src.hpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_IMPL_SRC_HPP
#define AVHTTP_IMPL_SRC_HPP

#if defined(AVHTTP_HEADER_ONLY)
# error Do not compile avhttp library source with AVHTTP_HEADER_ONLY defined
#endif

#include "avhttp/impl/file.ipp"
#include "avhttp/impl/file_upload.ipp"
#include "avhttp/impl/http_stream.ipp"
#include "avhttp/impl/multi_download.ipp"

#endif // AVHTTP_IMPL_SRC_HPP
