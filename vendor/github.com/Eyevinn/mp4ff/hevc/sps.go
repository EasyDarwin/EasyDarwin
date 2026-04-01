package hevc

import (
	"bytes"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/avc"
	"github.com/Eyevinn/mp4ff/bits"
)

// SPS - HEVC SPS parameters
// ISO/IEC 23008-2 Sec. 7.3.2.2
type SPS struct {
	VpsID                                byte
	MaxSubLayersMinus1                   byte
	TemporalIDNestingFlag                bool
	ProfileTierLevel                     ProfileTierLevel
	SpsID                                byte
	ChromaFormatIDC                      byte
	SeparateColourPlaneFlag              bool
	ConformanceWindowFlag                bool
	PicWidthInLumaSamples                uint32
	PicHeightInLumaSamples               uint32
	ConformanceWindow                    ConformanceWindow
	BitDepthLumaMinus8                   byte
	BitDepthChromaMinus8                 byte
	Log2MaxPicOrderCntLsbMinus4          byte
	SubLayerOrderingInfoPresentFlag      bool
	SubLayeringOrderingInfos             []SubLayerOrderingInfo
	Log2MinLumaCodingBlockSizeMinus3     byte
	Log2DiffMaxMinLumaCodingBlockSize    byte
	Log2MinLumaTransformBlockSizeMinus2  byte
	Log2DiffMaxMinLumaTransformBlockSize byte
	MaxTransformHierarchyDepthInter      byte
	MaxTransformHierarchyDepthIntra      byte
	ScalingListEnabledFlag               bool
	ScalingListDataPresentFlag           bool
	AmpEnabledFlag                       bool
	SampleAdaptiveOffsetEnabledFlag      bool
	PCMEnabledFlag                       bool
	PcmSampleBitDepthLumaMinus1          byte
	PcmSampleBitDepthChromaMinus1        byte
	Log2MinPcmLumaCodingBlockSize        uint16
	Log2DiffMaxMinPcmLumaCodingBlockSize uint16
	PcmLoopFilterDisabledFlag            bool
	NumShortTermRefPicSets               byte
	ShortTermRefPicSets                  []ShortTermRPS
	LongTermRefPicsPresentFlag           bool
	NumLongTermRefPics                   uint8
	LongTermRefPicSets                   []LongTermRPS
	SpsTemporalMvpEnabledFlag            bool
	StrongIntraSmoothingEnabledFlag      bool
	VUIParametersPresentFlag             bool
	VUI                                  *VUIParameters
	ExtensionPresentFlag                 bool
	Extension4bits                       uint8
	RangeExtensionFlag                   bool
	RangeExtension                       *SPSRangeExtension
	MultilayerExtensionFlag              bool
	MultilayerExtension                  *SPSMultilayerExtension
	// SPS 3D extension
	D3ExtensionFlag   bool
	D3Extension       *SPS3dExtension
	SccExtensionFlag  bool
	SccExtension      *SPSSccExtension
	ExtensionDataFlag []bool
}

// ProfileTierLevel according to ISO/IEC 23008-2 Section 7.3.3
type ProfileTierLevel struct {
	GeneralProfileSpace              byte
	GeneralTierFlag                  bool
	GeneralProfileIDC                byte
	GeneralProfileCompatibilityFlags uint32
	GeneralProgressiveSourceFlag     bool
	GeneralInterlacedSourceFlag      bool
	GeneralNonPackedConstraintFlag   bool
	GeneralFrameOnlyConstraintFlag   bool
	// GeneralConstraintIndicatorFlags is 4 + 43+1 bits including the 4 flags above from GeneralProgressiveSourceFlag
	GeneralConstraintIndicatorFlags uint64
	GeneralLevelIDC                 byte
	SubLayers                       []SubLayer
}

type SubLayer struct {
	ProfilePresentFlag        bool
	LevelPresentFlag          bool
	ProfileSpace              byte
	TierFlag                  bool
	ProfileIDC                byte
	ProfileCompatibilityFlags uint32
	ProgressiveSourceFlag     bool
	InterlacedSourceFlag      bool
	NonPackedConstraintFlag   bool
	FrameOnlyConstraintFlag   bool
	ConstraintFlags           uint64 // 43+1 bits
	LayerIDC                  byte
}

func flagFrom(flags uint64, bitNr uint) bool {
	return (flags & (1 << bitNr)) != 0
}

