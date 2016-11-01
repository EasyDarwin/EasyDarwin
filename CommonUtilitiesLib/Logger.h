#ifndef __COMMONUTILITIESLIB_LOGGER__
#define __COMMONUTILITIESLIB_LOGGER__

#include <spdlog/spdlog.h>

using namespace spdlog;

class Logger final
{
public:
	static std::shared_ptr<spdlog::logger>& Instance();

private:
	Logger() = delete;
	~Logger() = delete;
	Logger(const Logger&) = delete;
	Logger& operator =(const Logger&) = delete;

private:
	static std::shared_ptr<spdlog::logger> logger_;
	static std::vector<spdlog::sink_ptr> sinks_;

};


#endif //__COMMONUTILITIESLIB_LOGGER__
