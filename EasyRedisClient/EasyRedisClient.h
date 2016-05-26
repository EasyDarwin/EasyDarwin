/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyRedisClient.h
 * Author: Wellsen
 *
 * Created on 2015年4月23日, 下午5:11
 */

#ifndef AVSREDISCLIENT_H
#define	AVSREDISCLIENT_H

#include <string>
#include <vector>
#include <map>

class EasyRedisClient
{
public:
    EasyRedisClient();
    virtual ~EasyRedisClient();
    
public:
    // return 0 when there is no error
    int Connect(const char *host, std::size_t port);
    int ConnectWithTimeOut(const char *host, std::size_t port, std::size_t timeout/*second*/);
    
    int Get(const char *key, std::string &value);
    int Set(std::string &key, std::string &value);//we set a key using binary safe API
    
    int HashGet(const char *key, const char *field, std::string &value);
    int HashSet(std::string &key, std::string &field, std::string &value);    
	int HashSet(const char *key, const char *field, const char *value);
    
    int SetExpire(const char *key, std::size_t expire/*second*/);
    int Delete(const char* key);
    const char* GetErrorString();

	//Sorted-Sets
	typedef std::vector<std::string> ZSetsMembers_t;
	typedef std::map<std::string, float> ZSetsMembersWithScore_t;

	int ZAdd(const char* key, float score, const char* member);
	int ZRem(const char* key, ZSetsMembers_t &members);

	int ZRange(const char* key, ZSetsMembers_t &members);
	int ZRange(const char* key, ZSetsMembersWithScore_t &members);
		
	/*int ZRevRange(const char* key, ZSetsMembers_t &members);
	int ZRevRange(const char* key, ZSetsMembersWithScore_t &members);*/
	
	int ZIncrBy(const char* key, float value, const char* member);

	//useful cast
public:
    static std::string IntToString(int value);
    static int StringToInt(std::string value);
    static std::string FloatToString(float value);
    static float StringToFloat(std::string value);
        
private:
    typedef void* AVSRedisContextObject;
    typedef void* AVSRedisReplyObject;
    
private:
    int GetValue(AVSRedisReplyObject reply, std::string &value);
    

    
private:
    AVSRedisContextObject fRedisContext;
};

#endif	/* AVSREDISCLIENT_H */