// parseProfileTierLevel follows ISO/IEC 23008-2 Section 7.3.3
func parseProfileTierLevel(r *bits.EBSPReader, profilePresentFlag bool, maxNumSubLayersMinus1 byte) ProfileTierLevel {
	ptl := ProfileTierLevel{}
	if profilePresentFlag {
		ptl.GeneralProfileSpace = byte(r.Read(2))
		ptl.GeneralTierFlag = r.ReadFlag()
		ptl.GeneralProfileIDC = byte(r.Read(5))
		ptl.GeneralProfileCompatibilityFlags = uint32(r.Read(32))
		ptl.GeneralConstraintIndicatorFlags = uint64(r.Read(48))
		ptl.GeneralProgressiveSourceFlag = flagFrom(ptl.GeneralConstraintIndicatorFlags, 47)
		ptl.GeneralInterlacedSourceFlag = flagFrom(ptl.GeneralConstraintIndicatorFlags, 46)
		ptl.GeneralNonPackedConstraintFlag = flagFrom(ptl.GeneralConstraintIndicatorFlags, 45)
		ptl.GeneralFrameOnlyConstraintFlag = flagFrom(ptl.GeneralConstraintIndicatorFlags, 44)
	}
	ptl.GeneralLevelIDC = byte(r.Read(8))
	if maxNumSubLayersMinus1 > 0 {
		ptl.SubLayers = make([]SubLayer, maxNumSubLayersMinus1)
		for i := byte(0); i < maxNumSubLayersMinus1; i++ {
			ptl.SubLayers[i].ProfilePresentFlag = r.ReadFlag()
			ptl.SubLayers[i].LevelPresentFlag = r.ReadFlag()
		}
		if maxNumSubLayersMinus1 < 8 {
			nrReservedZeroBits := 2 * (8 - int(maxNumSubLayersMinus1))
			_ = r.Read(nrReservedZeroBits)
		}
		for i := byte(0); i < maxNumSubLayersMinus1; i++ {
			if ptl.SubLayers[i].ProfilePresentFlag {
				ptl.SubLayers[i].ProfileSpace = byte(r.Read(2))
				ptl.SubLayers[i].TierFlag = r.ReadFlag()
				ptl.SubLayers[i].ProfileIDC = byte(r.Read(5))
				ptl.SubLayers[i].ProfileCompatibilityFlags = uint32(r.Read(32))
				ptl.SubLayers[i].ConstraintFlags = uint64(r.Read(48)) // Including 4 flags from ProgressiveSourceFlag and forward
				ptl.SubLayers[i].ProgressiveSourceFlag = flagFrom(ptl.SubLayers[i].ConstraintFlags, 47)
				ptl.SubLayers[i].InterlacedSourceFlag = flagFrom(ptl.SubLayers[i].ConstraintFlags, 46)
				ptl.SubLayers[i].NonPackedConstraintFlag = flagFrom(ptl.SubLayers[i].ConstraintFlags, 45)
				ptl.SubLayers[i].FrameOnlyConstraintFlag = flagFrom(ptl.SubLayers[i].ConstraintFlags, 44)
			}
			if ptl.SubLayers[i].LevelPresentFlag {
				ptl.SubLayers[i].LayerIDC = byte(r.Read(8))
			}
		}
	}
	return ptl
}

// ConformanceWindow according to ISO/IEC 23008-2
type ConformanceWindow struct {
	LeftOffset   uint32
	RightOffset  uint32
	TopOffset    uint32
	BottomOffset uint32
}

// SubLayerOrderingInfo according to ISO/IEC 23008-2
type SubLayerOrderingInfo struct {
	MaxDecPicBufferingMinus1 byte
	MaxNumReorderPics        byte
	MaxLatencyIncreasePlus1  byte
}

// VUIParameters - Visual Usability Information as defined in Section E.2
type VUIParameters struct {
	SampleAspectRatioWidth         uint
	SampleAspectRatioHeight        uint
	OverscanInfoPresentFlag        bool
	OverscanAppropriateFlag        bool
	VideoSignalTypePresentFlag     bool
	VideoFormat                    byte
	VideoFullRangeFlag             bool
	ColourDescriptionFlag          bool
	ColourPrimaries                byte
	TransferCharacteristics        byte
	MatrixCoefficients             byte
	ChromaLocInfoPresentFlag       bool
	ChromaSampleLocTypeTopField    uint
	ChromaSampleLocTypeBottomField uint
	NeutralChromaIndicationFlag    bool
	FieldSeqFlag                   bool
	FrameFieldInfoPresentFlag      bool
	DefaultDisplayWindowFlag       bool
	DefDispWinLeftOffset           uint
	DefDispWinRightOffset          uint
	DefDispWinTopOffset            uint
	DefDispWinBottomOffset         uint
	TimingInfoPresentFlag          bool
	NumUnitsInTick                 uint
	TimeScale                      uint
	PocProportionalToTimingFlag    bool
	NumTicksPocDiffOneMinus1       uint
	HrdParametersPresentFlag       bool
	HrdParameters                  *HrdParameters
	BitstreamRestrictionFlag       bool
	BitstreamResctrictions         *BitstreamRestrictions
}

type HrdParameters struct {
	NalHrdParametersPresentFlag            bool
	VclHrdParametersPresentFlag            bool
	SubPicHrdParamsPresentFlag             bool
	TickDivisorMinus2                      uint8
	DuCpbRemovalDelayIncrementLengthMinus1 uint8
	SubPicCpbParamsInPicTimingSeiFlag      bool
	DpbOutputDelayDuLengthMinus1           uint8
	BitRateScale                           uint8
	CpbSizeScale                           uint8
	CpbSizeDuScale                         uint8
	InitialCpbRemovalDelayLengthMinus1     uint8
	AuCpbRemovalDelayLengthMinus1          uint8
	DpbOutputDelayLengthMinus1             uint8
	SubLayerHrd                            []SubLayerHrd
}

