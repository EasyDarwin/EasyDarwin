// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

import "time"

// 通过收到的rtp包和rtcp sr包，产生rtcp rr包

type RrProducer struct {
	senderSsrc uint32
	mediaSsrc  uint32

	clockRate int

	maxSeq      int32
	baseSeq     int32
	cycles      uint32
	received    uint32
	extendedSeq uint32

	transit int64
	jitter  uint32

	expectedPrior uint32
	receivedPrior uint32
}

func NewRrProducer(clockRate int) *RrProducer {
	return &RrProducer{
		clockRate: clockRate,
		baseSeq:   -1,
		maxSeq:    -1,
		transit:   -1,
	}
}

// FeedRtpPacket 每次收到rtp包，都将seq序号传入这个函数
func (r *RrProducer) FeedRtpPacket(seq uint16) {
	r.received++

	if r.baseSeq == -1 {
		r.baseSeq = int32(seq)
	}

	if r.maxSeq == -1 {
		r.maxSeq = int32(seq)
	} else {
		if CompareSeq(seq, uint16(r.maxSeq)) > 0 {
			if seq < uint16(r.maxSeq) {
				r.cycles++
			}
			r.maxSeq = int32(seq)
		}
	}

	r.extendedSeq = (r.cycles << 16) | uint32(r.maxSeq)
}

// Produce 收到sr包时，产生rr包
//
// @param lsr: 从sr包中获取，见func SR.GetMiddleNtp
// @return:    rr包的二进制数据
func (r *RrProducer) Produce(lsr uint32) []byte {
	if r.baseSeq == -1 {
		return nil
	}

	var lost uint32
	expected := r.extendedSeq - uint32(r.baseSeq) + 1
	if expected < r.received {
		lost = 0
	} else {
		lost = expected - r.received
	}

	var fraction uint8
	expectedInterval := expected - r.expectedPrior
	r.expectedPrior = expected
	receivedInterval := r.received - r.receivedPrior
	r.receivedPrior = r.received
	lostInterval := expectedInterval - receivedInterval
	if expectedInterval == 0 || lostInterval <= 0 {
		fraction = 0
	} else {
		fraction = uint8((lostInterval << 8) / expectedInterval)
	}

	var rr Rr
	rr.senderSsrc = r.senderSsrc
	rr.mediaSsrc = r.mediaSsrc
	rr.fraction = fraction
	rr.lost = lost
	rr.cycles = uint16(r.cycles)
	rr.extendedSeq = r.extendedSeq
	rr.jitter = r.getJitter()
	rr.lsr = lsr

	return rr.Pack()
}

// @param timestamp 当前收到的rtp包头中的时间戳
func (r *RrProducer) updateJitter(timestamp uint32) {
	// rfc3550 6.4.1 SR: Sender Report RTCP Packet
	// rfc3550 A.8 Estimating the Interarrival Jitter

	// 当前收到rtp包的本地物理时间
	arrival := int64(time.Now().UnixNano() / 1e6)

	// 物理时间和包时间的差值，都换算成包时间戳格式
	transit := arrival*(int64(r.clockRate)/1000) - int64(timestamp)

	// 第一次跳过
	if r.transit == -1 {
		r.transit = transit
		return
	}

	// 这次差值，和上一次差值相减
	d := transit - r.transit
	if d < 0 {
		d = -d
	}

	// 一种设置jitter的方式
	// set: r.jitter += (float32(1)/16) * (d - r.jitter)
	// get: return r.jitter
	//
	// 另外一种方式
	// 对应的get: return r.jitter >> 4
	// 注意，右边的计算结果肯定是正数
	r.jitter = r.jitter + uint32(d) - ((r.jitter + 8) >> 4)
}

func (r *RrProducer) getJitter() uint32 {
	return r.jitter >> 4
}
