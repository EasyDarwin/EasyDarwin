#ifndef __SDPCACHE_H__
#define __SDPCACHE_H__
#include <stdio.h>
class CSdpCache
{
private:
	static CSdpCache* cache;
	CSdpCache()
	{

	}
public:
	~CSdpCache()
	{}

	static CSdpCache* GetInstance();

	void setSdpMap(char* path,char* context);
		
	char* getSdpMap(char* path);

	bool eraseSdpMap(char* path);

	unsigned long long getSdpCacheDate(char *path);

	int getSdpCacheLen(char *path);

};
#endif
