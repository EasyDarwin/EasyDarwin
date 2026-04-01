// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

import (
	"fmt"
	"github.com/q191201771/naza/pkg/nazabytes"
)

type RtpPacketListItem struct {
	Packet RtpPacket
	Next   *RtpPacketListItem
}

// RtpPacketList rtp packet的有序链表，前面的seq小于后面的seq
//
// 为什么不用红黑树等查找性能更高的有序kv结构？
// 第一，容器有最大值，这个数量级用啥容器都差不多，
// 第二，插入时，99.99%的seq号是当前最大号附近的，遍历找就可以了，
// 注意，这个链表并不是一个定长容器，当数据有序时，容器内缓存的数据是一个帧的数据。
type RtpPacketList struct {
	// TODO(chef): [refactor] 隐藏这两个变量的访问权限 202207
	Head RtpPacketListItem // 哨兵，自身不存放rtp包，第一个rtp包存在在head.next中
	Size int               // 实际元素个数

	doneSeqFlag bool   // 如果为false，则说明我们是初始化阶段，还不知道需要的packet的seq是多少
	doneSeq     uint16 // 已处理的seq号，之后我们需要seq+1的packet

	maxSize int
}

// IsStale 是否过期
func (l *RtpPacketList) IsStale(seq uint16) bool {
	if !l.doneSeqFlag {
		return false
	}

	// 序号太小
	return CompareSeq(seq, l.doneSeq) <= 0
}

// Insert 插入有序链表，并去重
func (l *RtpPacketList) Insert(pkt RtpPacket) {
	// 遍历查找插入位置
	p := &l.Head
	// TODO(chef): [perf] 考虑优化成从后往前查找，提高查找效率 202207
	for ; p.Next != nil; p = p.Next {
		res := CompareSeq(pkt.Header.Seq, p.Next.Packet.Header.Seq)
		switch res {
		case 0:
			// 包已经存在，不需要插入了
			return
		case 1:
			// noop
		case -1:
			item := &RtpPacketListItem{
				Packet: pkt,
				Next:   p.Next,
			}
			p.Next = item
			l.Size++
			return
		}
	}

	item := &RtpPacketListItem{
		Packet: pkt,
		Next:   p.Next,
	}
	p.Next = item
	l.Size++
	return
}

// PopFirst 弹出第一个包。注意，调用方保证容器不为空时调用
func (l *RtpPacketList) PopFirst() RtpPacket {
	pkt := l.Head.Next.Packet
	l.Head.Next = l.Head.Next.Next
	l.Size--
	return pkt
}

// PeekFirst 查看第一个包。注意，调用方保证容器不为空时调用
func (l *RtpPacketList) PeekFirst() RtpPacket {
	return l.Head.Next.Packet
}

// InitMaxSize 设置容器最大容量
func (l *RtpPacketList) InitMaxSize(maxSize int) {
	l.maxSize = maxSize
}

// Full 是否已经满了
func (l *RtpPacketList) Full() bool {
	return l.Size >= l.maxSize
}

// IsFirstSequential 第一个包是否是需要的（与之前已处理的是连续的）
func (l *RtpPacketList) IsFirstSequential() bool {
	first := l.Head.Next
	if first == nil {
		return false
	}

	if !l.doneSeqFlag {
		return true
	}

	return SubSeq(first.Packet.Header.Seq, l.doneSeq) == 1
}

// SetDoneSeq 设置已处理的包序号，比如已经成功合成了，或者主动丢弃到该位置结束丢弃了
func (l *RtpPacketList) SetDoneSeq(seq uint16) {
	l.doneSeqFlag = true
	l.doneSeq = seq
}

func (l *RtpPacketList) Reset() {
	l.doneSeqFlag = false
	l.doneSeq = 0
	l.Head.Next = nil
}

func (l *RtpPacketList) DebugString() string {
	p := l.Head.Next
	buf := nazabytes.NewBuffer(65535)
	buf.WriteString(fmt.Sprintf("size=%d, doneSeq=%d", l.Size, l.doneSeq))
	buf.WriteString(" [")
	for p != nil {
		buf.WriteString(fmt.Sprintf("%d ", p.Packet.Header.Seq))
		p = p.Next
	}
	buf.WriteString("]")
	return buf.String()
}
