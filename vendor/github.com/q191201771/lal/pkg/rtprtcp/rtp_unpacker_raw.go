// Copyright 2023, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

import "github.com/q191201771/lal/pkg/base"

type RtpUnpackerRaw struct {
	payloadType base.AvPacketPt
	clockRate   int
	onAvPacket  OnAvPacket
}

func NewRtpUnpackerRaw(payloadType base.AvPacketPt, clockRate int, onAvPacket OnAvPacket) *RtpUnpackerRaw {
	return &RtpUnpackerRaw{
		payloadType: payloadType,
		clockRate:   clockRate,
		onAvPacket:  onAvPacket,
	}
}

func (unpacker *RtpUnpackerRaw) CalcPositionIfNeeded(pkt *RtpPacket) {
	// noop
}

func (unpacker *RtpUnpackerRaw) TryUnpackOne(list *RtpPacketList) (unpackedFlag bool, unpackedSeq uint16) {
	p := list.Head.Next // first
	if p == nil {
		return false, 0
	}

	// 暂时认为一个rtp为一帧数据(G711A/G711U)
	b := p.Packet.Body()
	var outPkt base.AvPacket
	outPkt.PayloadType = unpacker.payloadType
	outPkt.Timestamp = int64(p.Packet.Header.Timestamp / uint32(unpacker.clockRate/1000))
	outPkt.Payload = b
	unpacker.onAvPacket(outPkt)

	list.Head.Next = p.Next
	list.Size--
	return true, p.Packet.Header.Seq
}
