//
// multi_download.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MULTI_DOWNLOAD_HPP__
#define MULTI_DOWNLOAD_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>
#include <list>
#include <algorithm>    // for std::min/std::max

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>

#include "file.hpp"
#include "http_stream.hpp"
#include "rangefield.hpp"
#include "entry.hpp"
#include "settings.hpp"

namespace avhttp
{

// multi_download类的具体实现.
class multi_download : public boost::noncopyable
{
	// 重定义http_stream_ptr指针.
	typedef boost::shared_ptr<http_stream> http_stream_ptr;

	// 定义http_stream_obj.
	struct http_stream_object
	{
		http_stream_object()
			: request_range(0, 0)
			, bytes_transferred(0)
			, bytes_downloaded(0)
			, done(false)
			, direct_reconnect(false)
		{}

		// http_stream对象.
		http_stream_ptr m_stream;

		// 数据缓冲, 下载时的缓冲.
		boost::array<char, default_buffer_size> m_buffer;

		// 请求的数据范围, 每次由multi_download分配一个下载范围, stream按这个范围去下载.
		range request_range;

		// 本次请求已经下载的数据, 相对于request_range, 当一个request_range下载完成后,
		// bytes_transferred自动置为0.
		boost::int64_t bytes_transferred;

		// 当前对象下载的数据统计.
		boost::int64_t bytes_downloaded;

		// 最后请求的时间.
		boost::posix_time::ptime m_last_request_time;

		// 是否操作功能完成.
		bool done;

		// 立即重新尝试连接.
		bool direct_reconnect;
	};

	// 重定义http_object_ptr指针.
	typedef boost::shared_ptr<http_stream_object> http_object_ptr;

	// 用于计算下载速率.
	struct byte_rate
	{
		byte_rate()
			: current_byte_rate(0)
			, index(0)
			, seconds(5)
		{
			last_byte_rate.resize(seconds);
			for (int i = 0; i < seconds; i++)
				last_byte_rate[i] = 0;
		}

		// 用于统计速率的时间.
		const int seconds;

		// 最后的byte_rate.
		std::vector<int> last_byte_rate;

		// last_byte_rate的下标.
		int index;

		// 当前byte_rate.
		int current_byte_rate;
	};

public:
	multi_download(boost::asio::io_service &io)
		: m_io_service(io)
		, m_accept_multi(false)
		, m_keep_alive(false)
		, m_file_size(-1)
		, m_drop_size(-1)
		, m_number_of_connections(0)
		, m_timer(io, boost::posix_time::seconds(0))
		, m_abort(false)
	{}
	~multi_download()
	{}

public:

	///开始multi_download开始下载.
	// @param u指定的url.
	// @param ec当发生错误时, 包含详细的错误信息.
	// @备注: 直接使用内部的file.hpp下载数据到文件, 若想自己指定数据下载到指定的地方
	// 可以通过调用另一个open来完成, 具体见另一个open的详细说明.
	void start(const url &u, boost::system::error_code &ec)
	{
		settings s;
		start(u, s, ec);
	}

	///开始multi_download开始下载, 打开失败抛出一个异常.
	// @param u指定的url.
	// @备注: 直接使用内部的file.hpp下载数据到文件, 若想自己指定数据下载到指定的地方
	// 可以通过调用另一个open来完成, 具体见另一个open的详细说明.
	void start(const url &u)
	{
		settings s;
		boost::system::error_code ec;
		start(u, s, ec);
		if (ec)
		{
			boost::throw_exception(boost::system::system_error(ec));
		}
	}

	///开始multi_download开始下载.
	// @param u指定的url.
	// @param s指定的设置信息.
	// @失败抛出一个boost::system::system_error异常, 包含详细的错误信息.
	void start(const url &u, const settings &s)
	{
		boost::system::error_code ec;
		start(u, s, ec);
		if (ec)
		{
			boost::throw_exception(boost::system::system_error(ec));
		}
	}

