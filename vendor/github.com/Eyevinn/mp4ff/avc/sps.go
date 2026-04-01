package avc

import (
	"bytes"
	"errors"
	"fmt"

	"github.com/Eyevinn/mp4ff/bits"
)

// ExtendedSAR - Extended Sample Aspect Ratio Code
const ExtendedSAR = 255

// SPS errors
var (
	ErrNotSPS = errors.New("Not an SPS NAL unit")
)

// SPS - AVC SPS parameters
type SPS struct {
	Profile                         uint32
	ProfileCompatibility            uint32
	Level                           uint32
	ParameterID                     uint32
	ChromaFormatIDC                 byte
	SeparateColourPlaneFlag         bool
	BitDepthLumaMinus8              uint
	BitDepthChromaMinus8            uint
	QPPrimeYZeroTransformBypassFlag bool
	SeqScalingMatrixPresentFlag     bool
	SeqScalingLists                 []ScalingList
	Log2MaxFrameNumMinus4           uint
	PicOrderCntType                 uint
	Log2MaxPicOrderCntLsbMinus4     uint
	DeltaPicOrderAlwaysZeroFlag     bool
	OffsetForNonRefPic              uint
	OffsetForTopToBottomField       uint
	RefFramesInPicOrderCntCycle     []uint
	NumRefFrames                    uint
	GapsInFrameNumValueAllowedFlag  bool
	FrameMbsOnlyFlag                bool
	MbAdaptiveFrameFieldFlag        bool
	Direct8x8InferenceFlag          bool
	FrameCroppingFlag               bool
	FrameCropLeftOffset             uint
	FrameCropRightOffset            uint
	FrameCropTopOffset              uint
	FrameCropBottomOffset           uint
	Width                           uint
	Height                          uint
	NrBytesBeforeVUI                int
	NrBytesRead                     int
	VUI                             *VUIParameters
}

// ScalingList - 4x4 or 8x8 Scaling lists. Nil if not present
type ScalingList []int

// VUIParameters - extra parameters according to 14496-10, E.1
type VUIParameters struct {
	SampleAspectRatioWidth             uint
	SampleAspectRatioHeight            uint
	OverscanInfoPresentFlag            bool
	OverscanAppropriateFlag            bool
	VideoSignalTypePresentFlag         bool
	VideoFormat                        uint
	VideoFullRangeFlag                 bool
	ColourDescriptionFlag              bool
	ColourPrimaries                    uint
	TransferCharacteristics            uint
	MatrixCoefficients                 uint
	ChromaLocInfoPresentFlag           bool
	ChromaSampleLocTypeTopField        uint
	ChromaSampleLocTypeBottomField     uint
	TimingInfoPresentFlag              bool
	NumUnitsInTick                     uint
	TimeScale                          uint
	FixedFrameRateFlag                 bool
	NalHrdParametersPresentFlag        bool
	NalHrdParameters                   *HrdParameters
	VclHrdParametersPresentFlag        bool
	VclHrdParameters                   *HrdParameters
	LowDelayHrdFlag                    bool // Only present with HrdParameters
	PicStructPresentFlag               bool
	BitstreamRestrictionFlag           bool
	MotionVectorsOverPicBoundariesFlag bool
	MaxBytesPerPicDenom                uint
	MaxBitsPerMbDenom                  uint
	Log2MaxMvLengthHorizontal          uint
	Log2MaxMvLengthVertical            uint
	MaxNumReorderFrames                uint
	MaxDecFrameBuffering               uint
}

// HrdParameters inside VUI
type HrdParameters struct {
	CpbCountMinus1                     uint
	BitRateScale                       uint
	CpbSizeScale                       uint
	CpbEntries                         []CpbEntry
	InitialCpbRemovalDelayLengthMinus1 uint
	CpbRemovalDelayLengthMinus1        uint
	DpbOutputDelayLengthMinus1         uint
	TimeOffsetLength                   uint
}

// CpbEntry inside HrdParameters
type CpbEntry struct {
	BitRateValueMinus1 uint
	CpbSizeValueMinus1 uint
	CbrFlag            bool
}

