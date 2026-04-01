package hevc

import (
	"encoding/binary"
	"fmt"
)

// NaluType - HEVC nal type according to ISO/IEC 23008-2 Table 7.1
type NaluType uint16

// HEVC NALU types
const (
	NALU_TRAIL_N = NaluType(0)
	NALU_TRAIL_R = NaluType(1)
	NALU_TSA_N   = NaluType(2)
	NALU_TSA_R   = NaluType(3)
	NALU_STSA_N  = NaluType(4)
	NALU_STSA_R  = NaluType(5)
	NALU_RADL_N  = NaluType(6)
	NALU_RADL_R  = NaluType(7)
	NALU_RASL_N  = NaluType(8)
	NALU_RASL_R  = NaluType(9)
	// BLA_W_LP and the following types are Random Access
	NALU_BLA_W_LP   = NaluType(16)
	NALU_BLA_W_RADL = NaluType(17)
	NALU_BLA_N_LP   = NaluType(18)
	NALU_IDR_W_RADL = NaluType(19)
	NALU_IDR_N_LP   = NaluType(20)
	NALU_CRA        = NaluType(21)
	// Reserved IRAP VCL NAL unit types
	NALU_IRAP_VCL22 = NaluType(22)
	NALU_IRAP_VCL23 = NaluType(23)
	// NALU_VPS - VideoParameterSet NAL Unit
	NALU_VPS = NaluType(32)
	// NALU_SPS - SequenceParameterSet NAL Unit
	NALU_SPS = NaluType(33)
	// NALU_PPS - PictureParameterSet NAL Unit
	NALU_PPS = NaluType(34)
	// NALU_AUD - AccessUnitDelimiter NAL Unit
	NALU_AUD = NaluType(35)
	//NALU_EOS - End of Sequence NAL Unit
	NALU_EOS = NaluType(36)
	//NALU_EOB - End of Bitstream NAL Unit
	NALU_EOB = NaluType(37)
	//NALU_FD - Filler data NAL Unit
	NALU_FD = NaluType(38)
	//NALU_SEI_PREFIX - Prefix SEI NAL Unit
	NALU_SEI_PREFIX = NaluType(39)
	//NALU_SEI_SUFFIX - Suffix SEI NAL Unit
	NALU_SEI_SUFFIX = NaluType(40)

	highestVideoNaluType = 31
)

func (n NaluType) String() string {
	switch n {
	case NALU_TRAIL_N, NALU_TRAIL_R:
		return fmt.Sprintf("NonRAP_Trail_%d", n)
	case NALU_TSA_N, NALU_TSA_R:
		return fmt.Sprintf("NonRAP_TSA_%d", n)
	case NALU_STSA_N, NALU_STSA_R:
		return fmt.Sprintf("NonRAP_STSA_%d", n)
	case NALU_RADL_N, NALU_RADL_R:
		return fmt.Sprintf("NonRAP_RADL_%d", n)
	case NALU_RASL_N, NALU_RASL_R:
		return fmt.Sprintf("NonRAP_RASL_%d", n)
	case NALU_BLA_N_LP, NALU_BLA_W_LP, NALU_BLA_W_RADL:
		return fmt.Sprintf("RAP_BLA_%d", n)
	case NALU_IDR_N_LP, NALU_IDR_W_RADL:
		return fmt.Sprintf("RAP_IDR_%d", n)
	case NALU_CRA:
		return fmt.Sprintf("RAP_CRA_%d", n)
	case NALU_VPS:
		return fmt.Sprintf("VPS_%d", n)
	case NALU_SPS:
		return fmt.Sprintf("SPS_%d", n)
	case NALU_PPS:
		return fmt.Sprintf("PPS_%d", n)
	case NALU_AUD:
		return fmt.Sprintf("AUD_%d", n)
	case NALU_SEI_PREFIX, NALU_SEI_SUFFIX:
		return fmt.Sprintf("SEI_%d", n)
	default:
		return fmt.Sprintf("Other_%d", n)
	}
}

