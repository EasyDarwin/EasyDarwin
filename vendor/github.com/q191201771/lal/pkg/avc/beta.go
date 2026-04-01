// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package avc

import (
	"encoding/hex"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/naza/pkg/nazaerrors"

	"github.com/q191201771/naza/pkg/bele"
	"github.com/q191201771/naza/pkg/nazabits"
	"github.com/q191201771/naza/pkg/nazabytes"
)

func ParseSps(payload []byte, ctx *Context) error {
	br := nazabits.NewBitReader(payload)
	var sps Sps
	if err := parseSpsBasic(&br, &sps); err != nil {
		Log.Errorf("parseSpsBasic failed. err=%+v, payload=%s", err, hex.Dump(nazabytes.Prefix(payload, 128)))
		return err
	}
	ctx.Profile = sps.ProfileIdc
	ctx.Level = sps.LevelIdc

	//if err := parseSpsBeta(&br, &sps); err != nil {
	//	// 注意，这里不将错误返回给上层，因为可能是Beta自身解析的问题
	//}

	if err := parseSpsGamma(&br, &sps); err != nil {
		// 注意，这里不将错误返回给上层，因为可能是Beta自身解析的问题
		Log.Errorf("parseSpsGamma failed. err=%+v, payload=%s", err, hex.Dump(nazabytes.Prefix(payload, 128)))
	}
	Log.Debugf("sps=%+v", sps)

	ctx.Width = (sps.PicWidthInMbsMinusOne+1)*16 - (sps.FrameCropLeftOffset+sps.FrameCropRightOffset)*2
	ctx.Height = (2-uint32(sps.FrameMbsOnlyFlag))*(sps.PicHeightInMapUnitsMinusOne+1)*16 - (sps.FrameCropTopOffset+sps.FrameCropBottomOffset)*2

	ctx.Sps = sps
	return nil
}

// TryParsePps 尝试解析PPS所有字段，实验中，请勿直接使用该函数
func TryParsePps(payload []byte) error {
	// ISO-14496-10.pdf
	// 7.3.2.2 Picture parameter set RBSP syntax

	// TODO impl me
	return nil
}

// TryParseSeqHeader 尝试解析SeqHeader所有字段，实验中，请勿直接使用该函数。
//
// @param payload:
// rtmp message的payload部分或者flv tag的payload部分。
// 注意，包含了头部2字节类型以及3字节的cts。
func TryParseSeqHeader(payload []byte) error {
	if len(payload) < 5 {
		return nazaerrors.Wrap(base.ErrShortBuffer)
	}
	if payload[0] != 0x17 || payload[1] != 0x00 || payload[2] != 0 || payload[3] != 0 || payload[4] != 0 {
		return nazaerrors.Wrap(base.ErrAvc)
	}

	// H.264-AVC-ISO_IEC_14496-15.pdf
	// 5.2.4 Decoder configuration information
	var dcr DecoderConfigurationRecord
	var err error
	br := nazabits.NewBitReader(payload[5:])

	// TODO check error
	dcr.ConfigurationVersion, err = br.ReadBits8(8)
	dcr.AvcProfileIndication, err = br.ReadBits8(8)
	dcr.ProfileCompatibility, err = br.ReadBits8(8)
	dcr.AvcLevelIndication, err = br.ReadBits8(8)
	_, err = br.ReadBits8(6) // reserved = '111111'b
	dcr.LengthSizeMinusOne, err = br.ReadBits8(2)

	_, err = br.ReadBits8(3) // reserved = '111'b
	dcr.NumOfSps, err = br.ReadBits8(5)
	b, err := br.ReadBytes(2)
	dcr.SpsLength = bele.BeUint16(b)

	_, _ = br.ReadBytes(uint(dcr.SpsLength))

	_, err = br.ReadBits8(3) // reserved = '111'b
	dcr.NumOfPps, err = br.ReadBits8(5)
	b, err = br.ReadBytes(2)
	dcr.PpsLength = bele.BeUint16(b)

	Log.Debugf("%+v", dcr)

	// 5 + 5 + 1 + 2
	var ctx Context
	_ = ParseSps(payload[13:13+dcr.SpsLength], &ctx)
	// 13 + 1 + 2
	_ = TryParsePps(payload[16 : 16+dcr.PpsLength])

	return err
}