	///开始multi_download开始下载.
	// @param u指定的url.
	// @param s指定的设置信息.
	// @返回error_code, 包含详细的错误信息.
	void start(const url &u, const settings &s, boost::system::error_code &ec)
	{
		// 清空所有连接.
		{
#ifndef AVHTTP_DISABLE_THREAD
			boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
			m_streams.clear();
		}

		// 默认文件大小为-1.
		m_file_size = -1;

		// 保存设置.
		m_settings = s;

		// 解析meta文件.
		if (!fs::exists(m_settings.meta_file))
		{
			// filename + ".meta".
			m_settings.meta_file = (boost::filesystem::path(u.path()).leaf().string() + ".meta");
		}

		// 打开meta文件, 如果打开成功, 则表示解析出相应的位图了.
		if (!open_meta(m_settings.meta_file))
		{
			// 位图打开失败, 无所谓, 下载过程中会创建新的位图, 删除meta文件.
			m_file_meta.close();
			boost::system::error_code ignore;
			fs::remove(m_settings.meta_file, ignore);
		}

		// 创建一个http_stream对象.
		http_object_ptr obj(new http_stream_object);

		request_opts req_opt;
		req_opt.insert("Range", "bytes=0-");
		req_opt.insert("Connection", "keep-alive");

		// 创建http_stream并同步打开, 检查返回状态码是否为206, 如果非206则表示该http服务器不支持多点下载.
		obj->m_stream.reset(new http_stream(m_io_service));
		http_stream &h = *obj->m_stream;
		h.request_options(req_opt);
		// 如果是ssl连接, 默认为检查证书.
		h.check_certificate(m_settings.check_certificate);
		// 添加代理设置.
		h.proxy(m_settings.proxy);
		// 打开http_stream.
		h.open(u, ec);
		// 打开失败则退出.
		if (ec)
		{
			return;
		}

		// 保存最终url信息.
		std::string location = h.location();
		if (!location.empty())
			m_final_url = location;
		else
			m_final_url = u;

		// 判断是否支持多点下载.
		std::string status_code;
		h.response_options().find("_status_code", status_code);
		if (status_code != "206")
			m_accept_multi = false;
		else
			m_accept_multi = true;

		// 得到文件大小.
		std::string length;
		h.response_options().find("Content-Length", length);
		if (length.empty())
		{
			h.response_options().find("Content-Range", length);
			std::string::size_type f = length.find('/');
			if (f++ != std::string::npos)
				length = length.substr(f);
			else
				length = "";
			if (length.empty())
			{
				// 得到不文件长度, 设置为不支持多下载模式.
				m_accept_multi = false;
			}
		}

		boost::int64_t file_size = -1;
		if (!length.empty())
		{
			try
			{
				file_size = boost::lexical_cast<boost::int64_t>(length);
			}
			catch (boost::bad_lexical_cast &)
			{
				// 得不到正确的文件长度, 设置为不支持多下载模式.
				m_accept_multi = false;
			}
		}

		// 按文件大小重新分配rangefield.
		if (file_size != -1 && file_size != m_file_size)
		{
			m_file_size = file_size;
			m_rangefield.reset(m_file_size);
			m_downlaoded_field.reset(m_file_size);
		}

		// 是否支持长连接模式.
		std::string keep_alive;
		h.response_options().find("Connection", keep_alive);
		boost::to_lower(keep_alive);
		if (keep_alive == "keep-alive")
			m_keep_alive = true;
		else
			m_keep_alive = false;

		// 创建存储对象.
		if (!s.storage)
			m_storage.reset(default_storage_constructor());
		else
			m_storage.reset(s.storage());
		BOOST_ASSERT(m_storage);

		// 打开文件, 构造文件名.
		std::string file_name = boost::filesystem::path(m_final_url.path()).leaf().string();
		if (file_name == "/" || file_name == "")
			file_name = boost::filesystem::path(m_final_url.query()).leaf().string();
		if (file_name == "/" || file_name == "" || file_name == ".")
			file_name = "index.html";
		if (!m_settings.save_path.empty())
		{
			if (fs::is_directory(m_settings.save_path))
			{
				fs::path p = m_settings.save_path / file_name;
				file_name = p.string();
			}
			else
			{
				file_name = m_settings.save_path.string();
			}
		}
		m_storage->open(boost::filesystem::path(file_name), ec);
		if (ec)
		{
			return;
		}

		// 保存限速大小.
		m_drop_size = s.download_rate_limit;

		// 处理默认设置.
		if (m_settings.connections_limit == -1)
			m_settings.connections_limit = default_connections_limit;
		if (m_settings.piece_size == -1 && m_file_size != -1)
			m_settings.piece_size = default_piece_size;

		// 关闭stream.
		h.close(ec);
		if (ec)
		{
			return;
		}

		{
#ifndef AVHTTP_DISABLE_THREAD
			boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
			m_streams.push_back(obj);
		}

		// 根据第1个连接返回的信息, 重新设置请求选项.
		req_opt.clear();
		if (m_keep_alive)
			req_opt.insert("Connection", "keep-alive");
		else
			req_opt.insert("Connection", "close");

		// 修改终止状态.
		m_abort = false;

		// 连接计数清0.
		m_number_of_connections = 0;

		// 如果支持多点下载, 按设置创建其它http_stream.
		if (m_accept_multi)
		{
			for (int i = 1; i < m_settings.connections_limit; i++)
			{
				http_object_ptr p(new http_stream_object());
				http_stream_ptr ptr(new http_stream(m_io_service));
				range req_range;

				// 从文件间区中得到一段空间.
				if (!allocate_range(req_range))
				{
					// 分配空间失败, 说明可能已经没有空闲的空间提供给这个stream进行下载了直接跳过好了.
					p->done = true;
					continue;
				}

				// 保存请求区间.
				p->request_range = req_range;

				// 设置请求区间到请求选项中.
				req_opt.remove("Range");
				req_opt.insert("Range",
					boost::str(boost::format("bytes=%lld-%lld") % req_range.left % req_range.right));

				// 设置请求选项.
				ptr->request_options(req_opt);
				// 如果是ssl连接, 默认为检查证书.
				ptr->check_certificate(m_settings.check_certificate);
				// 添加代理设置.
				ptr->proxy(m_settings.proxy);

				// 将连接添加到容器中.
				p->m_stream = ptr;

				{
#ifndef AVHTTP_DISABLE_THREAD
					boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
					m_streams.push_back(p);
				}

				// 保存最后请求时间, 方便检查超时重置.
				p->m_last_request_time = boost::posix_time::microsec_clock::local_time();

				m_number_of_connections++;

				// 开始异步打开, 传入指针http_object_ptr, 以确保多线程安全.
				p->m_stream->async_open(m_final_url,
					boost::bind(&multi_download::handle_open, this,
					i, p, boost::asio::placeholders::error));
			}
		}

		// 为已经第1个已经打开的http_stream发起异步数据请求, index标识为0.
		do
		{
			if (m_accept_multi)
			{
				range req_range;

				// 从文件间区中得到一段空间.
				if (!allocate_range(req_range))
				{
					// 分配空间失败, 说明可能已经没有空闲的空间提供给这个stream进行下载了直接跳过好了.
					obj->done = true;
					break;
				}

				// 保存请求区间.
				obj->request_range = req_range;

				// 设置请求区间到请求选项中.
				req_opt.remove("Range");
				req_opt.insert("Range",
					boost::str(boost::format("bytes=%lld-%lld") % req_range.left % req_range.right));

				// 设置请求选项.
				h.request_options(req_opt);
			}

			// 保存最后请求时间, 方便检查超时重置.
			obj->m_last_request_time = boost::posix_time::microsec_clock::local_time();

			m_number_of_connections++;

			// 如果是ssl连接, 默认为检查证书.
			h.check_certificate(m_settings.check_certificate);

			// 添加代理设置.
			h.proxy(m_settings.proxy);

			// 开始异步打开.
			h.async_open(m_final_url,
				boost::bind(&multi_download::handle_open, this,
				0, obj, boost::asio::placeholders::error));

		} while (0);

		// 开启定时器, 执行任务.
		m_timer.async_wait(boost::bind(&multi_download::on_tick, this));

		return;
	}

