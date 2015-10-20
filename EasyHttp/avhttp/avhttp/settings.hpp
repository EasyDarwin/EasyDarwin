//
// settings.hpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>
#include <map>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>

#include "storage_interface.hpp"

namespace avhttp {

// 如果没有定义最大重定向次数, 则默认为5次最大重定向.
#ifndef AVHTTP_MAX_REDIRECTS
#define AVHTTP_MAX_REDIRECTS 5
#endif

namespace http_options {

	// 定义一些常用的　http 选项为 const string , 这样就不用记忆那些单词了，呵呵.
	static const std::string request_method("_request_method");
	static const std::string request_body("_request_body");
	static const std::string status_code("_status_code");
	static const std::string cookie("cookie");
	static const std::string referer("Referer");
	static const std::string content_type("Content-Type");
	static const std::string content_length("Content-Length");
	static const std::string connection("Connection");
} // namespace http_options

// 具体的http的option选项实现.

class option
{
public:
	// 定义option_item类型.
	typedef std::pair<std::string, std::string> option_item;
	// 定义option_item_list类型.
	typedef std::vector<option_item> option_item_list;
	// for boost::assign::insert
	typedef option_item value_type;
public:
	option() {}
	~option() {}

public:

	// 这样就允许这样的应用:
	// http_stream s;
	// s.request_options(request_opts()("cookie","XXXXXX"));
	option & operator()(const std::string &key, const std::string &val)
	{
		insert(key, val);
		return *this;
	}

	// 添加选项, 由key/value形式添加.
	void insert(const std::string &key, const std::string &val)
	{
		m_opts.push_back(option_item(key, val));
	}

	// 添加选项，由 std::part 形式.
	void insert(value_type & item)
	{
		m_opts.push_back(item);
	}

	// 删除选项.
	void remove(const std::string &key)
	{
		for (option_item_list::iterator i = m_opts.begin(); i != m_opts.end(); i++)
		{
			if (i->first == key)
			{
				m_opts.erase(i);
				return;
			}
		}
	}

	// 查找指定key的value.
	bool find(const std::string &key, std::string &val) const
	{
		std::string s = key;
		boost::to_lower(s);
		for (option_item_list::const_iterator f = m_opts.begin(); f != m_opts.end(); f++)
		{
			std::string temp = f->first;
			boost::to_lower(temp);
			if (temp == s)
			{
				val = f->second;
				return true;
			}
		}
		return false;
	}

	// 查找指定的 key 的 value. 没找到返回 ""，　这是个偷懒的帮助.
	std::string find(const std::string & key) const
	{
		std::string v;
		find(key,v);
		return v;
	}

	// 得到Header字符串.
	std::string header_string() const
	{
		std::string str;
		for (option_item_list::const_iterator f = m_opts.begin(); f != m_opts.end(); f++)
		{
			if (f->first != http_options::status_code)
				str += (f->first + ": " + f->second + "\r\n");
		}
		return str;
	}

	// 清空.
	void clear()
	{
		m_opts.clear();
	}

	// 返回所有option.
	option_item_list& option_all()
	{
		return m_opts;
	}

protected:
	option_item_list m_opts;
};

// 请求时的http选项.
// 以下选项为必http选项:
// _request_method, 取值 "GET/POST/HEAD", 默认为"GET".
// _request_body, 请求中的body内容, 取值任意, 默认为空.
// Host, 取值为http服务器, 默认为http服务器.
// Accept, 取值任意, 默认为"*/*".
typedef option request_opts;

// http服务器返回的http选项.
// 一般会包括以下几个选项:
// _status_code, http返回状态.
// Server, 服务器名称.
// Content-Length, 数据内容长度.
// Connection, 连接状态标识.
typedef option response_opts;



// Http请求的代理设置.

struct proxy_settings
{
	proxy_settings()
		: type (none)
	{}

	std::string hostname;
	int port;

	std::string username;
	std::string password;

	enum proxy_type
	{
		// 没有设置代理.
		none,
		// socks4代理, 需要username.
		socks4,
		// 不需要用户密码的socks5代理.
		socks5,
		// 需要用户密码认证的socks5代理.
		socks5_pw,
		// TODO: http代理, 不需要认证.
		http,
		// TODO: http代理, 需要认证.
		http_pw,
	};

	proxy_type type;
};


// 一些默认的值.
static const int default_request_piece_num = 10;
static const int default_time_out = 11;
static const int default_piece_size = 32768;
static const int default_connections_limit = 5;
static const int default_buffer_size = 1024;

// multi_download下载设置.

struct settings
{
	// 下载模式.
	enum downlad_mode
	{
		// 紧凑模式下载, 紧凑模式是指, 将文件分片后, 从文件头开始, 一片紧接着一片,
		// 连续不断的下载.
		compact_mode,

		// TODO: 松散模式下载, 是指将文件分片后, 按连接数平分为N大块进行下载.
		dispersion_mode,

		// TODO: 快速读取模式下载, 这个模式是根据用户读取数据位置开始下载数据, 是尽快响应
		// 下载用户需要的数据.
		quick_read_mode
	};

	settings ()
		: download_rate_limit(-1)
		, connections_limit(default_connections_limit)
		, piece_size(default_piece_size)
		, time_out(default_time_out)
		, request_piece_num(default_request_piece_num)
		, current_downlad_mode(dispersion_mode)
		, check_certificate(true)
		, storage(NULL)
	{}

	// 下载速率限制, -1为无限制, 单位为: byte/s.
	int download_rate_limit;

	// 连接数限制, -1为默认.
	int connections_limit;

	// 分块大小, 默认根据文件大小自动计算.
	int piece_size;

	// 超时断开, 默认为11秒.
	int time_out;

	// 每次请求的分片数, 默认为10.
	int request_piece_num;

	// 下载模式, 默认为dispersion_mode.
	downlad_mode current_downlad_mode;

	// meta_file路径, 默认为当前路径下同文件名的.meta文件.
	fs::path meta_file;

	// 下载文件路径, 默认为当前目录.
	fs::path save_path;

	// 设置是否检查证书, 默认检查证书.
	bool check_certificate;

	// 存储接口创建函数指针, 默认为multi_download提供的file.hpp实现.
	storage_constructor_type storage;

	// 代理设置.
	proxy_settings proxy;
};

} // namespace avhttp

#endif // __SETTINGS_HPP__
