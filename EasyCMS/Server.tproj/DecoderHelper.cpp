/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include "DecoderHelper.h"



DecoderHelper::DecoderHelper()
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
	if (width_ != width || height_ != height || codec_ != codec)
	{
		releaseVideoDecoder();
	}

	if (videoCodecContext_)
	{
		return -1;
	}

	AVCodec* avcodec = avcodec_find_decoder(static_cast<AVCodecID>(codec));
	if (!avcodec)
	{
		return -1;
	}

	videoCodecContext_ = avcodec_alloc_context3(avcodec);
	videoCodecContext_->pix_fmt = AV_PIX_FMT_YUV420P;
	videoCodecContext_->width = width;
	videoCodecContext_->height = height;

	int numBytes;
	int ret = avcodec_open2(videoCodecContext_, avcodec, nullptr);

	if (ret < 0)
	{
		goto $fail;
	}

	numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, width, height);
	buffYUV420_ = static_cast<uint8_t *>(av_malloc(numBytes * sizeof(uint8_t)));
	videoFrame420_ = av_frame_alloc();
	if (avpicture_fill(reinterpret_cast<AVPicture *>(videoFrame420_), buffYUV420_, AV_PIX_FMT_YUV420P,
		width_, height_) < 0)
	{

	}

	av_init_packet(&videoAVPacket_);
	width_ = width;
	height_ = height;
	codec_ = codec;
	outputFormat_ = format;

	return 0;

$fail:
	{
		return -1;
	}

}

int DecoderHelper::DecodeVideo(char* inBuff, int inBuffSize, void* yuvBuff, int width, int height)
{
	if (nullptr == inBuff)			return -1;
	if (1 > inBuffSize)			return -1;
	if (nullptr == yuvBuff)		return -1;
	if (nullptr == videoCodecContext_)		return -2;

	videoAVPacket_.size = inBuffSize;
	videoAVPacket_.data = reinterpret_cast<uint8_t*>(inBuff);

	int frameFinished = 0;
	int nDecode = avcodec_decode_video2(videoCodecContext_, videoFrame420_, &frameFinished, &videoAVPacket_);//(uint8_t*)pInBuffer, inputSize);
	if (nDecode < 0)	return -3;
	if (!frameFinished)	return -4;

	if (width != width_ || height != height_)
	{
		if (avframeYUV_)
		{
			av_frame_free(&avframeYUV_);
			avframeYUV_ = nullptr;
		}

		if (swsContext_)
		{
			sws_freeContext(swsContext_);
			swsContext_ = nullptr;
		}

		width_ = width;
		height_ = height;
	}

	if (nullptr == avframeYUV_)
	{
		int numBytes = avpicture_get_size(static_cast<AVPixelFormat>(outputFormat_), width, height);
		avframeYUV_ = av_frame_alloc();
	}
	if (nullptr == avframeYUV_)		return -5;

	if (avpicture_fill(reinterpret_cast<AVPicture *>(avframeYUV_), static_cast<uint8_t*>(yuvBuff), static_cast<AVPixelFormat>(outputFormat_),
		width, height) < 0)
	{
		return -1;
	}

	if (nullptr == swsContext_)
	{
		swsContext_ = sws_getCachedContext(swsContext_, videoCodecContext_->width, videoCodecContext_->height, static_cast<AVPixelFormat>(AV_PIX_FMT_YUV420P),
			width, height, static_cast<AVPixelFormat>(outputFormat_), SWS_BICUBIC, nullptr, nullptr, nullptr);
	}
	if (nullptr == swsContext_)		return -1;

	int ret = sws_scale(swsContext_, videoFrame420_->data, videoFrame420_->linesize, 0, videoCodecContext_->height,
		avframeYUV_->data, avframeYUV_->linesize);

	return 0;
}

void DecoderHelper::releaseVideoDecoder()
{
	if (nullptr != videoFrame420_)
	{
		av_frame_free(&videoFrame420_);
		videoFrame420_ = nullptr;
	}
	if (nullptr != buffYUV420_)
	{
		av_free(buffYUV420_);
		buffYUV420_ = nullptr;
	}
	if (nullptr != avframeSWS_)
	{
		av_frame_free(&avframeSWS_);
		avframeSWS_ = nullptr;
	}
	if (nullptr != avframeYUV_)
	{
		av_frame_free(&avframeYUV_);
		avframeYUV_ = nullptr;
	}
	if (nullptr != buffYUV_)
	{
		av_free(buffYUV_);
		buffYUV_ = nullptr;
	}

	if (nullptr != swsContext_)
	{
		sws_freeContext(swsContext_);
		swsContext_ = nullptr;
	}

	if (nullptr != videoCodecContext_)
	{
		avcodec_close(videoCodecContext_);
		av_free(videoCodecContext_);
		videoCodecContext_ = nullptr;
	}
}

