// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

import (
	"encoding/hex"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/hevc"
	"github.com/q191201771/naza/pkg/bele"
	"github.com/q191201771/naza/pkg/nazabytes"
)

type RtpUnpackerAvcHevc struct {
	payloadType base.AvPacketPt
	clockRate   int
	onAvPacket  OnAvPacket
}

func NewRtpUnpackerAvcHevc(payloadType base.AvPacketPt, clockRate int, onAvPacket OnAvPacket) *RtpUnpackerAvcHevc {
	return &RtpUnpackerAvcHevc{
		payloadType: payloadType,
		clockRate:   clockRate,
		onAvPacket:  onAvPacket,
	}
}

func (unpacker *RtpUnpackerAvcHevc) CalcPositionIfNeeded(pkt *RtpPacket) {
	switch unpacker.payloadType {
	case base.AvPacketPtAvc:
		calcPositionIfNeededAvc(pkt)
	case base.AvPacketPtHevc:
		calcPositionIfNeededHevc(pkt)
	}
}

func (unpacker *RtpUnpackerAvcHevc) TryUnpackOne(list *RtpPacketList) (unpackedFlag bool, unpackedSeq uint16) {
	first := list.Head.Next
	if first == nil {
		return false, 0
	}

	switch first.Packet.positionType {
	case PositionTypeSingle:
		var pkt base.AvPacket
		pkt.PayloadType = unpacker.payloadType
		pkt.Timestamp = int64(first.Packet.Header.Timestamp / uint32(unpacker.clockRate/1000))

		pkt.Payload = make([]byte, len(first.Packet.Body())+4)
		bele.BePutUint32(pkt.Payload, uint32(len(first.Packet.Body())))
		copy(pkt.Payload[4:], first.Packet.Body())

		list.Head.Next = first.Next
		list.Size--
		unpacker.onAvPacket(pkt)
		return true, first.Packet.Header.Seq

	case PositionTypeStapa, PositionTypeAp:
		skip := uint32(1)
		if first.Packet.positionType == PositionTypeStapa {
			skip = 1
		} else if first.Packet.positionType == PositionTypeAp {
			skip = 2
		}

		var pkt base.AvPacket
		pkt.PayloadType = unpacker.payloadType
		pkt.Timestamp = int64(first.Packet.Header.Timestamp / uint32(unpacker.clockRate/1000))

		// 跳过前面的字节，并且将多nalu前的2字节长度，替换成4字节长度
		// skip后：
		// rtp中的数据格式 [<2字节的nalu长度>, <nalu>, <2字节的nalu长度>, <nalu> ...]
		// 转变后的数据格式 [<4字节的nalu长度>, <nalu>, <4字节的nalu长度>, <nalu> ...]
		buf := first.Packet.Body()[skip:]

		// 使用两次遍历，第一次遍历找出总大小，第二次逐个拷贝，目的是使得内存块一次就申请好，不用动态扩容造成额外性能开销
		totalSize := 0
		for i := 0; i != len(buf); {
			if len(buf)-i < 2 {
				Log.Errorf("[%p] invalid STAP-A packet. len(buf)=%d, i=%d", unpacker, len(buf), i)
				return false, 0
			}
			naluSize := int(bele.BeUint16(buf[i:]))
			totalSize += 4 + naluSize
			i += 2 + naluSize
		}

		pkt.Payload = make([]byte, totalSize)
		j := 0
		for i := 0; i != len(buf); {
			naluSize := int(bele.BeUint16(buf[i:]))
			bele.BePutUint32(pkt.Payload[j:], uint32(naluSize))
			copy(pkt.Payload[j+4:], buf[i+2:i+2+naluSize])
			j += 4 + naluSize
			i += 2 + naluSize
		}

		list.Head.Next = first.Next
		list.Size--
		unpacker.onAvPacket(pkt)

		return true, first.Packet.Header.Seq

	case PositionTypeFuaStart:
		prev := first
		p := first.Next
		for {
			if prev == nil || p == nil {
				return false, 0
			}
			if SubSeq(p.Packet.Header.Seq, prev.Packet.Header.Seq) != 1 {
				return false, 0
			}

			if p.Packet.positionType == PositionTypeFuaMiddle {
				prev = p
				p = p.Next
				continue
			} else if p.Packet.positionType == PositionTypeFuaEnd {
				var pkt base.AvPacket
				pkt.PayloadType = unpacker.payloadType
				pkt.Timestamp = int64(p.Packet.Header.Timestamp / uint32(unpacker.clockRate/1000))

				var naluTypeLen int
				var naluType []byte
				if unpacker.payloadType == base.AvPacketPtAvc {
					naluTypeLen = 1
					naluType = make([]byte, naluTypeLen)

					fuIndicator := first.Packet.Body()[0]
					fuHeader := first.Packet.Body()[1]
					naluType[0] = (fuIndicator & 0xE0) | (fuHeader & 0x1F)
				} else {
					naluTypeLen = 2
					naluType = make([]byte, naluTypeLen)

					buf := first.Packet.Body()
					fuType := buf[2] & 0x3f
					// ffmpeg rtpdec_hevc.c
					// 取buf[0]的头尾各1位
					naluType[0] = (buf[0] & 0x81) | (fuType << 1)
					naluType[1] = buf[1]
				}

				// 使用两次遍历，第一次遍历找出总大小，第二次逐个拷贝，目的是使得内存块一次就申请好，不用动态扩容造成额外性能开销
				totalSize := 0
				pp := first
				for {
					// naluTypeLen表示的是合帧之后的nalu type的长度。而+1，是在rtp时头的长度。
					// 比如h264，帧数据时naluTypeLen是1字节，rtp包时长度是2字节。
					totalSize += len(pp.Packet.Body()) - (naluTypeLen + 1)
					if pp == p {
						break
					}
					pp = pp.Next
				}

				// 三部分组成： len + type + data
				pkt.Payload = make([]byte, 4+naluTypeLen+totalSize)
				// len
				bele.BePutUint32(pkt.Payload, uint32(totalSize+naluTypeLen))
				// type
				var index int
				if unpacker.payloadType == base.AvPacketPtAvc {
					pkt.Payload[4] = naluType[0]
					index = 5
				} else {
					pkt.Payload[4] = naluType[0]
					pkt.Payload[5] = naluType[1]
					index = 6
				}
				// data
				packetCount := 0
				pp = first
				for {
					copy(pkt.Payload[index:], pp.Packet.Body()[naluTypeLen+1:])
					index += len(pp.Packet.Body()) - (naluTypeLen + 1)
					packetCount++

					if pp == p {
						break
					}
					pp = pp.Next
				}

				list.Head.Next = p.Next
				list.Size -= packetCount
				unpacker.onAvPacket(pkt)

				return true, p.Packet.Header.Seq
			} else {
				// 不应该出现其他类型
				Log.Errorf("[%p] invalid position type. position=%d, first=(h=%+v, pos=%d), prev=(h=%+v, pos=%d), p=(h=%+v, pos=%d)",
					unpacker, p.Packet.positionType, first.Packet.Header, first.Packet.positionType, prev.Packet.Header, prev.Packet.positionType, p.Packet.Header, p.Packet.positionType)
				return false, 0
			}
		}

	case PositionTypeFuaMiddle:
		// noop
	case PositionTypeFuaEnd:
		// noop
	default:
		Log.Errorf("invalid position. pos=%d", first.Packet.positionType)
	}

	return false, 0
}

