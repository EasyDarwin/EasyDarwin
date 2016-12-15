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

#ifndef EASYREDISCLIENT_H
#define	EASYREDISCLIENT_H

#include <string>
#include <vector>
#include <unordered_map>

#define EASY_REDIS_ERR -1
#define EASY_REDIS_OK 0

#define EASY_REDIS_REPLY_STRING 1
#define EASY_REDIS_REPLY_ARRAY 2
#define EASY_REDIS_REPLY_INTEGER 3
#define EASY_REDIS_REPLY_NIL 4
#define EASY_REDIS_REPLY_STATUS 5
#define EASY_REDIS_REPLY_ERROR 6

 /* This is the reply object returned by redisCommand() */
typedef struct easyRedisReply {
	int type; /* REDIS_REPLY_* */
	long long integer; /* The integer when type is REDIS_REPLY_INTEGER */
	int len; /* Length of string */
	char *str; /* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
	size_t elements; /* number of elements, for REDIS_REPLY_ARRAY */
	struct easyRedisReply **element; /* elements vector for REDIS_REPLY_ARRAY */
} easyRedisReply;

void EasyFreeReplyObject(void* reply);


class EasyRedisClient
{
public:
	EasyRedisClient();
	virtual ~EasyRedisClient();

	typedef void* EasyRedisContextObject;
	typedef void* EasyRedisReplyObject;

	// return 0 when there is no error
	int Connect(const char* host, std::size_t port);
	int ConnectWithTimeOut(const char* host, std::size_t port, std::size_t timeout/*second*/);

	int Get(const char* key, std::string& value);
	int Set(std::string& key, std::string& value);//we set a key using binary safe API

	int HashGet(const char* key, const char* field, std::string& value);
	int HashSet(std::string& key, std::string& field, std::string& value);
	int HashSet(const char* key, const char* field, const char* value);

	int SetExpire(const char* key, std::size_t expire/*second*/);
	int Delete(const char* key);
	const char* GetErrorString();

	//Sorted-Sets
	typedef std::vector<std::string> ZSetsMembers_t;
	typedef std::unordered_map<std::string, float> ZSetsMembersWithScore_t;

	int ZAdd(const char* key, float score, const char* member);
	int ZRem(const char* key, ZSetsMembers_t& members);

	int ZRange(const char* key, ZSetsMembers_t& members);
	int ZRange(const char* key, ZSetsMembersWithScore_t& members);

	/*int ZRevRange(const char* key, ZSetsMembers_t& members);
	int ZRevRange(const char* key, ZSetsMembersWithScore_t& members);*/

	int ZIncrBy(const char* key, float value, const char* member);

	//useful cast
	int SetTimeout(std::size_t timeout);
	void AppendCommand(const char* command);//redisAppendCommand return void
	int GetReply(void** reply);
	void Free();
	int SAdd(const char* key, const char* member);
	int SRem(const char* key, const char* member);
	int SetEX(const char* key, std::size_t timeout, const char* value);

	EasyRedisReplyObject Exists(const char* key);
	EasyRedisReplyObject SMembers(const char* key);
	EasyRedisReplyObject Keys(const char* key);

	static std::string IntToString(int value);
	static int StringToInt(std::string value);
	static std::string FloatToString(float value);
	static float StringToFloat(std::string value);

private:
	int getValue(EasyRedisReplyObject reply, std::string& value);

	EasyRedisContextObject fRedisContext;
};

#endif	/* EASYREDISCLIENT_H */

