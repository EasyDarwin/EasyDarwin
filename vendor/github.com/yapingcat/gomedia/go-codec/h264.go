package codec

import (
	"encoding/binary"
	"errors"
)

// nal_unit( NumBytesInNALunit ) {
//     forbidden_zero_bit All         f(1)
//     nal_ref_idc All             u(2)
//     nal_unit_type                u(5)
// }

type H264NaluHdr struct {
	Forbidden_zero_bit uint8
	Nal_ref_idc        uint8
	Nal_unit_type      uint8
}

func (hdr *H264NaluHdr) Decode(bs *BitStream) {
	hdr.Forbidden_zero_bit = bs.GetBit()
	hdr.Nal_ref_idc = bs.Uint8(2)
	hdr.Nal_unit_type = bs.Uint8(5)
}

type SliceHeader struct {
	First_mb_in_slice    uint64
	Slice_type           uint64
	Pic_parameter_set_id uint64
	Frame_num            uint64
}

// 调用方根据sps中的log2_max_frame_num_minus4的值来解析Frame_num
func (sh *SliceHeader) Decode(bs *BitStream) {
	sh.First_mb_in_slice = bs.ReadUE()
	sh.Slice_type = bs.ReadUE()
	sh.Pic_parameter_set_id = bs.ReadUE()
}

type SPS struct {
	Profile_idc                          uint8
	Constraint_set0_flag                 uint8
	Constraint_set1_flag                 uint8
	Constraint_set2_flag                 uint8
	Constraint_set3_flag                 uint8
	Constraint_set4_flag                 uint8
	Constraint_set5_flag                 uint8
	Reserved_zero_2bits                  uint8
	Level_idc                            uint8
	Seq_parameter_set_id                 uint64
	Chroma_format_idc                    uint64
	Separate_colour_plane_flag           uint8
	Bit_depth_luma_minus8                uint64
	Bit_depth_chroma_minus8              uint64
	Log2_max_frame_num_minus4            uint64
	Pic_order_cnt_type                   uint64
	Log2_max_pic_order_cnt_lsb_minus4    uint64
	Delta_pic_order_always_zero_flag     uint8
	Offset_for_non_ref_pic               int64
	Offset_for_top_to_bottom_field       int64
	Offset_for_ref_frame                 []int64
	Max_num_ref_frames                   uint64
	Gaps_in_frame_num_value_allowed_flag uint8
	Pic_width_in_mbs_minus1              uint64
	Pic_height_in_map_units_minus1       uint64
	Frame_mbs_only_flag                  uint8
	Mb_adaptive_frame_field_flag         uint8
	Direct_8x8_inference_flag            uint8
	Frame_cropping_flag                  uint8
	Frame_crop_left_offset               uint64
	Frame_crop_right_offset              uint64
	Frame_crop_top_offset                uint64
	Frame_crop_bottom_offset             uint64
	Vui_parameters_present_flag          uint8
	VuiParameters                        H264VuiParameters
}