	///异步启动下载, 启动完成将回调对应的Handler.
	// @param u 将要下载的URL.
	// @param handler 将被调用在启动完成时. 它必须满足以下条件:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec // 用于返回操作状态.
	//  );
	// @end code
	// @begin example
	//  void start_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // 启动下载成功!
	//    }
	//  }
	//  ...
	//  avhttp::multi_download h(io_service);
	//  h.async_open("http://www.boost.org", start_handler);
	// @end example
	// @备注: handler也可以使用boost.bind来绑定一个符合规定的函数作
	// 为async_start的参数handler.
	template <typename Handler>
	void async_start(const url &u, Handler handler)
	{
		settings s;
		async_start(u, s, handler);
	}

	///异步启动下载, 启动完成将回调对应的Handler.
	// @param u 将要下载的URL.
	// @param s 下载设置参数信息.
	// @param handler 将被调用在启动完成时. 它必须满足以下条件:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec // 用于返回操作状态.
	//  );
	// @end code
	// @begin example
	//  void start_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // 启动下载成功!
	//    }
	//  }
	//  ...
	//  avhttp::multi_download h(io_service);
	//  settings s;
	//  h.async_open("http://www.boost.org", s, start_handler);
	// @end example
	// @备注: handler也可以使用boost.bind来绑定一个符合规定的函数作
	// 为async_start的参数handler.
	template <typename Handler>
	void async_start(const url &u, const settings &s, Handler handler)
	{
		// 清空所有连接.
		{
#ifndef AVHTTP_DISABLE_THREAD
			boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
			m_streams.clear();
		}

		// 清空文件大小.
		m_file_size = -1;

		// 保存参数.
		m_final_url = u;
		m_settings = s;

		// 解析meta文件.
		if (!fs::exists(m_settings.meta_file))
		{
			// filename + ".meta".
			m_settings.meta_file = (boost::filesystem::path(u.path()).leaf().string() + ".meta");
		}

		// 打开meta文件, 如果打开成功, 则表示解析出相应的位图了.
		if (!open_meta(m_settings.meta_file))
		{
			// 位图打开失败, 无所谓, 下载过程中会创建新的位图, 删除meta文件.
			m_file_meta.close();
			boost::system::error_code ignore;
			fs::remove(m_settings.meta_file, ignore);
		}

		// 创建一个http_stream对象.
		http_object_ptr obj(new http_stream_object);

		request_opts req_opt;
		req_opt.insert("Range", "bytes=0-");
		req_opt.insert("Connection", "keep-alive");

		// 创建http_stream并同步打开, 检查返回状态码是否为206, 如果非206则表示该http服务器不支持多点下载.
		obj->m_stream.reset(new http_stream(m_io_service));
		http_stream &h = *obj->m_stream;

		// 设置请求选项.
		h.request_options(req_opt);

		// 如果是ssl连接, 默认为检查证书.
		h.check_certificate(m_settings.check_certificate);

		// 添加代理设置.
		h.proxy(m_settings.proxy);

		typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
		h.async_open(u, boost::bind(&multi_download::handle_start<HandlerWrapper>, this,
			HandlerWrapper(handler), obj, boost::asio::placeholders::error));

		return;
	}