// GetNaluType - extract NALU type from first byte of NALU Header
func GetNaluType(naluHeaderStart byte) NaluType {
	return NaluType((naluHeaderStart >> 1) & 0x3f)
}

// FindNaluTypes - find list of nalu types in sample
func FindNaluTypes(sample []byte) []NaluType {
	naluList := make([]NaluType, 0)
	length := len(sample)
	if length < 4 {
		return naluList
	}
	var pos uint32 = 0
	for pos < uint32(length-4) {
		naluLength := binary.BigEndian.Uint32(sample[pos : pos+4])
		pos += 4
		naluType := GetNaluType(sample[pos])
		naluList = append(naluList, naluType)
		pos += naluLength
	}
	return naluList
}

// FindNaluTypesUpToFirstVideoNalu - all nalu types up to first video nalu
func FindNaluTypesUpToFirstVideoNalu(sample []byte) []NaluType {
	naluList := make([]NaluType, 0)
	length := len(sample)
	if length < 4 {
		return naluList
	}
	var pos uint32 = 0
	for pos < uint32(length-4) {
		naluLength := binary.BigEndian.Uint32(sample[pos : pos+4])
		pos += 4
		naluType := GetNaluType(sample[pos])
		naluList = append(naluList, naluType)
		pos += naluLength
		if naluType <= highestVideoNaluType {
			break // Video has started
		}
	}
	return naluList
}

// ContainsNaluType - is specific NaluType present in sample
func ContainsNaluType(sample []byte, specificNaluType NaluType) bool {
	var pos uint32 = 0
	length := len(sample)
	if length < 4 {
		return false
	}
	for pos < uint32(length-4) {
		naluLength := binary.BigEndian.Uint32(sample[pos : pos+4])
		pos += 4
		naluType := GetNaluType(sample[pos])
		if naluType == specificNaluType {
			return true
		}
		pos += naluLength
	}
	return false
}

// IsRAPSample - is Random Access picture (NALU 16-23)
func IsRAPSample(sample []byte) bool {
	for _, naluType := range FindNaluTypes(sample) {
		if 16 <= naluType && naluType <= 23 {
			return true
		}
	}
	return false
}

// IsIDRSample - is IDR picture (NALU 19-20)
func IsIDRSample(sample []byte) bool {
	for _, naluType := range FindNaluTypes(sample) {
		if 19 <= naluType && naluType <= 20 {
			return true
		}
	}
	return false
}

// HasParameterSets - Check if HEVC VPS, SPS and PPS are present
func HasParameterSets(b []byte) bool {
	naluTypeList := FindNaluTypesUpToFirstVideoNalu(b)
	var hasVPS, hasSPS, hasPPS bool
	for _, naluType := range naluTypeList {
		switch naluType {
		case NALU_VPS:
			hasVPS = true
		case NALU_SPS:
			hasSPS = true
		case NALU_PPS:
			hasPPS = true
		}
		if hasVPS && hasSPS && hasPPS {
			return true
		}
	}
	return false
}

// GetParameterSets - get (multiple) VPS,  SPS, and PPS from a sample
func GetParameterSets(sample []byte) (vps, sps, pps [][]byte) {
	sampleLength := uint32(len(sample))
	var pos uint32 = 0
naluLoop:
	for {
		if pos >= sampleLength {
			break
		}
		naluLength := binary.BigEndian.Uint32(sample[pos : pos+4])
		pos += 4
		switch naluType := GetNaluType(sample[pos]); {
		case naluType == NALU_VPS:
			vps = append(vps, sample[pos:pos+naluLength])
		case naluType == NALU_SPS:
			sps = append(sps, sample[pos:pos+naluLength])
		case naluType == NALU_PPS:
			pps = append(pps, sample[pos:pos+naluLength])
		case naluType <= highestVideoNaluType:
			break naluLoop
		}
		pos += naluLength
	}
	return vps, sps, pps
}
