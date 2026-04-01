package av1

import (
	"fmt"

	"github.com/bluenviron/mediacommon/pkg/bits"
)

// SequenceHeader_ColorPrimaries is a ColorPrimaries value.
type SequenceHeader_ColorPrimaries uint8 //nolint:revive

const (
	SequenceHeader_ColorPrimaries_CP_BT_709      SequenceHeader_ColorPrimaries = 1 //nolint:revive
	SequenceHeader_ColorPrimaries_CP_UNSPECIFIED SequenceHeader_ColorPrimaries = 2 //nolint:revive
)

// SequenceHeader_TransferCharacteristics is a TransferCharacteristics value.
type SequenceHeader_TransferCharacteristics uint8 //nolint:revive

const (
	SequenceHeader_TransferCharacteristics_TC_UNSPECIFIED SequenceHeader_TransferCharacteristics = 2  //nolint:revive
	SequenceHeader_TransferCharacteristics_TC_SRGB        SequenceHeader_TransferCharacteristics = 13 //nolint:revive
)

// SequenceHeader_MatrixCoefficients is a MatrixCoefficients value.
type SequenceHeader_MatrixCoefficients uint8 //nolint:revive

const (
	SequenceHeader_MatrixCoefficients_MC_IDENTITY    SequenceHeader_MatrixCoefficients = 0 //nolint:revive
	SequenceHeader_MatrixCoefficients_MC_UNSPECIFIED SequenceHeader_MatrixCoefficients = 2 //nolint:revive
)

// SequenceHeader_ChromaSamplePosition is a ChromaSamplePosition value.
type SequenceHeader_ChromaSamplePosition uint8 //nolint:revive

const (
	SequenceHeader_ChromaSamplePosition_CSP_UNKNOWN SequenceHeader_ChromaSamplePosition = 0 //nolint:revive
)

// SequenceHeader_SeqForceScreenContentTools is a SeqForceScreenContentTools value.
type SequenceHeader_SeqForceScreenContentTools uint8 //nolint:revive

const (
	SequenceHeader_SeqForceScreenContentTools_SELECT_SCREEN_CONTENT_TOOLS SequenceHeader_SeqForceScreenContentTools = 2 //nolint:revive,lll
)

// SequenceHeader_SeqForceIntegerMv is a SeqForceIntegerMv value.
type SequenceHeader_SeqForceIntegerMv uint8 //nolint:revive

const (
	SequenceHeader_SeqForceIntegerMv_SELECT_INTEGER_MV SequenceHeader_SeqForceIntegerMv = 2 //nolint:revive
)

// SequenceHeader_ColorConfig is a color configuration of a sequence header.
type SequenceHeader_ColorConfig struct { //nolint:revive
	HighBitDepth                bool
	TwelveBit                   bool
	BitDepth                    int
	MonoChrome                  bool
	ColorDescriptionPresentFlag bool
	ColorPrimaries              SequenceHeader_ColorPrimaries
	TransferCharacteristics     SequenceHeader_TransferCharacteristics
	MatrixCoefficients          SequenceHeader_MatrixCoefficients
	ColorRange                  bool
	SubsamplingX                bool
	SubsamplingY                bool
	ChromaSamplePosition        SequenceHeader_ChromaSamplePosition
}

