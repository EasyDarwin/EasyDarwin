// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtmp

import (
	"fmt"
	"io"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/naza/pkg/bele"
)

const (
	peerBandwidthLimitTypeHard    = uint8(0)
	peerBandwidthLimitTypeSoft    = uint8(1)
	peerBandwidthLimitTypeDynamic = uint8(2)
)

// MessagePacker 打包并发送 rtmp 信令
type MessagePacker struct {
	b *Buffer
}

func NewMessagePacker() *MessagePacker {
	return &MessagePacker{
		b: NewBuffer(256),
	}
}

// 注意，这个函数只会打包一个chunk头，所以调用方应自己保证在`bodyLen`小于chunk size时使用
func writeSingleChunkHeader(out []byte, csid int, bodyLen int, typeid uint8, streamid int) {
	// 目前这个函数只供发送信令时调用，信令的 csid 都是小于等于 63 的，如果传入的 csid 大于 63，直接 panic
	if csid > 63 {
		panic(csid)
	}

	format := 0
	out[0] = uint8(format<<6 | csid)
	// 0 0 0 是时间戳
	out[1] = 0
	out[2] = 0
	out[3] = 0
	bele.BePutUint24(out[4:], uint32(bodyLen))
	out[7] = typeid
	bele.LePutUint32(out[8:], uint32(streamid))
}

func (packer *MessagePacker) ChunkAndWrite(writer io.Writer, csid int, typeid uint8, streamid int) error {
	bodyLen := packer.b.Len() - 12

	if bodyLen <= LocalChunkSize {
		// 如果一个chunk就够放（大部分信令都是这种情况），我们直接在buffer前面预留的空间写入chunk header内容，避免造成拷贝
		writeSingleChunkHeader(packer.b.Bytes(), csid, bodyLen, typeid, streamid)
		_, err := packer.b.WriteTo(writer)
		return err
	}

	var h base.RtmpHeader
	h.Csid = csid
	h.MsgLen = uint32(bodyLen)
	h.MsgTypeId = typeid
	h.MsgStreamId = streamid
	h.TimestampAbs = 0
	chunks := Message2Chunks(packer.b.Bytes()[12:], &h)
	packer.b.Reset()
	_, err := writer.Write(chunks)
	return err
}

func (packer *MessagePacker) writeProtocolControlMessage(writer io.Writer, typeid uint8, val int) error {
	packer.b.ModWritePos(12)

	// 4
	_ = bele.WriteBe(packer.b, uint32(val))

	return packer.ChunkAndWrite(writer, csidProtocolControl, typeid, 0)
}

func (packer *MessagePacker) writeChunkSize(writer io.Writer, val int) error {
	return packer.writeProtocolControlMessage(writer, base.RtmpTypeIdSetChunkSize, val)
}

func (packer *MessagePacker) writeWinAckSize(writer io.Writer, val int) error {
	return packer.writeProtocolControlMessage(writer, base.RtmpTypeIdWinAckSize, val)
}

func (packer *MessagePacker) writePeerBandwidth(writer io.Writer, val int, limitType uint8) error {
	packer.b.ModWritePos(12)

	// 5
	_ = bele.WriteBe(packer.b, uint32(val))
	_ = packer.b.WriteByte(limitType)

	return packer.ChunkAndWrite(writer, csidProtocolControl, base.RtmpTypeIdBandwidth, 0)
}

// @param isPush: 推流为true，拉流为false
func (packer *MessagePacker) writeConnect(writer io.Writer, appName, tcUrl string, isPush bool) error {
	packer.b.ModWritePos(12)

	_ = Amf0.WriteString(packer.b, "connect")
	_ = Amf0.WriteNumber(packer.b, float64(tidClientConnect))

	var objs []ObjectPair
	objs = append(objs, ObjectPair{Key: "app", Value: appName})
	objs = append(objs, ObjectPair{Key: "type", Value: "nonprivate"})
	var flashVer string
	if isPush {
		flashVer = fmt.Sprintf("FMLE/3.0 (compatible; %s)", base.LalRtmpPushSessionConnectVersion)
	} else {
		flashVer = "LNX 9,0,124,2"
	}
	objs = append(objs, ObjectPair{Key: "flashVer", Value: flashVer})
	// fpad True if proxy is being used.
	objs = append(objs, ObjectPair{Key: "fpad", Value: false})
	objs = append(objs, ObjectPair{Key: "tcUrl", Value: tcUrl})
	_ = Amf0.WriteObject(packer.b, objs)

	return packer.ChunkAndWrite(writer, csidOverConnection, base.RtmpTypeIdCommandMessageAmf0, 0)
}

