// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package hevc

import (
	"bytes"

	"github.com/q191201771/naza/pkg/nazaerrors"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/naza/pkg/nazabits"

	"github.com/q191201771/naza/pkg/bele"
)

// HVCC
//
// ISO_IEC_23008-2_2013.pdf

// NAL Unit Header
//
// +---------------+---------------+
// |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |F|   Type    |  LayerId  | TID |
// +-------------+-----------------+

var (
	NaluStartCode4 = []byte{0x0, 0x0, 0x0, 0x1}

	// AudNalu aud nalu
	AudNalu = []byte{0x00, 0x00, 0x00, 0x01, 0x46, 0x01, 0x10}
)

var NaluTypeMapping = map[uint8]string{
	NaluTypeSliceTrailN: "TrailN",
	NaluTypeSliceTrailR: "TrailR",
	NaluTypeSliceTsaN:   "TsaN",
	NaluTypeSliceTsaR:   "TsaR",
	NaluTypeSliceStsaN:  "StsaN",
	NaluTypeSliceStsaR:  "StsaR",
	NaluTypeSliceRadlN:  "RadlN",
	NaluTypeSliceRadlR:  "RadlR",
	NaluTypeSliceRaslN:  "RaslN",
	NaluTypeSliceRaslR:  "RaslR",

	NaluTypeSliceBlaWlp:       "BlaWlp",
	NaluTypeSliceBlaWradl:     "BlaWradl",
	NaluTypeSliceBlaNlp:       "BlaNlp",
	NaluTypeSliceIdr:          "IDR",
	NaluTypeSliceIdrNlp:       "IDRNLP",
	NaluTypeSliceCranut:       "CRANUT",
	NaluTypeSliceRsvIrapVcl22: "IrapVcl22",
	NaluTypeSliceRsvIrapVcl23: "IrapVcl23",

	NaluTypeVps:       "VPS",
	NaluTypeSps:       "SPS",
	NaluTypePps:       "PPS",
	NaluTypeAud:       "AUD",
	NaluTypeSei:       "SEI",
	NaluTypeSeiSuffix: "SEISuffix",
}

// ISO_IEC_23008-2_2013.pdf
// Table 7-1 – NAL unit type codes and NAL unit type classes
const (
	NaluTypeSliceTrailN uint8 = 0 // 0x0
	NaluTypeSliceTrailR uint8 = 1 // 0x01
	NaluTypeSliceTsaN   uint8 = 2 // 0x02
	NaluTypeSliceTsaR   uint8 = 3 // 0x03
	NaluTypeSliceStsaN  uint8 = 4 // 0x04
	NaluTypeSliceStsaR  uint8 = 5 // 0x05
	NaluTypeSliceRadlN  uint8 = 6 // 0x06
	NaluTypeSliceRadlR  uint8 = 7 // 0x07
	NaluTypeSliceRaslN  uint8 = 8 // 0x06
	NaluTypeSliceRaslR  uint8 = 9 // 0x09

	NaluTypeSliceBlaWlp       uint8 = 16 // 0x10
	NaluTypeSliceBlaWradl     uint8 = 17 // 0x11
	NaluTypeSliceBlaNlp       uint8 = 18 // 0x12
	NaluTypeSliceIdr          uint8 = 19 // 0x13
	NaluTypeSliceIdrNlp       uint8 = 20 // 0x14
	NaluTypeSliceCranut       uint8 = 21 // 0x15
	NaluTypeSliceRsvIrapVcl22 uint8 = 22 // 0x16
	NaluTypeSliceRsvIrapVcl23 uint8 = 23 // 0x17

	NaluTypeVps       uint8 = 32 // 0x20
	NaluTypeSps       uint8 = 33 // 0x21
	NaluTypePps       uint8 = 34 // 0x22
	NaluTypeAud       uint8 = 35 // 0x23
	NaluTypeSei       uint8 = 39 // 0x27
	NaluTypeSeiSuffix uint8 = 40 // 0x28
)

type Context struct {
	PicWidthInLumaSamples  uint32 // sps
	PicHeightInLumaSamples uint32 // sps

	ConfigurationVersion uint8 // const value: 1

	GeneralProfileSpace              uint8
	GeneralTierFlag                  uint8
	GeneralProfileIdc                uint8
	GeneralProfileCompatibilityFlags uint32 // const value: 0xffffffff
	GeneralConstraintIndicatorFlags  uint64 // const value: 0xffffffffffff
	GeneralLevelIdc                  uint8

	LengthSizeMinusOne uint8 // const value: 3

	NumTemporalLayers uint8
	TemporalIdNested  uint8

	ChromaFormat         uint8
	BitDepthLumaMinus8   uint8
	BitDepthChromaMinus8 uint8
}