	// stop当前所有连接, 停止工作.
	void stop()
	{
		m_abort = true;

		boost::system::error_code ignore;
		m_timer.cancel(ignore);

#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
		for (std::size_t i = 0; i < m_streams.size(); i++)
		{
			const http_object_ptr &ptr = m_streams[i];
			if (ptr && ptr->m_stream)
				ptr->m_stream->close(ignore);
		}
	}

	///返回当前设置信息.
	const settings& set() const
	{
		return m_settings;
	}

	///设置是否检查证书, 默认检查证书.
	// @param check指定是否检查ssl证书.
	void check_certificate(bool check)
	{
		m_settings.check_certificate = check;
	}

	///返回当前下载的文件大小.
	// @如果服务器不支持多点下载, 则可能文件大小为-1.
	boost::int64_t file_size() const
	{
		return m_file_size;
	}

	///得到当前下载的文件名.
	// @如果请求的url不太规则, 则可能返回错误的文件名.
	std::string file_name() const
	{
		return boost::filesystem::path(m_final_url.path()).leaf().string();
	}

	///当前已经下载的字节总数.
	boost::int64_t bytes_download() const
	{
		if (m_file_size != -1)
			return m_downlaoded_field.range_size();

		boost::int64_t bytes_count = 0;

#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
		for (std::size_t i = 0; i < m_streams.size(); i++)
		{
			const http_object_ptr &ptr = m_streams[i];
			if (ptr)
				bytes_count += ptr->bytes_downloaded;
		}

		return bytes_count;
	}

	///当前下载速率, 单位byte/s.
	int download_rate() const
	{
		return m_byte_rate.current_byte_rate;
	}

	///设置下载速率, -1为无限制, 单位byte/s.
	void download_rate_limit(int rate)
	{
		m_settings.download_rate_limit = rate;
	}

	///返回当前限速.
	int download_rate_limit() const
	{
		return m_settings.download_rate_limit;
	}

protected:
	void handle_open(const int index,
		http_object_ptr object_ptr, const boost::system::error_code &ec)
	{
		http_stream_object &object = *object_ptr;
		if (ec || m_abort)
		{
			// 输出错误信息, 然后退出, 让on_tick检查到超时后重新连接.
			// std::cerr << index << " handle_open: " << ec.message().c_str() << std::endl;

			// 单连接模式, 表示下载停止, 终止下载.
			if (!m_accept_multi)
			{
				m_abort = true;
				boost::system::error_code ignore;
				m_timer.cancel(ignore);
			}

			return;
		}

		// 保存最后请求时间, 方便检查超时重置.
		object.m_last_request_time = boost::posix_time::microsec_clock::local_time();

		// 计算可请求的字节数.
		int available_bytes = default_buffer_size;
		if (m_drop_size != -1)
		{
			available_bytes = std::min(m_drop_size, default_buffer_size);
			m_drop_size -= available_bytes;
			if (available_bytes == 0)
			{
				// 避免空请求占用大量CPU, 让出CPU资源.
				boost::this_thread::sleep(boost::posix_time::millisec(1));
			}
		}

		// 发起数据读取请求.
		http_stream_ptr &stream_ptr = object.m_stream;

		// 传入指针http_object_ptr, 以确保多线程安全.
		stream_ptr->async_read_some(boost::asio::buffer(object.m_buffer, available_bytes),
			boost::bind(&multi_download::handle_read, this,
			index, object_ptr,
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error));
	}

