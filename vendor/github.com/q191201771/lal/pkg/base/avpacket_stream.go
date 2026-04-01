// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

type (
	AvPacketStreamAudioFormat int
	AvPacketStreamVideoFormat int
)

const (
	AvPacketStreamAudioFormatUnknown AvPacketStreamAudioFormat = 0
	AvPacketStreamAudioFormatRawAac  AvPacketStreamAudioFormat = 1
	AvPacketStreamAudioFormatAdtsAac AvPacketStreamAudioFormat = 2

	AvPacketStreamVideoFormatUnknown AvPacketStreamVideoFormat = 0
	AvPacketStreamVideoFormatAvcc    AvPacketStreamVideoFormat = 1
	AvPacketStreamVideoFormatAnnexb  AvPacketStreamVideoFormat = 2
)

type AvPacketStreamOption struct {
	AudioFormat AvPacketStreamAudioFormat
	VideoFormat AvPacketStreamVideoFormat // 视频流的格式，注意，不是指编码格式，而是编码格式确定后，流的格式
}

var DefaultApsOption = AvPacketStreamOption{
	AudioFormat: AvPacketStreamAudioFormatRawAac,
	VideoFormat: AvPacketStreamVideoFormatAvcc,
}

type IAvPacketStream interface {
	// WithOption 修改配置项
	//
	// TODO(chef): [refactor] 重命名为WithAvPacketStreamOption 202301
	//
	WithOption(modOption func(option *AvPacketStreamOption))

	// FeedAudioSpecificConfig 传入音频AAC的初始化数据
	//
	// @param asc:
	//
	// AudioSpecificConfig。含义可参考 aac.AscContext, aac.MakeAscWithAdtsHeader 等内容。
	// 注意，调用 FeedAvPacket 传入AAC音频数据前，需要先调用 FeedAudioSpecificConfig。
	// FeedAudioSpecificConfig 在最开始总共调用一次，后面就可以一直调用 FeedAvPacket
	//
	FeedAudioSpecificConfig(asc []byte) error

	// FeedAvPacket
	//
	// @param packet:
	//
	// PayloadType: 类型，支持avc(h264)，hevc(h265)，aac
	//
	// Timestamp: 时间戳，单位毫秒。注意，是累计递增值，不是单个包的duration时长。
	//
	// Payload: 音视频数据，格式如下
	//
	// 如果是音频AAC，格式是裸数据，不需要adts头。
	// 注意，调用 FeedAvPacket 传入AAC音频数据前，需要先调用 FeedAudioSpecificConfig。
	// FeedAudioSpecificConfig 在最开始总共调用一次，后面就可以一直调用 FeedAvPacket
	//
	// 如果是视频，支持Avcc和Annexb两种格式。
	// Avcc也即[<4字节长度 + nal>...]，Annexb也即[<4字节start code 00 00 00 01 + nal>...]。
	// 注意，sps和pps也通过 FeedAvPacket 传入。sps和pps可以单独调用 FeedAvPacket，也可以sps+pps+I帧组合在一起调用一次 FeedAvPacket
	//
	FeedAvPacket(packet AvPacket) error
}