func ParseNaluTypeReadable(v uint8) string {
	b, ok := NaluTypeMapping[ParseNaluType(v)]
	if !ok {
		return "unknown"
	}
	return b
}

// ParseNaluType
//
// @param v 第一个字节
func ParseNaluType(v uint8) uint8 {
	// 6 bit in middle
	// 0*** ***0
	// or return (nalu[0] >> 1) & 0x3F
	return (v & 0x7E) >> 1
}

// IsIrapNalu 是否是关键帧
//
// @param typ 帧类型。注意，是经过 ParseNaluType 解析后的帧类型
func IsIrapNalu(typ uint8) bool {
	// [16, 23] irap nal
	// [19, 20] idr nal
	return typ >= NaluTypeSliceBlaWlp && typ <= NaluTypeSliceRsvIrapVcl23
}

// VpsSpsPpsSeqHeader2Annexb
//
// HVCC Seq Header -> Annexb
//
// @return 返回的内存块为内部独立新申请
func VpsSpsPpsSeqHeader2Annexb(payload []byte) ([]byte, error) {
	vps, sps, pps, err := ParseVpsSpsPpsFromSeqHeaderWithoutMalloc(payload)
	if err != nil {
		return nil, err
	}
	var ret []byte
	ret = append(ret, NaluStartCode4...)
	ret = append(ret, vps...)
	ret = append(ret, NaluStartCode4...)
	ret = append(ret, sps...)
	ret = append(ret, NaluStartCode4...)
	ret = append(ret, pps...)
	return ret, nil
}

func VpsSpsPpsEnhancedSeqHeader2Annexb(payload []byte) ([]byte, error) {
	vps, sps, pps, err := ParseVpsSpsPpsFromEnhancedSeqHeader(payload)
	if err != nil {
		return nil, err
	}
	var ret []byte
	ret = append(ret, NaluStartCode4...)
	ret = append(ret, vps...)
	ret = append(ret, NaluStartCode4...)
	ret = append(ret, sps...)
	ret = append(ret, NaluStartCode4...)
	ret = append(ret, pps...)
	return ret, nil
}

func BuildVpsSpsPps2Annexb(vps, sps, pps []byte) ([]byte, error) {
	ctx := newContext()
	if err := ParseVps(vps, ctx); err != nil {
		return nil, err
	}
	if err := ParseSps(sps, ctx); err != nil {
		return nil, err
	}
	var ret []byte
	ret = append(ret, NaluStartCode4...)
	ret = append(ret, vps...)
	ret = append(ret, NaluStartCode4...)
	ret = append(ret, sps...)
	ret = append(ret, NaluStartCode4...)
	ret = append(ret, pps...)
	return ret, nil
}

// ParseVpsSpsPpsFromSeqHeader
//
// 见func ParseVpsSpsPpsFromSeqHeaderWithoutMalloc
//
// @return vps, sps, pps: 内存块为内部独立新申请
func ParseVpsSpsPpsFromSeqHeader(payload []byte) (vps, sps, pps []byte, err error) {
	v, s, p, e := ParseVpsSpsPpsFromSeqHeaderWithoutMalloc(payload)
	if e != nil {
		return nil, nil, nil, e
	}
	vps = append(vps, v...)
	sps = append(sps, s...)
	pps = append(pps, p...)
	return
}

func ParseVpsSpsPpsFromEnhancedSeqHeader(payload []byte) (vps, sps, pps []byte, err error) {
	packetType := payload[0] & 0x0f

	if packetType == 0 {
		return parseVpsSpsPpsFromRecord(payload)
	}

	return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
}

// ParseVpsSpsPpsFromSeqHeaderWithoutMalloc
//
// 从HVCC格式的Seq Header中得到VPS，SPS，PPS内存块。
//
// @param payload: rtmp message的payload部分或者flv tag的payload部分。
//
//	注意，包含了头部2字节类型以及3字节的cts。
//
// @return vps, sps, pps: 复用传入参数`payload`的内存块。
func ParseVpsSpsPpsFromSeqHeaderWithoutMalloc(payload []byte) (vps, sps, pps []byte, err error) {
	if len(payload) < 5 {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrShortBuffer)
	}

	if payload[0] != 0x1c || payload[1] != 0x00 || payload[2] != 0 || payload[3] != 0 || payload[4] != 0 {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	//Log.Debugf("%s", hex.Dump(payload))

	if len(payload) < 33 {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}

	return parseVpsSpsPpsFromRecord(payload)
}

