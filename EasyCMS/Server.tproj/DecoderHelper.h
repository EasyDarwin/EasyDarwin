#ifndef _DECODER_HELPER_
#define _DECODER_HELPER_

#define __STDC_CONSTANT_MACROS

extern "C" 
{
#include <inttypes.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}

class DecoderHelper
{
public:
	DecoderHelper();
	~DecoderHelper();

	int		SetVideoDecoderParam(int width, int height, int codec, int format);
	int		DecodeVideo(char *inBuff, int inBuffSize, void *yuvBuff, int width, int height);

private:
	void	releaseVideoDecoder();

private:
	AVFormatContext*	_formatContext;
	AVCodecContext*		_videoCodecContext;
	AVFrame*			_videoFrame420;

	struct SwsContext*	_swsContext;
	uint8_t*			_buffYUV420;
	AVPacket			_videoAVPacket;

	uint8_t*			_buffYUV;
	AVFrame*			_avframeYUV;

	AVFrame*			_avframeSWS;

	int					_codec;
	int					_width;
	int					_height;
	int					_outputFormat;

};

#endif //_DECODER_HELPER_