// CpbDpbDelaysPresentFlag is defined in ISO/IEC 23008-2 Section E.3.2.
func (h *HrdParameters) CpbDpbDelaysPresentFlag() bool {
	return h.NalHrdParametersPresentFlag || h.VclHrdParametersPresentFlag
}

type SubLayerHrd struct {
	FixedPicRateGeneralFlag     bool
	FixedPicRateWithinCvsFlag   bool
	ElementalDurationInTcMinus1 uint16
	LowDelayHrdFlag             bool
	CpbCntMinus1                uint8
	NalHrdParameters            []SubLayerHrdParameters
	VclHrdParameters            []SubLayerHrdParameters
}

type SubLayerHrdParameters struct {
	BitRateValueMinus1   uint32
	CpbSizeValueMinus1   uint32
	CpbSizeDuValueMinus1 uint32
	BitRateDuValueMinus1 uint32
	CbrFlag              bool
}

// BitstreamRestrictrictions - optional information
type BitstreamRestrictions struct {
	TilesFixedStructureFlag     bool
	MVOverPicBoundariesFlag     bool
	RestrictedRefsPicsListsFlag bool
	MinSpatialSegmentationIDC   uint
	MaxBytesPerPicDenom         uint
	MaxBitsPerMinCuDenom        uint
	Log2MaxMvLengthHorizontal   uint
	Log2MaxMvLengthVertical     uint
}

type LongTermRPS struct {
	PocLsbLt               uint16
	UsedByCurrPicLtFlag    bool
	DeltaPocMsbPresentFlag bool
	DeltaPocMsbCycleLt     uint
}

type SPSRangeExtension struct {
	TransformSkipRotationEnabledFlag    bool
	TransformSkipContextEnabledFlag     bool
	ImplicitRdpcmEnabledFlag            bool
	ExplicitRdpcmEnabledFlag            bool
	ExtendedPrecisionProcessingFlag     bool
	IntraSmoothingDisabledFlag          bool
	HighPrecisionOffsetsEnabledFlag     bool
	PersistentRiceAdaptationEnabledFlag bool
	CabacBypassAlignmentEnabledFlag     bool
}

type SPSMultilayerExtension struct {
	InterViewMvVertConstraintFlag bool
}

type SPS3dExtension struct {
	IvDiMcEnabledFlag0     bool
	IvMvScalEnabledFlag0   bool
	Og2IvmcSubPbSizeMinus3 uint
	IvResPredEnabledFlag   bool
	DepthRefEnabledFlag    bool
	VspMcEnabledFlag       bool
	DbbpEnabledFlag        bool

	IvDiMcEnabledFlag1          bool
	IvMvScalEnabledFlag1        bool
	TexMcEnabledFlag            bool
	Log2TexmcSubPbSizeMinus3    uint
	IntraContourEnabledFlag     bool
	IntraDcOnlyWedgeEnabledFlag bool
	CqtCuPartPredEnabledFlag    bool
	InterDcOnlyEnabledFlag      bool
	SkipIntraEnabledFlag        bool
}

type SPSSccExtension struct {
	CurrPicRefEnabledFlag                   bool
	PaletteModeEnabledFlag                  bool
	PaletteMaxSize                          uint
	DeltaPaletteMaxPredictorSize            uint
	PalettePredictorInitializersPresentFlag bool
	NumPalettePredictorInitializersMinus1   uint
	PalettePredictorInitializer             [][]uint
	MotionVectorResolutionControlIdc        uint8
	IntraBoundaryFilteringDisabledFlag      bool
}

