#include "DecoderHelper.h"
#ifndef __linux__
DecoderHelper::DecoderHelper()
	: _formatContext(NULL),
	_videoCodecContext(NULL),
	_videoFrame420(NULL),
	_buffYUV420(NULL),
	_buffYUV(NULL),
	_avframeYUV(NULL),
	_avframeSWS(NULL),
	_swsContext(NULL),
	_codec(0),
	_width(0),
	_height(0),
	_outputFormat(0)
{
	//Register all the codec
	avcodec_register_all();
	//Register all types of decoding
	av_register_all();
}

DecoderHelper::~DecoderHelper()
{
	releaseVideoDecoder();
}

int DecoderHelper::SetVideoDecoderParam(int width, int height, int codec, int format)
{
	if (_width != width || _height != height || _codec != codec)
	{
		releaseVideoDecoder();
	}

	if (NULL != _videoCodecContext)
	{
		return -1;
	}

	AVCodec	*avcodec = avcodec_find_decoder((AVCodecID)codec);
	if (NULL == avcodec)
	{
		return -1;
	}

	_videoCodecContext = avcodec_alloc_context3(avcodec);
	_videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	_videoCodecContext->width   = width;
	_videoCodecContext->height  = height;

	int ret = avcodec_open2(_videoCodecContext, avcodec, NULL);
	if (ret < 0)
	{
		goto $fail;
	}

	int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, width, height);
	_buffYUV420 = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
	_videoFrame420 = av_frame_alloc();
	if (avpicture_fill((AVPicture *)_videoFrame420, _buffYUV420, AV_PIX_FMT_YUV420P,
		_width, _height) < 0)
	{

	}

	av_init_packet(&_videoAVPacket);
	_width = width;
	_height = height;
	_codec = codec;
	_outputFormat = format;
	
	return 0;

$fail:
	{
		return -1;
	}

}

int DecoderHelper::DecodeVideo(char *inBuff, int inBuffSize, void *yuvBuff, int width, int height)
{
	if (NULL == inBuff)			return -1;
	if (1 > inBuffSize)			return -1;
	if (NULL == yuvBuff)		return -1;
	if (NULL == _videoCodecContext)		return -2;

	_videoAVPacket.size = inBuffSize;
	_videoAVPacket.data	= (uint8_t*)inBuff;

	int frameFinished = 0;
	int nDecode = avcodec_decode_video2(_videoCodecContext, _videoFrame420, &frameFinished, &_videoAVPacket);//(uint8_t*)pInBuffer, inputSize);
	if (nDecode < 0)	return -3;
	if (!frameFinished)	return -4;

	if  (width != _width || height != _height)
	{
		if (NULL != _avframeYUV)
		{
			av_frame_free(&_avframeYUV);
			_avframeYUV = NULL;
		}

		if (NULL != _swsContext)
		{
			sws_freeContext(_swsContext);
			_swsContext = NULL;
		}

		_width = width;
		_height = height;
	}

	if (NULL == _avframeYUV)
	{
		int numBytes = avpicture_get_size((AVPixelFormat)_outputFormat, width, height);
		_avframeYUV = av_frame_alloc();
	}
	if (NULL == _avframeYUV)		return -5;

	if (avpicture_fill((AVPicture *)_avframeYUV, (uint8_t*)yuvBuff, (AVPixelFormat)_outputFormat,
		width, height) < 0)
	{
		return -1;
	}

	if (NULL == _swsContext)
	{
		_swsContext = sws_getCachedContext(_swsContext, _videoCodecContext->width, _videoCodecContext->height, (AVPixelFormat)AV_PIX_FMT_YUV420P, 
			width, height, (AVPixelFormat)_outputFormat, SWS_BICUBIC, NULL, NULL, NULL);
	}
	if (NULL == _swsContext)		return -1;

	int ret = sws_scale(_swsContext, _videoFrame420->data, _videoFrame420->linesize, 0, _videoCodecContext->height, 
		_avframeYUV->data, _avframeYUV->linesize);

	return 0;
}

void DecoderHelper::releaseVideoDecoder()
{
	if (NULL != _videoFrame420)
	{
		av_frame_free(&_videoFrame420);
		_videoFrame420 = NULL;
	}
	if (NULL != _buffYUV420)
	{
		av_free(_buffYUV420);
		_buffYUV420 = NULL;
	}
	if (NULL != _avframeSWS)
	{
		av_frame_free(&_avframeSWS);
		_avframeSWS = NULL;
	}
	if (NULL != _avframeYUV)
	{
		av_frame_free(&_avframeYUV);
		_avframeYUV = NULL;
	}
	if (NULL != _buffYUV)
	{
		av_free(_buffYUV);
		_buffYUV = NULL;
	}

	if (NULL != _swsContext)
	{
		sws_freeContext(_swsContext);
		_swsContext = NULL;
	}

	if (NULL != _videoCodecContext)
	{
		avcodec_close(_videoCodecContext);
		av_free(_videoCodecContext);
		_videoCodecContext = NULL;
	}
}
#endif
