// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtmp

// 将message切割成chunk

import (
	"net"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/bele"
)

type ChunkDivider struct {
	localChunkSize int
}

var defaultChunkDivider = ChunkDivider{
	localChunkSize: LocalChunkSize,
}

// Message2Chunks
//
// @return 返回的内存块由内部申请，不依赖参数<message>内存块
func Message2Chunks(message []byte, header *base.RtmpHeader) []byte {
	return defaultChunkDivider.Message2Chunks(message, header)
}

// Message2ChunksV
//
// @param message: 待打包的message支持放在多个字节切片中
func Message2ChunksV(message net.Buffers, header *base.RtmpHeader) []byte {
	return defaultChunkDivider.Message2ChunksV(message, header)
}

// Message2Chunks
//
// TODO chef: [opt] 新的 message 的第一个 chunk 始终使用 fmt0 格式，没有参考前一个 message
func (d *ChunkDivider) Message2Chunks(message []byte, header *base.RtmpHeader) []byte {
	return message2Chunks(message, header, nil, d.localChunkSize)
}

func (d *ChunkDivider) Message2ChunksV(message net.Buffers, header *base.RtmpHeader) []byte {
	return message2ChunksV(message, header, nil, d.localChunkSize)
}

// ---------------------------------------------------------------------------------------------------------------------

// @param 返回头的大小
func calcHeader(header *base.RtmpHeader, prevHeader *base.RtmpHeader, out []byte) int {
	var index int

	// 计算fmt和timestamp
	fmt := uint8(0)
	var timestamp uint32
	if prevHeader == nil {
		timestamp = header.TimestampAbs
	} else {
		if header.MsgStreamId == prevHeader.MsgStreamId {
			fmt++
			if header.MsgLen == prevHeader.MsgLen && header.MsgTypeId == prevHeader.MsgTypeId {
				fmt++
				if header.TimestampAbs == prevHeader.TimestampAbs {
					fmt++
				}
			}
			if header.TimestampAbs > maxTimestampInMessageHeader {
				// 将数据打包成rtmp chunk发送给vlc，时间戳超过3字节最大范围时，
				// vlc认为fmt0和fmt3两种格式，都需要携带扩展时间戳字段，并且该时间戳字段必须使用绝对时间戳。
				timestamp = header.TimestampAbs
			} else {
				timestamp = header.TimestampAbs - prevHeader.TimestampAbs
			}
		} else {
			timestamp = header.TimestampAbs
		}
	}

	// 设置fmt
	out[index] = fmt << 6

	// 设置csid
	if header.Csid >= 2 && header.Csid <= 63 {
		out[index] |= uint8(header.Csid)
		index++
	} else if header.Csid >= 64 && header.Csid <= 319 {
		// value 0
		index++
		out[index] = uint8(header.Csid - 64)
		index++
	} else {
		out[index] |= 1
		index++
		out[index] = uint8(header.Csid - 64)
		index++
		out[index] = uint8((header.Csid - 64) >> 8)
		index++
	}

	// 设置timestamp msgLen msgTypeId msgStreamId
	if fmt <= 2 {
		if timestamp > maxTimestampInMessageHeader {
			bele.BePutUint24(out[index:], maxTimestampInMessageHeader)
		} else {
			bele.BePutUint24(out[index:], timestamp)
		}
		index += 3

		if fmt <= 1 {
			bele.BePutUint24(out[index:], header.MsgLen)
			index += 3
			out[index] = header.MsgTypeId
			index++

			if fmt == 0 {
				bele.LePutUint32(out[index:], uint32(header.MsgStreamId))
				index += 4
			}
		}
	}

	// 设置扩展时间戳
	if timestamp > maxTimestampInMessageHeader {
		bele.BePutUint32(out[index:], timestamp)
		index += 4
	}

	return index
}

func message2Chunks(message []byte, header *base.RtmpHeader, prevHeader *base.RtmpHeader, chunkSize int) []byte {
	//if header.Csid < minCsid || header.Csid > maxCsid {
	//	return nil, ErrRtmp
	//}

	// 注意，这里我们要尽量缩小预分配内存的大小
	var maxNeededLen int

	numOfChunk := len(message) / chunkSize
	maxNeededLen = numOfChunk * (chunkSize + maxHeaderSize)

	// 计算chunk数量，最后一个chunk的大小
	lastChunkSize := chunkSize
	if len(message)%chunkSize != 0 {
		numOfChunk++
		lastChunkSize = len(message) % chunkSize
		maxNeededLen += lastChunkSize + maxHeaderSize
	}

	out := make([]byte, maxNeededLen)

	var index int

	// NOTICE 和srs交互时，发现srs要求message中的非第一个chunk不能使用fmt0
	// 将message切割成chunk放入chunk body中
	for i := 0; i < numOfChunk; i++ {
		headLen := calcHeader(header, prevHeader, out[index:])
		index += headLen

		if i != numOfChunk-1 {
			copy(out[index:], message[i*chunkSize:i*chunkSize+chunkSize])
			index += chunkSize
		} else {
			copy(out[index:], message[i*chunkSize:i*chunkSize+lastChunkSize])
			index += lastChunkSize
		}
		prevHeader = header
	}

	return out[:index]
}

// copyBufferFromBuffers
//
// TODO(chef): [refactor] move to naza 202206
// TODO(chef): [perf] impl me
func copyBufferFromBuffers(out []byte, bs net.Buffers, pos int, length int) {
}

func message2ChunksV(message net.Buffers, header *base.RtmpHeader, prevHeader *base.RtmpHeader, chunkSize int) []byte {
	var totalLen int
	for _, b := range message {
		totalLen += len(b)
	}

	// 计算chunk数量，最后一个chunk的大小
	numOfChunk := totalLen / chunkSize
	lastChunkSize := chunkSize
	if totalLen%chunkSize != 0 {
		numOfChunk++
		lastChunkSize = totalLen % chunkSize
	}

	maxNeededLen := (chunkSize + maxHeaderSize) * numOfChunk
	out := make([]byte, maxNeededLen)

	var index int

	// NOTICE 和srs交互时，发现srs要求message中的非第一个chunk不能使用fmt0
	// 将message切割成chunk放入chunk body中
	for i := 0; i < numOfChunk; i++ {
		headLen := calcHeader(header, prevHeader, out[index:])
		index += headLen

		if i != numOfChunk-1 {
			//copy(out[index:], message[i*chunkSize:i*chunkSize+chunkSize])
			copyBufferFromBuffers(out[index:], message, i*chunkSize, chunkSize)
			index += chunkSize
		} else {
			//copy(out[index:], message[i*chunkSize:i*chunkSize+lastChunkSize])
			copyBufferFromBuffers(out[index:], message, i*chunkSize, lastChunkSize)
			index += lastChunkSize
		}
		prevHeader = header
	}

	return out[:index]
}
