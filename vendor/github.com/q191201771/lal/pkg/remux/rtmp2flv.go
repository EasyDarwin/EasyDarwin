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
	"github.com/q191201771/naza/pkg/nazalog"
)

// RtmpMsg2FlvTag @return 返回的内存块为新申请的独立内存块
func RtmpMsg2FlvTag(msg base.RtmpMsg) *httpflv.Tag {
	var tag httpflv.Tag
	tag.Header.Type = msg.Header.MsgTypeId
	tag.Header.DataSize = msg.Header.MsgLen
	tag.Header.Timestamp = msg.Header.TimestampAbs
	tag.Raw = httpflv.PackHttpflvTag(msg.Header.MsgTypeId, msg.Header.TimestampAbs, msg.Payload)
	return &tag
}

// -------------------------------------------------------------------------------------------------------------------

// LazyRtmpMsg2FlvTag 在必要时，有且仅有一次做转换操作
type LazyRtmpMsg2FlvTag struct {
	msg base.RtmpMsg
	//tagWithSdf []byte
	tagWithoutSdf []byte
}

func (l *LazyRtmpMsg2FlvTag) Init(msg base.RtmpMsg) {
	l.msg = msg
}

func (l *LazyRtmpMsg2FlvTag) GetEnsureWithSdf() []byte {
	// TODO(chef): [refactor] 这个函数目前没有实际用途 202207
	nazalog.Errorf("LazyRtmpMsg2FlvTag::GetEnsureWithSdf() is not implemented")
	return l.GetEnsureWithoutSdf()
}

func (l *LazyRtmpMsg2FlvTag) GetEnsureWithoutSdf() []byte {
	if l.tagWithoutSdf == nil {
		if l.msg.Header.MsgTypeId == base.RtmpTypeIdMetadata {
			msg2 := l.msg.Clone()
			msg2.Payload, _ = rtmp.MetadataEnsureWithoutSdf(msg2.Payload)
			l.tagWithoutSdf = RtmpMsg2FlvTag(msg2).Raw
		} else {
			l.tagWithoutSdf = RtmpMsg2FlvTag(l.msg).Raw
		}
	}
	return l.tagWithoutSdf
}