// ParseSPSNALUnit - Parse AVC SPS NAL unit starting with NAL header
func ParseSPSNALUnit(data []byte, parseVUIBeyondAspectRatio bool) (*SPS, error) {

	sps := &SPS{}

	rd := bytes.NewReader(data)
	reader := bits.NewEBSPReader(rd)
	// Note! First byte is NAL Header

	nalHdr := reader.Read(8)
	nalType := GetNaluType(byte(nalHdr))
	if nalType != NALU_SPS {
		return nil, ErrNotSPS
	}

	sps.Profile = uint32(reader.Read(8))
	sps.ProfileCompatibility = uint32(reader.Read(8))
	sps.Level = uint32(reader.Read(8))
	sps.ParameterID = uint32(reader.ReadExpGolomb())
	sps.ChromaFormatIDC = 1 // Default value if no explicit value present

	if sps.Profile == 138 {
		sps.ChromaFormatIDC = 0
	}

	// The following table is from 14496-10:2020 Section 7.3.2.1.1
	switch sps.Profile {
	case 100, 110, 122, 244, 44, 83, 86, 118, 128, 138, 139, 134, 135:
		sps.ChromaFormatIDC = byte(reader.ReadExpGolomb())
		if sps.ChromaFormatIDC == 3 {
			sps.SeparateColourPlaneFlag = reader.ReadFlag()
		}
		sps.BitDepthLumaMinus8 = reader.ReadExpGolomb()
		sps.BitDepthChromaMinus8 = reader.ReadExpGolomb()
		sps.QPPrimeYZeroTransformBypassFlag = reader.ReadFlag()
		sps.SeqScalingMatrixPresentFlag = reader.ReadFlag()
		if sps.SeqScalingMatrixPresentFlag {
			nrScalingLists := 12
			if sps.ChromaFormatIDC != 3 {
				nrScalingLists = 8
			}
			sps.SeqScalingLists = make([]ScalingList, nrScalingLists)

			for i := 0; i < nrScalingLists; i++ {
				seqScalingPresent := reader.ReadFlag()
				if !seqScalingPresent {
					sps.SeqScalingLists[i] = nil
					continue
				}
				sizeOfScalingList := 16 // 4x4 for i < 6
				if i >= 6 {
					sizeOfScalingList = 64 // 8x8 for i >= 6
				}
				sps.SeqScalingLists[i] = readScalingList(reader, sizeOfScalingList)
			}
		}
	default:
		// Empty
	}

	sps.Log2MaxFrameNumMinus4 = reader.ReadExpGolomb()
	sps.PicOrderCntType = reader.ReadExpGolomb()
	if sps.PicOrderCntType == 0 {
		sps.Log2MaxPicOrderCntLsbMinus4 = reader.ReadExpGolomb()
	} else if sps.PicOrderCntType == 1 {
		sps.DeltaPicOrderAlwaysZeroFlag = reader.ReadFlag()
		sps.OffsetForNonRefPic = reader.ReadExpGolomb()
		sps.OffsetForTopToBottomField = reader.ReadExpGolomb()
		numRefFramesInPicOrderCntCycle := reader.ReadExpGolomb()
		sps.RefFramesInPicOrderCntCycle = make([]uint, numRefFramesInPicOrderCntCycle)
		for i := 0; i < int(numRefFramesInPicOrderCntCycle); i++ {
			sps.RefFramesInPicOrderCntCycle[i] = reader.ReadExpGolomb()
		}
	}

	sps.NumRefFrames = reader.ReadExpGolomb()
	sps.GapsInFrameNumValueAllowedFlag = reader.ReadFlag()

	picWidthInMbsUnitsMinus1 := reader.ReadExpGolomb()
	picHeightInMbsUnitsMinus1 := reader.ReadExpGolomb()

	sps.Width = (picWidthInMbsUnitsMinus1 + 1) * 16
	sps.Height = (picHeightInMbsUnitsMinus1 + 1) * 16

	sps.FrameMbsOnlyFlag = reader.ReadFlag()
	if !sps.FrameMbsOnlyFlag {
		sps.MbAdaptiveFrameFieldFlag = reader.ReadFlag()
	}
	sps.Direct8x8InferenceFlag = reader.ReadFlag()
	sps.FrameCroppingFlag = reader.ReadFlag()
	var cropUnitX, cropUnitY uint
	var frameMbsOnly uint = 0
	if sps.FrameMbsOnlyFlag {
		frameMbsOnly = 1
	} else { // Interlaced so the height should be doubled
		sps.Height *= 2
	}
	if sps.FrameCroppingFlag {
		switch sps.ChromaFormatIDC {
		case 0:
			cropUnitX, cropUnitY = 1, 2-frameMbsOnly
		case 1:
			cropUnitX, cropUnitY = 2, 2*(2-frameMbsOnly)
		case 2:
			cropUnitX, cropUnitY = 2, 1*(2-frameMbsOnly)
		case 3: //This lacks one extra check?
			cropUnitX, cropUnitY = 1, 1*(2-frameMbsOnly)
		default:
			return nil, fmt.Errorf("Non-vaild chroma_format_idc value: %d", sps.ChromaFormatIDC)
		}

		sps.FrameCropLeftOffset = reader.ReadExpGolomb()
		sps.FrameCropRightOffset = reader.ReadExpGolomb()
		sps.FrameCropTopOffset = reader.ReadExpGolomb()
		sps.FrameCropBottomOffset = reader.ReadExpGolomb()

		frameCropWidth := sps.FrameCropLeftOffset + sps.FrameCropRightOffset
		frameCropHeight := sps.FrameCropTopOffset + sps.FrameCropBottomOffset

		sps.Width -= frameCropWidth * cropUnitX
		sps.Height -= frameCropHeight * cropUnitY
	}

	vuiParametersPresentFlag := reader.ReadFlag()
	sps.NrBytesBeforeVUI = reader.NrBytesRead()
	if vuiParametersPresentFlag {
		sps.VUI = parseVUI(reader, parseVUIBeyondAspectRatio)
	}
	sps.NrBytesRead = reader.NrBytesRead()

	return sps, reader.AccError()
}

