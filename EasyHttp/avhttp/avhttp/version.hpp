//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_VERSION_HPP
#define AVHTTP_VERSION_HPP

#define AVHTTP_VERSION_MAJOR 2
#define AVHTTP_VERSION_MINOR 9
#define AVHTTP_VERSION_TINY 9

// the format of this version is: MMmmtt
// M = Major version, m = minor version, t = tiny version
#define AVHTTP_VERSION_NUM ((AVHTTP_VERSION_MAJOR * 10000) + (AVHTTP_VERSION_MINOR * 100) + AVHTTP_VERSION_TINY)
#define AVHTTP_VERSION "2.9.9"
#define AVHTTP_VERSION_MIME "avhttp/" AVHTTP_VERSION
// #define AVHTTP_REVISION "$Git-Rev$"


#endif // AVHTTP_VERSION_HPP
