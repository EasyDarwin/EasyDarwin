// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

type RtpPackerPayloadPcm struct {
}

func NewRtpPackerPayloadPcm() *RtpPackerPayloadPcm {
	return &RtpPackerPayloadPcm{}
}

func (r *RtpPackerPayloadPcm) Pack(in []byte, maxSize int) (out [][]byte) {
	if in == nil || maxSize <= 0 {
		return
	}

	if len(in) > maxSize {
		Log.Warnf("frame size bigger than rtp payload size while packing. len(in)=%d, maxSize=%d", len(in), maxSize)
	}

	item := make([]byte, len(in))
	copy(item, in)
	out = append(out, item)
	return
}