func calcPositionIfNeededAvc(pkt *RtpPacket) {
	b := pkt.Body()

	// rfc3984 5.3.  NAL Unit Octet Usage
	//
	// +---------------+
	// |0|1|2|3|4|5|6|7|
	// +-+-+-+-+-+-+-+-+
	// |F|NRI|  Type   |
	// +---------------+

	outerNaluType := avc.ParseNaluType(b[0])
	if outerNaluType <= NaluTypeAvcSingleMax {
		pkt.positionType = PositionTypeSingle
		return
	} else if outerNaluType == NaluTypeAvcFua {

		// rfc3984 5.8.  Fragmentation Units (FUs)
		//
		// 0                   1                   2                   3
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		// | FU indicator  |   FU header   |                               |
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
		// |                                                               |
		// |                         FU payload                            |
		// |                                                               |
		// |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		// |                               :...OPTIONAL RTP padding        |
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		//
		// FU indicator:
		// +---------------+
		// |0|1|2|3|4|5|6|7|
		// +-+-+-+-+-+-+-+-+
		// |F|NRI|  Type   |
		// +---------------+
		//
		// Fu header:
		// +---------------+
		// |0|1|2|3|4|5|6|7|
		// +-+-+-+-+-+-+-+-+
		// |S|E|R|  Type   |
		// +---------------+

		fuIndicator := b[0]
		_ = fuIndicator
		fuHeader := b[1]

		startCode := (fuHeader & 0x80) != 0
		endCode := (fuHeader & 0x40) != 0

		if startCode {
			pkt.positionType = PositionTypeFuaStart
			return
		}

		if endCode {
			pkt.positionType = PositionTypeFuaEnd
			return
		}

		pkt.positionType = PositionTypeFuaMiddle
		return
	} else if outerNaluType == NaluTypeAvcStapa {
		pkt.positionType = PositionTypeStapa
	} else {
		Log.Errorf("unknown nalu type. outerNaluType=%d(%d), header=%+v, len=%d, raw=%s",
			b[0], outerNaluType, pkt.Header, len(pkt.Raw), hex.Dump(nazabytes.Prefix(pkt.Raw, 128)))
	}

	return
}

