//
// Copyright (C) 2013 Jack.
//
// Author: jack
// Email:  jack.wgm@gmail.com
//

#ifndef AVHTTP_LOGGING_HPP
#define AVHTTP_LOGGING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#ifdef WIN32
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif // !WIN32_LEAN_AND_MEAN
#	include <Windows.h>	 // for win32 Console api.
#endif // WIN32

#define LOGGER_THREAD_SAFE

#include <iostream>
#include <string>
#include <fstream>

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace avhttp {

	///内部使用的简易日志类.
	// 使用说明:
	//	在程序入口(如:main)函数调用 INIT_LOGGER 宏, 它有两个参数, 第一个参数指定了日志文件保存
	//	的路径, 第二个参数指定了日志文件保存的文件名, 详细见INIT_LOGGER.
	//	然后就可以使用LOG_DBG/LOG_INFO/LOG_WARN/LOG_ERR这几个宏来输出日志信息.
	// @begin example
	//  #include "logging.hpp"
	//  int main()
	//  {
	//     AVHTTP_AUTO_LOGGER(".");					// 在当前目录创建以日期命名的日志文件.
	//     // 也可 AVHTTP_INIT_LOGGER("example.log");	// 指定日志文件名.
	//     AVHTTP_LOG_DBG << "Initialized.";
	//     std::string result = do_something();
	//     AVHTTP_LOG_DBG << "do_something return : " << result;	// 输出do_something返回结果到日志.
	//     ...
	//  }
	// @end example
	//
	// 一些可选参数:
	// AVHTTP_LOG_FILE_NUM, 定义 AVHTTP_LOG_FILE_NUM 可指定最大连续运行日志文件个数, 超出将删除旧日志文件.
	// 默认是每天生成按日期的日志文件, 所以, AVHTTP_LOG_FILE_NUM参数也相当是指定保留日志的天数.
	//
	// AVHTTP_LOG_FILE_BUFFER, 定义写入日志文件的写入缓冲大小, 默认为系统指定大小.
	// 该参数实际上指定的是 std::ofstream 的写入缓冲大小.


#ifndef AVHTTP_LOG_FILE_NUM
#	define AVHTTP_LOG_FILE_NUM 3
#endif

#ifndef AVHTTP_LOG_FILE_BUFFER
#	define AVHTTP_LOG_FILE_BUFFER -1
#endif

	class auto_logger_file
	{
	public:
		auto_logger_file()
			: m_auto_mode(false)
		{}
		~auto_logger_file()
		{}

		typedef boost::shared_ptr<std::ofstream> ofstream_ptr;
		typedef std::map<std::string, ofstream_ptr> loglist;

		void open(const char * filename, std::ios_base::openmode flag)
		{
			m_auto_mode = false;
			const char* pos = std::strstr(filename, "*");
			if (pos)
			{
				m_auto_mode = true;
				char save_path[65536] = { 0 };
				std::ptrdiff_t len = pos - filename;
				if (len < 0 || len > 65536)
					return;
				strncpy(save_path, filename, pos - filename);
				m_log_path = save_path;
			}
			else
			{
				m_file.open(filename, flag);
			}
			std::string start_string = "\n\n\n*** starting log ***\n\n\n";
			write(start_string.c_str(), start_string.size());
		}

		bool is_open() const
		{
			if (m_auto_mode) return true;
			return m_file.is_open();
		}

		void write(const char* str, std::streamsize size)
		{
			if (!m_auto_mode)
			{
				if (m_file.is_open())
					m_file.write(str, size);
				return;
			}

			ofstream_ptr of;
			std::string fn = make_filename(m_log_path.string());
			loglist::iterator iter = m_log_list.find(fn);
			if (iter == m_log_list.end())
			{
				of.reset(new std::ofstream);
				of->open(fn.c_str(), std::ios_base::out | std::ios_base::app);
				if (AVHTTP_LOG_FILE_BUFFER != -1)
					of->rdbuf()->pubsetbuf(NULL, AVHTTP_LOG_FILE_BUFFER);
				m_log_list.insert(std::make_pair(fn, of));
				if (m_log_list.size() > AVHTTP_LOG_FILE_NUM)
				{
					iter = m_log_list.begin();
					fn = iter->first;
					ofstream_ptr f = iter->second;
					m_log_list.erase(iter);
					f->close();
					f.reset();
					boost::system::error_code ignore_ec;
					boost::filesystem::remove(fn, ignore_ec);
					if (ignore_ec)
						std::cout << "delete log failed: " << fn <<
							", error code: " << ignore_ec.message() << std::endl;
				}
			}
			else
			{
				of = iter->second;
			}

			if (of->is_open())
			{
				(*of).write(str, size);
				(*of).flush();
			}
		}

		void flush()
		{
			m_file.flush();
		}

		std::string make_filename(const std::string &p = "") const
		{
			boost::posix_time::ptime time = boost::posix_time::second_clock::local_time();
			if (m_last_day != boost::posix_time::not_a_date_time && m_last_day.date().day() == time.date().day())
				return m_last_filename;
			m_last_day = time;
			std::ostringstream oss;
			boost::posix_time::time_facet* _facet = new boost::posix_time::time_facet("%Y%m%d");
			oss.imbue(std::locale(std::locale::classic(), _facet));
			oss << boost::posix_time::second_clock::local_time();
			if (!boost::filesystem::exists(p)) {
				boost::system::error_code ignore_ec;
				boost::filesystem::create_directories(p, ignore_ec);
			}
			m_last_filename = (boost::filesystem::path(p) / (oss.str() + ".log")).string();
			return m_last_filename;
		}

	private:
		std::fstream m_file;
		bool m_auto_mode;
		boost::filesystem::path m_log_path;
		loglist m_log_list;
		mutable boost::posix_time::ptime m_last_day;
		mutable std::string m_last_filename;
	};

	namespace aux {
		template<class Lock>
		Lock& lock_single()
		{
			static Lock lock_instance;
			return lock_instance;
		}

		template<class Writer>
		Writer& writer_single()
		{
			static Writer writer_instance;
			return writer_instance;
		}

		inline char const* time_now_string()
		{
			static std::ostringstream oss;
			if (oss.str().empty())
			{
				boost::posix_time::time_facet* _facet = new boost::posix_time::time_facet("%Y-%m-%d %H:%M:%S.%f");
				oss.imbue(std::locale(std::locale::classic(), _facet));
			}
			oss.str("");
			oss << boost::posix_time::microsec_clock::local_time();
			std::string s = oss.str();
			if (s.size() > 3)
				s = std::string(s.substr(0, s.size() - 3));
			static char str[200];
			std::sprintf(str, "%s ", s.c_str());
			return str;
		}
	}

#ifndef DISABLE_LOGGER_THREAD_SAFE
#	define LOGGER_LOCKS_() boost::mutex::scoped_lock lock(aux::lock_single<boost::mutex>())
#else
#	define LOGGER_LOCKS_() ((void)0)
#endif // DISABLE_LOGGER_THREAD_SAFE

#if defined(WIN32) && defined(LOGGER_DBG_VIEW)
#	define LOGGER_DBG_VIEW_(x) do { ::OutputDebugStringA(x.c_str()); } while (0)
#else
#	define LOGGER_DBG_VIEW_(x) ((void)0)
#endif // WIN32 && LOGGER_DBG_VIEW

	static std::string LOGGER_DEBUG_STR = "DEBUG";
	static std::string LOGGER_INFO_STR = "INFO";
	static std::string LOGGER_WARN_STR = "WARNING";
	static std::string LOGGER_ERR_STR = "ERROR";
	static std::string LOGGER_FILE_STR = "FILE";

	inline void output_console(std::string& level, const std::string& prefix, const std::string& message)
	{
#ifdef WIN32
		HANDLE handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(handle_stdout, &csbi);
		if (level == LOGGER_INFO_STR)
			SetConsoleTextAttribute(handle_stdout, FOREGROUND_GREEN);
		else if (level == LOGGER_DEBUG_STR)
			SetConsoleTextAttribute(handle_stdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		else if (level == LOGGER_WARN_STR)
			SetConsoleTextAttribute(handle_stdout, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
		else if (level == LOGGER_ERR_STR)
			SetConsoleTextAttribute(handle_stdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
		std::cout << prefix;
		SetConsoleTextAttribute(handle_stdout, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
		std::cout << message;
		SetConsoleTextAttribute(handle_stdout, csbi.wAttributes);
#else
		if (level == LOGGER_INFO_STR)
			std::cout << "\033[32m" << prefix << "\033[0m" << message;
		else if (level == LOGGER_DEBUG_STR)
			std::cout << "\033[1;32m" << prefix << "\033[0m" << message;
		else if (level == LOGGER_WARN_STR)
			std::cout << "\033[1;33m" << prefix << "\033[0m" << message;
		else if (level == LOGGER_ERR_STR)
			std::cout << "\033[1;31m" << prefix << "\033[0m" << message;
#endif
		std::cout.flush();
	}

	inline void logger_writer(std::string& level, std::string& message, bool disable_cout = false)
	{
		LOGGER_LOCKS_();
		std::string prefix = aux::time_now_string() + std::string("[") + level + std::string("]: ");
		std::string tmp = message + "\n";
		std::string whole = prefix + tmp;
		if (aux::writer_single<auto_logger_file>().is_open())
		{
			aux::writer_single<auto_logger_file>().write(whole.c_str(), whole.size());
			aux::writer_single<auto_logger_file>().flush();
		}
		LOGGER_DBG_VIEW_(whole);
#ifndef AVHTTP_DISABLE_LOGGER_TO_CONSOLE
		if (!disable_cout)
			output_console(level, prefix, tmp);
#endif
	}

	class logger : boost::noncopyable
	{
	public:
		logger(std::string& level)
			: level_(level)
			, m_disable_cout(false)
		{}
		logger(std::string& level, bool disable_cout)
			: level_(level)
			, m_disable_cout(disable_cout)
		{}
		~logger()
		{
			std::string message = oss_.str();
			logger_writer(level_, message, m_disable_cout);
		}

		template <class T>
		logger& operator << (T const& v)
		{
			oss_ << v;
			return *this;
		}

		std::ostringstream oss_;
		std::string& level_;
		bool m_disable_cout;
	};

	class empty_logger : boost::noncopyable
	{
	public:
		template <class T>
		empty_logger& operator << (T const& v)
		{
			return *this;
		}
	};
} // namespace avhttp

#define AVHTTP_INIT_LOGGER(logfile) do \
	{ \
		avhttp::auto_logger_file& file = avhttp::aux::writer_single<avhttp::auto_logger_file>(); \
		std::string filename = logfile; \
		if (!filename.empty()) \
			file.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::app); \
	} while (0)

#define AVHTTP_AUTO_LOGGER(path) do \
	{ \
		avhttp::auto_logger_file& file = avhttp::aux::writer_single<avhttp::auto_logger_file>(); \
		std::string filename = "*"; \
		filename = std::string(path) + filename; \
		if (!filename.empty()) \
			file.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::app); \
	} while (0)


#if (defined(DEBUG) || defined(_DEBUG) || defined(ENABLE_LOGGER)) && !defined(DISABLE_LOGGER)

#define AVHTTP_LOG_DBG avhttp::logger(avhttp::LOGGER_DEBUG_STR)
#define AVHTTP_LOG_INFO avhttp::logger(avhttp::LOGGER_INFO_STR)
#define AVHTTP_LOG_WARN avhttp::logger(avhttp::LOGGER_WARN_STR)
#define AVHTTP_LOG_ERR avhttp::logger(avhttp::LOGGER_ERR_STR)
#define AVHTTP_LOG_FILE avhttp::logger(avhttp::LOGGER_FILE_STR, true)

#else

#define AVHTTP_LOG_DBG avhttp::empty_logger()
#define AVHTTP_LOG_INFO avhttp::empty_logger()
#define AVHTTP_LOG_WARN avhttp::empty_logger()
#define AVHTTP_LOG_ERR avhttp::empty_logger()
#define AVHTTP_LOG_OUT avhttp::empty_logger()
#define AVHTTP_LOG_FILE avhttp::empty_logger()

#endif

#endif // AVHTTP_LOGGING_HPP