// CpbDbpDelaysPresent signals if Cpb and Dbp can be found in Picture Timing SEI
func (s *SPS) CpbDpbDelaysPresent() bool {
	if s.VUI == nil {
		return false
	}
	return (s.VUI.NalHrdParametersPresentFlag ||
		s.VUI.VclHrdParametersPresentFlag)
}

// PicStructPresent signals if pic struct can be found in Picture Timing SEI
func (s *SPS) PicStructPresent() bool {
	if s.VUI == nil {
		return false
	}
	return s.VUI.PicStructPresentFlag
}

// ChromaArrayType as defined in Section 7.4.2.1.1 under separate_colour_plane_flag
func (s *SPS) ChromaArrayType() byte {
	if !s.SeparateColourPlaneFlag {
		return byte(s.ChromaFormatIDC)
	}
	return 0
}

// parseVUI - parse VUI (Visual Usability Information)
// if parseVUIBeyondAspectRatio is false, stop after AspectRatio has been parsed
func parseVUI(reader *bits.EBSPReader, parseVUIBeyondAspectRatio bool) *VUIParameters {
	vui := &VUIParameters{}
	var err error
	aspectRatioInfoPresentFlag := reader.ReadFlag()
	if aspectRatioInfoPresentFlag {
		aspectRatioIDC := reader.Read(8)
		if aspectRatioIDC == ExtendedSAR {
			vui.SampleAspectRatioWidth = reader.Read(16)
			vui.SampleAspectRatioHeight = reader.Read(16)
		} else {
			vui.SampleAspectRatioWidth, vui.SampleAspectRatioHeight, err = GetSARfromIDC(aspectRatioIDC)
			if err != nil {
				reader.SetError(fmt.Errorf("GetSARFromIDC: %w", err))
			}
		}
	}
	if !parseVUIBeyondAspectRatio {
		return vui
	}
	vui.OverscanInfoPresentFlag = reader.ReadFlag()
	if vui.OverscanInfoPresentFlag {
		vui.OverscanAppropriateFlag = reader.ReadFlag()
	}
	vui.VideoSignalTypePresentFlag = reader.ReadFlag()
	if vui.VideoSignalTypePresentFlag {
		vui.VideoFormat = reader.Read(3)
		vui.VideoFullRangeFlag = reader.ReadFlag()
		vui.ColourDescriptionFlag = reader.ReadFlag()
		if vui.ColourDescriptionFlag {
			vui.ColourPrimaries = reader.Read(8)
			vui.TransferCharacteristics = reader.Read(8)
			vui.MatrixCoefficients = reader.Read(8)
		}
	}
	vui.ChromaLocInfoPresentFlag = reader.ReadFlag()
	if vui.ChromaLocInfoPresentFlag {
		vui.ChromaSampleLocTypeTopField = reader.ReadExpGolomb()
		vui.ChromaSampleLocTypeBottomField = reader.ReadExpGolomb()
	}
	vui.TimingInfoPresentFlag = reader.ReadFlag()
	if vui.TimingInfoPresentFlag {
		vui.NumUnitsInTick = reader.Read(32)
		vui.TimeScale = reader.Read(32)
		vui.FixedFrameRateFlag = reader.ReadFlag()
	}
	vui.NalHrdParametersPresentFlag = reader.ReadFlag()
	if vui.NalHrdParametersPresentFlag {
		vui.NalHrdParameters = parseHrdParameters(reader)
	}
	vui.VclHrdParametersPresentFlag = reader.ReadFlag()
	if vui.VclHrdParametersPresentFlag {
		vui.VclHrdParameters = parseHrdParameters(reader)
	}
	if vui.NalHrdParametersPresentFlag || vui.VclHrdParametersPresentFlag {
		vui.LowDelayHrdFlag = reader.ReadFlag()
	}
	vui.PicStructPresentFlag = reader.ReadFlag()
	vui.BitstreamRestrictionFlag = reader.ReadFlag()
	if vui.BitstreamRestrictionFlag {
		vui.MotionVectorsOverPicBoundariesFlag = reader.ReadFlag()
		vui.MaxBytesPerPicDenom = reader.ReadExpGolomb()
		vui.MaxBitsPerMbDenom = reader.ReadExpGolomb()
		vui.Log2MaxMvLengthHorizontal = reader.ReadExpGolomb()
		vui.Log2MaxMvLengthVertical = reader.ReadExpGolomb()
		vui.MaxNumReorderFrames = reader.ReadExpGolomb()
		vui.MaxDecFrameBuffering = reader.ReadExpGolomb()
	}

	return vui
}