func calcPositionIfNeededHevc(pkt *RtpPacket) {
	b := pkt.Body()

	// +---------------+---------------+
	// |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	// |F|   Type    |  LayerId  | TID |
	// +-------------+-----------------+

	outerNaluType := hevc.ParseNaluType(b[0])
	if _, ok := hevc.NaluTypeMapping[outerNaluType]; ok {
		pkt.positionType = PositionTypeSingle
		return
	}

	if outerNaluType == NaluTypeHevcFua {
		// Figure 1: The Structure of the HEVC NAL Unit Header

		// 0                   1                   2                   3
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		// |    PayloadHdr (Type=49)       |   FU header   | DONL (cond)   |
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
		// | DONL (cond)   |                                               |
		// |-+-+-+-+-+-+-+-+                                               |
		// |                         FU payload                            |
		// |                                                               |
		// |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		// |                               :...OPTIONAL RTP padding        |
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

		// Figure 9: The Structure of an FU

		// +---------------+
		// |0|1|2|3|4|5|6|7|
		// +-+-+-+-+-+-+-+-+
		// |S|E|  FuType   |
		// +---------------+

		// Figure 10: The Structure of FU Header

		startCode := (b[2] & 0x80) != 0
		endCode := (b[2] & 0x40) != 0

		if startCode {
			pkt.positionType = PositionTypeFuaStart
			return
		}

		if endCode {
			pkt.positionType = PositionTypeFuaEnd
			return
		}

		pkt.positionType = PositionTypeFuaMiddle
		return
	} else if outerNaluType == NaluTypeHevcAp {
		pkt.positionType = PositionTypeAp
		return
	}

	Log.Errorf("unknown nalu type. outerNaluType=%d(%d), header=%+v, len=%d, raw=%s",
		b[0], outerNaluType, pkt.Header, len(pkt.Raw), hex.Dump(nazabytes.Prefix(pkt.Raw, 128)))
}
