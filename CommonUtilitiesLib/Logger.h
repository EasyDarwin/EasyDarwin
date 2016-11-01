#ifndef __COMMONUTILITIESLIB_LOGGER__
#define __COMMONUTILITIESLIB_LOGGER__

#include <spdlog/spdlog.h>

using namespace spdlog;

class Logger final
{
public:
	static std::shared_ptr<spdlog::logger>& Instance()
	{
		if (!logger_)
		{
			sinks_.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>());
			sinks_.push_back(std::make_shared<spdlog::sinks::daily_file_sink_st>("log", "txt", 23, 59));
			logger_ = std::make_shared<spdlog::logger>("EasyDarwin", begin(sinks_), end(sinks_));
			spdlog::register_logger(logger_);
		}

		return logger_;
	}

	~Logger()
	{
		spdlog::drop_all();
	}

private:
	Logger() = delete;
	Logger(const Logger&) = delete;

private:
	static std::shared_ptr<spdlog::logger> logger_;
	static std::vector<spdlog::sink_ptr> sinks_;

};

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;
std::vector<spdlog::sink_ptr> Logger::sinks_;


#endif //__COMMONUTILITIESLIB_LOGGER__
