/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyRedisClient.cpp
 * Author: Wellsen
 * 
 * Created on 2015年4月23日, 下午5:11
 */

#include "EasyRedisClient.h"

#ifndef _WIN32
#undef unix
#include "hiredis.h"
#else
#include "Windows/hiredis.h"
#endif
#include <string.h>
#include <boost/lexical_cast.hpp>

#ifdef _WIN32
#include <winsock2.h>  
#endif

#define RedisContextPtr(redis) ((redisContext*)(redis))
#define RedisReplyPtr(reply) ((redisReply*)(reply))

EasyRedisClient::EasyRedisClient()
: fRedisContext(NULL)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

EasyRedisClient::~EasyRedisClient()
{
    if (fRedisContext != NULL)
    {
        redisFree(RedisContextPtr(fRedisContext));
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

int EasyRedisClient::Connect(const char* host, std::size_t port)//connect asynchronous
{
    fRedisContext = redisConnect(host, port);
    if (fRedisContext != NULL)
        return RedisContextPtr(fRedisContext)->err;
    return -1;
}

int EasyRedisClient::ConnectWithTimeOut(const char* host, std::size_t port, std::size_t timeout)//connect synchronous with timeout
{
    struct timeval timeout_ = {timeout, 0};
    fRedisContext = redisConnectWithTimeout(host, port, timeout_);
    if (fRedisContext != NULL)
        return RedisContextPtr(fRedisContext)->err;
    return -1;
}

int EasyRedisClient::Delete(const char* key)//del
{
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "DEL %s", key);
        freeReplyObject(reply);
        return RedisContextPtr(fRedisContext)->err;
    }

    return -1;
}

int EasyRedisClient::Get(const char* key, std::string& value)//get
{
    int ret = -1;

    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "GET %s", key);
        ret = GetValue(reply, value);
        //printf("[EasyRedisClient]GET: %s\n",  reply->str);        
        if(ret < 0)
        {
            printf("EasyRedisClient::Get(key[%s]) error: %s\n", key, value.c_str());
        }
        freeReplyObject(reply);
        return RedisContextPtr(fRedisContext)->err;
    }
    return ret;
}

int EasyRedisClient::Set(std::string& key, std::string& value)//set
{
    int ret = -1;
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "SET %b %b", key.c_str(), key.size(), value.c_str(), value.size());
        //printf("[EasyRedisClient]SET (binary API): %s\n",  reply->str);
        if(reply == NULL)
        {
            printf("EasyRedisClient::Set error:key[%s], value[%s](%s)\n", key.c_str(), value.c_str(), GetErrorString());
            return -1;
        }
        std::string res;
        ret = GetValue(reply, res);
        if(ret < 0)
        {
            printf("EasyRedisClient::Set(key[%s], value[%s]) error: %s\n", key.c_str(), value.c_str(), res.c_str());
        }
        freeReplyObject(reply);
        return RedisContextPtr(fRedisContext)->err;
    }
    return ret;
}

const char* EasyRedisClient::GetErrorString()//get error string
{
    if (fRedisContext != NULL)
    {
        return RedisContextPtr(fRedisContext)->errstr;
    }
    return std::string().c_str();
}

int EasyRedisClient::HashGet(const char* key, const char* field, std::string& value)//hget
{
    int ret = -1;
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "HGET %s %s", key, field);
        
        if(reply == NULL)
        {
           printf("EasyRedisClient::HashGet error:key[%s], field[%s], value[%s](%s)\n", key, field, value.c_str(), GetErrorString());
           return -1;
        }
        ret = GetValue(reply, value);
        if(ret < 0)
        {
            printf("EasyRedisClient::HashGet(key[%s], field[%s]) error: %s\n", key, field, value.c_str());
        }
        freeReplyObject(reply);       
    }

    return ret;
}