	void handle_read(const int index,
		http_object_ptr object_ptr, int bytes_transferred, const boost::system::error_code &ec)
	{
		http_stream_object &object = *object_ptr;

		// 保存数据, 当远程服务器断开时, ec为eof, 保证数据全部写入.
		if (m_storage && bytes_transferred != 0 && (!ec || ec == boost::asio::error::eof))
		{
			// 计算offset.
			boost::int64_t offset = object.request_range.left + object.bytes_transferred;

			// 更新完成下载区间位图.
			if (m_file_size != -1)
				m_downlaoded_field.update(offset, offset + bytes_transferred);

			// 使用m_storage写入.
			m_storage->write(object.m_buffer.c_array(),	offset, bytes_transferred);
		}

		// 如果发生错误或终止.
		if (ec || m_abort)
		{
			// 输出错误信息, 然后退出, 让on_tick检查到超时后重新连接.
			// std::cerr << index << " handle_read: " << ec.message().c_str() << std::endl;

			// 单连接模式, 表示下载停止, 终止下载.
			if (!m_accept_multi)
			{
				m_abort = true;
				boost::system::error_code ignore;
				m_timer.cancel(ignore);
			}

			return;
		}

		// 统计本次已经下载的总字节数.
		object.bytes_transferred += bytes_transferred;

		// 统计总下载字节数.
		object.bytes_downloaded += bytes_transferred;

		// 用于计算下载速率.
		m_byte_rate.last_byte_rate[m_byte_rate.index] += bytes_transferred;

		// 判断请求区间的数据已经下载完成, 如果下载完成, 则分配新的区间, 发起新的请求.
		if (m_accept_multi && object.bytes_transferred >= object.request_range.size())
		{
			// 不支持长连接, 则创建新的连接.
			if (!m_keep_alive)
			{
				// 新建新的http_stream对象.
				object.direct_reconnect = true;
				return;
			}

			http_stream &stream = *object.m_stream;

			// 配置请求选项.
			request_opts req_opt;

			// 设置是否为长连接.
			if (m_keep_alive)
				req_opt.insert("Connection", "keep-alive");

			// 如果分配空闲空间失败, 则跳过这个socket, 并立即尝试连接这个socket.
			if (!allocate_range(object.request_range))
			{
				object.direct_reconnect = true;
				return;
			}

			// 清空计数.
			object.bytes_transferred = 0;

			// 插入新的区间请求.
			req_opt.insert("Range",
				boost::str(boost::format("bytes=%lld-%lld")
				% object.request_range.left % object.request_range.right));

			// 设置到请求选项中.
			stream.request_options(req_opt);

			// 如果是ssl连接, 默认为检查证书.
			stream.check_certificate(m_settings.check_certificate);

			// 添加代理设置.
			stream.proxy(m_settings.proxy);

			// 保存最后请求时间, 方便检查超时重置.
			object.m_last_request_time = boost::posix_time::microsec_clock::local_time();

			// 发起异步http数据请求, 传入指针http_object_ptr, 以确保多线程安全.
			if (!m_keep_alive)
				stream.async_open(m_final_url, boost::bind(&multi_download::handle_open, this,
					index, object_ptr, boost::asio::placeholders::error));
			else
				stream.async_request(req_opt, boost::bind(&multi_download::handle_request, this,
					index, object_ptr, boost::asio::placeholders::error));
		}
		else
		{
			// 服务器不支持多点下载, 说明数据已经下载完成.
			if (!m_accept_multi &&
				(m_file_size != -1 && object.bytes_downloaded == m_file_size))
			{
				m_abort = true;
				boost::system::error_code ignore;
				m_timer.cancel(ignore);
				return;
			}

			// 保存最后请求时间, 方便检查超时重置.
			object.m_last_request_time = boost::posix_time::microsec_clock::local_time();

			// 计算可请求的字节数.
			int available_bytes = default_buffer_size;
			if (m_drop_size != -1)
			{
				available_bytes = std::min(m_drop_size, default_buffer_size);
				m_drop_size -= available_bytes;
				if (available_bytes == 0)
				{
					// 避免空请求占用大量CPU, 让出CPU资源.
					boost::this_thread::sleep(boost::posix_time::millisec(1));
				}
			}

			// 继续读取数据, 传入指针http_object_ptr, 以确保多线程安全.
			object.m_stream->async_read_some(boost::asio::buffer(object.m_buffer, available_bytes),
				boost::bind(&multi_download::handle_read, this,
				index, object_ptr,
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error));
		}
	}

	void handle_request(const int index,
		http_object_ptr object_ptr, const boost::system::error_code &ec)
	{
		http_stream_object &object = *object_ptr;
		if (ec || m_abort)
		{
			// 输出错误信息, 然后退出, 让on_tick检查到超时后重新连接.
			// std::cerr << index << " handle_request: " << ec.message().c_str() << std::endl;

			// 单连接模式, 表示下载停止, 终止下载.
			if (!m_accept_multi)
			{
				m_abort = true;
				boost::system::error_code ignore;
				m_timer.cancel(ignore);
			}

			return;
		}

		// 保存最后请求时间, 方便检查超时重置.
		object.m_last_request_time = boost::posix_time::microsec_clock::local_time();

		// 计算可请求的字节数.
		int available_bytes = default_buffer_size;
		if (m_drop_size != -1)
		{
			available_bytes = std::min(m_drop_size, default_buffer_size);
			m_drop_size -= available_bytes;
			if (available_bytes == 0)
			{
				// 避免空请求占用大量CPU, 让出CPU资源.
				boost::this_thread::sleep(boost::posix_time::millisec(1));
			}
		}

		// 发起数据读取请求, 传入指针http_object_ptr, 以确保多线程安全.
		object_ptr->m_stream->async_read_some(boost::asio::buffer(object.m_buffer, available_bytes),
			boost::bind(&multi_download::handle_read, this,
			index, object_ptr,
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error));
	}