// ParseSPSNALUnit parses SPS NAL unit starting with NAL unit header
func ParseSPSNALUnit(data []byte) (*SPS, error) {

	sps := &SPS{}

	rd := bytes.NewReader(data)
	r := bits.NewEBSPReader(rd)
	// Note! First two bytes are NALU Header

	naluHdrBits := r.Read(16)
	naluType := GetNaluType(byte(naluHdrBits >> 8))
	if naluType != NALU_SPS {
		return nil, fmt.Errorf("NALU type is %s not SPS", naluType)
	}
	sps.VpsID = byte(r.Read(4))
	sps.MaxSubLayersMinus1 = byte(r.Read(3))
	sps.TemporalIDNestingFlag = r.ReadFlag()
	sps.ProfileTierLevel = parseProfileTierLevel(r, true, sps.MaxSubLayersMinus1)
	sps.SpsID = byte(r.ReadExpGolomb())
	sps.ChromaFormatIDC = byte(r.ReadExpGolomb())
	if sps.ChromaFormatIDC == 3 {
		sps.SeparateColourPlaneFlag = r.ReadFlag()
	}
	sps.PicWidthInLumaSamples = uint32(r.ReadExpGolomb())
	sps.PicHeightInLumaSamples = uint32(r.ReadExpGolomb())
	sps.ConformanceWindowFlag = r.ReadFlag()
	if sps.ConformanceWindowFlag {
		sps.ConformanceWindow = ConformanceWindow{
			LeftOffset:   uint32(r.ReadExpGolomb()),
			RightOffset:  uint32(r.ReadExpGolomb()),
			TopOffset:    uint32(r.ReadExpGolomb()),
			BottomOffset: uint32(r.ReadExpGolomb()),
		}
	}
	sps.BitDepthLumaMinus8 = byte(r.ReadExpGolomb())
	sps.BitDepthChromaMinus8 = byte(r.ReadExpGolomb())
	sps.Log2MaxPicOrderCntLsbMinus4 = byte(r.ReadExpGolomb())
	sps.SubLayerOrderingInfoPresentFlag = r.ReadFlag()
	startValue := sps.MaxSubLayersMinus1
	if sps.SubLayerOrderingInfoPresentFlag {
		startValue = 0
	}
	for i := startValue; i <= sps.MaxSubLayersMinus1; i++ {
		sps.SubLayeringOrderingInfos = append(
			sps.SubLayeringOrderingInfos,
			SubLayerOrderingInfo{
				MaxDecPicBufferingMinus1: byte(r.ReadExpGolomb()),
				MaxNumReorderPics:        byte(r.ReadExpGolomb()),
				MaxLatencyIncreasePlus1:  byte(r.ReadExpGolomb()),
			})
	}
	sps.Log2MinLumaCodingBlockSizeMinus3 = byte(r.ReadExpGolomb())
	sps.Log2DiffMaxMinLumaCodingBlockSize = byte(r.ReadExpGolomb())
	sps.Log2MinLumaTransformBlockSizeMinus2 = byte(r.ReadExpGolomb())
	sps.Log2DiffMaxMinLumaTransformBlockSize = byte(r.ReadExpGolomb())
	sps.MaxTransformHierarchyDepthInter = byte(r.ReadExpGolomb())
	sps.MaxTransformHierarchyDepthIntra = byte(r.ReadExpGolomb())
	sps.ScalingListEnabledFlag = r.ReadFlag()
	if sps.ScalingListEnabledFlag {
		sps.ScalingListDataPresentFlag = r.ReadFlag()
		if sps.ScalingListDataPresentFlag {
			readPastScalingListData(r)
		}
	}
	sps.AmpEnabledFlag = r.ReadFlag()
	sps.SampleAdaptiveOffsetEnabledFlag = r.ReadFlag()
	sps.PCMEnabledFlag = r.ReadFlag()
	if sps.PCMEnabledFlag {
		sps.PcmSampleBitDepthLumaMinus1 = byte(r.Read(4))
		sps.PcmSampleBitDepthChromaMinus1 = byte(r.Read(4))
		sps.Log2MinPcmLumaCodingBlockSize = uint16(r.ReadExpGolomb())
		sps.Log2DiffMaxMinPcmLumaCodingBlockSize = uint16(r.ReadExpGolomb())
		sps.PcmLoopFilterDisabledFlag = r.ReadFlag()
	}
	sps.NumShortTermRefPicSets = byte(r.ReadExpGolomb())
	if sps.NumShortTermRefPicSets > 0 {
		sps.ShortTermRefPicSets = make([]ShortTermRPS, sps.NumShortTermRefPicSets)
		for idx := byte(0); idx < sps.NumShortTermRefPicSets; idx++ {
			sps.ShortTermRefPicSets[idx] = parseShortTermRPS(r, idx, sps.NumShortTermRefPicSets, sps)
			if r.AccError() != nil { // Don't continue if we have an issue
				return sps, r.AccError()
			}
		}
	}
	sps.LongTermRefPicsPresentFlag = r.ReadFlag()
	if sps.LongTermRefPicsPresentFlag {
		// value shall be in the range of 0 to 32, inclusive
		sps.NumLongTermRefPics = uint8(r.ReadExpGolomb())
		if sps.NumLongTermRefPics > 0 {
			sps.LongTermRefPicSets = make([]LongTermRPS, sps.NumLongTermRefPics)
			for i := uint8(0); i < sps.NumLongTermRefPics; i++ {
				sps.LongTermRefPicSets[i] = LongTermRPS{
					PocLsbLt:            uint16(r.Read(int(sps.Log2MaxPicOrderCntLsbMinus4 + 4))),
					UsedByCurrPicLtFlag: r.ReadFlag(),
				}
			}
		}
	}
	sps.SpsTemporalMvpEnabledFlag = r.ReadFlag()
	sps.StrongIntraSmoothingEnabledFlag = r.ReadFlag()
	sps.VUIParametersPresentFlag = r.ReadFlag()
	if sps.VUIParametersPresentFlag {
		sps.VUI = parseVUI(r, sps.MaxSubLayersMinus1)
	}

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	sps.ExtensionPresentFlag = r.ReadFlag()
	if sps.ExtensionPresentFlag {
		sps.RangeExtensionFlag = r.ReadFlag()
		sps.MultilayerExtensionFlag = r.ReadFlag()
		sps.D3ExtensionFlag = r.ReadFlag()
		sps.SccExtensionFlag = r.ReadFlag()
		sps.Extension4bits = uint8(r.Read(4))
	}

	if sps.RangeExtensionFlag {
		sps.RangeExtension = &SPSRangeExtension{
			TransformSkipRotationEnabledFlag:    r.ReadFlag(),
			TransformSkipContextEnabledFlag:     r.ReadFlag(),
			ImplicitRdpcmEnabledFlag:            r.ReadFlag(),
			ExplicitRdpcmEnabledFlag:            r.ReadFlag(),
			ExtendedPrecisionProcessingFlag:     r.ReadFlag(),
			IntraSmoothingDisabledFlag:          r.ReadFlag(),
			HighPrecisionOffsetsEnabledFlag:     r.ReadFlag(),
			PersistentRiceAdaptationEnabledFlag: r.ReadFlag(),
			CabacBypassAlignmentEnabledFlag:     r.ReadFlag(),
		}
	}
	if sps.MultilayerExtensionFlag {
		sps.MultilayerExtension = &SPSMultilayerExtension{
			InterViewMvVertConstraintFlag: r.ReadFlag(),
		}
	}
	if sps.D3ExtensionFlag {
		sps.D3Extension = parseSPS3dExtension(r)
	}
	if sps.SccExtensionFlag {
		sps.SccExtension = parseSPSSccExtension(r, sps.ChromaFormatIDC,
			sps.BitDepthLumaMinus8, sps.BitDepthChromaMinus8)
	}
	if sps.Extension4bits > 0 {
		// Reserved for future use. Shall be empty
		more, err := r.MoreRbspData()
		if err != nil {
			return nil, err
		}
		for more {
			sps.ExtensionDataFlag = append(sps.ExtensionDataFlag, r.ReadFlag())
			more, err = r.MoreRbspData()
			if err != nil {
				return nil, err
			}
		}
	}

	err := r.ReadRbspTrailingBits()
	if err != nil {
		if r.AccError() != nil {
			return nil, r.AccError()
		}
		return nil, err
	}
	if r.AccError() != nil {
		return nil, r.AccError()
	}
	_ = r.Read(1)
	if r.AccError() != io.EOF {
		return nil, fmt.Errorf("not at end after reading rbsp_trailing_bits")
	}

	return sps, nil
}

