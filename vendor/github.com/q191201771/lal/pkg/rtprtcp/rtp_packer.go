// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

import (
	"math/rand"
	"time"

	"github.com/q191201771/lal/pkg/base"
)

type RtpPacker struct {
	payloadPacker IRtpPackerPayload
	clockRate     int
	ssrc          uint32
	option        RtpPackerOption

	seq uint16
}

type RtpPackerOption struct {
	MaxPayloadSize int
	FirstSeq       uint16 // 初始seq，如果不设置，则随机产生
}

var defaultRtpPackerOption = RtpPackerOption{
	MaxPayloadSize: 1200, // TODO(chef) 这个值弄个更合适的
}

type ModRtpPackerOption func(option *RtpPackerOption)

func NewRtpPacker(payloadPacker IRtpPackerPayload, clockRate int, ssrc uint32, modOptions ...ModRtpPackerOption) *RtpPacker {
	option := defaultRtpPackerOption
	option.FirstSeq = uint16(rand.Int() % 65536)

	for _, fn := range modOptions {
		fn(&option)
	}

	return &RtpPacker{
		payloadPacker: payloadPacker,
		clockRate:     clockRate,
		ssrc:          ssrc,
		option:        option,
		seq:           option.FirstSeq,
	}
}

// Pack
//
// @param pkt:
//
// - pkt.Timestamp   绝对时间戳，单位毫秒。
// - pkt.PayloadType rtp包头中的packet type。
func (r *RtpPacker) Pack(pkt base.AvPacket) (out []RtpPacket) {
	payloads := r.payloadPacker.Pack(pkt.Payload, r.option.MaxPayloadSize)
	for i, payload := range payloads {
		h := MakeDefaultRtpHeader()
		if i == len(payloads)-1 {
			h.Mark = 1
		}
		h.PacketType = uint8(pkt.PayloadType)
		h.Seq = r.genSeq()
		h.Timestamp = uint32(float64(pkt.Timestamp) * float64(r.clockRate) / 1000)
		h.Ssrc = r.ssrc
		pkt := MakeRtpPacket(h, payload)
		out = append(out, pkt)
	}
	return
}

func (r *RtpPacker) genSeq() (ret uint16) {
	ret = r.seq
	r.seq++
	return
}

func init() {
	rand.Seed(time.Now().UnixNano())
}
