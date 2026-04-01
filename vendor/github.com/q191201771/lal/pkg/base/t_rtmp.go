// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

import (
	"encoding/hex"
	"fmt"

	"github.com/q191201771/naza/pkg/bele"
	"github.com/q191201771/naza/pkg/nazabytes"
)

const (
	// RtmpTypeIdAudio spec-rtmp_specification_1.0.pdf
	// 7.1. Types of Messages
	RtmpTypeIdAudio        uint8 = 8
	RtmpTypeIdVideo        uint8 = 9
	RtmpTypeIdMetadata     uint8 = 18 // RtmpTypeIdDataMessageAmf0
	RtmpTypeIdSetChunkSize uint8 = 1
	// RtmpTypeIdAck 和 RtmpTypeIdWinAckSize 的含义：
	//
	// 一端向另一端发送 RtmpTypeIdWinAckSize ，要求对端每收够一定数据（一定数据的阈值包含在 RtmpTypeIdWinAckSize 信令中）后，向本端回复 RtmpTypeIdAck 。
	//
	// 常见的应用场景：数据发送端要求数据接收端定时发送心跳信令给本端。
	RtmpTypeIdAck         uint8 = 3
	RtmpTypeIdUserControl uint8 = 4
	// RtmpTypeIdWinAckSize 见 RtmpTypeIdAck
	RtmpTypeIdWinAckSize         uint8 = 5
	RtmpTypeIdBandwidth          uint8 = 6
	RtmpTypeIdCommandMessageAmf3 uint8 = 17
	RtmpTypeIdCommandMessageAmf0 uint8 = 20
	RtmpTypeIdAggregateMessage   uint8 = 22

	// RtmpUserControlStreamBegin RtmpUserControlXxx...
	//
	// user control message type
	//
	RtmpUserControlStreamBegin  uint8 = 0
	RtmpUserControlRecorded     uint8 = 4
	RtmpUserControlPingRequest  uint8 = 6
	RtmpUserControlPingResponse uint8 = 7

	// RtmpFrameTypeKey spec-video_file_format_spec_v10.pdf
	// Video tags
	//   VIDEODATA
	//     FrameType UB[4]
	//     CodecId   UB[4]
	//   AVCVIDEOPACKET
	//     AVCPacketType   UI8
	//     CompositionTime SI24
	//     Data            UI8[n]
	RtmpFrameTypeKey   uint8 = 1
	RtmpFrameTypeInter uint8 = 2

	// RtmpCodecIdAvc
	//
	// Video tags -> VIDEODATA -> CodecID
	//
	// 1: JPEG (currently unused)
	// 2: Sorenson H.263
	// 3: Screen video
	// 4: On2 VP6
	// 5: On2 VP6 with alpha channel
	// 6: Screen video version 2
	// 7: AVC
	//
	RtmpCodecIdAvc  uint8 = 7
	RtmpCodecIdHevc uint8 = 12

	// RtmpAvcPacketTypeSeqHeader RtmpAvcPacketTypeNalu RtmpHevcPacketTypeSeqHeader RtmpHevcPacketTypeNalu
	// 注意，按照标准文档上描述，PacketType还有可能为2：
	// 2: AVC end of sequence (lower level NALU sequence ender is not required or supported)
	//
	// 我自己遇到过在流结尾时，对端发送 27 02 00 00 00的情况（比如我们的使用wontcry.flv的单元测试，最后一个包）
	//
	RtmpAvcPacketTypeSeqHeader  uint8 = 0
	RtmpAvcPacketTypeNalu       uint8 = 1
	RtmpHevcPacketTypeSeqHeader       = RtmpAvcPacketTypeSeqHeader
	RtmpHevcPacketTypeNalu            = RtmpAvcPacketTypeNalu

	// enhanced-rtmp packetType https://github.com/veovera/enhanced-rtmp
	RtmpExPacketTypeSequenceStart uint8 = 0
	RtmpExPacketTypeCodedFrames   uint8 = 1 // CompositionTime不为0时有这个类型
	RtmpExPacketTypeSequenceEnd   uint8 = 2
	RtmpExPacketTypeCodedFramesX  uint8 = 3

	// RtmpExFrameTypeKeyFrame RtmpExFrameTypeXXX...
	//
	// The following FrameType values are defined:
	// 0 = reserved
	// 1 = key frame (a seekable frame)
	// 2 = inter frame (a non-seekable frame)
	// ...
	RtmpExFrameTypeKeyFrame uint8 = 1

	RtmpAvcKeyFrame    = RtmpFrameTypeKey<<4 | RtmpCodecIdAvc
	RtmpHevcKeyFrame   = RtmpFrameTypeKey<<4 | RtmpCodecIdHevc
	RtmpAvcInterFrame  = RtmpFrameTypeInter<<4 | RtmpCodecIdAvc
	RtmpHevcInterFrame = RtmpFrameTypeInter<<4 | RtmpCodecIdHevc

	// RtmpSoundFormatAac spec-video_file_format_spec_v10.pdf
	// Audio tags
	//   AUDIODATA
	//     SoundFormat UB[4]
	//     SoundRate   UB[2]
	//     SoundSize   UB[1]
	//     SoundType   UB[1]
	//   AACAUDIODATA
	//     AACPacketType UI8
	//     Data          UI8[n]
	// 注意，视频的CodecId是后4位，音频是前4位
	RtmpSoundFormatG711A uint8 = 7
	RtmpSoundFormatG711U uint8 = 8
	RtmpSoundFormatAac   uint8 = 10
	RtmpSoundFormatOpus  uint8 = 13

	RtmpAacPacketTypeSeqHeader = 0
	RtmpAacPacketTypeRaw       = 1
)