// ImageSize - calculated width and height using ConformanceWindow
func (s *SPS) ImageSize() (width, height uint32) {
	encWidth, encHeight := s.PicWidthInLumaSamples, s.PicHeightInLumaSamples
	var subWidthC, subHeightC uint32 = 1, 1
	switch s.ChromaFormatIDC {
	case 1: // 4:2:0
		subWidthC, subHeightC = 2, 2
	case 2: // 4:2:2
		subWidthC = 2
	}
	width = encWidth - (s.ConformanceWindow.LeftOffset+s.ConformanceWindow.RightOffset)*subWidthC
	height = encHeight - (s.ConformanceWindow.TopOffset+s.ConformanceWindow.BottomOffset)*subHeightC
	return width, height
}

// parseVUI - parse VUI (Visual Usability Information)
// if parseVUIBeyondAspectRatio is false, stop after AspectRatio has been parsed
func parseVUI(r *bits.EBSPReader, MaxSubLayersMinus1 byte) *VUIParameters {
	vui := &VUIParameters{}
	aspectRatioInfoPresentFlag := r.ReadFlag()
	if aspectRatioInfoPresentFlag {
		aspectRatioIDC := r.Read(8)
		if aspectRatioIDC == avc.ExtendedSAR {
			vui.SampleAspectRatioWidth = r.Read(16)
			vui.SampleAspectRatioHeight = r.Read(16)
		} else {
			var err error
			vui.SampleAspectRatioWidth, vui.SampleAspectRatioHeight, err = avc.GetSARfromIDC(aspectRatioIDC)
			if err != nil {
				r.SetError(fmt.Errorf("GetSARFromIDC: %w", err))
			}
		}
	}
	vui.OverscanInfoPresentFlag = r.ReadFlag()
	if vui.OverscanInfoPresentFlag {
		vui.OverscanAppropriateFlag = r.ReadFlag()
	}
	vui.VideoSignalTypePresentFlag = r.ReadFlag()
	if vui.VideoSignalTypePresentFlag {
		vui.VideoFormat = byte(r.Read(3))
		vui.VideoFullRangeFlag = r.ReadFlag()
		vui.ColourDescriptionFlag = r.ReadFlag()
		if vui.ColourDescriptionFlag {
			vui.ColourPrimaries = byte(r.Read(8))
			vui.TransferCharacteristics = byte(r.Read(8))
			vui.MatrixCoefficients = byte(r.Read(8))
		}
	}
	vui.ChromaLocInfoPresentFlag = r.ReadFlag()
	if vui.ChromaLocInfoPresentFlag {
		vui.ChromaSampleLocTypeTopField = r.ReadExpGolomb()
		vui.ChromaSampleLocTypeBottomField = r.ReadExpGolomb()
	}
	vui.NeutralChromaIndicationFlag = r.ReadFlag()
	vui.FieldSeqFlag = r.ReadFlag()
	vui.FrameFieldInfoPresentFlag = r.ReadFlag()
	vui.DefaultDisplayWindowFlag = r.ReadFlag()
	if vui.DefaultDisplayWindowFlag {
		vui.DefDispWinLeftOffset = r.ReadExpGolomb()
		vui.DefDispWinRightOffset = r.ReadExpGolomb()
		vui.DefDispWinTopOffset = r.ReadExpGolomb()
		vui.DefDispWinBottomOffset = r.ReadExpGolomb()
	}
	vui.TimingInfoPresentFlag = r.ReadFlag()
	if vui.TimingInfoPresentFlag {
		vui.NumUnitsInTick = r.Read(32)
		vui.TimeScale = r.Read(32)
		vui.PocProportionalToTimingFlag = r.ReadFlag()
		if vui.PocProportionalToTimingFlag {
			vui.NumTicksPocDiffOneMinus1 = r.ReadExpGolomb()
		}
		vui.HrdParametersPresentFlag = r.ReadFlag()
		if vui.HrdParametersPresentFlag {
			vui.HrdParameters = parseHrdParameters(r, true, MaxSubLayersMinus1)
		}
	}
	vui.BitstreamRestrictionFlag = r.ReadFlag()
	if vui.BitstreamRestrictionFlag {
		vui.BitstreamResctrictions = parseBitstreamRestrictions(r)
	}

	return vui
}

