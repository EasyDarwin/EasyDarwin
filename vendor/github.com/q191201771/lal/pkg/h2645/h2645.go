// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package h2645

import (
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/hevc"
	"github.com/q191201771/naza/pkg/bele"
)

// TODO(chef): 逐渐将package avc, hevc迁移到package h2645，这个package处于开发阶段，如果内容不全，请使用package avc, hevc

// 无特殊说明的函数则同时支持h264和h265两种格式

var (
	NaluStartCode4 = []byte{0x0, 0x0, 0x0, 0x1}
)

const (
	H264NaluTypeSlice    uint8 = 1
	H264NaluTypeIdrSlice uint8 = 5
	H264NaluTypeSei      uint8 = 6
	H264NaluTypeSps      uint8 = 7
	H264NaluTypePps      uint8 = 8
	H264NaluTypeAud      uint8 = 9  // Access Unit Delimiter
	H264NaluTypeFd       uint8 = 12 // Filler Data
)

// ISO_IEC_23008-2_2013.pdf
// Table 7-1 – NAL unit type codes and NAL unit type classes
const (
	H265NaluTypeSliceTrailN uint8 = 0 // 0x0
	H265NaluTypeSliceTrailR uint8 = 1 // 0x01
	H265NaluTypeSliceTsaN   uint8 = 2 // 0x02
	H265NaluTypeSliceTsaR   uint8 = 3 // 0x03
	H265NaluTypeSliceStsaN  uint8 = 4 // 0x04
	H265NaluTypeSliceStsaR  uint8 = 5 // 0x05
	H265NaluTypeSliceRadlN  uint8 = 6 // 0x06
	H265NaluTypeSliceRadlR  uint8 = 7 // 0x07
	H265NaluTypeSliceRaslN  uint8 = 8 // 0x06
	H265NaluTypeSliceRaslR  uint8 = 9 // 0x09

	H265NaluTypeSliceBlaWlp       uint8 = 16 // 0x10
	H265NaluTypeSliceBlaWradl     uint8 = 17 // 0x11
	H265NaluTypeSliceBlaNlp       uint8 = 18 // 0x12
	H265NaluTypeSliceIdr          uint8 = 19 // 0x13
	H265NaluTypeSliceIdrNlp       uint8 = 20 // 0x14
	H265NaluTypeSliceCranut       uint8 = 21 // 0x15
	H265NaluTypeSliceRsvIrapVcl22 uint8 = 22 // 0x16
	H265NaluTypeSliceRsvIrapVcl23 uint8 = 23 // 0x17

	H265NaluTypeVps       uint8 = 32 // 0x20
	H265NaluTypeSps       uint8 = 33 // 0x21
	H265NaluTypePps       uint8 = 34 // 0x22
	H265NaluTypeAud       uint8 = 35 // 0x23
	H265NaluTypeSei       uint8 = 39 // 0x27
	H265NaluTypeSeiSuffix uint8 = 40 // 0x28
)

// IterateNaluAvcc 遍历Avcc格式的nalu流
func IterateNaluAvcc(nals []byte, handler func(nal []byte)) error {
	return avc.IterateNaluAvcc(nals, handler)
}

func IterateNaluStartCode(nalu []byte, start int) (pos, length int) {
	return avc.IterateNaluStartCode(nalu, start)
}

func ParseNaluType(isH264 bool, v uint8) uint8 {
	if isH264 {
		return avc.ParseNaluType(v)
	}
	return hevc.ParseNaluType(v)
}

func SeqHeader2Annexb(isH264 bool, payload []byte) ([]byte, error) {
	if isH264 {
		return avc.SpsPpsSeqHeader2Annexb(payload)
	}
	return hevc.VpsSpsPpsSeqHeader2Annexb(payload)
}

func H265IsIrapNalu(typ uint8) bool {
	return hevc.IsIrapNalu(typ)
}

func JoinNaluAvcc(naluList ...[]byte) []byte {
	n := len(naluList)
	if n == 0 {
		return nil
	}
	n *= 4
	for _, item := range naluList {
		n += len(item)
	}
	ret := make([]byte, n)

	pos := 0
	for _, item := range naluList {
		bele.BePutUint32(ret[pos:], uint32(len(item)))
		pos += 4
		copy(ret[pos:], item)
		pos += len(item)
	}

	return ret
}
