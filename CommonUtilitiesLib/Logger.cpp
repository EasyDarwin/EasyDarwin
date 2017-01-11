#include "Logger.h"

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;
std::vector<spdlog::sink_ptr> Logger::sinks_;
std::once_flag Logger::flag_;

std::shared_ptr<spdlog::logger>& Logger::Instance()
{
	if (!logger_)
	{
		std::call_once(flag_, [&]()
		{
			sinks_.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>());
			sinks_.push_back(std::make_shared<spdlog::sinks::daily_file_sink_st>("log", "txt", 23, 59));
			logger_ = std::make_shared<spdlog::logger>("EasyDarwin", begin(sinks_), end(sinks_));
			spdlog::register_logger(logger_);			
		});
	}

	return logger_;
}