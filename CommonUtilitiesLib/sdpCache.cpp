#include "sdpCache.h"
#include <unordered_map>
#include <string>
#include <time.h>

using namespace std;

CSdpCache* CSdpCache::cache = nullptr;

typedef struct sdpCache_Tag
{
	unsigned long long date;
	string context;
} SdpCache;

static unordered_map<string, SdpCache> sdpmap;

CSdpCache* CSdpCache::GetInstance()
{
	if (cache == nullptr)
	{
		cache = new CSdpCache();
	}
	return cache;
}

void CSdpCache::setSdpMap(char* path, char* context)
{
	if (path == nullptr || context == nullptr)
	{
		return;
	}
	SdpCache cache = { 0 };
	cache.date = time(nullptr);
	cache.context = string(context);

	sdpmap[string(path)] = cache;
}

char* CSdpCache::getSdpMap(char* path)
{
	auto it = sdpmap.find(string(path));
	if (it == sdpmap.end())
	{
		return nullptr;
	}

	return (char*)it->second.context.c_str();
}

bool CSdpCache::eraseSdpMap(char* path)
{
	auto it = sdpmap.find(string(path));
	if (it == sdpmap.end())
	{
		return true;
	}
	sdpmap.erase(it);
	return true;
}

unsigned long long CSdpCache::getSdpCacheDate(char *path)
{
	unsigned long long date = 0;
	int length = 0;
	auto it = sdpmap.find(string(path));
	if (it == sdpmap.end())
	{
		return 0;
	}

	date = it->second.date;
	return date;
}

int CSdpCache::getSdpCacheLen(char* path)
{
	int length = 0;
	auto it = sdpmap.find(string(path));
	if (it == sdpmap.end())
	{
		return 0;
	}

	length = it->second.context.size();
	return length;
}