func (c *SequenceHeader_ColorConfig) unmarshal(seqProfile uint8, buf []byte, pos *int) error {
	var err error
	c.HighBitDepth, err = bits.ReadFlag(buf, pos)
	if err != nil {
		return err
	}

	if seqProfile == 2 && c.HighBitDepth {
		c.TwelveBit, err = bits.ReadFlag(buf, pos)
		if err != nil {
			return err
		}

		if c.TwelveBit {
			c.BitDepth = 12
		} else {
			c.BitDepth = 10
		}
	} else if seqProfile <= 2 {
		if c.HighBitDepth {
			c.BitDepth = 10
		} else {
			c.BitDepth = 8
		}
	}

	if seqProfile == 1 {
		c.MonoChrome = false
	} else {
		c.MonoChrome, err = bits.ReadFlag(buf, pos)
		if err != nil {
			return err
		}
	}

	c.ColorDescriptionPresentFlag, err = bits.ReadFlag(buf, pos)
	if err != nil {
		return err
	}

	if c.ColorDescriptionPresentFlag {
		err = bits.HasSpace(buf, *pos, 24)
		if err != nil {
			return err
		}

		c.ColorPrimaries = SequenceHeader_ColorPrimaries(bits.ReadBitsUnsafe(buf, pos, 8))
		c.TransferCharacteristics = SequenceHeader_TransferCharacteristics(bits.ReadBitsUnsafe(buf, pos, 8))
		c.MatrixCoefficients = SequenceHeader_MatrixCoefficients(bits.ReadBitsUnsafe(buf, pos, 8))
	} else {
		c.ColorPrimaries = SequenceHeader_ColorPrimaries_CP_UNSPECIFIED
		c.TransferCharacteristics = SequenceHeader_TransferCharacteristics_TC_UNSPECIFIED
		c.MatrixCoefficients = SequenceHeader_MatrixCoefficients_MC_UNSPECIFIED
	}

	switch {
	case c.MonoChrome:
		c.ColorRange, err = bits.ReadFlag(buf, pos)
		if err != nil {
			return err
		}

		c.SubsamplingX = true
		c.SubsamplingY = true
		c.ChromaSamplePosition = SequenceHeader_ChromaSamplePosition_CSP_UNKNOWN
	case c.ColorPrimaries == SequenceHeader_ColorPrimaries_CP_BT_709 &&
		c.TransferCharacteristics == SequenceHeader_TransferCharacteristics_TC_SRGB &&
		c.MatrixCoefficients == SequenceHeader_MatrixCoefficients_MC_IDENTITY:
		c.ColorRange = true
		c.SubsamplingX = false
		c.SubsamplingY = false
	default:
		c.ColorRange, err = bits.ReadFlag(buf, pos)
		if err != nil {
			return err
		}

		switch {
		case seqProfile == 0:
			c.SubsamplingX = true
			c.SubsamplingY = true
		case seqProfile == 1:
			c.SubsamplingX = false
			c.SubsamplingY = false
		default:
			if c.BitDepth == 12 {
				c.SubsamplingX, err = bits.ReadFlag(buf, pos)
				if err != nil {
					return err
				}

				if c.SubsamplingX {
					c.SubsamplingY, err = bits.ReadFlag(buf, pos)
					if err != nil {
						return err
					}
				} else {
					c.SubsamplingY = false
				}
			} else {
				c.SubsamplingX = true
				c.SubsamplingY = false
			}
		}

		if c.SubsamplingX && c.SubsamplingY {
			tmp, err := bits.ReadBits(buf, pos, 2)
			if err != nil {
				return err
			}
			c.ChromaSamplePosition = SequenceHeader_ChromaSamplePosition(tmp)
		}
	}

	return nil
}

// SequenceHeader is a AV1 Sequence header OBU.
// Specification: https://aomediacodec.github.io/av1-spec/#sequence-header-obu-syntax
type SequenceHeader struct {
	SeqProfile                     uint8
	StillPicture                   bool
	ReducedStillPictureHeader      bool
	TimingInfoPresentFlag          bool
	DecoderModelInfoPresentFlag    bool
	InitialDisplayDelayPresentFlag bool
	OperatingPointsCntMinus1       uint8
	OperatingPointIdc              []uint16
	SeqLevelIdx                    []uint8
	SeqTier                        []bool
	DecoderModelPresentForThisOp   []bool
	InitialDisplayPresentForThisOp []bool
	InitialDisplayDelayMinus1      []uint8
	MaxFrameWidthMinus1            uint32
	MaxFrameHeightMinus1           uint32
	FrameIDNumbersPresentFlag      bool
	Use128x128Superblock           bool
	EnableFilterIntra              bool
	EnableIntraEdgeFilter          bool
	EnableInterintraCompound       bool
	EnableMaskedCompound           bool
	EnableWarpedMotion             bool
	EnableDualFilter               bool
	EnableOrderHint                bool
	EnableJntComp                  bool
	EnableRefFrameMvs              bool
	SeqChooseScreenContentTools    bool
	SeqForceScreenContentTools     SequenceHeader_SeqForceScreenContentTools
	SeqChooseIntegerMv             bool
	SeqForceIntegerMv              SequenceHeader_SeqForceIntegerMv
	OrderHintBitsMinus1            uint8
	EnableSuperRes                 bool
	EnableCdef                     bool
	EnableRestoration              bool
	ColorConfig                    SequenceHeader_ColorConfig
}

