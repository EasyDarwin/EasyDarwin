#include "keyframecache.h"
#include "OSMutex.h"

static OSMutex sMaxFDPosMutex;		//锁

bool CKeyFrameCache::PutOnePacket(char* buf, int len, int nalutype, int start)
{
	if (buf == NULL || len == 0)
	{
		return false;
	}

	bool ret = false;
	unsigned char cachebuf[1024 * 5] = { 0 };
	int cachelen = 1024 * 5;

	if (nalutype == 7 && start == 1)
	{
		curdatalen = 0;
	}

	const unsigned char nalu_head_byte_I = 0x67;
	const unsigned char nalu_head_byte_P = 0x41;

	OSMutexLocker locker(&sMaxFDPosMutex);
	FILE* fp = fopen("data.264", "ab+");
	if (fp)
	{

		fseek(fp, 0, SEEK_END);
		if (start == 1)
		{
			if (nalutype == 7)
			{
				buf[13] = nalu_head_byte_I;
			}
			else
			{
				buf[13] = nalu_head_byte_P;
			}
			char head[4] = { 0x00,0x00,0x00,0x01 };
			fwrite(head, 4, 1, fp);
			fwrite(buf + 13, len - 13, 1, fp);
		}
		else
		{
			fwrite(buf + 14, len - 14, 1, fp);
		}
		fclose(fp);
	}


	FrameBuffer frame_elem = { 0 };
	frame_elem.STX = BUF_STX;
	frame_elem.bufLen = len;
	frame_elem.buf = buf;
	frame_elem.ETX = BUF_ETX;
	cachelen = frame_elem.Encode(cachebuf, cachelen);

	if (cachelen <= 0)
	{
		return false;
	}
	ret = SetBuf((char *)cachebuf, cachelen);
	return ret;
}

bool CKeyFrameCache::GetOnePacket(char* outbuf, int& outLen, int curOffset)
{//do not copy mem,we just point the address

	if (curOffset >= curdatalen)
	{
		return false;
	}

	int pkgLen = 0;

	if (_memory[curOffset] != BUF_STX)
	{
		printf("offset here error\n");
		return false;
	}

	pkgLen = ntohs(*(unsigned short *)(_memory + curOffset + 1));
	if (pkgLen >= curdatalen)
	{
		printf("pkg data error\n");
		return false;
	}

	if (_memory[curOffset + 3 + pkgLen] != BUF_ETX)
	{
		printf("");
		return false;
	}

	memcpy(outbuf, _memory + curOffset + 3, pkgLen);
	outLen = pkgLen;
	return true;
}


bool CKeyFrameCache::SetBuf(char *frameBuf, int len)
{
	if (frameBuf == NULL || len == 0)
	{
		return false;
	}

	if (len + curdatalen > mem_size)
	{
		return false;
	}

	memcpy(_memory + curdatalen, frameBuf, len);
	curdatalen += len;
	return true;
}


