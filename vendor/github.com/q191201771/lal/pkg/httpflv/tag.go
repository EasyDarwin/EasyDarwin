// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package httpflv

import (
	"io"

	"github.com/q191201771/naza/pkg/bele"
)

type TagHeader struct {
	Type      uint8  // type
	DataSize  uint32 // body大小，不包含 header 和 prev tag size 字段
	Timestamp uint32 // 绝对时间戳，单位毫秒
	StreamId  uint32 // always 0
}

type Tag struct {
	Header TagHeader
	Raw    []byte // 结构为 (11字节的 tag header) + (body) + (4字节的 prev tag size)
}

// Payload 只包含数据部分，去除了前面11字节的tag header和后面4字节的prev tag size
func (tag *Tag) Payload() []byte {
	return tag.Raw[TagHeaderSize : len(tag.Raw)-PrevTagSizeFieldSize]
}

func (tag *Tag) IsMetadata() bool {
	return tag.Header.Type == TagTypeMetadata
}

func (tag *Tag) IsAvc() bool {
	return tag.Header.Type == TagTypeVideo && (tag.Raw[TagHeaderSize]&0xF == codecIdAvc)
}

func (tag *Tag) IsHevc() bool {
	return tag.Header.Type == TagTypeVideo && (tag.Raw[TagHeaderSize]&0xF == codecIdHevc)
}

func (tag *Tag) IsAvcKeySeqHeader() bool {
	return tag.Header.Type == TagTypeVideo && tag.Raw[TagHeaderSize] == AvcKeyFrame && tag.Raw[TagHeaderSize+1] == AvcPacketTypeSeqHeader
}

func (tag *Tag) IsHevcKeySeqHeader() bool {
	return tag.Header.Type == TagTypeVideo && tag.Raw[TagHeaderSize] == HevcKeyFrame && tag.Raw[TagHeaderSize+1] == HevcPacketTypeSeqHeader
}

// IsVideoKeySeqHeader AVC或HEVC的seq header
func (tag *Tag) IsVideoKeySeqHeader() bool {
	return tag.IsAvcKeySeqHeader() || tag.IsHevcKeySeqHeader()
}

func (tag *Tag) IsAvcKeyNalu() bool {
	return tag.Header.Type == TagTypeVideo && tag.Raw[TagHeaderSize] == AvcKeyFrame && tag.Raw[TagHeaderSize+1] == AvcPacketTypeNalu
}

func (tag *Tag) IsHevcKeyNalu() bool {
	return tag.Header.Type == TagTypeVideo && tag.Raw[TagHeaderSize] == HevcKeyFrame && tag.Raw[TagHeaderSize+1] == HevcPacketTypeNalu
}

// IsVideoKeyNalu AVC或HEVC的关键帧
func (tag *Tag) IsVideoKeyNalu() bool {
	return tag.IsAvcKeyNalu() || tag.IsHevcKeyNalu()
}

func (tag *Tag) IsAacSeqHeader() bool {
	return tag.Header.Type == TagTypeAudio && tag.Raw[TagHeaderSize]>>4 == SoundFormatAac && tag.Raw[TagHeaderSize+1] == AacPacketTypeSeqHeader
}

func (tag *Tag) clone() (out Tag) {
	out.Header = tag.Header
	out.Raw = append(out.Raw, tag.Raw...)
	return
}

func (tag *Tag) ModTagTimestamp(timestamp uint32) {
	tag.Header.Timestamp = timestamp

	bele.BePutUint24(tag.Raw[4:], timestamp&0xffffff)
	tag.Raw[7] = byte(timestamp >> 24)
}

// PackHttpflvTag 打包一个序列化后的 tag 二进制buffer，包含 tag header，body，prev tag size
func PackHttpflvTag(t uint8, timestamp uint32, in []byte) []byte {
	out := make([]byte, TagHeaderSize+len(in)+PrevTagSizeFieldSize)
	out[0] = t
	bele.BePutUint24(out[1:], uint32(len(in)))
	bele.BePutUint24(out[4:], timestamp&0xFFFFFF)
	out[7] = uint8(timestamp >> 24)
	out[8] = 0
	out[9] = 0
	out[10] = 0
	copy(out[11:], in)
	bele.BePutUint32(out[TagHeaderSize+len(in):], uint32(TagHeaderSize+len(in)))
	return out
}

// ReadTag 从`rd`中读取数据并解析至`tag`
func ReadTag(rd io.Reader) (tag Tag, err error) {
	rawHeader := make([]byte, TagHeaderSize)
	if _, err = io.ReadAtLeast(rd, rawHeader, TagHeaderSize); err != nil {
		return
	}
	header := parseTagHeader(rawHeader)

	needed := int(header.DataSize) + PrevTagSizeFieldSize
	tag.Header = header
	tag.Raw = make([]byte, TagHeaderSize+needed)
	copy(tag.Raw, rawHeader)

	if _, err = io.ReadAtLeast(rd, tag.Raw[TagHeaderSize:], needed); err != nil {
		return
	}

	return
}

func parseTagHeader(rawHeader []byte) TagHeader {
	var h TagHeader
	h.Type = rawHeader[0]
	h.DataSize = bele.BeUint24(rawHeader[1:])
	h.Timestamp = (uint32(rawHeader[7]) << 24) + bele.BeUint24(rawHeader[4:])
	return h
}
