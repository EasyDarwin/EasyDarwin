// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtsp

import (
	"bufio"
	"io"

	"github.com/q191201771/naza/pkg/bele"
)

// rfc2326 10.12 Embedded (Interleaved) Binary Data

func readInterleaved(r *bufio.Reader) (isInterleaved bool, packet []byte, channel uint8, err error) {
	flag, err := r.ReadByte()
	if err != nil {
		return false, nil, 0, err
	}

	if flag != Interleaved {
		_ = r.UnreadByte()
		return false, nil, 0, nil
	}

	channel, err = r.ReadByte()
	if err != nil {
		return false, nil, 0, err
	}
	rtpLenBuf := make([]byte, 2)
	_, err = io.ReadFull(r, rtpLenBuf)
	if err != nil {
		return false, nil, 0, err
	}
	rtpLen := int(bele.BeUint16(rtpLenBuf))
	// TODO chef: 这里为了安全性，应该检查大小
	rtpBuf := make([]byte, rtpLen)
	_, err = io.ReadFull(r, rtpBuf)
	if err != nil {
		return false, nil, 0, err
	}

	return true, rtpBuf, channel, nil
}

func packInterleaved(channel int, rtpPacket []byte) []byte {
	ret := make([]byte, 4+len(rtpPacket))
	ret[0] = Interleaved
	ret[1] = uint8(channel)
	bele.BePutUint16(ret[2:], uint16(len(rtpPacket)))
	copy(ret[4:], rtpPacket)
	return ret
}