func (sps *SPS) Decode(bs *BitStream) {
	sps.Profile_idc = bs.Uint8(8)
	sps.Constraint_set0_flag = bs.GetBit()
	sps.Constraint_set1_flag = bs.GetBit()
	sps.Constraint_set2_flag = bs.GetBit()
	sps.Constraint_set3_flag = bs.GetBit()
	sps.Constraint_set4_flag = bs.GetBit()
	sps.Constraint_set5_flag = bs.GetBit()
	sps.Reserved_zero_2bits = bs.Uint8(2)
	sps.Level_idc = bs.Uint8(8)
	sps.Seq_parameter_set_id = bs.ReadUE()
	if sps.Profile_idc == 100 || sps.Profile_idc == 110 ||
		sps.Profile_idc == 122 || sps.Profile_idc == 244 || sps.Profile_idc == 44 ||
		sps.Profile_idc == 83 || sps.Profile_idc == 86 || sps.Profile_idc == 118 ||
		sps.Profile_idc == 128 || sps.Profile_idc == 138 || sps.Profile_idc == 139 ||
		sps.Profile_idc == 134 || sps.Profile_idc == 135 {
		sps.Chroma_format_idc = bs.ReadUE()
		if sps.Chroma_format_idc == 3 {
			sps.Separate_colour_plane_flag = bs.Uint8(1) //separate_colour_plane_flag
		}
		sps.Bit_depth_luma_minus8 = bs.ReadUE()   //bit_depth_luma_minus8
		sps.Bit_depth_chroma_minus8 = bs.ReadUE() //bit_depth_chroma_minus8
		bs.SkipBits(1)                            //qpprime_y_zero_transform_bypass_flag
		seq_scaling_matrix_present_flag := bs.GetBit()
		if seq_scaling_matrix_present_flag == 1 {
			//seq_scaling_list_present_flag[i]
			if sps.Chroma_format_idc == 3 {
				bs.SkipBits(12)
			} else {
				bs.SkipBits(8)
			}
		}
	}
	sps.Log2_max_frame_num_minus4 = bs.ReadUE()
	sps.Pic_order_cnt_type = bs.ReadUE()
	if sps.Pic_order_cnt_type == 0 {
		sps.Log2_max_pic_order_cnt_lsb_minus4 = bs.ReadUE()
	} else if sps.Pic_order_cnt_type == 1 {
		sps.Delta_pic_order_always_zero_flag = bs.GetBit()
		sps.Offset_for_non_ref_pic = bs.ReadSE()         // offset_for_non_ref_pic
		sps.Offset_for_top_to_bottom_field = bs.ReadSE() // offset_for_top_to_bottom_field
		num_ref_frames_in_pic_order_cnt_cycle := bs.ReadUE()
		for i := 0; i < int(num_ref_frames_in_pic_order_cnt_cycle); i++ {
			sps.Offset_for_ref_frame[i] = bs.ReadSE() // offset_for_ref_frame
		}
	}
	sps.Max_num_ref_frames = bs.ReadUE()
	sps.Gaps_in_frame_num_value_allowed_flag = bs.GetBit()
	sps.Pic_width_in_mbs_minus1 = bs.ReadUE()
	sps.Pic_height_in_map_units_minus1 = bs.ReadUE()
	sps.Frame_mbs_only_flag = bs.GetBit()
	if sps.Frame_mbs_only_flag == 0 {
		sps.Mb_adaptive_frame_field_flag = bs.GetBit()
	}
	sps.Direct_8x8_inference_flag = bs.GetBit()
	sps.Frame_cropping_flag = bs.GetBit()
	if sps.Frame_cropping_flag == 1 {
		sps.Frame_crop_left_offset = bs.ReadUE()   //frame_crop_left_offset
		sps.Frame_crop_right_offset = bs.ReadUE()  //frame_crop_right_offset
		sps.Frame_crop_top_offset = bs.ReadUE()    //frame_crop_top_offset
		sps.Frame_crop_bottom_offset = bs.ReadUE() //frame_crop_bottom_offset
	}
	sps.Vui_parameters_present_flag = bs.GetBit()

	if sps.Vui_parameters_present_flag == 1 {
		sps.VuiParameters.Decode(bs)
	}
}

type PPS struct {
	Pic_parameter_set_id                         uint64
	Seq_parameter_set_id                         uint64
	Entropy_coding_mode_flag                     uint8
	Bottom_field_pic_order_in_frame_present_flag uint8
	Num_slice_groups_minus1                      uint64
}

func (pps *PPS) Decode(bs *BitStream) {
	pps.Pic_parameter_set_id = bs.ReadUE()
	pps.Seq_parameter_set_id = bs.ReadUE()
	pps.Entropy_coding_mode_flag = bs.GetBit()
	pps.Bottom_field_pic_order_in_frame_present_flag = bs.GetBit()
	pps.Num_slice_groups_minus1 = bs.ReadUE()
}

type SEIReaderWriter interface {
	Read(size uint16, bs *BitStream)
	Write(bsw *BitStreamWriter)
}

type UserDataUnregistered struct {
	UUID     []byte
	UserData []byte
}

func (udu *UserDataUnregistered) Read(size uint16, bs *BitStream) {
	udu.UUID = bs.GetBytes(16)
	udu.UserData = bs.GetBytes(int(size - 16))
}

func (udu *UserDataUnregistered) Write(bsw *BitStreamWriter) {
	bsw.PutBytes(udu.UUID)
	bsw.PutBytes(udu.UserData)
}

type SEI struct {
	PayloadType uint16
	PayloadSize uint16
	Sei_payload SEIReaderWriter
}