int EasyRedisClient::HashSet(std::string &key, std::string &field, std::string &value)//hset
{
    int ret = -1;
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
        //printf("[EasyRedisClient]HSET (binary API): %s\n",  reply->str);
        if(reply == NULL)
        {
            printf("EasyRedisClient::HashSet error:key[%s], field[%s], value[%s](%s)\n", key.c_str(), field.c_str(), value.c_str(), GetErrorString());
            return -1;
        }
        std::string res;
        ret = GetValue(reply, res);
        if(ret < 0)
        {
            printf("EasyRedisClient::HashSet(key[%s], field[%s], value[%s]) error: %s\n", key.c_str(), field.c_str(), value.c_str(), res.c_str());
        }
        freeReplyObject(reply);       
    }
    return ret;
}


int EasyRedisClient::HashSet(const char *key, const char *field, const char *value)//hset
{
	std::string key_ = key, field_ = field, value_ = value;
	return HashSet(key_, field_, value_);
}

int EasyRedisClient::GetValue(EasyRedisReplyObject reply, std::string& value)//get value based on defferent type
{
    int ret = -1;
    if (reply != NULL)
    {
        switch (RedisReplyPtr(reply)->type)
        {
        case REDIS_REPLY_INTEGER:
            value = IntToString(RedisReplyPtr(reply)->integer);
            ret = 0;
            break;

        case REDIS_REPLY_STRING:
        case REDIS_REPLY_STATUS:
            value.assign(RedisReplyPtr(reply)->str, RedisReplyPtr(reply)->len);
            ret = 0;
            break;
    
        case REDIS_REPLY_ERROR:
            value.assign(RedisReplyPtr(reply)->str, RedisReplyPtr(reply)->len);
            ret = -1;
            break;

        case REDIS_REPLY_NIL:
            value = "what you get is nil";
            ret = -1;
            break;
            
        case REDIS_REPLY_ARRAY:
        default:
            value = "what you get is not support";
            ret = -1;
            break;

        }
    }
    return ret;
}

std::string EasyRedisClient::IntToString(int value)//int to string
{
    std::string value_;
    try
    {
        value_ = boost::lexical_cast<std::string>(value);
    }
    catch(std::exception &e)
    {
        printf("EasyRedisClient::IntToString error: %s\n", e.what());
    }
    return value_;
}

int EasyRedisClient::StringToInt(std::string value)// string to int
{ 
    int value_ = -1;
    try
    {
        value_ = boost::lexical_cast<int>(value);
    }
    catch(std::exception &e)
    {
        printf("EasyRedisClient::StringToInt error: %s\n", e.what());
    }
    return value_;
}

std::string EasyRedisClient::FloatToString(float value)//float to string
{
    std::string value_;
    try
    {
        value_ = boost::lexical_cast<std::string>(value);
    }
    catch(std::exception &e)
    {
        printf("EasyRedisClient::FloatToString error: %s\n", e.what());
    }
    return value_;
}

float EasyRedisClient::StringToFloat(std::string value)//string to float
{
    float value_ = -1;
    try
    {
        value_ = boost::lexical_cast<float>(value);
    }
    catch(std::exception &e)
    {
        printf("EasyRedisClient::StringToFloat error: %s\n", e.what());
    }
    return value_;
}

int EasyRedisClient::SetExpire(const char* key, std::size_t expire)//expire key in seconds
{
    int ret = -1;
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "EXPIRE %s %d", key, expire);
        //printf("[EasyRedisClient]SET (binary API): %s\n",  reply->str);
        if(reply == NULL)
        {
            printf("EasyRedisClient::SetExpire[line:%d] error:key[%s], expire[%d](%s)\n", __LINE__, key, expire, GetErrorString());
            return -1;
        }
        std::string res;
        ret = GetValue(reply, res);
        if(ret < 0)
        {
            printf("EasyRedisClient::SetExpire[line:%d] error:key[%s], expire[%d](%s)\n", __LINE__, key, expire, res.c_str());
        }
        freeReplyObject(reply);
        return RedisContextPtr(fRedisContext)->err;
    }
    return ret;
}

