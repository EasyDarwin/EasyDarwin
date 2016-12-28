#pragma once
#include "QTSS.h"
#include <iostream>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libpostproc/postprocess.h"
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
}
#include "OS.h"
//MP4Box Package MP4
#include "EasyMP4Writer.h"
using namespace std;
#define SWAP(x,y)   ((x)^=(y)^=(x)^=(y))
class RTSPRecordSession {
public:
	RTSPRecordSession();
	~RTSPRecordSession();
	static void Initialize(QTSS_ModulePrefsObject inPrefs);
	void init();
	int play(const string& url,const string& subName);
	void stop();
	bool readFrame();
	int rtspRead();

	void close();

	static char *getRecordRootPath();//网络路径
	static char *getNetRecordRootPath();//网络存储路径
private:

	

	int CreateNewMp4Writer(int sample_rate, int channels);
	//写MP4文件(录制相关)
	int CreateMP4Writer(char* sFileName, int nSampleRate, int nChannel, int nBitsPerSample, int nFlag = ZOUTFILE_FLAG_FULL);
	int WriteMP4VideoFrame(unsigned char* pdata, int datasize, bool keyframe, long nTimestamp, int nWidth, int nHeight);
	int WriteMP4AudioFrame(unsigned char* pdata, int datasize, long timestamp);
	void CloseMP4Writer();


	string m_url;
	AVCodecContext *m_pCodecCtx;
	AVFormatContext *m_pFormatContext;
	AVPacket *m_packet;

	int m_videoIndex;
	int m_audioIndex;
	uint8_t *m_outBuffer;

private:
	bool	bPlaying;
	EasyMP4Writer* m_pMP4Writer;
	bool m_bRecording;
	__int64 fCreateTime;
	string fSubName;
	HANDLE hThread;

	string	sUri;

	OSMutex         fMutex;
};