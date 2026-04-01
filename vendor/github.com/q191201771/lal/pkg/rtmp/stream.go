// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtmp

import (
	"encoding/hex"
	"fmt"
	"github.com/q191201771/naza/pkg/nazabytes"

	"github.com/q191201771/lal/pkg/base"
)

// ----- Stream --------------------------------------------------------------------------------------------------------

type Stream struct {
	header base.RtmpHeader
	msg    StreamMsg

	absTsFlag bool   // 标记当这个stream收到新的msg的时候，是否收到过绝对时间
	timestamp uint32 // 注意，是rtmp chunk协议header中的时间戳，可能是绝对的，也可能是相对的。上层不应该使用这个字段，而应该使用Header.TimestampAbs
}

func NewStream() *Stream {
	return &Stream{
		msg: StreamMsg{
			buff: nazabytes.NewBuffer(initMsgLen),
		},
	}
}

// 序列化成可读字符串，一般用于发生错误时打印日志
func (stream *Stream) toDebugString() string {
	return fmt.Sprintf("header=%+v, b=%s, hex=%s",
		stream.header, stream.msg.buff.DebugString(), hex.Dump(stream.msg.buff.Peek(4096)))
}

func (stream *Stream) toAvMsg() base.RtmpMsg {
	// TODO chef: 考虑可能出现header中的len和buf的大小不一致的情况
	if stream.header.MsgLen != uint32(stream.msg.buff.Len()) {
		Log.Errorf("toAvMsg. headerMsgLen=%d, bufLen=%d", stream.header.MsgLen, stream.msg.buff.Len())
	}
	return base.RtmpMsg{
		Header:  stream.header,
		Payload: stream.msg.buff.Bytes(),
	}
}

// ----- StreamMsg -----------------------------------------------------------------------------------------------------

type StreamMsg struct {
	// TODO(chef): [refactor] 考虑外部(chunk_composer)不要直接访问buff，封装一层 202206
	buff *nazabytes.Buffer
}

// Grow 确保可写空间，如果不够会扩容
func (msg *StreamMsg) Grow(n uint32) {
	msg.buff.Grow(int(n))
}

func (msg *StreamMsg) Len() uint32 {
	return uint32(msg.buff.Len())
}

func (msg *StreamMsg) Flush(n uint32) {
	msg.buff.Flush(int(n))
}

func (msg *StreamMsg) Skip(n uint32) {
	msg.buff.Skip(int(n))
}

func (msg *StreamMsg) Reset() {
	msg.buff.Reset()
}

func (msg *StreamMsg) ResetAndFree() {
	msg.buff.ResetAndFree()
}

func (msg *StreamMsg) peekStringWithType() (string, error) {
	str, _, err := Amf0.ReadString(msg.buff.Bytes())
	return str, err
}

func (msg *StreamMsg) readStringWithType() (string, error) {
	str, l, err := Amf0.ReadString(msg.buff.Bytes())
	if err == nil {
		msg.Skip(uint32(l))
	}
	return str, err
}

func (msg *StreamMsg) readNumberWithType() (int, error) {
	val, l, err := Amf0.ReadNumber(msg.buff.Bytes())
	if err == nil {
		msg.Skip(uint32(l))
	}
	return int(val), err
}

func (msg *StreamMsg) readObjectWithType() (ObjectPairArray, error) {
	opa, l, err := Amf0.ReadObject(msg.buff.Bytes())
	if err == nil {
		msg.Skip(uint32(l))
	}
	return opa, err
}

func (msg *StreamMsg) readNull() error {
	l, err := Amf0.ReadNull(msg.buff.Bytes())
	if err == nil {
		msg.Skip(uint32(l))
	}
	return err
}
