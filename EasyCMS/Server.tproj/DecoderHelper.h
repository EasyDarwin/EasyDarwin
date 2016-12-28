#ifndef _DECODER_HELPER_
#define _DECODER_HELPER_

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

class DecoderHelper
{
public:
	DecoderHelper();
	~DecoderHelper();

	int SetVideoDecoderParam(int width, int height, int codec, int format);
	int DecodeVideo(char* inBuff, int inBuffSize, void* yuvBuff, int width, int height);

private:
	void releaseVideoDecoder();

	AVFormatContext* formatContext_ = nullptr;
	AVCodecContext* videoCodecContext_ = nullptr;
	AVFrame* videoFrame420_ = nullptr;

	SwsContext*	swsContext_ = nullptr;
	uint8_t* buffYUV420_ = nullptr;

	AVPacket videoAVPacket_;

	uint8_t* buffYUV_ = nullptr;
	AVFrame* avframeYUV_ = nullptr;

	AVFrame* avframeSWS_ = nullptr;

	int codec_ = 0;
	int width_ = 0;
	int height_ = 0;
	int outputFormat_ = 0;

};

#endif //_DECODER_HELPER_