int EasyRedisClient::ZAdd(const char* key, float score, const char* member)//zset zadd
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "ZADD %s %s %s", key, FloatToString(score).c_str(), member);
		//printf("[EasyRedisClient]SET (binary API): %s\n",  reply->str);
		if (reply == NULL)
		{
			printf("EasyRedisClient::ZAdd error:key[%s], score[%f], member[%s](%s)\n", key, score, member, GetErrorString());
			return -1;
		}
		std::string res;
		ret = GetValue(reply, res);
		if (ret < 0)
		{
			printf("EasyRedisClient::ZAdd(key[%s], score[%f], member[%s]) error: %s\n", key, score, member, res.c_str());
		}
		freeReplyObject(reply);
		return RedisContextPtr(fRedisContext)->err;
	}
	return ret;
}

int EasyRedisClient::ZRem(const char* key, ZSetsMembers_t& members)//zset zrem
{
	if (members.empty())
	{
		printf("EasyRedisClient::ZRem members is empty\n");
		return -1;
	}
	int ret = -1;
	if (fRedisContext != NULL)
	{
		std::string strMembers;
		for (int i = 0; i < members.size(); i++)
		{
			strMembers += members[i] + " ";
		}
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "ZREM %s %s", key, strMembers.c_str());
		//printf("[EasyRedisClient]SET (binary API): %s\n",  reply->str);
		if (reply == NULL)
		{
			printf("EasyRedisClient::ZRem error:key[%s], member[%s](%s)\n", key, strMembers.c_str(), GetErrorString());
			return -1;
		}
		std::string res;
		ret = GetValue(reply, res);
		if (ret < 0)
		{
			printf("EasyRedisClient::ZAdd(key[%s], member[%s]) error: %s\n", key, strMembers.c_str(), res.c_str());
		}
		freeReplyObject(reply);
		return RedisContextPtr(fRedisContext)->err;
	}
	return ret;
}

int EasyRedisClient::ZRange(const char* key, ZSetsMembers_t& members)//zset zrange
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "ZRANGE %s 0 -1", key);

		if (reply == NULL)
		{
			printf("EasyRedisClient::ZRange error:key[%s](%s)\n", key, GetErrorString());
			return -1;
		}
		
		if (reply->type == REDIS_REPLY_ARRAY)
		{
			for (int i = 0; i < reply->elements; i++)
			{
				members.push_back(reply->element[i]->str);
			}
		}
		else
		{
			printf("EasyRedisClient::ZRange(key[%s]) error: %s\n", key, reply->str);
		}
		freeReplyObject(reply);
	}

	return ret;
}

int EasyRedisClient::ZRange(const char* key, ZSetsMembersWithScore_t &members)//zset zrange
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "ZRANGE %s 0 -1 withscores", key);

		if (reply == NULL)
		{
			printf("EasyRedisClient::ZRange error:key[%s](%s)\n", key, GetErrorString());
			return -1;
		}

		if (reply->type == REDIS_REPLY_ARRAY)
		{
			for (int i = 0; i < reply->elements; i += 2)
			{
				members.insert(std::make_pair(reply->element[i]->str, StringToFloat(reply->element[i+1]->str)));
			}
		}
		else
		{
			printf("EasyRedisClient::ZRange(key[%s]) error: %s\n", key, reply->str);
		}
		freeReplyObject(reply);
	}

	return ret;
}

/*
int EasyRedisClient::ZRevRange(const char* key, ZSetsMembers_t& members)
{
	return -1;
}

int EasyRedisClient::ZRevRange(const char* key, ZSetsMembersWithScore_t &members)
{
	return -1;
}
*/

