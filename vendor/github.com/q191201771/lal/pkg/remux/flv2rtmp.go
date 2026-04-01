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
	"github.com/q191201771/lal/pkg/httpflv"
	"github.com/q191201771/lal/pkg/rtmp"
)

func FlvTagHeader2RtmpHeader(in httpflv.TagHeader) (out base.RtmpHeader) {
	out.MsgLen = in.DataSize
	out.MsgTypeId = in.Type
	out.MsgStreamId = rtmp.Msid1
	switch in.Type {
	case httpflv.TagTypeMetadata:
		out.Csid = rtmp.CsidAmf
	case httpflv.TagTypeAudio:
		out.Csid = rtmp.CsidAudio
	case httpflv.TagTypeVideo:
		out.Csid = rtmp.CsidVideo
	}
	out.TimestampAbs = in.Timestamp
	return
}

// FlvTag2RtmpMsg @return msg: 返回的内存块引用参数`tag`的内存块
func FlvTag2RtmpMsg(tag httpflv.Tag) (msg base.RtmpMsg) {
	msg.Header = FlvTagHeader2RtmpHeader(tag.Header)
	msg.Payload = tag.Payload()
	return
}

// FlvTag2RtmpChunks @return 返回的内存块为内部新申请
func FlvTag2RtmpChunks(tag httpflv.Tag) []byte {
	rtmpHeader := FlvTagHeader2RtmpHeader(tag.Header)
	return rtmp.Message2Chunks(tag.Payload(), &rtmpHeader)
}
