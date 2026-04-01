// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

import (
	"github.com/q191201771/lal/pkg/base"
)

type RtpUnpackerAac struct {
	payloadType base.AvPacketPt
	clockRate   int
	onAvPacket  OnAvPacket
}

func NewRtpUnpackerAac(payloadType base.AvPacketPt, clockRate int, onAvPacket OnAvPacket) *RtpUnpackerAac {
	return &RtpUnpackerAac{
		payloadType: payloadType,
		clockRate:   clockRate,
		onAvPacket:  onAvPacket,
	}
}

func (unpacker *RtpUnpackerAac) CalcPositionIfNeeded(pkt *RtpPacket) {
	// noop
}

func (unpacker *RtpUnpackerAac) TryUnpackOne(list *RtpPacketList) (unpackedFlag bool, unpackedSeq uint16) {
	// rfc3640 2.11.  Global Structure of Payload Format
	//
	// +---------+-----------+-----------+---------------+
	// | RTP     | AU Header | Auxiliary | Access Unit   |
	// | Header  | Section   | Section   | Data Section  |
	// +---------+-----------+-----------+---------------+
	//
	//           <----------RTP Packet Payload----------->
	//
	// rfc3640 3.2.1.  The AU Header Section
	//
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- .. -+-+-+-+-+-+-+-+-+-+
	// |AU-headers-length|AU-header|AU-header|      |AU-header|padding|
	// |                 |   (1)   |   (2)   |      |   (n)   | bits  |
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- .. -+-+-+-+-+-+-+-+-+-+
	//
	// rfc3640 3.3.6.  High Bit-rate AAC
	//
	// rtp_parse_mp4_au()
	//
	//
	// 3.2.3.1.  Fragmentation
	//
	//   A packet SHALL carry either one or more complete Access Units, or a
	//   single fragment of an Access Unit.  Fragments of the same Access Unit
	//   have the same time stamp but different RTP sequence numbers.  The
	//   marker bit in the RTP header is 1 on the last fragment of an Access
	//   Unit, and 0 on all other fragments.
	//

	p := list.Head.Next // first
	if p == nil {
		return false, 0
	}
	b := p.Packet.Body()

	aus := parseAu(b)

	// 只有一个描述
	if len(aus) == 1 {

		// 描述的音频帧完整的在当前的rtp packet中，没有跨越到下个rtp packet
		if aus[0].size <= uint32(len(b[aus[0].pos:])) {
			// one complete access unit
			var outPkt base.AvPacket
			outPkt.PayloadType = unpacker.payloadType
			outPkt.Timestamp = int64(p.Packet.Header.Timestamp / uint32(unpacker.clockRate/1000))
			outPkt.Payload = b[aus[0].pos : aus[0].pos+aus[0].size]
			unpacker.onAvPacket(outPkt)

			list.Head.Next = p.Next
			list.Size--
			return true, p.Packet.Header.Seq
		}

		// fragmented
		// 注意，这里我们参考size和rtp包头中的timestamp，不参考rtp包头中的mark位

		totalSize := aus[0].size
		timestamp := p.Packet.Header.Timestamp

		var as [][]byte
		as = append(as, b[aus[0].pos:])
		cacheSize := uint32(len(b[aus[0].pos:]))

		seq := p.Packet.Header.Seq
		p = p.Next
		packetCount := 0
		for {
			packetCount++
			if p == nil {
				return false, 0
			}
			if SubSeq(p.Packet.Header.Seq, seq) != 1 {
				return false, 0
			}
			if p.Packet.Header.Timestamp != timestamp {
				Log.Errorf("fragments of the same access shall have the same timestamp. first=%d, curr=%d",
					timestamp, p.Packet.Header.Timestamp)
				return false, 0
			}

			// 注意，非第一个fragment，也会包含au，au的size和第一个fragment里au的size应该相等
			b = p.Packet.Body()
			aus := parseAu(b)
			if len(aus) != 1 {
				Log.Errorf("shall be a single fragment. len(aus)=%d", len(aus))
				return false, 0
			}
			if aus[0].size != totalSize {
				Log.Errorf("fragments of the same access shall have the same size. first=%d, curr=%d",
					totalSize, aus[0].size)
				return false, 0
			}

			cacheSize += uint32(len(b[aus[0].pos:]))
			seq = p.Packet.Header.Seq
			as = append(as, b[aus[0].pos:])
			if cacheSize < totalSize {
				p = p.Next
			} else if cacheSize == totalSize {
				var outPkt base.AvPacket
				outPkt.PayloadType = unpacker.payloadType
				outPkt.Timestamp = int64(p.Packet.Header.Timestamp / uint32(unpacker.clockRate/1000))
				for _, a := range as {
					outPkt.Payload = append(outPkt.Payload, a...)
				}
				unpacker.onAvPacket(outPkt)

				list.Head.Next = p.Next
				list.Size -= packetCount
				return true, p.Packet.Header.Seq
			} else {
				Log.Errorf("cache size bigger then total size. cacheSize=%d, totalSize=%d",
					cacheSize, totalSize)
				return false, 0
			}
		}
		// can reach here
	}

	// more complete access unit
	for i := range aus {
		var outPkt base.AvPacket
		outPkt.PayloadType = unpacker.payloadType
		outPkt.Timestamp = int64(p.Packet.Header.Timestamp / uint32(unpacker.clockRate/1000))
		// TODO chef: 这里1024的含义
		outPkt.Timestamp += int64(uint32(i * (1024 * 1000) / unpacker.clockRate))
		outPkt.Payload = b[aus[i].pos : aus[i].pos+aus[i].size]
		unpacker.onAvPacket(outPkt)
	}

	list.Head.Next = p.Next
	list.Size--
	return true, p.Packet.Header.Seq
}

type au struct {
	size uint32 // 该音频帧的大小
	pos  uint32 // 相对rtp body的位置
}

func parseAu(b []byte) (ret []au) {
	// TODO(chef): [fix] 解析b时，没有判断长度有效性 202207

	// AU Header Section
	var auHeadersLength uint32
	auHeadersLength = uint32(b[0])<<8 + uint32(b[1])
	auHeadersLength = (auHeadersLength + 7) / 8

	// TODO chef: 这里的2是写死的，正常是外部传入auSize和auIndex所占位数的和
	const auHeaderSize = 2
	nbAuHeaders := uint32(auHeadersLength) / auHeaderSize // 有多少个AU-Header

	pauh := uint32(2)                  // AU Header pos
	pau := uint32(2) + auHeadersLength // AU pos

	for i := uint32(0); i < nbAuHeaders; i++ {
		// TODO chef: auSize和auIndex所在的位数是写死的13bit，3bit，标准的做法应该从外部传入，比如从sdp中获取后传入
		auSize := uint32(b[pauh])<<8 | uint32(b[pauh+1]&0xF8) // 13bit
		auSize /= 8
		// 注意，fragment时，auIndex并不可靠。见TestAacCase1
		//auIndex := b[pauh+1] & 0x7
		//Log.Debugf("~ %d %d", auSize, auIndex)

		ret = append(ret, au{
			size: auSize,
			pos:  pau,
		})

		pauh += 2
		pau += auSize
	}

	if (nbAuHeaders > 1 && pau != uint32(len(b))) ||
		(nbAuHeaders == 1 && pau < uint32(len(b))) {
		Log.Warnf("rtp packet size invalid. nbAuHeaders=%d, pau=%d, len(b)=%d, auHeadersLength=%d", nbAuHeaders, pau, len(b), auHeadersLength)
	}

	return
}