func (sei *SEI) Decode(bs *BitStream) {
	for bs.NextBits(8) == 0xFF {
		sei.PayloadType += 255
	}
	sei.PayloadType += uint16(bs.Uint8(8))
	for bs.NextBits(8) == 0xFF {
		sei.PayloadSize += 255
	}
	sei.PayloadSize += uint16(bs.Uint8(8))
	if sei.PayloadType == 5 {
		sei.Sei_payload = new(UserDataUnregistered)
		sei.Sei_payload.Read(sei.PayloadSize, bs)
	}
}

func (sei *SEI) Encode(bsw *BitStreamWriter) []byte {
	payloadType := sei.PayloadType
	payloadSize := sei.PayloadSize
	for payloadType >= 0xFF {
		bsw.PutByte(0xFF)
		payloadType -= 255
	}
	bsw.PutByte(uint8(payloadType))
	for payloadSize >= 0xFF {
		bsw.PutByte(0xFF)
		payloadSize -= 255
	}
	bsw.PutByte(uint8(payloadSize))
	sei.Sei_payload.Write(bsw)
	return bsw.Bits()
}

func GetSPSIdWithStartCode(sps []byte) uint64 {
	start, sc := FindStartCode(sps, 0)
	return GetSPSId(sps[start+int(sc):])
}

func GetSPSId(sps []byte) uint64 {
	sps = sps[1:]
	bs := NewBitStream(sps)
	bs.SkipBits(24)
	return bs.ReadUE()
}

func GetPPSIdWithStartCode(pps []byte) uint64 {
	start, sc := FindStartCode(pps, 0)
	return GetPPSId(pps[start+int(sc):])
}

func GetPPSId(pps []byte) uint64 {
	pps = pps[1:]
	bs := NewBitStream(pps)
	return bs.ReadUE()
}

// https://stackoverflow.com/questions/12018535/get-the-width-height-of-the-video-from-h-264-nalu
// int Width = ((pic_width_in_mbs_minus1 +1)*16) - frame_crop_right_offset *2 - frame_crop_left_offset *2;
// int Height = ((2 - frame_mbs_only_flag)* (pic_height_in_map_units_minus1 +1) * 16) - (frame_crop_bottom_offset* 2) - (frame_crop_top_offset* 2);
func GetH264Resolution(sps []byte) (width uint32, height uint32) {
	start, sc := FindStartCode(sps, 0)
	sodb := CovertRbspToSodb(sps[start+int(sc)+1:])
	bs := NewBitStream(sodb)
	var s SPS
	s.Decode(bs)

	widthInSample := (uint32(s.Pic_width_in_mbs_minus1) + 1) * 16
	widthCrop := uint32(s.Frame_crop_left_offset)*2 + uint32(s.Frame_crop_right_offset)*2
	width = widthInSample - widthCrop

	heightInSample := ((2 - uint32(s.Frame_mbs_only_flag)) * (uint32(s.Pic_height_in_map_units_minus1) + 1) * 16)
	heightCrop := uint32(s.Frame_crop_bottom_offset)*2 - uint32(s.Frame_crop_top_offset)*2
	height = heightInSample - heightCrop

	return
}

// aligned(8) class AVCDecoderConfigurationRecord {
// 	unsigned int(8) configurationVersion = 1;
// 	unsigned int(8) AVCProfileIndication;
// 	unsigned int(8) profile_compatibility;
// 	unsigned int(8) AVCLevelIndication;
// 	bit(6) reserved = ‘111111’b;
// 	unsigned int(2) lengthSizeMinusOne;
// 	bit(3) reserved = ‘111’b;
// 	unsigned int(5) numOfSequenceParameterSets;
// 	for (i=0; i< numOfSequenceParameterSets;  i++) {
// 		unsigned int(16) sequenceParameterSetLength ;
// 		bit(8*sequenceParameterSetLength) sequenceParameterSetNALUnit;
// 	}
// 	unsigned int(8) numOfPictureParameterSets;
// 	for (i=0; i< numOfPictureParameterSets;  i++) {
// 		unsigned int(16) pictureParameterSetLength;
// 		bit(8*pictureParameterSetLength) pictureParameterSetNALUnit;
// 	}
// if( profile_idc  ==  100  ||  profile_idc  ==  110  ||
//     profile_idc  ==  122  ||  profile_idc  ==  144 )
// {
//     bit(6) reserved = ‘111111’b;
//     unsigned int(2) chroma_format;
//     bit(5) reserved = ‘11111’b;
//     unsigned int(3) bit_depth_luma_minus8;
//     bit(5) reserved = ‘11111’b;
//     unsigned int(3) bit_depth_chroma_minus8;
//     unsigned int(8) numOfSequenceParameterSetExt;
//     for (i=0; i< numOfSequenceParameterSetExt; i++) {
//      unsigned int(16) sequenceParameterSetExtLength;
//      bit(8*sequenceParameterSetExtLength) sequenceParameterSetExtNALUnit;
//     }
//  }

