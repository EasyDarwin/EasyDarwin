//
// storage_interface.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __STORAGE_INTERFACE_HPP__
#define __STORAGE_INTERFACE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <boost/cstdint.hpp>

namespace avhttp {

namespace fs = boost::filesystem;

// 数据存储接口.
struct storage_interface
{
	storage_interface() {}
	virtual ~storage_interface() {}

	// 存储组件初始化.
	// @param file_path指定了文件名路径信息.
	// @param ec在出错时保存了详细的错误信息.
	virtual void open(const fs::path &file_path, boost::system::error_code &ec) = 0;

	// 关闭存储组件.
	virtual void close() = 0;

	// 写入数据.
	// @param buf是需要写入的数据缓冲.
	// @param offset是写入的偏移位置.
	// @param size指定了写入的数据缓冲大小.
	// @返回值为实际写入的字节数, 返回-1表示写入失败.
	virtual int write(const char *buf, boost::uint64_t offset, int size) = 0;

	// 读取数据.
	// @param buf是需要读取的数据缓冲.
	// @param offset是读取的偏移位置.
	// @param size指定了读取的数据缓冲大小.
	// @返回值为实际读取的字节数, 返回-1表示读取失败.
	virtual int read(char *buf, boost::uint64_t offset, int size) = 0;
};

// 重定义storage_interface创建函数指针, 在multi_download内部通过调用它来完成创建storage_interface.
typedef storage_interface* (*storage_constructor_type)();

}

#endif // __STORAGE_INTERFACE_HPP__