// Unmarshal decodes a SequenceHeader.
func (h *SequenceHeader) Unmarshal(buf []byte) error {
	var oh OBUHeader
	err := oh.Unmarshal(buf)
	if err != nil {
		return err
	}
	buf = buf[1:]

	if oh.HasSize {
		var size uint
		var sizeN int
		size, sizeN, err = LEB128Unmarshal(buf)
		if err != nil {
			return err
		}

		buf = buf[sizeN:]
		if len(buf) != int(size) {
			return fmt.Errorf("wrong buffer size: expected %d, got %d", size, len(buf))
		}
	}

	pos := 0

	err = bits.HasSpace(buf, pos, 5)
	if err != nil {
		return err
	}

	h.SeqProfile = uint8(bits.ReadBitsUnsafe(buf, &pos, 3))
	h.StillPicture = bits.ReadFlagUnsafe(buf, &pos)
	h.ReducedStillPictureHeader = bits.ReadFlagUnsafe(buf, &pos)

	if h.ReducedStillPictureHeader {
		h.TimingInfoPresentFlag = false
		h.DecoderModelInfoPresentFlag = false
		h.InitialDisplayDelayPresentFlag = false
		h.OperatingPointsCntMinus1 = 0
		h.OperatingPointIdc = []uint16{0}

		err = bits.HasSpace(buf, pos, 5)
		if err != nil {
			return err
		}

		h.SeqLevelIdx = []uint8{uint8(bits.ReadBitsUnsafe(buf, &pos, 5))}
		h.SeqTier = []bool{false}
		h.DecoderModelPresentForThisOp = []bool{false}
		h.InitialDisplayPresentForThisOp = []bool{false}
	} else {
		h.TimingInfoPresentFlag, err = bits.ReadFlag(buf, &pos)
		if err != nil {
			return err
		}

		if h.TimingInfoPresentFlag {
			return fmt.Errorf("timing_info_present_flag is not supported yet")
		}
		h.DecoderModelInfoPresentFlag = false

		err = bits.HasSpace(buf, pos, 6)
		if err != nil {
			return err
		}

		h.InitialDisplayDelayPresentFlag = bits.ReadFlagUnsafe(buf, &pos)
		h.OperatingPointsCntMinus1 = uint8(bits.ReadBitsUnsafe(buf, &pos, 5))

		h.OperatingPointIdc = make([]uint16, h.OperatingPointsCntMinus1+1)
		h.SeqLevelIdx = make([]uint8, h.OperatingPointsCntMinus1+1)
		h.SeqTier = make([]bool, h.OperatingPointsCntMinus1+1)
		h.DecoderModelPresentForThisOp = make([]bool, h.OperatingPointsCntMinus1+1)
		h.InitialDisplayPresentForThisOp = make([]bool, h.OperatingPointsCntMinus1+1)
		h.InitialDisplayDelayMinus1 = make([]uint8, h.OperatingPointsCntMinus1+1)

		for i := uint8(0); i <= h.OperatingPointsCntMinus1; i++ {
			err = bits.HasSpace(buf, pos, 17)
			if err != nil {
				return err
			}

			h.OperatingPointIdc[i] = uint16(bits.ReadBitsUnsafe(buf, &pos, 12))
			h.SeqLevelIdx[i] = uint8(bits.ReadBitsUnsafe(buf, &pos, 5))

			if h.SeqLevelIdx[i] > 7 {
				h.SeqTier[i], err = bits.ReadFlag(buf, &pos)
				if err != nil {
					return err
				}
			} else {
				h.SeqTier[i] = false
			}

			if h.DecoderModelInfoPresentFlag {
				return fmt.Errorf("decoder_model_info_present_flag is not supported yet")
			}
			h.DecoderModelPresentForThisOp[i] = false

			if h.InitialDisplayDelayPresentFlag {
				h.InitialDisplayPresentForThisOp[i], err = bits.ReadFlag(buf, &pos)
				if err != nil {
					return err
				}

				if h.InitialDisplayPresentForThisOp[i] {
					var tmp uint64
					tmp, err = bits.ReadBits(buf, &pos, 4)
					if err != nil {
						return err
					}
					h.InitialDisplayDelayMinus1[i] = uint8(tmp)
				}
				return fmt.Errorf("initial_display_delay_present_flag is not supported yet")
			}
		}
	}

	err = bits.HasSpace(buf, pos, 8)
	if err != nil {
		return err
	}

	frameWidthBitsMinus1 := int(bits.ReadBitsUnsafe(buf, &pos, 4))
	frameHeightBitsMinus1 := int(bits.ReadBitsUnsafe(buf, &pos, 4))

	n1 := (frameWidthBitsMinus1 + 1)
	n2 := (frameHeightBitsMinus1 + 1)

	err = bits.HasSpace(buf, pos, n1+n2)
	if err != nil {
		return err
	}

	h.MaxFrameWidthMinus1 = uint32(bits.ReadBitsUnsafe(buf, &pos, n1))
	h.MaxFrameHeightMinus1 = uint32(bits.ReadBitsUnsafe(buf, &pos, n2))

	if h.ReducedStillPictureHeader {
		h.FrameIDNumbersPresentFlag = false
	} else {
		h.FrameIDNumbersPresentFlag, err = bits.ReadFlag(buf, &pos)
		if err != nil {
			return err
		}

		if h.FrameIDNumbersPresentFlag {
			return fmt.Errorf("frame_id_numbers_present_flag is not supported yet")
		}
	}

	err = bits.HasSpace(buf, pos, 3)
	if err != nil {
		return err
	}

	h.Use128x128Superblock = bits.ReadFlagUnsafe(buf, &pos)
	h.EnableFilterIntra = bits.ReadFlagUnsafe(buf, &pos)
	h.EnableIntraEdgeFilter = bits.ReadFlagUnsafe(buf, &pos)

	if h.ReducedStillPictureHeader {
		h.EnableInterintraCompound = false
		h.EnableMaskedCompound = false
		h.EnableWarpedMotion = false
		h.EnableDualFilter = false
		h.EnableOrderHint = false
		h.EnableJntComp = false
		h.EnableRefFrameMvs = false
		h.SeqForceScreenContentTools = SequenceHeader_SeqForceScreenContentTools_SELECT_SCREEN_CONTENT_TOOLS
		h.SeqForceIntegerMv = SequenceHeader_SeqForceIntegerMv_SELECT_INTEGER_MV
	} else {
		err = bits.HasSpace(buf, pos, 5)
		if err != nil {
			return err
		}

		h.EnableInterintraCompound = bits.ReadFlagUnsafe(buf, &pos)
		h.EnableMaskedCompound = bits.ReadFlagUnsafe(buf, &pos)
		h.EnableWarpedMotion = bits.ReadFlagUnsafe(buf, &pos)
		h.EnableDualFilter = bits.ReadFlagUnsafe(buf, &pos)
		h.EnableOrderHint = bits.ReadFlagUnsafe(buf, &pos)

		if h.EnableOrderHint {
			err = bits.HasSpace(buf, pos, 2)
			if err != nil {
				return err
			}

			h.EnableJntComp = bits.ReadFlagUnsafe(buf, &pos)
			h.EnableRefFrameMvs = bits.ReadFlagUnsafe(buf, &pos)
		}

		h.SeqChooseScreenContentTools, err = bits.ReadFlag(buf, &pos)
		if err != nil {
			return err
		}

		if h.SeqChooseScreenContentTools {
			h.SeqForceScreenContentTools = SequenceHeader_SeqForceScreenContentTools_SELECT_SCREEN_CONTENT_TOOLS
		} else {
			var tmp uint64
			tmp, err = bits.ReadBits(buf, &pos, 1)
			if err != nil {
				return err
			}
			h.SeqForceScreenContentTools = SequenceHeader_SeqForceScreenContentTools(tmp)
		}

		if h.SeqForceScreenContentTools > 0 {
			h.SeqChooseIntegerMv, err = bits.ReadFlag(buf, &pos)
			if err != nil {
				return err
			}

			if h.SeqChooseIntegerMv {
				h.SeqForceIntegerMv = SequenceHeader_SeqForceIntegerMv_SELECT_INTEGER_MV
			} else {
				var tmp uint64
				tmp, err = bits.ReadBits(buf, &pos, 1)
				if err != nil {
					return err
				}
				h.SeqForceIntegerMv = SequenceHeader_SeqForceIntegerMv(tmp)
			}
		} else {
			h.SeqForceIntegerMv = SequenceHeader_SeqForceIntegerMv_SELECT_INTEGER_MV
		}

		if h.EnableOrderHint {
			var tmp uint64
			tmp, err = bits.ReadBits(buf, &pos, 3)
			if err != nil {
				return err
			}
			h.OrderHintBitsMinus1 = uint8(tmp)
		}
	}

	err = bits.HasSpace(buf, pos, 3)
	if err != nil {
		return err
	}

	h.EnableSuperRes = bits.ReadFlagUnsafe(buf, &pos)
	h.EnableCdef = bits.ReadFlagUnsafe(buf, &pos)
	h.EnableRestoration = bits.ReadFlagUnsafe(buf, &pos)

	return h.ColorConfig.unmarshal(h.SeqProfile, buf, &pos)
}

// Width returns the video width.
func (h SequenceHeader) Width() int {
	return int(h.MaxFrameWidthMinus1 + 1)
}

// Height returns the video height.
func (h SequenceHeader) Height() int {
	return int(h.MaxFrameHeightMinus1 + 1)
}