//  }
// }
// bits
// 8   version ( always 0x01 )
// 8   avc profile ( sps[0][1] )
// 8   avc compatibility ( sps[0][2] )
// 8   avc level ( sps[0][3] )
// 6   reserved ( all bits on )
// 2   NALULengthSizeMinusOne
// 3   reserved ( all bits on )
// 5   number of SPS NALUs (usually 1)

// repeated once per SPS:
// 16         SPS size
// variable   SPS NALU data

// 8   number of PPS NALUs (usually 1)
// repeated once per PPS:
//   16       PPS size
//   variable PPS NALU data

func CreateH264AVCCExtradata(spss [][]byte, ppss [][]byte) ([]byte, error) {

	if len(spss) == 0 || len(ppss) == 0 {
		return nil, errors.New("lack of sps or pps")
	}

	extradata := make([]byte, 6, 256)
	for i, sps := range spss {
		start, sc := FindStartCode(sps, 0)
		spss[i] = sps[start+int(sc):]
	}

	for i, pps := range ppss {
		start, sc := FindStartCode(pps, 0)
		ppss[i] = pps[start+int(sc):]
	}

	extradata[0] = 0x01
	extradata[1] = spss[0][1]
	extradata[2] = spss[0][2]
	extradata[3] = spss[0][3]
	extradata[4] = 0xFF
	extradata[5] = 0xE0 | uint8(len(spss))
	for _, sps := range spss {
		spssize := make([]byte, 2)
		binary.BigEndian.PutUint16(spssize, uint16(len(sps)))
		extradata = append(extradata, spssize...)
		extradata = append(extradata, sps...)
	}
	extradata = append(extradata, uint8(len(ppss)))
	for _, pps := range ppss {
		ppssize := make([]byte, 2)
		binary.BigEndian.PutUint16(ppssize, uint16(len(pps)))
		extradata = append(extradata, ppssize...)
		extradata = append(extradata, pps...)
	}
	var h264sps SPS
	sodb := CovertRbspToSodb(spss[0][1:])
	h264sps.Decode(NewBitStream(sodb))
	if h264sps.Profile_idc == 100 ||
		h264sps.Profile_idc == 110 ||
		h264sps.Profile_idc == 122 ||
		h264sps.Profile_idc == 144 {
		tmp := make([]byte, 4)
		tmp[0] = 0xFC | uint8(h264sps.Chroma_format_idc&0x03)
		tmp[1] = 0xF8 | uint8(h264sps.Bit_depth_luma_minus8&0x07)
		tmp[2] = 0xF8 | uint8(h264sps.Bit_depth_chroma_minus8&0x07)
		tmp[3] = 0
		extradata = append(extradata, tmp...)
	}

	return extradata, nil
}

func CovertExtradata(extraData []byte) ([][]byte, [][]byte) {
	spsnum := extraData[5] & 0x1F
	spss := make([][]byte, spsnum)
	offset := 6
	for i := 0; i < int(spsnum); i++ {
		spssize := binary.BigEndian.Uint16(extraData[offset:])
		sps := make([]byte, spssize+4)
		copy(sps, []byte{0x00, 0x00, 0x00, 0x01})
		copy(sps[4:], extraData[offset+2:offset+2+int(spssize)])
		offset += 2 + int(spssize)
		spss[i] = sps
	}
	ppsnum := extraData[offset]
	ppss := make([][]byte, ppsnum)
	offset++
	for i := 0; i < int(ppsnum); i++ {
		ppssize := binary.BigEndian.Uint16(extraData[offset:])
		pps := make([]byte, ppssize+4)
		copy(pps, []byte{0x00, 0x00, 0x00, 0x01})
		copy(pps[4:], extraData[offset+2:offset+2+int(ppssize)])
		offset += 2 + int(ppssize)
		ppss[i] = pps
	}
	return spss, ppss
}