type RtmpHeader struct {
	Csid         int
	MsgLen       uint32 // 不包含header的大小
	MsgTypeId    uint8  // 8 audio 9 video 18 metadata
	MsgStreamId  int
	TimestampAbs uint32 // dts, 经过计算得到的流上的绝对时间戳，单位毫秒
}

type RtmpMsg struct {
	Header  RtmpHeader
	Payload []byte // Payload不包含Header内容。如果需要将RtmpMsg序列化成RTMP chunk，可调用 rtmp.ChunkDivider 相关的函数
}

func (msg RtmpMsg) IsAvcKeySeqHeader() bool {
	return msg.Header.MsgTypeId == RtmpTypeIdVideo && msg.Payload[0] == RtmpAvcKeyFrame && msg.Payload[1] == RtmpAvcPacketTypeSeqHeader
}

func (msg RtmpMsg) IsHevcKeySeqHeader() bool {
	if msg.Header.MsgTypeId != RtmpTypeIdVideo {
		return false
	}

	isExtHeader := msg.Payload[0] & 0x80
	if isExtHeader != 0 {
		packetType := msg.Payload[0] & 0x0f
		if msg.Payload[1] == 'h' && msg.Payload[2] == 'v' && msg.Payload[3] == 'c' && msg.Payload[4] == '1' && packetType == RtmpExPacketTypeSequenceStart {
			return true
		}
	} else {
		return msg.Payload[0] == RtmpHevcKeyFrame && msg.Payload[1] == RtmpHevcPacketTypeSeqHeader
	}

	return false
}

func (msg RtmpMsg) IsEnhanced() bool {
	isExtHeader := msg.Payload[0] & 0x80
	if isExtHeader != 0 {
		return true
	}

	return false
}

func (msg RtmpMsg) IsVideoKeySeqHeader() bool {
	return msg.IsAvcKeySeqHeader() || msg.IsHevcKeySeqHeader()
}

func (msg RtmpMsg) IsAvcKeyNalu() bool {
	return msg.Header.MsgTypeId == RtmpTypeIdVideo && msg.Payload[0] == RtmpAvcKeyFrame && msg.Payload[1] == RtmpAvcPacketTypeNalu
}

func (msg RtmpMsg) IsHevcKeyNalu() bool {
	if msg.Header.MsgTypeId != RtmpTypeIdVideo {
		return false
	}

	isExtHeader := msg.Payload[0] & 0x80
	if isExtHeader != 0 {
		frameType := msg.Payload[0] >> 4 & 0x07
		packetType := msg.Payload[0] & 0x0F
		return frameType == RtmpExFrameTypeKeyFrame && packetType != RtmpExPacketTypeSequenceStart
	}

	return msg.Payload[0] == RtmpHevcKeyFrame && msg.Payload[1] == RtmpHevcPacketTypeNalu
}