func parseHrdParameters(r *bits.EBSPReader) *HrdParameters {
	hp := &HrdParameters{}
	hp.CpbCountMinus1 = r.ReadExpGolomb()

	hp.BitRateScale = r.Read(4)
	hp.CpbSizeScale = r.Read(4)
	for schedSelIdx := uint(0); schedSelIdx <= hp.CpbCountMinus1; schedSelIdx++ {
		ce := CpbEntry{}
		ce.BitRateValueMinus1 = r.ReadExpGolomb()
		ce.CpbSizeValueMinus1 = r.ReadExpGolomb()
		ce.CbrFlag = r.ReadFlag()
		hp.CpbEntries = append(hp.CpbEntries, ce)
	}
	hp.InitialCpbRemovalDelayLengthMinus1 = r.Read(5)
	hp.CpbRemovalDelayLengthMinus1 = r.Read(5)
	hp.DpbOutputDelayLengthMinus1 = r.Read(5)
	hp.TimeOffsetLength = r.Read(5)
	return hp
}

// ConstraintFlags - return the four ConstraintFlag bits
func (a *SPS) ConstraintFlags() byte {
	return byte(a.ProfileCompatibility >> 4)
}

// GetSARfromIDC - get Sample Aspect Ratio from IDC index
func GetSARfromIDC(index uint) (uint, uint, error) {
	if index < 1 || index > 16 {
		return 0, 0, fmt.Errorf("SAR bad index %d", index)
	}
	aspectRatioTable := [][]uint{
		{1, 1}, {12, 11}, {10, 11}, {16, 11},
		{40, 33}, {24, 11}, {20, 11}, {32, 11},
		{80, 33}, {18, 11}, {15, 11}, {64, 33},
		{160, 99}, {4, 3}, {3, 2}, {2, 1}}
	return aspectRatioTable[index-1][0], aspectRatioTable[index-1][1], nil
}

func readScalingList(reader *bits.EBSPReader, sizeOfScalingList int) ScalingList {
	scalingList := make([]int, sizeOfScalingList)
	lastScale := 8
	nextScale := 8
	for j := 0; j < sizeOfScalingList; j++ {
		if nextScale != 0 {
			deltaScale := reader.ReadSignedGolomb()
			nextScale = (lastScale + deltaScale + 256) % 256
		}
		if nextScale == 0 {
			scalingList[j] = lastScale
		} else {
			scalingList[j] = nextScale
		}
		lastScale = scalingList[j]
	}
	return scalingList
}