func parseVpsSpsPpsFromRecord(payload []byte) (vps, sps, pps []byte, err error) {
	index := 27
	if numOfArrays := payload[index]; numOfArrays != 3 && numOfArrays != 4 {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	index++

	// 注意，seq header中，是最后6个字节而不是中间6个字节
	if payload[index]&0x3f != NaluTypeVps {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	if numNalus := int(bele.BeUint16(payload[index+1:])); numNalus != 1 {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	vpsLen := int(bele.BeUint16(payload[index+3:]))

	if len(payload) < 33+vpsLen {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}

	vps = payload[index+5 : index+5+vpsLen]
	index += 5 + vpsLen

	if len(payload) < 38+vpsLen {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	if payload[index]&0x3f != NaluTypeSps {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	if numNalus := int(bele.BeUint16(payload[index+1:])); numNalus != 1 {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	spsLen := int(bele.BeUint16(payload[index+3:]))
	if len(payload) < 38+vpsLen+spsLen {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	sps = payload[index+5 : index+5+spsLen]
	index += 5 + spsLen

	if len(payload) < 43+vpsLen+spsLen {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	if payload[index]&0x3f != NaluTypePps {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	if numNalus := int(bele.BeUint16(payload[index+1:])); numNalus != 1 {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	ppsLen := int(bele.BeUint16(payload[index+3:]))
	if len(payload) < 43+vpsLen+spsLen+ppsLen {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrHevc)
	}
	pps = payload[index+5 : index+5+ppsLen]

	return
}

// BuildSeqHeaderFromVpsSpsPps
//
// @return 内存块为内部独立新申请
func BuildSeqHeaderFromVpsSpsPps(vps, sps, pps []byte) ([]byte, error) {
	var sh []byte
	sh = make([]byte, 43+len(vps)+len(sps)+len(pps))
	sh[0] = 0x1c
	sh[1] = 0x0
	sh[2] = 0x0
	sh[3] = 0x0
	sh[4] = 0x0

	// unsigned int(8) configurationVersion = 1;
	sh[5] = 0x1

	ctx := newContext()
	if err := ParseVps(vps, ctx); err != nil {
		return nil, err
	}
	if err := ParseSps(sps, ctx); err != nil {
		return nil, err
	}

	// unsigned int(2) general_profile_space;
	// unsigned int(1) general_tier_flag;
	// unsigned int(5) general_profile_idc;
	sh[6] = ctx.GeneralProfileSpace<<6 | ctx.GeneralTierFlag<<5 | ctx.GeneralProfileIdc
	// unsigned int(32) general_profile_compatibility_flags
	bele.BePutUint32(sh[7:], ctx.GeneralProfileCompatibilityFlags)
	// unsigned int(48) general_constraint_indicator_flags
	bele.BePutUint32(sh[11:], uint32(ctx.GeneralConstraintIndicatorFlags>>16))
	bele.BePutUint16(sh[15:], uint16(ctx.GeneralConstraintIndicatorFlags))
	// unsigned int(8) general_level_idc;
	sh[17] = ctx.GeneralLevelIdc

	// bit(4) reserved = ‘1111’b;
	// unsigned int(12) min_spatial_segmentation_idc;
	// bit(6) reserved = ‘111111’b;
	// unsigned int(2) parallelismType;
	// TODO chef: 这两个字段没有解析
	bele.BePutUint16(sh[18:], 0xf000)
	sh[20] = 0xfc

	// bit(6) reserved = ‘111111’b;
	// unsigned int(2) ChromaFormat;
	sh[21] = ctx.ChromaFormat | 0xfc

	// bit(5) reserved = ‘11111’b;
	// unsigned int(3) BitDepthLumaMinus8;
	sh[22] = ctx.BitDepthLumaMinus8 | 0xf8

	// bit(5) reserved = ‘11111’b;
	// unsigned int(3) BitDepthChromaMinus8;
	sh[23] = ctx.BitDepthChromaMinus8 | 0xf8

	// bit(16) avgFrameRate;
	bele.BePutUint16(sh[24:], 0)

	// bit(2) constantFrameRate;
	// bit(3) NumTemporalLayers;
	// bit(1) TemporalIdNested;
	// unsigned int(2) lengthSizeMinusOne;
	sh[26] = 0<<6 | ctx.NumTemporalLayers<<3 | ctx.TemporalIdNested<<2 | ctx.LengthSizeMinusOne

	// num of vps sps pps
	sh[27] = 0x03
	i := 28
	sh[i] = NaluTypeVps
	// num of vps
	bele.BePutUint16(sh[i+1:], 1)
	// length
	bele.BePutUint16(sh[i+3:], uint16(len(vps)))
	copy(sh[i+5:], vps)
	i = i + 5 + len(vps)
	sh[i] = NaluTypeSps
	bele.BePutUint16(sh[i+1:], 1)
	bele.BePutUint16(sh[i+3:], uint16(len(sps)))
	copy(sh[i+5:], sps)
	i = i + 5 + len(sps)
	sh[i] = NaluTypePps
	bele.BePutUint16(sh[i+1:], 1)
	bele.BePutUint16(sh[i+3:], uint16(len(pps)))
	copy(sh[i+5:], pps)

	return sh, nil
}

func ParseVps(vps []byte, ctx *Context) error {
	if len(vps) < 2 {
		return nazaerrors.Wrap(base.ErrHevc)
	}

	rbsp := nal2rbsp(vps[2:])
	br := nazabits.NewBitReader(rbsp)

	// skip
	// vps_video_parameter_set_id u(4)
	// vps_reserved_three_2bits   u(2)
	// vps_max_layers_minus1      u(6)
	if _, err := br.ReadBits16(12); err != nil {
		return nazaerrors.Wrap(base.ErrHevc)
	}

	vpsMaxSubLayersMinus1, err := br.ReadBits8(3)
	if err != nil {
		return nazaerrors.Wrap(base.ErrHevc)
	}
	if vpsMaxSubLayersMinus1+1 > ctx.NumTemporalLayers {
		ctx.NumTemporalLayers = vpsMaxSubLayersMinus1 + 1
	}

	// skip
	// vps_temporal_id_nesting_flag u(1)
	// vps_reserved_0xffff_16bits   u(16)
	if _, err := br.ReadBits32(17); err != nil {
		return nazaerrors.Wrap(base.ErrHevc)
	}

	return parsePtl(&br, ctx, vpsMaxSubLayersMinus1)
}

func ParseSps(sps []byte, ctx *Context) error {
	var err error

	if len(sps) < 2 {
		return nazaerrors.Wrap(base.ErrHevc)
	}

	rbsp := nal2rbsp(sps[2:])
	br := nazabits.NewBitReader(rbsp)

	// sps_video_parameter_set_id
	if _, err = br.ReadBits8(4); err != nil {
		return err
	}

	spsMaxSubLayersMinus1, err := br.ReadBits8(3)
	if err != nil {
		return err
	}

	if spsMaxSubLayersMinus1+1 > ctx.NumTemporalLayers {
		ctx.NumTemporalLayers = spsMaxSubLayersMinus1 + 1
	}

	// sps_temporal_id_nesting_flag
	if ctx.TemporalIdNested, err = br.ReadBit(); err != nil {
		return err
	}

	if err = parsePtl(&br, ctx, spsMaxSubLayersMinus1); err != nil {
		return err
	}

	// sps_seq_parameter_set_id
	if _, err = br.ReadGolomb(); err != nil {
		return err
	}

	var cf uint32
	if cf, err = br.ReadGolomb(); err != nil {
		return err
	}
	ctx.ChromaFormat = uint8(cf)
	if ctx.ChromaFormat == 3 {
		if _, err = br.ReadBit(); err != nil {
			return err
		}
	}

	// https://github.com/OpenVisualCloud/SVT-HEVC/issues/319
	if ctx.PicWidthInLumaSamples, err = br.ReadGolomb(); err != nil {
		return err
	}
	if ctx.PicHeightInLumaSamples, err = br.ReadGolomb(); err != nil {
		return err
	}

	conformanceWindowFlag, err := br.ReadBit()
	if err != nil {
		return err
	}
	if conformanceWindowFlag != 0 {
		if _, err = br.ReadGolomb(); err != nil {
			return err
		}
		if _, err = br.ReadGolomb(); err != nil {
			return err
		}
		if _, err = br.ReadGolomb(); err != nil {
			return err
		}
		if _, err = br.ReadGolomb(); err != nil {
			return err
		}
	}

	var bdlm8 uint32
	if bdlm8, err = br.ReadGolomb(); err != nil {
		return err
	}
	ctx.BitDepthLumaMinus8 = uint8(bdlm8)
	var bdcm8 uint32
	if bdcm8, err = br.ReadGolomb(); err != nil {
		return err
	}
	ctx.BitDepthChromaMinus8 = uint8(bdcm8)

	_, err = br.ReadGolomb()
	if err != nil {
		return err
	}
	spsSubLayerOrderingInfoPresentFlag, err := br.ReadBit()
	if err != nil {
		return err
	}
	var i uint8
	if spsSubLayerOrderingInfoPresentFlag != 0 {
		i = 0
	} else {
		i = spsMaxSubLayersMinus1
	}
	for ; i <= spsMaxSubLayersMinus1; i++ {
		if _, err = br.ReadGolomb(); err != nil {
			return err
		}
		if _, err = br.ReadGolomb(); err != nil {
			return err
		}
		if _, err = br.ReadGolomb(); err != nil {
			return err
		}
	}

	if _, err = br.ReadGolomb(); err != nil {
		return err
	}
	if _, err = br.ReadGolomb(); err != nil {
		return err
	}
	if _, err = br.ReadGolomb(); err != nil {
		return err
	}
	if _, err = br.ReadGolomb(); err != nil {
		return err
	}
	if _, err = br.ReadGolomb(); err != nil {
		return err
	}
	if _, err = br.ReadGolomb(); err != nil {
		return err
	}

	return nil
}

func parsePtl(br *nazabits.BitReader, ctx *Context, maxSubLayersMinus1 uint8) error {
	var err error
	var ptl Context
	if ptl.GeneralProfileSpace, err = br.ReadBits8(2); err != nil {
		return err
	}
	if ptl.GeneralTierFlag, err = br.ReadBit(); err != nil {
		return err
	}
	if ptl.GeneralProfileIdc, err = br.ReadBits8(5); err != nil {
		return err
	}
	if ptl.GeneralProfileCompatibilityFlags, err = br.ReadBits32(32); err != nil {
		return err
	}
	if ptl.GeneralConstraintIndicatorFlags, err = br.ReadBits64(48); err != nil {
		return err
	}
	if ptl.GeneralLevelIdc, err = br.ReadBits8(8); err != nil {
		return err
	}
	updatePtl(ctx, &ptl)

	if maxSubLayersMinus1 == 0 {
		return nil
	}

	subLayerProfilePresentFlag := make([]uint8, maxSubLayersMinus1)
	subLayerLevelPresentFlag := make([]uint8, maxSubLayersMinus1)
	for i := uint8(0); i < maxSubLayersMinus1; i++ {
		if subLayerProfilePresentFlag[i], err = br.ReadBit(); err != nil {
			return err
		}
		if subLayerLevelPresentFlag[i], err = br.ReadBit(); err != nil {
			return err
		}
	}
	if maxSubLayersMinus1 > 0 {
		for i := maxSubLayersMinus1; i < 8; i++ {
			if _, err = br.ReadBits8(2); err != nil {
				return err
			}
		}
	}

	for i := uint8(0); i < maxSubLayersMinus1; i++ {
		if subLayerProfilePresentFlag[i] != 0 {
			if _, err = br.ReadBits32(32); err != nil {
				return err
			}
			if _, err = br.ReadBits32(32); err != nil {
				return err
			}
			if _, err = br.ReadBits32(24); err != nil {
				return err
			}
		}

		if subLayerLevelPresentFlag[i] != 0 {
			if _, err = br.ReadBits8(8); err != nil {
				return err
			}
		}
	}

	return nil
}

func updatePtl(ctx, ptl *Context) {
	ctx.GeneralProfileSpace = ptl.GeneralProfileSpace

	if ptl.GeneralTierFlag > ctx.GeneralTierFlag {
		ctx.GeneralLevelIdc = ptl.GeneralLevelIdc

		ctx.GeneralTierFlag = ptl.GeneralTierFlag
	} else {
		if ptl.GeneralLevelIdc > ctx.GeneralLevelIdc {
			ctx.GeneralLevelIdc = ptl.GeneralLevelIdc
		}
	}

	if ptl.GeneralProfileIdc > ctx.GeneralProfileIdc {
		ctx.GeneralProfileIdc = ptl.GeneralProfileIdc
	}

	ctx.GeneralProfileCompatibilityFlags &= ptl.GeneralProfileCompatibilityFlags

	ctx.GeneralConstraintIndicatorFlags &= ptl.GeneralConstraintIndicatorFlags
}

func newContext() *Context {
	return &Context{
		ConfigurationVersion:             1,
		LengthSizeMinusOne:               3, // 4 bytes
		GeneralProfileCompatibilityFlags: 0xffffffff,
		GeneralConstraintIndicatorFlags:  0xffffffffffff,
	}
}

func nal2rbsp(nal []byte) []byte {
	// TODO chef:
	// 1. 输出应该可由外部申请
	// 2. 替换性能
	// 3. 该函数应该放入avc中
	return bytes.Replace(nal, []byte{0x0, 0x0, 0x3}, []byte{0x0, 0x0}, -1)
}
