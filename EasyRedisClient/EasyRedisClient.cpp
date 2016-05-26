/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   AVSRedisClient.cpp
 * Author: Wellsen
 * 
 * Created on 2015年4月23日, 下午5:11
 */

#include "EasyRedisClient.h"

#ifndef _WIN32
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

AVSRedisClient::AVSRedisClient()
: fRedisContext(NULL)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

AVSRedisClient::~AVSRedisClient()
{
    if (fRedisContext != NULL)
    {
        redisFree(RedisContextPtr(fRedisContext));
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

int AVSRedisClient::Connect(const char* host, std::size_t port)
{
    fRedisContext = redisConnect(host, port);
    if (fRedisContext != NULL)
        return RedisContextPtr(fRedisContext)->err;
    return -1;
}

int AVSRedisClient::ConnectWithTimeOut(const char* host, std::size_t port, std::size_t timeout)
{
    struct timeval timeout_ = {timeout, 0};
    fRedisContext = redisConnectWithTimeout(host, port, timeout_);
    if (fRedisContext != NULL)
        return RedisContextPtr(fRedisContext)->err;
    return -1;
}

int AVSRedisClient::Delete(const char* key)
{
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "DEL %s", key);
        freeReplyObject(reply);
        return RedisContextPtr(fRedisContext)->err;
    }

    return -1;
}

int AVSRedisClient::Get(const char* key, std::string& value)
{
    int ret = -1;

    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "GET %s", key);
        ret = GetValue(reply, value);
        //printf("[AVSRedisClient]GET: %s\n",  reply->str);        
        if(ret < 0)
        {
            printf("AVSRedisClient::Get(key[%s]) error: %s\n", key, value.c_str());
        }
        freeReplyObject(reply);
        return RedisContextPtr(fRedisContext)->err;
    }
    return ret;
}

int AVSRedisClient::Set(std::string& key, std::string& value)
{
    int ret = -1;
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "SET %b %b", key.c_str(), key.size(), value.c_str(), value.size());
        //printf("[AVSRedisClient]SET (binary API): %s\n",  reply->str);
        if(reply == NULL)
        {
            printf("AVSRedisClient::Set error:key[%s], value[%s](%s)\n", key.c_str(), value.c_str(), GetErrorString());
            return -1;
        }
        std::string res;
        ret = GetValue(reply, res);
        if(ret < 0)
        {
            printf("AVSRedisClient::Set(key[%s], value[%s]) error: %s\n", key.c_str(), value.c_str(), res.c_str());
        }
        freeReplyObject(reply);
        return RedisContextPtr(fRedisContext)->err;
    }
    return ret;
}

const char* AVSRedisClient::GetErrorString()
{
    if (fRedisContext != NULL)
    {
        return RedisContextPtr(fRedisContext)->errstr;
    }
    return std::string().c_str();
}

int AVSRedisClient::HashGet(const char* key, const char* field, std::string& value)
{
    int ret = -1;
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "HGET %s %s", key, field);
        
        if(reply == NULL)
        {
           printf("AVSRedisClient::HashGet error:key[%s], field[%s], value[%s](%s)\n", key, field, value.c_str(), GetErrorString());
           return -1;
        }
        ret = GetValue(reply, value);
        if(ret < 0)
        {
            printf("AVSRedisClient::HashGet(key[%s], field[%s]) error: %s\n", key, field, value.c_str());
        }
        freeReplyObject(reply);       
    }

    return ret;
}

int AVSRedisClient::HashSet(std::string &key, std::string &field, std::string &value)
{
    int ret = -1;
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
        //printf("[AVSRedisClient]HSET (binary API): %s\n",  reply->str);
        if(reply == NULL)
        {
            printf("AVSRedisClient::HashSet error:key[%s], field[%s], value[%s](%s)\n", key.c_str(), field.c_str(), value.c_str(), GetErrorString());
            return -1;
        }
        std::string res;
        ret = GetValue(reply, res);
        if(ret < 0)
        {
            printf("AVSRedisClient::HashSet(key[%s], field[%s], value[%s]) error: %s\n", key.c_str(), field.c_str(), value.c_str(), res.c_str());
        }
        freeReplyObject(reply);       
    }
    return ret;
}


int AVSRedisClient::HashSet(const char *key, const char *field, const char *value)
{
	std::string key_ = key, field_ = field, value_ = value;
	return HashSet(key_, field_, value_);
}

int AVSRedisClient::GetValue(AVSRedisReplyObject reply, std::string& value)
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

std::string AVSRedisClient::IntToString(int value)
{
    std::string value_;
    try
    {
        value_ = boost::lexical_cast<std::string>(value);
    }
    catch(std::exception &e)
    {
        printf("AVSRedisClient::IntToString error: %s\n", e.what());
    }
    return value_;
}

int AVSRedisClient::StringToInt(std::string value)
{ 
    int value_ = -1;
    try
    {
        value_ = boost::lexical_cast<int>(value);
    }
    catch(std::exception &e)
    {
        printf("AVSRedisClient::StringToInt error: %s\n", e.what());
    }
    return value_;
}

