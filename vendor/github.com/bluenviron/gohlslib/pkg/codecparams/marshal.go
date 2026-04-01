// Package codecparams contains utilities to deal with codec parameters.
package codecparams

import (
	"encoding/hex"
	"fmt"
	"strconv"
	"strings"

	"github.com/bluenviron/mediacommon/pkg/codecs/av1"
	"github.com/bluenviron/mediacommon/pkg/codecs/h265"

	"github.com/bluenviron/gohlslib/pkg/codecs"
)

func leadingZeros(v int, size int) string {
	out := strconv.FormatInt(int64(v), 10)
	if len(out) >= size {
		return out
	}

	out2 := ""
	for i := 0; i < (size - len(out)); i++ {
		out2 += "0"
	}

	return out2 + out
}

func av1EncodeTier(v bool) string {
	if v {
		return "H"
	}
	return "M"
}

func av1EncodeBool(v bool) string {
	if v {
		return "1"
	}
	return "0"
}

func h265EncodeProfileSpace(v uint8) string {
	if v >= 1 && v <= 3 {
		return string('A' + (v - 1))
	}
	return ""
}

func h265EncodeCompatibilityFlag(v [32]bool) string {
	var o uint32
	for i, b := range v {
		if b {
			o |= 1 << i
		}
	}
	return fmt.Sprintf("%x", o)
}

func h265EncodeGeneralTierFlag(v uint8) string {
	if v > 0 {
		return "H"
	}
	return "L"
}

func h265EncodeGeneralConstraintIndicatorFlags(v *h265.SPS_ProfileTierLevel) string {
	var ret []string

	var o1 uint8
	if v.GeneralProgressiveSourceFlag {
		o1 |= 1 << 7
	}
	if v.GeneralInterlacedSourceFlag {
		o1 |= 1 << 6
	}
	if v.GeneralNonPackedConstraintFlag {
		o1 |= 1 << 5
	}
	if v.GeneralFrameOnlyConstraintFlag {
		o1 |= 1 << 4
	}
	if v.GeneralMax12bitConstraintFlag {
		o1 |= 1 << 3
	}
	if v.GeneralMax10bitConstraintFlag {
		o1 |= 1 << 2
	}
	if v.GeneralMax8bitConstraintFlag {
		o1 |= 1 << 1
	}
	if v.GeneralMax422ChromeConstraintFlag {
		o1 |= 1 << 0
	}

	ret = append(ret, fmt.Sprintf("%x", o1))

	var o2 uint8
	if v.GeneralMax420ChromaConstraintFlag {
		o2 |= 1 << 7
	}
	if v.GeneralMaxMonochromeConstraintFlag {
		o2 |= 1 << 6
	}
	if v.GeneralIntraConstraintFlag {
		o2 |= 1 << 5
	}
	if v.GeneralOnePictureOnlyConstraintFlag {
		o2 |= 1 << 4
	}
	if v.GeneralLowerBitRateConstraintFlag {
		o2 |= 1 << 3
	}
	if v.GeneralMax14BitConstraintFlag {
		o2 |= 1 << 2
	}

	if o2 != 0 {
		ret = append(ret, fmt.Sprintf("%x", o2))
	}

	return strings.Join(ret, ".")
}

// Marshal generates codec parameters of given tracks.
func Marshal(codec codecs.Codec) string {
	switch codec := codec.(type) {
	case *codecs.AV1:
		var sh av1.SequenceHeader
		err := sh.Unmarshal(codec.SequenceHeader)
		if err == nil {
			v := "av01." +
				strconv.FormatInt(int64(sh.SeqProfile), 10) + "." +
				leadingZeros(int(sh.SeqLevelIdx[0]), 2) +
				av1EncodeTier(sh.SeqTier[0]) + "." +
				leadingZeros(sh.ColorConfig.BitDepth, 2) + "." +
				av1EncodeBool(sh.ColorConfig.MonoChrome) + "." +
				av1EncodeBool(sh.ColorConfig.SubsamplingX) +
				av1EncodeBool(sh.ColorConfig.SubsamplingY) +
				strconv.FormatInt(int64(sh.ColorConfig.ChromaSamplePosition), 10) + "."

			if sh.ColorConfig.ColorDescriptionPresentFlag {
				v += leadingZeros(int(sh.ColorConfig.ColorPrimaries), 2) + "." +
					leadingZeros(int(sh.ColorConfig.TransferCharacteristics), 2) + "." +
					leadingZeros(int(sh.ColorConfig.MatrixCoefficients), 2) + "." +
					av1EncodeBool(sh.ColorConfig.ColorRange)
			} else {
				v += "01.01.01.0"
			}

			return v
		}

	case *codecs.VP9:
		return "vp09." +
			leadingZeros(int(codec.Profile), 2) + "." +
			"10." + // level
			leadingZeros(int(codec.BitDepth), 2)

	case *codecs.H265:
		var sps h265.SPS
		err := sps.Unmarshal(codec.SPS)
		if err == nil {
			return "hvc1." +
				h265EncodeProfileSpace(sps.ProfileTierLevel.GeneralProfileSpace) +
				strconv.FormatInt(int64(sps.ProfileTierLevel.GeneralProfileIdc), 10) + "." +
				h265EncodeCompatibilityFlag(sps.ProfileTierLevel.GeneralProfileCompatibilityFlag) + "." +
				h265EncodeGeneralTierFlag(sps.ProfileTierLevel.GeneralTierFlag) +
				strconv.FormatInt(int64(sps.ProfileTierLevel.GeneralLevelIdc), 10) + "." +
				h265EncodeGeneralConstraintIndicatorFlags(&sps.ProfileTierLevel)
		}

	case *codecs.H264:
		if len(codec.SPS) >= 4 {
			return "avc1." + hex.EncodeToString(codec.SPS[1:4])
		}

	case *codecs.Opus:
		return "opus"

	case *codecs.MPEG4Audio:
		// https://developer.mozilla.org/en-US/docs/Web/Media/Formats/codecs_parameter
		return "mp4a.40." + strconv.FormatInt(int64(codec.Config.Type), 10)
	}

	return ""
}
