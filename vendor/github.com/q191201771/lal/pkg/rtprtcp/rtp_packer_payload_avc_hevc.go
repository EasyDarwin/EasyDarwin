// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

import (
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/hevc"
)

type RtpPackerPayloadAvcHevcType int

const (
	RtpPackerPayloadAvcHevcTypeNalu   RtpPackerPayloadAvcHevcType = 1
	RtpPackerPayloadAvcHevcTypeAvcc                               = 2
	RtpPackerPayloadAvcHevcTypeAnnexb                             = 3
)

type RtpPackerPayloadAvcHevcOption struct {
	Typ RtpPackerPayloadAvcHevcType
}

var defaultRtpPackerPayloadAvcHevcOption = RtpPackerPayloadAvcHevcOption{
	Typ: RtpPackerPayloadAvcHevcTypeNalu,
}

type RtpPackerPayloadAvcHevc struct {
	payloadType base.AvPacketPt
	option      RtpPackerPayloadAvcHevcOption
}

type ModRtpPackerPayloadAvcHevcOption func(option *RtpPackerPayloadAvcHevcOption)

func NewRtpPackerPayloadAvc(modOptions ...ModRtpPackerPayloadAvcHevcOption) *RtpPackerPayloadAvcHevc {
	return NewRtpPackerPayloadAvcHevc(base.AvPacketPtAvc, modOptions...)
}

func NewRtpPackerPayloadHevc(modOptions ...ModRtpPackerPayloadAvcHevcOption) *RtpPackerPayloadAvcHevc {
	return NewRtpPackerPayloadAvcHevc(base.AvPacketPtHevc, modOptions...)
}

func NewRtpPackerPayloadAvcHevc(payloadType base.AvPacketPt, modOptions ...ModRtpPackerPayloadAvcHevcOption) *RtpPackerPayloadAvcHevc {
	option := defaultRtpPackerPayloadAvcHevcOption
	for _, fn := range modOptions {
		fn(&option)
	}
	return &RtpPackerPayloadAvcHevc{
		payloadType: payloadType,
		option:      option,
	}
}

// Pack @param in: AVCC格式
//
// @return out: 内存块为独立新申请；函数返回后，内部不再持有该内存块
func (r *RtpPackerPayloadAvcHevc) Pack(in []byte, maxSize int) (out [][]byte) {
	if in == nil || maxSize <= 0 {
		return
	}

	var splitFn func([]byte) ([][]byte, error)

	switch r.option.Typ {
	case RtpPackerPayloadAvcHevcTypeAvcc:
		splitFn = avc.SplitNaluAvcc
	case RtpPackerPayloadAvcHevcTypeAnnexb:
		splitFn = avc.SplitNaluAnnexb
	default:
		// RtpPackerPayloadAvcHevcTypeNalu
		return r.PackNal(in, maxSize)
	}

	nals, err := splitFn(in)
	if err != nil {
		return
	}

	for _, nal := range nals {
		if r.payloadType == base.AvPacketPtAvc {
			if avc.ParseNaluType(nal[0]) == avc.NaluTypeAud {
				continue
			}
		} else {
			if hevc.ParseNaluType(nal[0]) == hevc.NaluTypeAud {
				continue
			}
		}

		out = append(out, r.PackNal(nal, maxSize)...)
	}
	return
}

func (r *RtpPackerPayloadAvcHevc) PackNal(nal []byte, maxSize int) (out [][]byte) {
	// pack逻辑
	//
	// avc
	//
	// 输入
	// nri     [01, 02]
	// nalType [03, 07]
	//
	// 输出
	// nri     [01, 02]
	// 28      [03, 07]    28是avc fua的nal type
	// start   [10]
	// end     [11]
	// nalType [13, 17]
	//
	// hevc
	//
	// 输入
	// nalType [01, 06]
	//
	// 输出
	// 49      [01, 06] 49是hevc fua的nal type
	// 1       [10, 17]
	// start   [20]
	// end     [21]
	// nalType [22, 27] 注意，和输入的nalType的所在type字节的位位置不同
	//

	// single
	if len(nal) <= maxSize {
		item := make([]byte, len(nal))
		copy(item, nal)
		out = append(out, item)
		return
	}

	// FU-A

	// var
	var pktLen int // rtp payload大小
	var bpos int
	isFirstFlag := true

	// const after set
	var headerSize int
	var sepos int // start-end标志所在位置
	var nalType uint8
	var nri uint8 // only avc
	epos := len(nal)

	if r.payloadType == base.AvPacketPtAvc {
		// 注意，跳过输入的nal type那个字节，使用FU-A自己的两个字节的头，避免重复
		bpos = 1
		sepos = 1

		headerSize = 2
		nalType = nal[0] & 0x1F
		nri = nal[0] & 0x60
	} else {
		bpos = 2
		sepos = 2

		headerSize = 3
		nalType = (nal[0] >> 1) & 0x3F
	}

	for {
		if epos-bpos > maxSize-headerSize {
			// 前面的包
			pktLen = maxSize
			item := make([]byte, pktLen)

			if r.payloadType == base.AvPacketPtAvc {
				item[0] = NaluTypeAvcFua | nri
				item[1] = nalType
			} else {
				item[0] = NaluTypeHevcFua << 1
				item[1] = 1 // ffmpeg, rtpenc_h264_hevc.c, func nal_send
				item[2] = nalType
			}

			// 当前帧切割后的首个RTP包
			if isFirstFlag {
				item[sepos] |= 0x80 // start
				isFirstFlag = false
			}

			//
			copy(item[headerSize:], nal[bpos:bpos+maxSize-headerSize])
			out = append(out, item)
			bpos += maxSize - headerSize
			continue
		}

		// 最后一包
		pktLen = epos - bpos + headerSize
		item := make([]byte, pktLen)

		if r.payloadType == base.AvPacketPtAvc {
			item[0] = NaluTypeAvcFua | nri
			item[1] = nalType | 0x40 // end
		} else {
			item[0] = NaluTypeHevcFua << 1
			item[1] = 1
			item[2] = nalType | 0x40
		}

		copy(item[headerSize:], nal[bpos:])
		out = append(out, item)
		break
	}
	return
}