std::string AVSRedisClient::FloatToString(float value)
{
    std::string value_;
    try
    {
        value_ = boost::lexical_cast<std::string>(value);
    }
    catch(std::exception &e)
    {
        printf("AVSRedisClient::FloatToString error: %s\n", e.what());
    }
    return value_;
}

float AVSRedisClient::StringToFloat(std::string value)
{
    float value_ = -1;
    try
    {
        value_ = boost::lexical_cast<float>(value);
    }
    catch(std::exception &e)
    {
        printf("AVSRedisClient::StringToFloat error: %s\n", e.what());
    }
    return value_;
}

int AVSRedisClient::SetExpire(const char* key, std::size_t expire)
{
    int ret = -1;
    if (fRedisContext != NULL)
    {
        redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "EXPIRE %s %d", key, expire);
        //printf("[AVSRedisClient]SET (binary API): %s\n",  reply->str);
        if(reply == NULL)
        {
            printf("AVSRedisClient::SetExpire[line:%d] error:key[%s], expire[%d](%s)\n", __LINE__, key, expire, GetErrorString());
            return -1;
        }
        std::string res;
        ret = GetValue(reply, res);
        if(ret < 0)
        {
            printf("AVSRedisClient::SetExpire[line:%d] error:key[%s], expire[%d](%s)\n", __LINE__, key, expire, res.c_str());
        }
        freeReplyObject(reply);
        return RedisContextPtr(fRedisContext)->err;
    }
    return ret;
}

int AVSRedisClient::ZAdd(const char* key, float score, const char* member)
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "ZADD %s %s %s", key, FloatToString(score).c_str(), member);
		//printf("[AVSRedisClient]SET (binary API): %s\n",  reply->str);
		if (reply == NULL)
		{
			printf("AVSRedisClient::ZAdd error:key[%s], score[%f], member[%s](%s)\n", key, score, member, GetErrorString());
			return -1;
		}
		std::string res;
		ret = GetValue(reply, res);
		if (ret < 0)
		{
			printf("AVSRedisClient::ZAdd(key[%s], score[%f], member[%s]) error: %s\n", key, score, member, res.c_str());
		}
		freeReplyObject(reply);
		return RedisContextPtr(fRedisContext)->err;
	}
	return ret;
}

int AVSRedisClient::ZRem(const char* key, ZSetsMembers_t& members)
{
	if (members.empty())
	{
		printf("AVSRedisClient::ZRem members is empty\n");
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
		//printf("[AVSRedisClient]SET (binary API): %s\n",  reply->str);
		if (reply == NULL)
		{
			printf("AVSRedisClient::ZRem error:key[%s], member[%s](%s)\n", key, strMembers.c_str(), GetErrorString());
			return -1;
		}
		std::string res;
		ret = GetValue(reply, res);
		if (ret < 0)
		{
			printf("AVSRedisClient::ZAdd(key[%s], member[%s]) error: %s\n", key, strMembers.c_str(), res.c_str());
		}
		freeReplyObject(reply);
		return RedisContextPtr(fRedisContext)->err;
	}
	return ret;
}

int AVSRedisClient::ZRange(const char* key, ZSetsMembers_t& members)
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "ZRANGE %s 0 -1", key);

		if (reply == NULL)
		{
			printf("AVSRedisClient::ZRange error:key[%s](%s)\n", key, GetErrorString());
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
			printf("AVSRedisClient::ZRange(key[%s]) error: %s\n", key, reply->str);
		}
		freeReplyObject(reply);
	}

	return ret;
}

int AVSRedisClient::ZRange(const char* key, ZSetsMembersWithScore_t &members)
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "ZRANGE %s 0 -1 withscores", key);

		if (reply == NULL)
		{
			printf("AVSRedisClient::ZRange error:key[%s](%s)\n", key, GetErrorString());
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
			printf("AVSRedisClient::ZRange(key[%s]) error: %s\n", key, reply->str);
		}
		freeReplyObject(reply);
	}

	return ret;
}

/*
int AVSRedisClient::ZRevRange(const char* key, ZSetsMembers_t& members)
{
	return -1;
}

int AVSRedisClient::ZRevRange(const char* key, ZSetsMembersWithScore_t &members)
{
	return -1;
}
*/

int AVSRedisClient::ZIncrBy(const char* key, float value, const char* member)
{
	int ret = -1;
	if (fRedisContext != NULL)
	{
		redisReply *reply = (redisReply*)redisCommand(RedisContextPtr(fRedisContext), "ZINCRBY %s %s %s", key, FloatToString(value).c_str(), member);
		//printf("[AVSRedisClient]SET (binary API): %s\n",  reply->str);
		if (reply == NULL)
		{
			printf("AVSRedisClient::ZIncrBy error:key[%s], score[%f], member[%s](%s)\n", key, value, member, GetErrorString());
			return -1;
		}
		std::string res;
		ret = GetValue(reply, res);
		if (ret < 0)
		{
			printf("AVSRedisClient::ZIncrBy(key[%s], score[%f], member[%s]) error: %s\n", key, value, member, res.c_str());
		}
		freeReplyObject(reply);
		return RedisContextPtr(fRedisContext)->err;
	}
	return ret;
}

