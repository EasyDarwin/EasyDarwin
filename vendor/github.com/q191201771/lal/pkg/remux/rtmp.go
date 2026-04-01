// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package remux

import (
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/rtmp"
)

// MakeDefaultRtmpHeader
//
// 使用场景：一般是输入流转换为输出流时。
// 目的：使得流格式更标准。
// 做法：设置 MsgStreamId 和 Csid，其他字段保持`in`的值。
func MakeDefaultRtmpHeader(in base.RtmpHeader) (out base.RtmpHeader) {
	out.MsgLen = in.MsgLen
	out.TimestampAbs = in.TimestampAbs
	out.MsgTypeId = in.MsgTypeId
	out.MsgStreamId = rtmp.Msid1
	switch in.MsgTypeId {
	case base.RtmpTypeIdMetadata:
		out.Csid = rtmp.CsidAmf
	case base.RtmpTypeIdAudio:
		out.Csid = rtmp.CsidAudio
	case base.RtmpTypeIdVideo:
		out.Csid = rtmp.CsidVideo
	}
	return
}

// ---------------------------------------------------------------------------------------------------------------------

// LazyRtmpChunkDivider 在必要时，有且仅有一次做切分成chunk的操作
type LazyRtmpChunkDivider struct {
	msg              base.RtmpMsg
	chunksWithSdf    []byte
	chunksWithoutSdf []byte
}

func (lcd *LazyRtmpChunkDivider) Init(msg base.RtmpMsg) {
	lcd.msg = msg
}

func (lcd *LazyRtmpChunkDivider) GetEnsureWithSdf() []byte {
	if lcd.chunksWithSdf == nil {
		var msg []byte
		if lcd.msg.Header.MsgTypeId == base.RtmpTypeIdMetadata {
			msg2 := lcd.msg.Clone()
			msg2.Payload, _ = rtmp.MetadataEnsureWithSdf(msg2.Payload)
			msg2.Header.MsgLen = uint32(len(msg2.Payload))
			msg2.Header = MakeDefaultRtmpHeader(msg2.Header)
			lcd.chunksWithSdf = rtmp.Message2Chunks(msg2.Payload, &msg2.Header)
		} else {
			msg = lcd.msg.Payload
			h := MakeDefaultRtmpHeader(lcd.msg.Header)
			lcd.chunksWithSdf = rtmp.Message2Chunks(msg, &h)
		}
	}
	return lcd.chunksWithSdf
}

func (lcd *LazyRtmpChunkDivider) GetEnsureWithoutSdf() []byte {
	if lcd.chunksWithoutSdf == nil {
		var msg []byte
		if lcd.msg.Header.MsgTypeId == base.RtmpTypeIdMetadata {
			msg2 := lcd.msg.Clone()
			msg2.Payload, _ = rtmp.MetadataEnsureWithoutSdf(msg2.Payload)
			msg2.Header.MsgLen = uint32(len(msg2.Payload))
			msg2.Header = MakeDefaultRtmpHeader(msg2.Header)
			lcd.chunksWithoutSdf = rtmp.Message2Chunks(msg2.Payload, &msg2.Header)
		} else {
			msg = lcd.msg.Payload
			h := MakeDefaultRtmpHeader(lcd.msg.Header)
			lcd.chunksWithoutSdf = rtmp.Message2Chunks(msg, &h)
		}
	}
	return lcd.chunksWithoutSdf
}
