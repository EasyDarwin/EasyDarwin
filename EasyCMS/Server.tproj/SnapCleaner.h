/*
Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
Github: https://github.com/EasyDarwin
WEChat: EasyDarwin
Website: http://www.easydarwin.org
*/
#ifndef __SNAP_CLEANER_H__
#define __SNAP_CLEANER_H__

#include "Logger.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/format.hpp>
#include <string>
#include <Task.h>
#include <TimeoutTask.h>

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

using std::string;

const int timeoutTime = 24* 60 * 60 * 1000; //min
const int days = 1;

class SnapCleaner : public Task
{
public:
	SnapCleaner(const string& path)
		: timeoutTask_(nullptr, timeoutTime)
		, snapPath_(path)
	{
		this->SetTaskName("SnapCleanerTask");
		timeoutTask_.SetTask(this);

		cleanSnapFiles();
	}

	virtual ~SnapCleaner()
	{
	}

	SInt64 Run() override
	{
		EventFlags theEvents = this->GetEvents();

		if (theEvents & kTimeoutEvent)
		{
			cleanSnapFiles();
			
			timeoutTask_.RefreshTimeout();
		}

		return 0;
	}

private:
	void cleanSnapFiles() const
	{
		fs::path snapPath(snapPath_);
		if (!fs::exists(snapPath) || !fs::is_directory(snapPath))
		{
			return;
		}

		vector<fs::path> pathList;
		fs::directory_iterator endIter;
		for (fs::directory_iterator dirIter(snapPath); dirIter != endIter; ++dirIter)
		{
			auto curPath = dirIter->path();

			auto lastModifyTime = fs::last_write_time(curPath);
			auto pt = pt::from_time_t(lastModifyTime);
			auto now(pt::second_clock::local_time());
			auto nowDate = now.date();
			auto lastModifyDate = pt.date();

			auto dur = nowDate - lastModifyDate;

			if (dur.days() > days)
			{
				pathList.emplace_back(curPath);
			}
		}

		for (auto& item : pathList)
		{
			Logger::Instance()->info("Remove snap path {}", item.string());
			try
			{
				fs::remove_all(item);
			}
			catch (...)
			{
				Logger::Instance()->error("Boost remove snap failed");
			}
		}
	}

	std::string snapPath_;
	TimeoutTask timeoutTask_;
};

#endif //__SNAP_CLEANER_H__
