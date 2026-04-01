// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

type IRtpPackerPayload interface {
	// Pack @param maxSize: rtp payload包体部分（不含包头）的最大大小
	//
	Pack(in []byte, maxSize int) (out [][]byte)
}

var _ IRtpPackerPayload = &RtpPackerPayloadAvcHevc{}