	template <typename Handler>
	void handle_start(Handler handler, http_object_ptr object_ptr, const boost::system::error_code &ec)
	{
		// 打开失败则退出.
		if (ec)
		{
			handler(ec);
			return;
		}

		// 下面使用引用http_stream_object对象.
		http_stream_object &object = *object_ptr;

		// 同样引用http_stream对象.
		http_stream &h = *object.m_stream;

		// 保存最终url信息.
		std::string location = h.location();
		if (!location.empty())
			m_final_url = location;

		// 判断是否支持多点下载.
		std::string status_code;
		h.response_options().find("_status_code", status_code);
		if (status_code != "206")
			m_accept_multi = false;
		else
			m_accept_multi = true;

		// 得到文件大小.
		std::string length;
		boost::int64_t file_size = -1;
		h.response_options().find("Content-Length", length);
		if (length.empty())
		{
			h.response_options().find("Content-Range", length);
			std::string::size_type f = length.find('/');
			if (f++ != std::string::npos)
				length = length.substr(f);
			else
				length = "";
			if (length.empty())
			{
				// 得到不文件长度, 设置为不支持多下载模式.
				m_accept_multi = false;
			}
		}

		if (!length.empty())
		{
			try
			{
				file_size = boost::lexical_cast<boost::int64_t>(length);
			}
			catch (boost::bad_lexical_cast &)
			{
				// 得不到正确的文件长度, 设置为不支持多下载模式.
				m_accept_multi = false;
			}
		}

		// 按文件大小分配rangefield.
		if (file_size == -1 && file_size != m_file_size)
		{
			m_file_size = file_size;
			m_rangefield.reset(m_file_size);
			m_downlaoded_field.reset(m_file_size);
		}

		// 是否支持长连接模式.
		std::string keep_alive;
		h.response_options().find("Connection", keep_alive);
		boost::to_lower(keep_alive);
		if (keep_alive == "keep-alive")
			m_keep_alive = true;
		else
			m_keep_alive = false;

		// 创建存储对象.
		if (!m_settings.storage)
			m_storage.reset(default_storage_constructor());
		else
			m_storage.reset(m_settings.storage());
		BOOST_ASSERT(m_storage);

		// 打开文件, 构造文件名.
		std::string file_name = boost::filesystem::path(m_final_url.path()).leaf().string();
		if (file_name == "/" || file_name == "")
			file_name = boost::filesystem::path(m_final_url.query()).leaf().string();
		if (file_name == "/" || file_name == "" || file_name == ".")
			file_name = "index.html";
		if (!m_settings.save_path.empty())
		{
			if (fs::is_directory(m_settings.save_path))
			{
				fs::path p = m_settings.save_path / file_name;
				file_name = p.string();
			}
			else
			{
				file_name = m_settings.save_path.string();
			}
		}
		boost::system::error_code ignore;
		m_storage->open(boost::filesystem::path(file_name), ignore);
		if (ignore)
		{
			handler(ignore);
			return;
		}

		// 处理默认设置.
		if (m_settings.connections_limit == -1)
			m_settings.connections_limit = default_connections_limit;
		if (m_settings.piece_size == -1 && m_file_size != -1)
			m_settings.piece_size = default_piece_size;

		// 关闭stream.
		h.close(ignore);
		if (ignore)
		{
			handler(ignore);
			return;
		}

		{
#ifndef AVHTTP_DISABLE_THREAD
			boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
			m_streams.push_back(object_ptr);
		}

		// 根据第1个连接返回的信息, 设置请求选项.
		request_opts req_opt;
		if (m_keep_alive)
			req_opt.insert("Connection", "keep-alive");
		else
			req_opt.insert("Connection", "close");

		// 修改终止状态.
		m_abort = false;

		// 连接计数清0.
		m_number_of_connections = 0;

		// 如果支持多点下载, 按设置创建其它http_stream.
		if (m_accept_multi)
		{
			for (int i = 1; i < m_settings.connections_limit; i++)
			{
				http_object_ptr p(new http_stream_object());
				http_stream_ptr ptr(new http_stream(m_io_service));
				range req_range;

				// 从文件间区中得到一段空间.
				if (!allocate_range(req_range))
				{
					// 分配空间失败, 说明可能已经没有空闲的空间提供给这个stream进行下载了直接跳过好了.
					p->done = true;
					continue;
				}

				// 保存请求区间.
				p->request_range = req_range;

				// 设置请求区间到请求选项中.
				req_opt.remove("Range");
				req_opt.insert("Range",
					boost::str(boost::format("bytes=%lld-%lld") % req_range.left % req_range.right));

				// 设置请求选项.
				ptr->request_options(req_opt);

				// 如果是ssl连接, 默认为检查证书.
				ptr->check_certificate(m_settings.check_certificate);

				// 添加代理设置.
				ptr->proxy(m_settings.proxy);

				// 将连接添加到容器中.
				p->m_stream = ptr;

				{
#ifndef AVHTTP_DISABLE_THREAD
					boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
					m_streams.push_back(p);
				}

				// 保存最后请求时间, 方便检查超时重置.
				p->m_last_request_time = boost::posix_time::microsec_clock::local_time();

				m_number_of_connections++;

				// 开始异步打开, 传入指针http_object_ptr, 以确保多线程安全.
				p->m_stream->async_open(m_final_url,
					boost::bind(&multi_download::handle_open, this,
					i, p, boost::asio::placeholders::error));
			}
		}

		// 为已经第1个已经打开的http_stream发起异步数据请求, index标识为0.
		do
		{
			if (m_accept_multi)
			{
				range req_range;

				// 从文件间区中得到一段空间.
				if (!allocate_range(req_range))
				{
					// 分配空间失败, 说明可能已经没有空闲的空间提供给这个stream进行下载了直接跳过好了.
					object.done = true;
					break;
				}

				// 保存请求区间.
				object.request_range = req_range;

				// 设置请求区间到请求选项中.
				req_opt.remove("Range");
				req_opt.insert("Range",
					boost::str(boost::format("bytes=%lld-%lld") % req_range.left % req_range.right));

				// 设置请求选项.
				h.request_options(req_opt);
			}

			// 保存最后请求时间, 方便检查超时重置.
			object.m_last_request_time = boost::posix_time::microsec_clock::local_time();

			m_number_of_connections++;

			// 添加代理设置.
			h.proxy(m_settings.proxy);

			// 如果是ssl连接, 默认为检查证书.
			h.check_certificate(m_settings.check_certificate);

			// 开始异步打开.
			h.async_open(m_final_url,
				boost::bind(&multi_download::handle_open, this,
				0, object_ptr, boost::asio::placeholders::error));

		} while (0);

		// 开启定时器, 执行任务.
		m_timer.async_wait(boost::bind(&multi_download::on_tick, this));

		// 回调通知用户, 已经成功启动下载.
		handler(ec);

		return;
	}