func (msg RtmpMsg) IsEnchanedHevcNalu() bool {
	isExtHeader := msg.Payload[0] & 0x80
	if isExtHeader != 0 {
		packetType := msg.Payload[0] & 0x0f
		if packetType == RtmpExPacketTypeCodedFrames || packetType == RtmpExPacketTypeCodedFramesX {
			return true
		}
	}

	return false
}

func (msg RtmpMsg) GetEnchanedHevcNaluIndex() int {
	isExtHeader := msg.Payload[0] & 0x80
	if isExtHeader != 0 {
		packetType := msg.Payload[0] & 0x0f
		switch packetType {
		case RtmpExPacketTypeCodedFrames:
			// NALU前面有3个字节CompositionTime
			return 5 + 3
		case RtmpExPacketTypeCodedFramesX:
			return 5
		}
	}

	return 0
}

func (msg RtmpMsg) IsVideoKeyNalu() bool {
	return msg.IsAvcKeyNalu() || msg.IsHevcKeyNalu()
}

func (msg RtmpMsg) IsAacSeqHeader() bool {
	return msg.Header.MsgTypeId == RtmpTypeIdAudio && msg.AudioCodecId() == RtmpSoundFormatAac && msg.Payload[1] == RtmpAacPacketTypeSeqHeader
}

func (msg RtmpMsg) VideoCodecId() uint8 {
	isExtHeader := msg.Payload[0] & 0x80
	if isExtHeader == 0 {
		return msg.Payload[0] & 0xF
	}

	if msg.Payload[1] == 'h' && msg.Payload[2] == 'v' && msg.Payload[3] == 'c' && msg.Payload[4] == '1' {
		return RtmpCodecIdHevc
	}

	return RtmpCodecIdAvc
}

func (msg RtmpMsg) AudioCodecId() uint8 {
	return msg.Payload[0] >> 4
}

func (msg RtmpMsg) Clone() (ret RtmpMsg) {
	ret.Header = msg.Header
	ret.Payload = make([]byte, len(msg.Payload))
	copy(ret.Payload, msg.Payload)
	return
}

func (msg RtmpMsg) Dts() uint32 {
	return msg.Header.TimestampAbs
}

// Pts
//
// 注意，只有视频才能调用该函数获取pts，音频的dts和pts都直接使用 RtmpMsg.Header.TimestampAbs
func (msg RtmpMsg) Pts() uint32 {
	return msg.Header.TimestampAbs + bele.BeUint24(msg.Payload[2:])
}

func (msg RtmpMsg) Cts() uint32 {
	if msg.Header.MsgTypeId == RtmpTypeIdAudio {
		return bele.BeUint24(msg.Payload[2:])
	}

	isExtHeader := msg.Payload[0] & 0x80
	if isExtHeader != 0 {
		packetType := msg.Payload[0] & 0x0F
		switch packetType {
		case RtmpExPacketTypeCodedFrames:
			return bele.BeUint24(msg.Payload[5:])
		case RtmpExPacketTypeCodedFramesX:
			return 0
		default:
			Log.Warnf("RtmpMsg.Cts: packetType invalid, packetType=%d", packetType)
			return 0
		}
	}

	return bele.BeUint24(msg.Payload[2:])
}

func (msg RtmpMsg) DebugString() string {
	isExtHeader := msg.Payload[0] & 0x80
	if msg.Header.MsgTypeId == RtmpTypeIdVideo && isExtHeader != 0 {
		frameType := msg.Payload[0] >> 4 & 0x07
		packetType := msg.Payload[0] & 0x0F // e.g. RtmpExPacketTypeSequenceStart
		if isExtHeader != 0 {
			return fmt.Sprintf("type=%d,len=%d,dts=%d, ext(%d, %d, %d), payload=%s",
				msg.Header.MsgTypeId, msg.Header.MsgLen, msg.Header.TimestampAbs,
				isExtHeader, frameType, packetType,
				hex.Dump(nazabytes.Prefix(msg.Payload, 64)))
		}
	}

	return fmt.Sprintf("type=%d,len=%d,dts=%d, payload=%s",
		msg.Header.MsgTypeId, msg.Header.MsgLen, msg.Header.TimestampAbs, hex.Dump(nazabytes.Prefix(msg.Payload, 64)))
}