func parseHrdParameters(r *bits.EBSPReader,
	commonInfPresentFlag bool, maxNumSubLayersMinus1 byte) *HrdParameters {
	hp := &HrdParameters{}
	if commonInfPresentFlag {
		hp.NalHrdParametersPresentFlag = r.ReadFlag()
		hp.VclHrdParametersPresentFlag = r.ReadFlag()
		if hp.NalHrdParametersPresentFlag || hp.VclHrdParametersPresentFlag {
			hp.SubPicHrdParamsPresentFlag = r.ReadFlag()
			if hp.SubPicHrdParamsPresentFlag {
				hp.TickDivisorMinus2 = uint8(r.Read(8))
				hp.DuCpbRemovalDelayIncrementLengthMinus1 = uint8(r.Read(5))
				hp.SubPicCpbParamsInPicTimingSeiFlag = r.ReadFlag()
				hp.DpbOutputDelayDuLengthMinus1 = uint8(r.Read(5))
			}
			hp.BitRateScale = uint8(r.Read(4))
			hp.CpbSizeScale = uint8(r.Read(4))
			if hp.SubPicHrdParamsPresentFlag {
				hp.CpbSizeDuScale = uint8(r.Read(4))
			}
			hp.InitialCpbRemovalDelayLengthMinus1 = uint8(r.Read(5))
			hp.AuCpbRemovalDelayLengthMinus1 = uint8(r.Read(5))
			hp.DpbOutputDelayLengthMinus1 = uint8(r.Read(5))
		}
	}
	hp.SubLayerHrd = make([]SubLayerHrd, maxNumSubLayersMinus1+1)
	for i := byte(0); i <= maxNumSubLayersMinus1; i++ {
		hp.SubLayerHrd[i].FixedPicRateGeneralFlag = r.ReadFlag()
		if !hp.SubLayerHrd[i].FixedPicRateGeneralFlag {
			hp.SubLayerHrd[i].FixedPicRateWithinCvsFlag = r.ReadFlag()
		} else {
			// when fixed_pic_rate_general_flag[ i ] is equal to 1, the value of
			// fixed_pic_rate_within_cvs_flag[ i ] is inferred to be equal to 1.
			hp.SubLayerHrd[i].FixedPicRateWithinCvsFlag = true
		}

		if hp.SubLayerHrd[i].FixedPicRateWithinCvsFlag {
			// value shall be in the range of 0 to 2 047, inclusive
			hp.SubLayerHrd[i].ElementalDurationInTcMinus1 = uint16(r.ReadExpGolomb())
		} else {
			hp.SubLayerHrd[i].LowDelayHrdFlag = r.ReadFlag()
		}

		if !hp.SubLayerHrd[i].LowDelayHrdFlag {
			// value shall be in the range of 0 to 31, inclusive
			hp.SubLayerHrd[i].CpbCntMinus1 = uint8(r.ReadExpGolomb())
		}
		if hp.NalHrdParametersPresentFlag {
			hp.SubLayerHrd[i].NalHrdParameters = parseSubLayerHrdParameters(r,
				hp.SubLayerHrd[i].CpbCntMinus1, hp.SubPicHrdParamsPresentFlag)
		}
		if hp.VclHrdParametersPresentFlag {
			hp.SubLayerHrd[i].VclHrdParameters = parseSubLayerHrdParameters(r,
				hp.SubLayerHrd[i].CpbCntMinus1, hp.SubPicHrdParamsPresentFlag)
		}
	}
	return hp
}

