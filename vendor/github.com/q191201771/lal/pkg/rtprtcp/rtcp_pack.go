// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

import "github.com/q191201771/naza/pkg/bele"

type Rr struct {
	senderSsrc  uint32
	mediaSsrc   uint32
	fraction    uint8
	lost        uint32
	cycles      uint16
	extendedSeq uint32
	jitter      uint32
	lsr         uint32
	dlsr        uint32 // default 0
}

func (r *Rr) Pack() []byte {
	const lenInWords = 8

	b := make([]byte, lenInWords*4)

	var h RtcpHeader
	h.Version = RtcpVersion
	h.Padding = 0
	h.CountOrFormat = 1
	h.PacketType = RtcpPacketTypeRr
	h.Length = lenInWords - 1
	h.PackTo(b)

	bele.BePutUint32(b[4:], r.senderSsrc)
	bele.BePutUint32(b[8:], r.mediaSsrc)
	b[12] = r.fraction
	// TODO chef: lost的表示是否正确
	bele.BePutUint24(b[13:], r.lost>>8)
	bele.BePutUint16(b[16:], r.cycles)
	bele.BePutUint32(b[18:], r.extendedSeq)
	bele.BePutUint32(b[20:], r.jitter)
	bele.BePutUint32(b[24:], r.lsr)
	bele.BePutUint32(b[28:], 0)

	return b
}
