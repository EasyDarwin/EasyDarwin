// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

type RtpUnpackContainer struct {
	unpackerProtocol IRtpUnpackerProtocol

	list RtpPacketList
}

func NewRtpUnpackContainer(maxSize int, unpackerProtocol IRtpUnpackerProtocol) *RtpUnpackContainer {
	p := &RtpUnpackContainer{
		unpackerProtocol: unpackerProtocol,
	}
	p.list.InitMaxSize(maxSize)

	return p
}

// Feed 输入收到的rtp包
func (r *RtpUnpackContainer) Feed(pkt RtpPacket) {
	// 过期的包
	if r.list.IsStale(pkt.Header.Seq) {
		return
	}

	// 计算位置
	r.unpackerProtocol.CalcPositionIfNeeded(&pkt)
	// 根据序号插入有序链表
	r.list.Insert(pkt)

	// 尽可能多的合成顺序的帧
	count := 0
	for {
		if !r.tryUnpackOneSequential() {
			break
		}
		count++
	}

	// 合成顺序的帧成功了，直接返回
	if count > 0 {
		return
	}

	// 缓存达到最大值
	if r.list.Full() {
		// 尝试合成一帧发生跳跃的帧
		packed := r.tryUnpackOne()

		if !packed {
			// 合成失败了，丢弃一包过期数据
			r.list.PopFirst()
			return
		}

		// 合成成功了，再次尝试，尽可能多的合成顺序的帧
		for {
			if !r.tryUnpackOneSequential() {
				break
			}
		}
	}
}

// tryUnpackOneSequential 从队列头部，尝试合成一个完整的帧。保证这次合成的帧的首个seq和上次合成帧的尾部seq是连续的
func (r *RtpUnpackContainer) tryUnpackOneSequential() bool {
	if !r.list.IsFirstSequential() {
		return false
	}

	return r.tryUnpackOne()
}

// tryUnpackOne 从队列头部，尝试合成一个完整的帧。不保证这次合成的帧的首个seq和上次合成帧的尾部seq是连续的
func (r *RtpUnpackContainer) tryUnpackOne() bool {
	unpackedFlag, unpackedSeq := r.unpackerProtocol.TryUnpackOne(&r.list)
	if unpackedFlag {
		r.list.SetDoneSeq(unpackedSeq)
	}
	return unpackedFlag
}