func ConvertAnnexBToAVCC(annexb []byte) []byte {
	start, sc := FindStartCode(annexb, 0)
	if sc == START_CODE_4 {
		binary.BigEndian.PutUint32(annexb[start:], uint32(len(annexb)-4))
		return annexb
	} else {
		avcc := make([]byte, 1+len(annexb))
		binary.BigEndian.PutUint32(avcc, uint32(len(annexb)-3))
		copy(avcc[4:], annexb[start+3:])
		return avcc
	}
}

func CovertAVCCToAnnexB(avcc []byte) {
	avcc[0] = 0x00
	avcc[1] = 0x00
	avcc[2] = 0x00
	avcc[3] = 0x01
}

// H264VuiParameters
// From ITU-T - https://www.itu.int/ITU-T/recommendations/rec.aspx?id=14659
//
//	Version 14 - https://www.itu.int/rec/T-REC-H.264-202108-I
//	Page 422 - E.1.1 VUI parameters syntax
type H264VuiParameters struct {
	AspectRatioInfoPresentFlag     uint8  // u(1)
	AspectRatioIdc                 uint8  // u(8)
	SarWidth                       uint16 // u(16)
	SarHeight                      uint16 // u(16)
	OverscanInfoPresentFlag        uint8  // u(1)
	OverscanAppropriateFlag        uint8  // u(1)
	VideoSignalTypePresentFlag     uint8  // u(1)
	VideoFormat                    uint8  // u(3)
	VideoFullRangeFlag             uint8  // u(1)
	ColourDescriptionPresentFlag   uint8  // u(1)
	ColourPrimaries                uint8  // u(8)
	TransferCharacteristics        uint8  // u(8)
	MatrixCoefficients             uint8  // u(8)
	ChromaLocInfoPresentFlag       uint8  // u(1)
	ChromaSampleLocTypeTopField    uint64 // ue(v)
	ChromaSampleLocTypeBottomField uint64 // ue(v)
	TimingInfoPresentFlag          uint8  // u(1)
	NumUnitsInTick                 uint32 // u(32)
	TimeScale                      uint32 // u(32)
	FixedFrameRateFlag             uint8  // u(1)
	NalHrdParametersPresentFlag    uint8  // u(1)
	NalHrdParameters               H264HrdParameters
	VclHrdParametersPresentFlag    uint8 // u(1)
	VclHrdParameters               H264HrdParameters
	LowDelayHrdFlag                uint8  // u(1)
	PicStructPresentFlag           uint8  // u(1)
	BitstreamRestrictionFlag       uint8  // u(1)
	MotionVectorsOverPicBoundaries uint8  // u(1)
	MaxBytesPerPicDenom            uint64 // ue(v)
	MaxBitsPerMbDenom              uint64 // ue(v)
	Log2MaxMvLengthHorizontal      uint64 // ue(v)
	Log2MaxMvLengthVertical        uint64 // ue(v)
	NumReorderFrames               uint64 // ue(v)
	MaxDecFrameBuffering           uint64 // ue(v)
}

type H264HrdParameters struct {
	CpbCntMinus1                       uint64                      // ue(v)
	BitRateScale                       uint8                       // u(4)
	CpbSizeScale                       uint8                       // u(4)
	H264BitRateCpbSizeCbrFlag          []H264BitRateCpbSizeCbrFlag // 0..cpb_cnt_minus1
	InitialCpbRemovalDelayLengthMinus1 uint8                       // u(5)
	CpbRemovalDelayLengthMinus1        uint8                       // u(5)
	DpbOutputDelayLengthMinus1         uint8                       // u(5)
	TimeOffsetLength                   uint8                       // u(5)
}

type H264BitRateCpbSizeCbrFlag struct {
	BitRateValueMinus1 uint64 // ue(v)
	CpbSizeValueMinus1 uint64 // ue(v)
	CbrFlag            uint8  // u(1)
}

const ExtendedSar = 255

