#include "sdpCache.h"
#include <map>
#include <string>
#include <time.h>
using namespace std;
CSdpCache* CSdpCache::cache = NULL;

typedef struct sdpCache_Tag
{
    unsigned long long date;
    string context;
}SdpCache;
static map<string,SdpCache> sdpmap;
CSdpCache* CSdpCache::GetInstance()
{
    if(cache == NULL)
    {
        cache = new CSdpCache();
    }
    return cache;
}

void CSdpCache::setSdpMap(char* path,char* context)
{
    if(path == NULL || context == NULL)
    {
        return ;
    }
    SdpCache cache = {0};
    cache.date = time(NULL);
    cache.context = string(context);

    sdpmap[string(path)] = cache;
}
    
char* CSdpCache::getSdpMap(char* path)
{
    map<string,SdpCache>::iterator it = sdpmap.find(string(path));
    if(it == sdpmap.end())
    {
        return NULL;
    }
    
    return (char*)it->second.context.c_str();
}

bool CSdpCache::eraseSdpMap(char* path)
{
    map<string,SdpCache>::iterator it = sdpmap.find(string(path));
    if(it == sdpmap.end())
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
    map<string,SdpCache>::iterator it = sdpmap.find(string(path));
    if(it == sdpmap.end())
    {
        return NULL;
    }
    
    date = it->second.date;
    return date;
}

int CSdpCache::getSdpCacheLen(char *path)
{
    int length = 0;
    map<string,SdpCache>::iterator it = sdpmap.find(string(path));
    if(it == sdpmap.end())
    {
        return 0;
    }
    
    length = it->second.context.size();
    return length;
}



