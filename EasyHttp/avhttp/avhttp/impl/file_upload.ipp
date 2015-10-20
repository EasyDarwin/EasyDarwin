//
// impl/file_upload.ipp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_FILE_UPLOAD_IPP
#define AVHTTP_FILE_UPLOAD_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/yield.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "avhttp/http_stream.hpp"

namespace avhttp {

#define FORMBOUNDARY "----AvHttpFormBound"

inline std::string form_boundary()
{
	boost::uuids::basic_random_generator<boost::mt19937> gen;
	std::string random_str = boost::uuids::to_string(boost::uuids::uuid(gen()));
	return FORMBOUNDARY + random_str;
}

namespace mime_types {

	static struct mapping
	{
		const char* extension;
		const char* mime_type;
	} mappings[] =
	{
		{ ".flv", "video/flv" },
		{ ".rmvb", "video/x-pn-realvideo" },
		{ ".gif", "image/gif" },
		{ ".htm", "text/html" },
		{ ".html", "text/html" },
		{ ".jpg", "image/jpeg" },
		{ ".png", "image/png" },

		{ ".htm",   "text/html" },
		{ ".html",  "text/html" },
		{ ".txt",   "text/plain" },
		{ ".xml",   "text/xml" },
		{ ".dtd",   "text/dtd" },
		{ ".css",   "text/css" },

		/* image mime */
		{ ".gif",   "image/gif" },
		{ ".jpe",   "image/jpeg" },
		{ ".jpg",   "image/jpeg" },
		{ ".jpeg",  "image/jpeg" },
		{ ".png",   "image/png" },

		/* media mime */
		{ ".flv", "video/flv" },
		{ ".rmvb", "video/x-pn-realvideo" },
		{ ".mp4", "video/mp4" },
		{ ".3gp", "video/3gpp" },
		{ ".divx", "video/divx" },
		{ ".avi",   "video/avi" },
		{ ".asf",   "video/x-ms-asf" },
		{ ".m1a",   "audio/mpeg" },
		{ ".m2a",   "audio/mpeg" },
		{ ".m1v",   "video/mpeg" },
		{ ".m2v",   "video/mpeg" },
		{ ".mp2",   "audio/mpeg" },
		{ ".mp3",   "audio/mpeg" },
		{ ".mpa",   "audio/mpeg" },
		{ ".mpg",   "video/mpeg" },
		{ ".mpeg",  "video/mpeg" },
		{ ".mpe",   "video/mpeg" },
		{ ".mov",   "video/quicktime" },
		{ ".moov",  "video/quicktime" },
		{ ".oga",   "audio/ogg" },
		{ ".ogg",   "application/ogg" },
		{ ".ogm",   "application/ogg" },
		{ ".ogv",   "video/ogg" },
		{ ".ogx",   "application/ogg" },
		{ ".opus",  "audio/ogg; codecs=opus" },
		{ ".spx",   "audio/ogg" },
		{ ".wav",   "audio/wav" },
		{ ".wma",   "audio/x-ms-wma" },
		{ ".wmv",   "video/x-ms-wmv" },
		{ ".webm",  "video/webm" },

		{ 0, 0 } // Marks end of list.
	};

	/// Convert a file extension into a MIME type.
	inline std::string extension_to_type(const std::string& extension)
	{
		for (mapping* m = mappings; m->extension; ++m)
		{
			if (m->extension == extension)
			{
				return m->mime_type;
			}
		}

		return "application/octet-stream";
	}

} // namespace mime_types

inline std::size_t calc_content_length(const std::string& filename, const std::string& file_of_form,
	const std::string& base_boundary, const file_upload::form_args& args, boost::system::error_code& ec)
{
	using mime_types::extension_to_type;

	std::string boundary = base_boundary;
	std::string short_filename = fs::path(filename).leaf().string();
	std::size_t content_length = fs::file_size(filename, ec);
	std::size_t boundary_size = boundary.size() + 4; // 4 是指 "--" + "\r\n"
	std::size_t extension_size = extension_to_type(boost::to_lower_copy(fs::extension(filename))).size();

	// 各属性选项长度.
	file_upload::form_args::const_iterator i = args.begin();
	for (; i != args.end(); i++)
	{
		content_length += boundary_size;
		content_length += std::string("Content-Disposition: form-data; name=\"\"\r\n\r\n\r\n").size();
		content_length += i->first.size();
		content_length += i->second.size();
	}
	// 文件名选项长度.
	content_length += boundary_size;
	content_length += std::string("Content-Disposition: form-data; name=\"\"; filename=\"\"\r\n"
		"Content-Type: \r\n\r\n").size();
	content_length += extension_size;
	content_length += file_of_form.size();
	content_length += short_filename.size();
	content_length += 4;
	// 结束边界长度.
	content_length += boundary_size;

	return content_length;
}

file_upload::file_upload(boost::asio::io_service& io, bool fake_continue)
	: m_io_service(io)
	, m_http_stream(io)
	, m_base_boundary(form_boundary())
	, m_fake_continue(fake_continue)
{}

file_upload::~file_upload()
{}

template <typename Handler>
struct file_upload::open_coro : boost::asio::coroutine
{
	open_coro(http_stream& http, const std::string& url, const std::string& filename, bool& fake_100,
		const std::string& file_of_form, const form_args& args, std::string& boundary, std::string& base_boundary, Handler handler)
		: m_handler(handler)
		, m_http_stream(http)
		, m_filename(filename)
		, m_file_of_form(file_of_form)
		, m_form_args(args)
		, m_boundary(boundary)
	{
		boost::system::error_code ec;
		request_opts opts = m_http_stream.request_options();

		// 设置为POST模式.
		opts.insert(http_options::request_method, "POST");
		if (!fake_100)
			opts.insert("Expect", "100-continue");
		opts.fake_continue(fake_100);

		// 计算Content-Length.
		std::size_t content_length = calc_content_length(filename, file_of_form, base_boundary, args, ec);
		if (ec)
		{
			m_handler(ec);
			return;
		}

		m_content_disposition = boost::make_shared<std::string>();

		// 转换成字符串.
		std::ostringstream content_length_stream;
		content_length_stream.imbue(std::locale("C"));
		content_length_stream << content_length;
		opts.insert(http_options::content_length, content_length_stream.str());

		// 添加边界等选项并打开url.
		m_boundary = base_boundary;
		opts.insert(http_options::content_type, "multipart/form-data; boundary=" + m_boundary);
		m_boundary = "--" + m_boundary + "\r\n";	// 之后都是单行的分隔.
		m_http_stream.request_options(opts);
		m_http_stream.async_open(url, *this);
	}

