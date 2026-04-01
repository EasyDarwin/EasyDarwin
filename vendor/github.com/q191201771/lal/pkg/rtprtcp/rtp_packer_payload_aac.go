// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

type RtpPackerPayloadAac struct {
}

func NewRtpPackerPayloadAac() *RtpPackerPayloadAac {
	return &RtpPackerPayloadAac{}
}

func (r *RtpPackerPayloadAac) Pack(in []byte, maxSize int) (out [][]byte) {
	if in == nil || maxSize <= 0 {
		return
	}
	// TODO(chef): 目前只支持一帧打成一个rtp包，不支持多帧打成一个rtp包，也不支持一帧跨越两个rtp包

	// 协议方面可以参考RtpUnpackerAac那边的代码和注释
	//
	// 简单来说，aac的rtp包分为三个部分，
	// 第一部分描述了au头的个数，
	// 第二部分是au头的数组，每个au头固定两字节，au头可以解析出每帧的大小
	// 第三部分是帧数据的数组

	if len(in) > maxSize {
		Log.Warnf("frame size bigger than rtp payload size while packing. len(in)=%d, maxSize=%d", len(in), maxSize)
	}

	auHeadersLength := 2 // auHeaderSize * nbAuHeaders = 2 * 1

	item := make([]byte, 4+len(in))
	item[0] = 0
	item[1] = uint8(auHeadersLength * 8)
	item[2] = uint8(len(in) >> 5)
	item[3] = uint8((len(in) & 0x1F) << 3)
	copy(item[4:], in)
	out = append(out, item)
	return
}