func parseSubLayerHrdParameters(r *bits.EBSPReader,
	cpbCntMinus1 uint8, subPicHrdParamsPresentFlag bool) []SubLayerHrdParameters {
	slhp := make([]SubLayerHrdParameters, cpbCntMinus1+1)
	for i := uint8(0); i <= cpbCntMinus1; i++ {
		// values shall be in the range of 0 to 2^32 − 2, inclusive
		slhp[i].BitRateValueMinus1 = uint32(r.ReadExpGolomb())
		slhp[i].CpbSizeValueMinus1 = uint32(r.ReadExpGolomb())
		if subPicHrdParamsPresentFlag {
			slhp[i].CpbSizeDuValueMinus1 = uint32(r.ReadExpGolomb())
			slhp[i].BitRateDuValueMinus1 = uint32(r.ReadExpGolomb())
		}
		slhp[i].CbrFlag = r.ReadFlag()
	}
	return slhp
}

func parseBitstreamRestrictions(r *bits.EBSPReader) *BitstreamRestrictions {
	br := BitstreamRestrictions{}
	br.TilesFixedStructureFlag = r.ReadFlag()
	br.MVOverPicBoundariesFlag = r.ReadFlag()
	br.RestrictedRefsPicsListsFlag = r.ReadFlag()
	br.MinSpatialSegmentationIDC = r.ReadExpGolomb()
	br.MaxBytesPerPicDenom = r.ReadExpGolomb()
	br.MaxBitsPerMinCuDenom = r.ReadExpGolomb()
	br.Log2MaxMvLengthHorizontal = r.ReadExpGolomb()
	br.Log2MaxMvLengthVertical = r.ReadExpGolomb()
	return &br
}

// ShortTermRPS - Short term Reference Picture Set
type ShortTermRPS struct {
	// Delta Picture Order Count
	DeltaPocS0      []uint32
	DeltaPocS1      []uint32
	UsedByCurrPicS0 []bool
	UsedByCurrPicS1 []bool
	NumNegativePics byte
	NumPositivePics byte
	NumDeltaPocs    byte
}

func (st ShortTermRPS) countInUsePics() uint8 {
	var NumPicTotalCurr uint8
	for _, n := range st.UsedByCurrPicS0 {
		if n {
			NumPicTotalCurr++
		}
	}
	for _, p := range st.UsedByCurrPicS1 {
		if p {
			NumPicTotalCurr++
		}
	}
	return NumPicTotalCurr
}

const maxSTRefPics = 16

// parseShortTermRPS - short-term reference pictures with syntax from 7.3.7.
// Focus is on reading/parsing beyond this structure in SPS (and possibly in slice header)
func parseShortTermRPS(r *bits.EBSPReader, idx, numSTRefPicSets byte, sps *SPS) ShortTermRPS {
	stps := ShortTermRPS{}

	interRPSPredFlag := false

	if idx > 0 {
		interRPSPredFlag = r.ReadFlag()
	}
	if interRPSPredFlag {
		deltaIdx := byte(1)
		if idx == numSTRefPicSets { // Slice header
			deltaIdx = byte(r.ReadExpGolomb() + 1)
			// parse delta_idx_minus1
		}
		if deltaIdx > idx {
			r.SetError(fmt.Errorf("deltaIdx > idx in parseShortTermRPS"))
		}
		/* deltaRpsSign */ _ = r.Read(1)
		/* absDeltaRpsMinus1*/ _ = r.ReadExpGolomb()
		//deltaRps := (1 - (deltaRpsSign << 1)) * (absDeltaRpsMinus1 + 1)
		refIdx := idx - deltaIdx
		numDeltaPocs := sps.ShortTermRefPicSets[refIdx].NumDeltaPocs
		for j := byte(0); j <= numDeltaPocs; j++ {
			usedByCurrPicFlag := r.ReadFlag()
			useDeltaFlag := true
			if !usedByCurrPicFlag {
				useDeltaFlag = r.ReadFlag()
			}
			if usedByCurrPicFlag || useDeltaFlag {
				stps.NumDeltaPocs++
			}
		}
	} else {
		stps.NumNegativePics = byte(r.ReadExpGolomb())
		stps.NumPositivePics = byte(r.ReadExpGolomb())
		if stps.NumNegativePics > maxSTRefPics || stps.NumPositivePics > maxSTRefPics {
			r.SetError(fmt.Errorf("more than %d short term reference pictures", maxSTRefPics))
			return stps
		}
		stps.NumDeltaPocs = stps.NumNegativePics + stps.NumPositivePics
		stps.DeltaPocS0 = make([]uint32, stps.NumNegativePics)
		stps.UsedByCurrPicS0 = make([]bool, stps.NumNegativePics)
		for i := byte(0); i < stps.NumNegativePics; i++ {
			stps.DeltaPocS0[i] = uint32(r.ReadExpGolomb() + 1)
			stps.UsedByCurrPicS0[i] = r.ReadFlag()
		}
		stps.DeltaPocS1 = make([]uint32, stps.NumPositivePics)
		stps.UsedByCurrPicS1 = make([]bool, stps.NumPositivePics)
		for i := byte(0); i < stps.NumPositivePics; i++ {
			stps.DeltaPocS1[i] = uint32(r.ReadExpGolomb() + 1)
			stps.UsedByCurrPicS1[i] = r.ReadFlag()
		}
	}

	return stps
}

