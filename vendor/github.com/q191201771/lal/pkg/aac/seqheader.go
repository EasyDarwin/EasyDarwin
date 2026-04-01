// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package aac

import (
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazaerrors"

	"github.com/q191201771/naza/pkg/nazabits"
)

// TODO(chef) 这个文件的部分内容可以考虑放到package base中

// SequenceHeaderContext
//
// <spec-video_file_format_spec_v10.pdf>, <Audio tags, AUDIODATA>, <page 10/48>
// ----------------------------------------------------------------------------
// soundFormat    [4b] 10=AAC
// soundRate      [2b] 3=44kHz. AAC always 3
// soundSize      [1b] 0=snd8Bit, 1=snd16Bit
// soundType      [1b] 0=sndMono, 1=sndStereo. AAC always 1
// aacPackageType [8b] 0=seq header, 1=AAC raw
type SequenceHeaderContext struct {
	SoundFormat   uint8 // [4b]
	SoundRate     uint8 // [2b]
	SoundSize     uint8 // [1b]
	SoundType     uint8 // [1b]
	AacPacketType uint8 // [8b]
}

// Unpack
//
// @param b: rtmp/flv的message/tag的payload的前2个字节。
//
//	函数调用结束后，内部不持有该内存块
func (shCtx *SequenceHeaderContext) Unpack(b []byte) {
	br := nazabits.NewBitReader(b)
	shCtx.SoundFormat, _ = br.ReadBits8(4)
	shCtx.SoundRate, _ = br.ReadBits8(2)
	shCtx.SoundSize, _ = br.ReadBits8(1)
	shCtx.SoundType, _ = br.ReadBits8(1)
	shCtx.AacPacketType, _ = br.ReadBits8(8)
}

// MakeAudioDataSeqHeaderWithAsc
//
// @param asc: 函数调用结束后，内部不持有该内存块
//
// @return out: 内存块为独立新申请；函数调用结束后，内部不持有该内存块
func MakeAudioDataSeqHeaderWithAsc(asc []byte) (out []byte, err error) {
	if len(asc) < minAscLength {
		return nil, nazaerrors.Wrap(base.ErrShortBuffer)
	}

	// 注意，前两个字节是SequenceHeaderContext，后面跟着asc
	out = make([]byte, 2+len(asc))
	out[0] = 0xaf
	out[1] = 0
	copy(out[2:], asc)
	return
}

// MakeAudioDataSeqHeaderWithAdtsHeader
//
// @param adtsHeader: 函数调用结束后，内部不持有该内存块
//
// @return out: 内存块为独立新申请；函数调用结束后，内部不持有该内存块
func MakeAudioDataSeqHeaderWithAdtsHeader(adtsHeader []byte) (out []byte, err error) {
	var asc []byte
	if asc, err = MakeAscWithAdtsHeader(adtsHeader); err != nil {
		return nil, err
	}
	return MakeAudioDataSeqHeaderWithAsc(asc)
}
