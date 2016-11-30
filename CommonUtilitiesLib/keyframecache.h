#ifndef _KEYFRAMECACHE_H__
#define _KEYFRAMECACHE_H__

#include "OSHeaders.h"
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <netinet/in.h>
#endif

#define BUF_STX 0x28
#define BUF_ETX 0x29

typedef struct FrameBuffer_tag
{
	char STX;
	unsigned short bufLen;
	char* buf;
	char ETX;

	int Encode(unsigned char* output, int& outlen)//outlen is output buffer len first
	{
		int dataLen = 0;
		if (output == NULL || outlen == 0)
		{
			return -1;
		}

		unsigned short netLen = htons(bufLen);

		output[dataLen++] = STX;
		memcpy(output + dataLen, &netLen, 2);
		dataLen += 2;

		memcpy(output + dataLen, buf, bufLen);
		dataLen += bufLen;

		output[dataLen++] = ETX;
		outlen = dataLen;
		return outlen;
	}

} FrameBuffer;

class CKeyFrameCache
{
public:
	char* _memory;
	int curdatalen;
	int mem_size;
public:
	CKeyFrameCache(int len) :mem_size(len)
	{
		_memory = (char *)malloc(mem_size);
		curdatalen = 0;
	}

	~CKeyFrameCache()
	{
		free(_memory);
		_memory = NULL;
		mem_size = 0;
		curdatalen = 0;
	}

	bool PutOnePacket(char* buf, int len, int nalutype, int start);

	bool GetOnePacket(char* outbuf, int& outLen, int curOffset);

	bool SetBuf(char* frameBuf, int len);

};

#endif