// readPastScalingListData - read and parse all bits of scaling list, without storing values
func readPastScalingListData(r *bits.EBSPReader) {
	for sizeId := 0; sizeId < 4; sizeId++ {
		nrMatrixIds := 6
		if sizeId == 3 {
			nrMatrixIds = 2
		}
		for matrixId := 0; matrixId < nrMatrixIds; matrixId++ {
			flag := r.ReadFlag() // scaling_list_pred_mode_flag[sizeId][matrixId]
			if !flag {
				_ = r.ReadExpGolomb() // scaling_list_pred_matrix_id_delta[sizeId][matrixId]
			} else {
				// nextCoef = 8;
				coefNum := (1 << (4 + (sizeId << 1)))
				if coefNum > 64 {
					coefNum = 64
				}
				if sizeId > 1 {
					_ = r.ReadExpGolomb() // scaling_list_dc_coef_minus8[sizeId − 2][matrixId]
					// nextCoef = scaling_list_dc_coef_minus8[sizeId − 2][matrixId] + 8
				}
				for i := 0; i < coefNum; i++ {
					_ = r.ReadExpGolomb() // scaling_list_delta_coef
					// nextCoef = ( nextCoef + scaling_list_delta_coef + 256 ) % 256
					// ScalingList[sizeId][matrixId][i] = nextCoef
				}
			}
		}
	}
}

func parseSPS3dExtension(r *bits.EBSPReader) *SPS3dExtension {
	ext := &SPS3dExtension{
		IvDiMcEnabledFlag0:     r.ReadFlag(),
		IvMvScalEnabledFlag0:   r.ReadFlag(),
		Og2IvmcSubPbSizeMinus3: r.ReadExpGolomb(),
		IvResPredEnabledFlag:   r.ReadFlag(),
		DepthRefEnabledFlag:    r.ReadFlag(),
		VspMcEnabledFlag:       r.ReadFlag(),
		DbbpEnabledFlag:        r.ReadFlag(),

		IvDiMcEnabledFlag1:          r.ReadFlag(),
		IvMvScalEnabledFlag1:        r.ReadFlag(),
		TexMcEnabledFlag:            r.ReadFlag(),
		Log2TexmcSubPbSizeMinus3:    r.ReadExpGolomb(),
		IntraContourEnabledFlag:     r.ReadFlag(),
		IntraDcOnlyWedgeEnabledFlag: r.ReadFlag(),
		CqtCuPartPredEnabledFlag:    r.ReadFlag(),
		InterDcOnlyEnabledFlag:      r.ReadFlag(),
		SkipIntraEnabledFlag:        r.ReadFlag(),
	}
	return ext
}

func parseSPSSccExtension(r *bits.EBSPReader, ChromaFormatIDC,
	BitDepthLumaMinus8, BitDepthChromaMinus8 byte) *SPSSccExtension {
	ext := &SPSSccExtension{}
	ext.CurrPicRefEnabledFlag = r.ReadFlag()
	ext.PaletteModeEnabledFlag = r.ReadFlag()
	if ext.PaletteModeEnabledFlag {
		ext.PaletteMaxSize = r.ReadExpGolomb()
		ext.DeltaPaletteMaxPredictorSize = r.ReadExpGolomb()
		ext.PalettePredictorInitializersPresentFlag = r.ReadFlag()
		if ext.PalettePredictorInitializersPresentFlag {
			ext.NumPalettePredictorInitializersMinus1 = r.ReadExpGolomb()
			numComps := 3
			if ChromaFormatIDC == 0 {
				numComps = 1
			}
			ext.PalettePredictorInitializer = make([][]uint, numComps)
			// Fill luma
			for i := uint(0); i <= ext.NumPalettePredictorInitializersMinus1; i++ {
				ext.PalettePredictorInitializer[0] =
					append(ext.PalettePredictorInitializer[0], r.Read(int(BitDepthLumaMinus8+8)))
			}
			// Fill chroma if any
			for comp := 1; comp < numComps; comp++ {
				for i := uint(0); i <= ext.NumPalettePredictorInitializersMinus1; i++ {
					ext.PalettePredictorInitializer[comp] =
						append(ext.PalettePredictorInitializer[comp], r.Read(int(BitDepthChromaMinus8+8)))
				}
			}
		}
	}
	ext.MotionVectorResolutionControlIdc = uint8(r.Read(2))
	ext.IntraBoundaryFilteringDisabledFlag = r.ReadFlag()

	return ext
}