int EasyRedisClient::ZIncrBy(const char* key, float value, const char* member)//zset ZINCRBY
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "ZINCRBY %s %s %s", key, FloatToString(value).c_str(), member);
		//printf("[EasyRedisClient]SET (binary API): %s\n",  reply->str);
		if (reply == NULL)
		{
			printf("EasyRedisClient::ZIncrBy error:key[%s], score[%f], member[%s](%s)\n", key, value, member, GetErrorString());
			return -1;
		}
		std::string res;
		ret = GetValue(reply, res);
		if (ret < 0)
		{
			printf("EasyRedisClient::ZIncrBy(key[%s], score[%f], member[%s]) error: %s\n", key, value, member, res.c_str());
		}
		freeReplyObject(reply);
		return RedisContextPtr(fRedisContext)->err;
	}
	return ret;
}

int EasyRedisClient::SetTimeout(std::size_t timeout)//set socket timeout
{
	struct timeval timeout_ = {timeout, 0};

	if (fRedisContext != NULL)
		return redisSetTimeout(RedisContextPtr(fRedisContext),timeout_);
	return -1;
}

void EasyRedisClient::AppendCommand(const char * command)//redisAppendCommand
{
	if (fRedisContext != NULL)
        {
            redisAppendCommand(RedisContextPtr(fRedisContext),command);
        }
}

int EasyRedisClient::GetReply(void ** reply)
{
	if (fRedisContext != NULL)
		return redisGetReply(RedisContextPtr(fRedisContext),reply);
	return -1;
}

void EasyFreeReplyObject(void *reply)
{
	freeReplyObject(reply);
}

void EasyRedisClient::Free()
{
	if (fRedisContext != NULL)
		redisFree(RedisContextPtr(fRedisContext));
}

int EasyRedisClient::SAdd(const char* key, const char * member)//SADD
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "SADD %s %s", key, member);
		if(reply == NULL)
		{
			printf("EasyRedisClient::SADD error:key[%s], member[%s](%s)\n", key, member, GetErrorString());
			return -1;
		}
		std::string res;
		ret = GetValue(reply, res);
		if(ret < 0)
		{
			printf("EasyRedisClient::SADD(key[%s], member[%s]) error: %s\n", key, member, res.c_str());
		}
		freeReplyObject(reply);
		return RedisContextPtr(fRedisContext)->err;
	}
	return ret;
};


int EasyRedisClient::SRem(const char* key, const char * member)//SREM
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "SREM %s %s", key, member);
		if(reply == NULL)
		{
			printf("EasyRedisClient::SREM error:key[%s], member[%s](%s)\n", key, member, GetErrorString());
			return -1;
		}
		std::string res;
		ret = GetValue(reply, res);
		if(ret < 0)
		{
			printf("EasyRedisClient::SREM(key[%s], member[%s]) error: %s\n", key, member, res.c_str());
		}
		freeReplyObject(reply);
		return RedisContextPtr(fRedisContext)->err;
	}
	return ret;
};

int EasyRedisClient::SetEX(const char* key, std::size_t timeout,const char * value)
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "SETEX %s %d %s", key, timeout,value);
		if(reply == NULL)
		{
			printf("EasyRedisClient::SETEX error:key[%s], timeout[%d],value[%s](%s)\n", key, timeout, value,GetErrorString());
			return -1;
		}
		std::string res;
		ret = GetValue(reply, res);
		if(ret < 0)
		{
			printf("EasyRedisClient::SETEX(key[%s], timeout[%d], value[%s]) error: %s\n", key, timeout,value, res.c_str());
		}
		freeReplyObject(reply);
		return RedisContextPtr(fRedisContext)->err;
	}
	return ret;
}

EasyRedisClient::EasyRedisReplyObject EasyRedisClient::SMembers(const char * key)
{
	if (fRedisContext != NULL)
	{
		return redisCommand(RedisContextPtr(fRedisContext), "SMEMBERS %s", key);
	}
	return NULL;
}

EasyRedisClient::EasyRedisReplyObject EasyRedisClient::Exists(const char * key)
{
	if (fRedisContext != NULL)
	{
		return redisCommand(RedisContextPtr(fRedisContext), "EXISTS %s", key);
	}
	return NULL;
}