	void on_tick()
	{
		// 在这里更新位图.
		if (m_accept_multi)
		{
			update_meta();
		}

		// 每隔1秒进行一次on_tick.
		if (!m_abort)
		{
			m_timer.expires_at(m_timer.expires_at() + boost::posix_time::seconds(1));
			m_timer.async_wait(boost::bind(&multi_download::on_tick, this));
		}
		else
		{
			// 已经终止.
			return;
		}

		// 用于计算动态下载速率.
		{
			int bytes_count = 0;

			for (int i = 0; i < m_byte_rate.seconds; i++)
				bytes_count += m_byte_rate.last_byte_rate[i];

			m_byte_rate.current_byte_rate = (double)bytes_count / m_byte_rate.seconds;

			if (m_byte_rate.index + 1 >= m_byte_rate.seconds)
				m_byte_rate.last_byte_rate[m_byte_rate.index = 0] = 0;
			else
				m_byte_rate.last_byte_rate[++m_byte_rate.index] = 0;
		}

		// 计算限速.
		m_drop_size = m_settings.download_rate_limit;

#ifndef AVHTTP_DISABLE_THREAD
		// 锁定m_streams容器进行操作, 保证m_streams操作的唯一性.
		boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
		for (std::size_t i = 0; i < m_streams.size(); i++)
		{
			http_object_ptr &object_item_ptr = m_streams[i];
			boost::posix_time::time_duration duration =
				boost::posix_time::microsec_clock::local_time() - object_item_ptr->m_last_request_time;

			if (!object_item_ptr->done &&	(duration > boost::posix_time::seconds(m_settings.time_out)
				|| object_item_ptr->direct_reconnect))
			{
				// 超时, 关闭并重新创建连接.
				boost::system::error_code ec;
				object_item_ptr->m_stream->close(ec);

				// std::cerr << "connection: " << i << " time out!!!" << std::endl;

				// 单连接模式, 表示下载停止, 终止下载.
				if (!m_accept_multi)
				{
					m_abort = true;
					object_item_ptr->done = true;
					m_number_of_connections--;
					continue;
				}

				// 重置重连标识.
				object_item_ptr->direct_reconnect = false;

				// 重新创建http_object和http_stream.
				object_item_ptr.reset(new http_stream_object(*object_item_ptr));
				http_stream_object &object = *object_item_ptr;

				// 使用新的http_stream对象.
				object.m_stream.reset(new http_stream(m_io_service));

				http_stream &stream = *object.m_stream;

				// 配置请求选项.
				request_opts req_opt;

				// 设置是否为长连接.
				if (m_keep_alive)
					req_opt.insert("Connection", "keep-alive");

				// 继续从上次未完成的位置开始请求.
				if (m_accept_multi)
				{
					boost::int64_t begin = object.request_range.left + object.bytes_transferred;
					boost::int64_t end = object.request_range.right;

					if (end - begin <= 0)
					{
						// 如果分配空闲空间失败, 则跳过这个socket.
						if (!allocate_range(object.request_range))
						{
							object.done = true;	// 已经没什么可以下载了.
							m_number_of_connections--;
							continue;
						}

						object.bytes_transferred = 0;
						begin = object.request_range.left;
						end = object.request_range.right;
					}

					req_opt.insert("Range",
						boost::str(boost::format("bytes=%lld-%lld") % begin % end));
				}

				// 设置到请求选项中.
				stream.request_options(req_opt);

				// 如果是ssl连接, 默认为检查证书.
				stream.check_certificate(m_settings.check_certificate);

				// 添加代理设置.
				stream.proxy(m_settings.proxy);

				// 保存最后请求时间, 方便检查超时重置.
				object.m_last_request_time = boost::posix_time::microsec_clock::local_time();

				// 重新发起异步请求, 传入object_item_ptr指针, 以确保线程安全.
				stream.async_open(m_final_url, boost::bind(&multi_download::handle_open, this,
					i, object_item_ptr, boost::asio::placeholders::error));
			}
		}

		// 统计操作功能完成的http_stream的个数.
		int done = 0;
		for (std::size_t i = 0; i < m_streams.size(); i++)
		{
			http_object_ptr &object_item_ptr = m_streams[i];
			if (object_item_ptr->done)
				done++;
		}

		// 检查位图是否已经满以及异步操作是否完成.
		if (done == m_streams.size() && (m_downlaoded_field.is_full() || !m_accept_multi))
		{
			boost::system::error_code ignore;
			m_abort = true;
			m_timer.cancel(ignore);
			return;
		}
	}