	void operator()(boost::system::error_code ec, std::size_t bytes_transfered = 0)
	{
		using mime_types::extension_to_type;

		// 出错, 如果是errc::continue_request则忽略.
		if (ec && (ec != errc::continue_request && ec != errc::fake_continue))
		{
			m_handler(ec);
			return;
		}

		reenter (this)
		{
			// 循环发送表单参数.
			m_iter = m_form_args.begin();
			for (; m_iter != m_form_args.end(); m_iter++)
			{
				// 发送边界.
				yield boost::asio::async_write(m_http_stream, boost::asio::buffer(m_boundary), *this);

				// 发送 Content-Disposition.
				*m_content_disposition = "Content-Disposition: form-data; name=\""
					+ m_iter->first + "\"\r\n\r\n";
				*m_content_disposition += m_iter->second;
				*m_content_disposition += "\r\n";
				yield boost::asio::async_write(m_http_stream,
					boost::asio::buffer(*m_content_disposition), *this);
			}

			// 发送边界.
			yield boost::asio::async_write(m_http_stream, boost::asio::buffer(m_boundary), *this);

			// 发送文件名.
			*m_content_disposition = "Content-Disposition: form-data; name=\""
				+ m_file_of_form + "\"" + "; filename=" + "\""
				+ fs::path(m_filename).leaf().string() + "\"\r\n"
				+ "Content-Type: "
				+ extension_to_type(boost::to_lower_copy(fs::extension(m_filename)))
				+ "\r\n\r\n";
			yield boost::asio::async_write(m_http_stream,
				boost::asio::buffer(*m_content_disposition), *this);

			// 回调用户handler.
			m_handler(ec);
		}
	}

// private:
	Handler m_handler;
	http_stream& m_http_stream;
	std::string m_filename;
	const form_args& m_form_args;
	std::string m_file_of_form;
	std::string& m_boundary;
	boost::shared_ptr<std::string> m_content_disposition;
	form_args::const_iterator m_iter;
};

template <typename Handler>
file_upload::open_coro<Handler>
file_upload::make_open_coro(const std::string& url, const std::string& filename,
	const std::string& file_of_form, const form_args& args, BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	m_form_args = args;
	return open_coro<Handler>(m_http_stream, url, filename, m_fake_continue,
		file_of_form, m_form_args, m_boundary, m_base_boundary, BOOST_ASIO_MOVE_CAST(Handler)(handler));
}

template <typename Handler>
void file_upload::async_open(const std::string& url, const std::string& filename,
	const std::string& file_of_form, const form_args& args, BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	make_open_coro<Handler>(url, filename, file_of_form, args, BOOST_ASIO_MOVE_CAST(Handler)(handler));
}

void file_upload::open(const std::string& url, const std::string& filename,
	const std::string& file_of_form, const form_args& args, boost::system::error_code& ec)
{
	using mime_types::extension_to_type;

	request_opts opts = m_http_stream.request_options();
	m_form_args = args;

	// 设置边界字符串.
	m_boundary = m_base_boundary;

	// 作为发送名字.
	std::string short_filename = fs::path(filename).leaf().string();

	// 设置为POST模式.
	opts.insert(http_options::request_method, "POST");

	// 计算Content-Length.
	std::size_t content_length = calc_content_length(filename, file_of_form, m_base_boundary, args, ec);
	if (ec)
	{
		return;
	}

	// 转换成字符串.
	std::ostringstream content_length_stream;
	content_length_stream.imbue(std::locale("C"));
	content_length_stream << content_length;
	opts.insert(http_options::content_length, content_length_stream.str());

	if (!m_fake_continue)
		opts.insert("Expect", "100-continue");
	opts.fake_continue(m_fake_continue);

	// 添加边界等选项并打开url.
	opts.insert(http_options::content_type, "multipart/form-data; boundary=" + m_boundary);
	m_boundary = "--" + m_boundary + "\r\n";	// 之后都是单行的分隔.
	m_http_stream.request_options(opts);
	m_http_stream.open(url, ec);

	// 出错, 如果是errc::continue_request则忽略.
	if (ec && (ec != errc::continue_request && ec != errc::fake_continue))
	{
		return;
	}

	// 循环发送表单参数.
	std::string content_disposition;
	form_args::const_iterator i = args.begin();
	for (; i != args.end(); i++)
	{
		boost::asio::write(m_http_stream, boost::asio::buffer(m_boundary), ec);
		if (ec)
		{
			return;
		}
		// 发送 Content-Disposition.
		content_disposition = "Content-Disposition: form-data; name=\""
			+ i->first + "\"\r\n\r\n";
		content_disposition += i->second;
		content_disposition += "\r\n";
		boost::asio::write(m_http_stream, boost::asio::buffer(content_disposition), ec);
		if (ec)
		{
			return;
		}
	}

	// 发送文件名.
	boost::asio::write(m_http_stream, boost::asio::buffer(m_boundary), ec);
	if (ec)
	{
		return;
	}
	content_disposition = "Content-Disposition: form-data; name=\""
		+ file_of_form + "\"" + "; filename=" + "\"" + short_filename + "\"\r\n"
		+ "Content-Type: "
		+ extension_to_type(boost::to_lower_copy(fs::extension(short_filename)))
		+ "\r\n\r\n";
	boost::asio::write(m_http_stream, boost::asio::buffer(content_disposition), ec);
	if (ec)
	{
		return;
	}
}

void file_upload::open(const std::string& url, const std::string& filename,
	const std::string& file_of_form, const form_args& args)
{
	boost::system::error_code ec;
	open(url, filename, file_of_form, args, ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
}

template <typename ConstBufferSequence>
std::size_t file_upload::write_some(const ConstBufferSequence& buffers)
{
	return m_http_stream.write_some(buffers);
}

template <typename ConstBufferSequence>
std::size_t file_upload::write_some(const ConstBufferSequence& buffers,
	boost::system::error_code& ec)
{
	return m_http_stream.write_some(buffers, ec);
}

template <typename ConstBufferSequence, typename Handler>
void file_upload::async_write_some(const ConstBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	AVHTTP_WRITE_HANDLER_CHECK(Handler, handler) type_check;

	m_http_stream.async_write_some(buffers, handler);
}

void file_upload::write_tail(boost::system::error_code& ec)
{
	// 发送结尾.
	m_boundary = "\r\n--" + m_base_boundary + "--\r\n";
	boost::asio::write(m_http_stream, boost::asio::buffer(m_boundary), ec);
	// 继续读取http header.
	m_http_stream.receive_header(ec);
}

void file_upload::write_tail()
{
	// 发送结尾.
	m_boundary = "\r\n--" + m_base_boundary + "--\r\n";
	boost::asio::write(m_http_stream, boost::asio::buffer(m_boundary));
	// 继续读取http header.
	m_http_stream.receive_header();
}

template <typename Handler>
struct file_upload::tail_coro : boost::asio::coroutine
{
	tail_coro(std::string& boundary, std::string& base_boundary, http_stream& http, Handler handler)
		: m_boundary(boundary)
		, m_http_stream(http)
		, m_handler(handler)
	{
		// 发送结尾.
		m_boundary = "\r\n--" + base_boundary + "--\r\n";
		boost::asio::async_write(m_http_stream, boost::asio::buffer(m_boundary), *this);
	}

	void operator()(boost::system::error_code ec, std::size_t bytes_transfered = 0)
	{
		if (ec)
		{
			m_handler(ec);
			return;
		}

		reenter (this)
		{
			yield m_http_stream.async_receive_header(*this);
			m_handler(boost::system::error_code());
		}
	}

// private:
	Handler m_handler;
	std::string& m_boundary;
	http_stream& m_http_stream;
};

template <typename Handler>
file_upload::tail_coro<Handler> file_upload::make_tail_coro(BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	return tail_coro<Handler>(m_boundary, m_base_boundary, m_http_stream, BOOST_ASIO_MOVE_CAST(Handler)(handler));
}

template <typename Handler>
void file_upload::async_write_tail(BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	make_tail_coro<Handler>(BOOST_ASIO_MOVE_CAST(Handler)(handler));
}

void file_upload::request_option(request_opts& opts)
{
	m_http_stream.request_options(opts);
}

http_stream& file_upload::get_http_stream()
{
	return m_http_stream;
}

boost::asio::io_service& file_upload::get_io_service()
{
	return m_io_service;
}

} // namespace avhttp

#include <boost/asio/unyield.hpp>

#endif // AVHTTP_FILE_UPLOAD_IPP