func parseSpsBasic(br *nazabits.BitReader, sps *Sps) error {
	t, err := br.ReadBits8(8) //nalType SPS should be 0x67
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	_ = t

	sps.ProfileIdc, err = br.ReadBits8(8)
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	sps.ConstraintSet0Flag, err = br.ReadBits8(1)
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	sps.ConstraintSet1Flag, err = br.ReadBits8(1)
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	sps.ConstraintSet2Flag, err = br.ReadBits8(1)
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	_, err = br.ReadBits8(5)
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	sps.LevelIdc, err = br.ReadBits8(8)
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	sps.SpsId, err = br.ReadGolomb()
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	if sps.SpsId >= 32 {
		return nazaerrors.Wrap(base.ErrAvc)
	}
	return nil
}

func parseSpsGamma(br *nazabits.BitReader, sps *Sps) (err error) {
	switch sps.ProfileIdc {
	case 100, 110, 122, 244, 44, 83, 86, 118, 128, 138, 139, 134:
		sps.ChromaFormatIdc, err = br.ReadUeGolomb() // chroma_format_idc
		if err != nil {
			return nazaerrors.Wrap(err)
		}
		if sps.ChromaFormatIdc == 3 {
			sps.ResidualColorTransformFlag, err = br.ReadBits8(1) // separate_colour_plane_flag
			if err != nil {
				return nazaerrors.Wrap(err)
			}
		}
		sps.BitDepthLuma, err = br.ReadUeGolomb()
		if err != nil {
			return nazaerrors.Wrap(err)
		}
		sps.BitDepthLuma += 8

		sps.BitDepthChroma, err = br.ReadUeGolomb()
		if err != nil {
			return nazaerrors.Wrap(err)
		}
		sps.BitDepthChroma += 8

		sps.TransFormBypass, err = br.ReadBits8(1) // qpprime_y_zero_transform_bypass_flag
		if err != nil {
			return nazaerrors.Wrap(err)
		}

		flag, err := br.ReadBits8(1) // seq_scaling_matrix_present_flag
		if err != nil {
			return nazaerrors.Wrap(err)
		}
		if flag == 1 {
			loop := 8
			if sps.ChromaFormatIdc == 3 {
				loop = 12
			}
			deltaScale := 0
			lastScale := 8
			nextScale := 8
			for i := 0; i < loop; i++ {
				flag, err := br.ReadBits8(1) // seq_scaling_list_present_flag
				if err != nil {
					return nazaerrors.Wrap(err)
				}
				if flag == 0 {
					continue
				}
				lastScale = 8
				nextScale = 8
				sizeOfScalingList := 16
				if i >= 6 {
					sizeOfScalingList = 64
				}
				for j := 0; j < sizeOfScalingList; j++ {
					if nextScale != 0 {
						v, err := br.ReadSeGolomb()
						if err != nil {
							return nazaerrors.Wrap(err)
						}
						deltaScale = int(v)
						nextScale = (lastScale + deltaScale) & 0xff
					}
					if nextScale != 0 {
						lastScale = nextScale
					}
				}
			}
		}
	default:
		sps.ChromaFormatIdc = 1
		sps.BitDepthLuma = 8
		sps.BitDepthChroma = 8
	}

	sps.Log2MaxFrameNumMinus4, err = br.ReadUeGolomb() // log2_max_frame_num_minus4
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	sps.PicOrderCntType, err = br.ReadUeGolomb()
	if err != nil {
		return nazaerrors.Wrap(err)
	}

	if sps.PicOrderCntType == 0 {
		sps.Log2MaxPicOrderCntLsb, err = br.ReadUeGolomb() // log2_max_pic_order_cnt_lsb_minus4
		if err != nil {
			return nazaerrors.Wrap(err)
		}
		sps.Log2MaxPicOrderCntLsb += 4
	} else if sps.PicOrderCntType == 1 {
		_, _ = br.ReadBits8(1)   // delta_pic_order_always_zero
		_, _ = br.ReadSeGolomb() // offset_for_non_ref_pic
		_, _ = br.ReadSeGolomb() // offset_for_top_to_bottom_field
		if br.Err() != nil {
			return nazaerrors.Wrap(err)
		}
		nrfipocc, err := br.ReadUeGolomb() // num_ref_frames_in_pic_order_cnt_cycle
		if err != nil {
			return nazaerrors.Wrap(err)
		}
		for i := 0; i < int(nrfipocc); i++ {
			_, err := br.ReadSeGolomb() // offset_for_ref_frame
			if err != nil {
				return nazaerrors.Wrap(err)
			}
		}
	}

	sps.NumRefFrames, _ = br.ReadUeGolomb()                 // max_num_ref_frames
	sps.GapsInFrameNumValueAllowedFlag, _ = br.ReadBits8(1) // gaps_in_frame_num_value_allowed_flag
	sps.PicWidthInMbsMinusOne, _ = br.ReadUeGolomb()        // pic_width_in_mbs_minus1
	sps.PicHeightInMapUnitsMinusOne, _ = br.ReadUeGolomb()  // pic_height_in_map_units_minus1
	if br.Err() != nil {
		return nazaerrors.Wrap(err)
	}

	sps.FrameMbsOnlyFlag, err = br.ReadBits8(1)
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	if sps.FrameMbsOnlyFlag == 0 {
		sps.MbAdaptiveFrameFieldFlag, err = br.ReadBits8(1) // mb_adaptive_frame_field_flag
		if err != nil {
			return nazaerrors.Wrap(err)
		}
	}

	sps.Direct8X8InferenceFlag, err = br.ReadBits8(1) // direct_8x8_inference_flag
	if err != nil {
		return nazaerrors.Wrap(err)
	}

	sps.FrameCroppingFlag, err = br.ReadBits8(1)
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	if sps.FrameCroppingFlag == 1 { // frame_cropping_flag
		sps.FrameCropLeftOffset, _ = br.ReadUeGolomb()   // frame_crop_left_offset
		sps.FrameCropRightOffset, _ = br.ReadUeGolomb()  // frame_crop_right_offset
		sps.FrameCropTopOffset, _ = br.ReadUeGolomb()    // frame_crop_top_offset
		sps.FrameCropBottomOffset, _ = br.ReadUeGolomb() // frame_crop_bottom_offset
		if br.Err() != nil {
			return nazaerrors.Wrap(err)
		}
	}

	flag, err := br.ReadBits8(1) // vui_parameters_present_flag
	if err != nil {
		return nazaerrors.Wrap(err)
	}
	if flag == 1 {
		flag, err := br.ReadBits8(1) // aspect_ratio_info_present_flag
		if err != nil {
			return nazaerrors.Wrap(err)
		}
		if flag == 1 {
			ari, err := br.ReadBits8(8) // aspect_ratio_idc
			if err != nil {
				return nazaerrors.Wrap(err)
			}
			if ari == 0xff {
				v, err := br.ReadBits16(16)
				if err != nil {
					return nazaerrors.Wrap(err)
				}
				sps.SarNum = int(v)
				v, err = br.ReadBits16(16)
				if err != nil {
					return nazaerrors.Wrap(err)
				}
				sps.SarDen = int(v)
			} else if ari < 17 {
				mapping := []struct {
					num int
					den int
				}{
					{0, 1},
					{1, 1},
					{12, 11},
					{10, 11},
					{16, 11},
					{40, 33},
					{24, 11},
					{20, 11},
					{32, 11},
					{80, 33},
					{18, 11},
					{15, 11},
					{64, 33},
					{160, 99},
					{4, 3},
					{3, 2},
					{2, 1},
				}
				sps.SarNum = mapping[ari].num
				sps.SarDen = mapping[ari].den
			}
		}
	}

	if sps.SarDen == 0 {
		sps.SarNum = 1
		sps.SarDen = 1
	}

	return nil
}