// @param objectEncoding 设置0或者3，表示是Amf0或AMF3，上层可根据connect信令中的objectEncoding值设置该值
func (packer *MessagePacker) writeConnectResult(writer io.Writer, tid int, objectEncoding int) error {
	packer.b.ModWritePos(12)

	_ = Amf0.WriteString(packer.b, "_result")
	_ = Amf0.WriteNumber(packer.b, float64(tid))
	objs := []ObjectPair{
		{Key: "fmsVer", Value: "FMS/3,0,1,123"},
		{Key: "capabilities", Value: 31},
	}
	_ = Amf0.WriteObject(packer.b, objs)
	objs = []ObjectPair{
		{Key: "level", Value: "status"},
		{Key: "code", Value: "NetConnection.Connect.Success"},
		{Key: "description", Value: "Connection succeeded."},
		{Key: "objectEncoding", Value: objectEncoding},
		{Key: "version", Value: base.LalRtmpConnectResultVersion},
	}
	_ = Amf0.WriteObject(packer.b, objs)

	return packer.ChunkAndWrite(writer, csidOverConnection, base.RtmpTypeIdCommandMessageAmf0, 0)
}

func (packer *MessagePacker) writeCreateStream(writer io.Writer) error {
	packer.b.ModWritePos(12)

	// 25 = 15 + 9 + 1
	_ = Amf0.WriteString(packer.b, "createStream")
	_ = Amf0.WriteNumber(packer.b, float64(tidClientCreateStream))
	_ = Amf0.WriteNull(packer.b)

	return packer.ChunkAndWrite(writer, csidOverConnection, base.RtmpTypeIdCommandMessageAmf0, 0)
}

func (packer *MessagePacker) writeCreateStreamResult(writer io.Writer, tid int) error {
	packer.b.ModWritePos(12)

	// 29
	_ = Amf0.WriteString(packer.b, "_result")
	_ = Amf0.WriteNumber(packer.b, float64(tid))
	_ = Amf0.WriteNull(packer.b)
	_ = Amf0.WriteNumber(packer.b, float64(Msid1))

	return packer.ChunkAndWrite(writer, csidOverConnection, base.RtmpTypeIdCommandMessageAmf0, 0)
}

func (packer *MessagePacker) writePlay(writer io.Writer, streamName string, streamid int) error {
	packer.b.ModWritePos(12)

	_ = Amf0.WriteString(packer.b, "play")
	_ = Amf0.WriteNumber(packer.b, float64(tidClientPlay))
	_ = Amf0.WriteNull(packer.b)
	_ = Amf0.WriteString(packer.b, streamName)

	return packer.ChunkAndWrite(writer, csidOverStream, base.RtmpTypeIdCommandMessageAmf0, streamid)
}

func (packer *MessagePacker) writePublish(writer io.Writer, appName string, streamName string, streamid int) error {
	packer.b.ModWritePos(12)

	_ = Amf0.WriteString(packer.b, "publish")
	_ = Amf0.WriteNumber(packer.b, float64(tidClientPublish))
	_ = Amf0.WriteNull(packer.b)
	_ = Amf0.WriteString(packer.b, streamName)

	// <spec-rtmp_specification_1.0.pdf>
	// 7.2.2.6.  publish
	//
	// Type of publishing.
	// Set to "live": Live data is published without recording it in a file.
	//
	_ = Amf0.WriteString(packer.b, "live")

	return packer.ChunkAndWrite(writer, csidOverStream, base.RtmpTypeIdCommandMessageAmf0, streamid)
}

func (packer *MessagePacker) writeOnStatusPublish(writer io.Writer, streamid int) error {
	packer.b.ModWritePos(12)

	// 105
	_ = Amf0.WriteString(packer.b, "onStatus")
	_ = Amf0.WriteNumber(packer.b, 0)
	_ = Amf0.WriteNull(packer.b)
	objs := []ObjectPair{
		{Key: "level", Value: "status"},
		{Key: "code", Value: "NetStream.Publish.Start"},
		{Key: "description", Value: "Start publishing"},
	}
	_ = Amf0.WriteObject(packer.b, objs)

	return packer.ChunkAndWrite(writer, csidOverStream, base.RtmpTypeIdCommandMessageAmf0, streamid)
}

func (packer *MessagePacker) writeOnStatusPlay(writer io.Writer, streamid int) error {
	packer.b.ModWritePos(12)

	// 96
	_ = Amf0.WriteString(packer.b, "onStatus")
	_ = Amf0.WriteNumber(packer.b, 0)
	_ = Amf0.WriteNull(packer.b)
	objs := []ObjectPair{
		{Key: "level", Value: "status"},
		{Key: "code", Value: "NetStream.Play.Start"},
		{Key: "description", Value: "Start live"},
	}
	_ = Amf0.WriteObject(packer.b, objs)

	return packer.ChunkAndWrite(writer, csidOverStream, base.RtmpTypeIdCommandMessageAmf0, streamid)
}