	bool allocate_range(range &r)
	{
		boost::mutex::scoped_lock lock(m_rangefield_mutex);
		range temp(-1, -1);
		do
		{
			// 从文件间区中得到一段空间.
			if (!m_rangefield.out_space(temp))
				return false;

			// 用于调试.
			BOOST_ASSERT(temp != r);
			BOOST_ASSERT(temp.size() >= 0);

			// 重新计算为最大max_request_bytes大小.
			boost::int64_t max_request_bytes = m_settings.request_piece_num * m_settings.piece_size;
			if (temp.size() > max_request_bytes)
				temp.right = temp.left + max_request_bytes;

			r = temp;

			// 从m_rangefield中分配这个空间.
			if (!m_rangefield.update(temp))
				continue;
			else
				break;
		} while (!m_abort);

		// 右边边界减1, 因为http请求的区间是包含右边界值, 下载时会将right下标位置的字节下载.
		if (--r.right < r.left)
			return false;

		return true;
	}

	bool open_meta(const fs::path &file_path)
	{
		boost::system::error_code ec;

		// 得到文件大小.
		boost::uintmax_t size = fs::file_size(file_path, ec);
		if (ec)
			size = 0;

		// 打开文件.
		m_file_meta.open(file_path, ec);
		if (ec)
			return false;

		// 如果有数据, 则解码meta数据.
		if (size != 0)
		{
			std::vector<char> buffer;

			buffer.resize(size);
			m_file_meta.read(&buffer[0], 0, size);

			entry e = bdecode(buffer.begin(), buffer.end());

			// 最终的url.
			m_final_url = e["final_url"].string();

			// 文件大小.
			m_file_size = e["file_size"].integer();
			m_rangefield.reset(m_file_size);
			m_downlaoded_field.reset(m_file_size);

			// 分片大小.
			m_settings.piece_size = e["piece_size"].integer();

			// 分片数.
			int piece_num = e["piece_num"].integer();

			// 位图数据.
			std::string bitfield_data = e["bitfield"].string();

			// 构造到位图.
			bitfield bf(bitfield_data.c_str(), piece_num);

			// 更新到区间范围.
			m_rangefield.bitfield_to_range(bf, m_settings.piece_size);
			m_downlaoded_field.bitfield_to_range(bf, m_settings.piece_size);
		}

		return true;
	}

	void update_meta()
	{
		if (!m_file_meta.is_open())
		{
			boost::system::error_code ec;
			m_file_meta.open(m_settings.meta_file, ec);
			if (ec)
				return ;
		}

		entry e;

		e["final_url"] = m_final_url.to_string();
		e["file_size"] = m_file_size;
		e["piece_size"] = m_settings.piece_size;
		e["piece_num"] = (m_file_size / m_settings.piece_size) +
			(m_file_size % m_settings.piece_size == 0 ? 0 : 1);
		bitfield bf;
		m_downlaoded_field.range_to_bitfield(bf, m_settings.piece_size);
		std::string str(bf.bytes(), bf.bytes_size());
		e["bitfield"] = str;

		std::vector<char> buffer;
		bencode(back_inserter(buffer), e);

		m_file_meta.write(&buffer[0], 0, buffer.size());
	}

private:
	// io_service引用.
	boost::asio::io_service &m_io_service;

	// 每一个http_stream_obj是一个http连接.
	// 注意: 容器中的http_object_ptr只能在on_tick一处进行写操作, 并且确保其它地方
	// 是新的副本, 这主要体现在发起新的异步操作的时候将http_object_ptr作为参数形式
	// 传入, 这样在异步回调中只需要访问http_object_ptr的副本指针, 而不是直接访问
	// m_streams!!!
	std::vector<http_object_ptr> m_streams;

#ifndef AVHTTP_DISABLE_THREAD
	// 为m_streams在多线程环境下线程安全.
	mutable boost::mutex m_streams_mutex;
#endif

	// 最终的url, 如果有跳转的话, 是跳转最后的那个url.
	url m_final_url;

	// 是否支持多点下载.
	bool m_accept_multi;

	// 是否支持长连接.
	bool m_keep_alive;

	// 文件大小, 如果没有文件大小值为-1.
	boost::int64_t m_file_size;

	// 当前用户设置.
	settings m_settings;

	// 定时器, 用于定时执行一些任务, 比如检查连接是否超时之类.
	boost::asio::deadline_timer m_timer;

	// 动态计算速率.
	byte_rate m_byte_rate;

	// 实际连接数.
	int m_number_of_connections;

	// 下载数据存储接口指针, 可由用户定义, 并在open时指定.
	boost::scoped_ptr<storage_interface> m_storage;

	// meta文件, 用于续传.
	file m_file_meta;

	// 文件区间图, 每次请求将由m_rangefield来分配空间区间.
	rangefield m_rangefield;

	// 已经下载的文件区间.
	rangefield m_downlaoded_field;

	// 保证分配空闲区间的唯一性.
	boost::mutex m_rangefield_mutex;

	// 用于限速.
	int m_drop_size;

	// 是否中止工作.
	bool m_abort;
};

} // avhttp

#endif // MULTI_DOWNLOAD_HPP__
