//
// default_storage.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_DEFAULT_STORAGE_HPP
#define AVHTTP_DEFAULT_STORAGE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "avhttp/file.hpp"
#include "avhttp/storage_interface.hpp"

namespace avhttp {

class default_storge : public storage_interface
{
public:
	default_storge()
	{}

	virtual ~default_storge()
	{}

	// 存储组件初始化.
	// @param file_path指定了文件名路径信息.
	// @param ec在出错时保存了详细的错误信息.
	virtual void open(const fs::path& file_path, boost::system::error_code& ec)
	{
		m_file.open(file_path, file::read_write, ec);
	}

	// 关闭存储组件.
	virtual void close()
	{
		m_file.close();
	}

	// 写入数据到文件.
	// @param buf是需要写入的数据缓冲.
	// @param size指定了写入的数据缓冲大小.
	// @返回值为实际写入的字节数, 返回-1表示写入失败.
	// 备注: 在文件指针当前位置写入, 写入完成将自动移动文件指针到完成位置, 保证和fwrite行为一至.
	virtual std::streamsize write(const char* buf, int size)
	{
		return m_file.write(buf, size);
	}

	// 写入数据.
	// @param buf是需要写入的数据缓冲.
	// @param offset是写入的偏移位置.
	// @param size指定了写入的数据缓冲大小.
	// @返回值为实际写入的字节数, 返回-1表示写入失败.
	virtual std::streamsize write(const char* buf, boost::int64_t offset, int size)
	{
		return m_file.write(offset, buf, size);
	}

	// 从文件读取数据.
	// @param buf是需要读取的数据缓冲.
	// @param size指定了读取的数据缓冲大小.
	// @返回值为实际读取的字节数, 返回-1表示读取失败.
	// 备注: 在文件指针当前位置开始读取, 读取完成将自动移动文件指针到完成位置, 保证和fread行为一至.
	virtual std::streamsize read(char* buf, int size)
	{
		return m_file.read(buf, size);
	}

	// 读取数据.
	// @param buf是需要读取的数据缓冲.
	// @param offset是读取的偏移位置.
	// @param size指定了读取的数据缓冲大小.
	// @返回值为实际读取的字节数, 返回-1表示读取失败.
	virtual std::streamsize read(char* buf, boost::int64_t offset, int size)
	{
		return m_file.read(offset, buf, size);
	}

	// 判断是否文件结束.
	// 返回值true表示文件结束.
	virtual bool eof()
	{
		char tmp;
		boost::system::error_code ec;
		boost::int64_t offset = m_file.offset(ec);
		BOOST_ASSERT(!ec);
		int ret = m_file.read(&tmp, 1);
		m_file.offset(offset, ec);
		BOOST_ASSERT(!ec);
		return ret == 0 ? true : false;
	}

protected:
	file m_file;
};

// 默认存储对象.
static storage_interface* default_storage_constructor()
{
	return new default_storge();
}

} // namespace avhttp

#endif // AVHTTP_DEFAULT_STORAGE_HPP