func (packer *MessagePacker) writeStreamIsRecorded(writer io.Writer, streamid uint32) error {
	packer.b.ModWritePos(12)

	// 6
	_ = bele.WriteBe(packer.b, uint16(base.RtmpUserControlRecorded))
	_ = bele.WriteBe(packer.b, uint32(streamid))

	return packer.ChunkAndWrite(writer, csidProtocolControl, base.RtmpTypeIdUserControl, 0)
}

func (packer *MessagePacker) writeStreamBegin(writer io.Writer, streamid uint32) error {
	packer.b.ModWritePos(12)

	// 6
	_ = bele.WriteBe(packer.b, uint16(base.RtmpUserControlStreamBegin))
	_ = bele.WriteBe(packer.b, uint32(streamid))

	return packer.ChunkAndWrite(writer, csidProtocolControl, base.RtmpTypeIdUserControl, 0)
}

func (packer *MessagePacker) writePingRequest(writer io.Writer, timestamp uint32) error {
	packer.b.ModWritePos(12)

	// 6
	_ = bele.WriteBe(packer.b, uint16(base.RtmpUserControlPingRequest))
	_ = bele.WriteBe(packer.b, timestamp)

	return packer.ChunkAndWrite(writer, csidProtocolControl, base.RtmpTypeIdUserControl, 0)
}
func (packer *MessagePacker) writeAcknowledgement(writer io.Writer, seqNum uint32) error {
	packer.b.ModWritePos(12)

	// 5
	_ = bele.WriteBe(packer.b, seqNum)

	return packer.ChunkAndWrite(writer, csidProtocolControl, base.RtmpTypeIdAck, 0)
}

func (packer *MessagePacker) writePingResponse(writer io.Writer, timestamp uint32) error {
	packer.b.ModWritePos(12)

	// 6
	_ = bele.WriteBe(packer.b, uint16(base.RtmpUserControlPingResponse))
	_ = bele.WriteBe(packer.b, timestamp)

	return packer.ChunkAndWrite(writer, csidProtocolControl, base.RtmpTypeIdUserControl, 0)
}

// ---------------------------------------------------------------------------------------------------------------------

// TODO(chef): 整理所有的buffer

type Buffer struct {
	core     []byte
	readPos  int
	writePos int
}

func NewBuffer(n int) *Buffer {
	return &Buffer{
		core:     make([]byte, n),
		readPos:  0,
		writePos: 0,
	}
}

func (b *Buffer) Bytes() []byte {
	return b.core[b.readPos:b.writePos]
}

func (b *Buffer) Len() int {
	return b.writePos - b.readPos
}

func (b *Buffer) Reset() {
	b.readPos = 0
	b.writePos = 0
}

func (b *Buffer) Write(p []byte) (n int, err error) {
	b.grow(len(p))
	copy(b.core[b.writePos:], p)
	b.writePos += len(p)

	return len(p), nil
}

func (b *Buffer) WriteByte(c byte) error {
	b.grow(1)
	b.core[b.writePos] = c
	b.writePos++
	return nil
}

func (b *Buffer) WriteTo(w io.Writer) (n int64, err error) {
	if nBytes := b.Len(); nBytes > 0 {
		m, e := w.Write(b.Bytes())
		if m > nBytes {
			Log.Panicf("Buffer.WriteTo: invalid Write count. expected=%d, actual=%d", nBytes, m)
		}
		b.readPos += m
		n = int64(m)
		if e != nil {
			return n, e
		}
		if m != nBytes {
			return n, io.ErrShortWrite
		}
	}
	b.Reset()
	return n, nil
}

func (b *Buffer) ModWritePos(pos int) {
	b.writePos = pos
}

func (b *Buffer) grow(n int) {
	if cap(b.core)-b.writePos >= n {
		return
	}

	// TODO(chef): 可以先尝试是否能挪出空闲位置

	var newLen int
	if cap(b.core) == 0 {
		newLen = 128
	} else {
		newLen = cap(b.core) * 2
	}
	buf := make([]byte, newLen)
	Log.Debugf("Buffer::grow. need=%d, old len=%d, cap=%d, new len=%d", n, b.Len(), cap(b.core), newLen)
	copy(buf, b.core[b.readPos:b.writePos])
	b.core = buf
	b.readPos = 0
	b.writePos = b.writePos - b.readPos
}