func (h264Vui *H264VuiParameters) Decode(bs *BitStream) {
	h264Vui.AspectRatioInfoPresentFlag = bs.Uint8(1)

	if h264Vui.AspectRatioInfoPresentFlag == 1 {
		h264Vui.AspectRatioIdc = bs.Uint8(8)

		if h264Vui.AspectRatioIdc == ExtendedSar {
			h264Vui.SarWidth = bs.Uint16(16)
			h264Vui.SarWidth = bs.Uint16(16)
		}
	}

	h264Vui.OverscanInfoPresentFlag = bs.Uint8(1)

	if h264Vui.OverscanInfoPresentFlag == 1 {
		h264Vui.OverscanAppropriateFlag = bs.Uint8(1)
	}

	h264Vui.VideoSignalTypePresentFlag = bs.Uint8(1)

	if h264Vui.VideoSignalTypePresentFlag == 1 {
		h264Vui.VideoFormat = bs.Uint8(3)
		h264Vui.VideoFullRangeFlag = bs.Uint8(1)
		h264Vui.ColourDescriptionPresentFlag = bs.Uint8(1)

		if h264Vui.ColourDescriptionPresentFlag == 1 {
			h264Vui.ColourPrimaries = bs.Uint8(8)
			h264Vui.TransferCharacteristics = bs.Uint8(8)
			h264Vui.MatrixCoefficients = bs.Uint8(8)
		}
	}

	h264Vui.ChromaLocInfoPresentFlag = bs.Uint8(1)

	if h264Vui.ChromaLocInfoPresentFlag == 1 {
		h264Vui.ChromaSampleLocTypeTopField = bs.ReadUE()
		h264Vui.ChromaSampleLocTypeBottomField = bs.ReadUE()
	}

	h264Vui.TimingInfoPresentFlag = bs.Uint8(1)

	if h264Vui.TimingInfoPresentFlag == 1 {
		h264Vui.NumUnitsInTick = bs.Uint32(32)
		h264Vui.TimeScale = bs.Uint32(32)
		h264Vui.FixedFrameRateFlag = bs.Uint8(1)
	}

	h264Vui.NalHrdParametersPresentFlag = bs.Uint8(1)

	if h264Vui.NalHrdParametersPresentFlag == 1 {
		h264Vui.NalHrdParameters.Decode(bs)
	}

	h264Vui.VclHrdParametersPresentFlag = bs.Uint8(1)

	if h264Vui.VclHrdParametersPresentFlag == 1 {
		h264Vui.VclHrdParameters.Decode(bs)
	}

	if h264Vui.NalHrdParametersPresentFlag == 1 || h264Vui.VclHrdParametersPresentFlag == 1 {
		h264Vui.LowDelayHrdFlag = bs.Uint8(1)
	}

	/*
		TODO - These fields were causing problems because we'd run out of bits when parsing. Maybe they're optional in certain versions/levels/configurations?
			h264Vui.PicStructPresentFlag = bs.GetBit()
			h264Vui.BitstreamRestrictionFlag = bs.GetBit()

			if h264Vui.BitstreamRestrictionFlag == 1 {
				h264Vui.MotionVectorsOverPicBoundaries = bs.GetBit()
				h264Vui.MaxBytesPerPicDenom = bs.ReadUE()
				h264Vui.MaxBitsPerMbDenom = bs.ReadUE()
				h264Vui.Log2MaxMvLengthHorizontal = bs.ReadUE()
				h264Vui.Log2MaxMvLengthVertical = bs.ReadUE()
				h264Vui.NumReorderFrames = bs.ReadUE()
				h264Vui.MaxDecFrameBuffering = bs.ReadUE()
			}
	*/
}

func (h264Hrd *H264HrdParameters) Decode(bs *BitStream) {
	h264Hrd.CpbCntMinus1 = bs.ReadUE()
	h264Hrd.BitRateScale = bs.Uint8(4)
	h264Hrd.CpbSizeScale = bs.Uint8(4)

	h264Hrd.H264BitRateCpbSizeCbrFlag = make([]H264BitRateCpbSizeCbrFlag, h264Hrd.CpbCntMinus1+1)

	for i := 0; i <= int(h264Hrd.CpbCntMinus1); i++ {
		h264Hrd.H264BitRateCpbSizeCbrFlag[i].BitRateValueMinus1 = bs.ReadUE()
		h264Hrd.H264BitRateCpbSizeCbrFlag[i].CpbSizeValueMinus1 = bs.ReadUE()
		h264Hrd.H264BitRateCpbSizeCbrFlag[i].CbrFlag = bs.Uint8(1)
	}

	h264Hrd.InitialCpbRemovalDelayLengthMinus1 = bs.Uint8(5)
	h264Hrd.CpbRemovalDelayLengthMinus1 = bs.Uint8(5)
	h264Hrd.DpbOutputDelayLengthMinus1 = bs.Uint8(5)
	h264Hrd.TimeOffsetLength = bs.Uint8(5)
}
