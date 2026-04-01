package hevc

import (
	"bytes"
	"errors"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// HEVC PPS errors
var (
	ErrNotPPS = errors.New("not an PPS NAL unit")
)

// This parser based on Rec. ITU-T H.265 v5 (02/2018) and ISO/IEC 23008-2 Ed. 5

// PPS - Picture Parameter Set
type PPS struct {
	PicParameterSetID                      uint32
	SeqParameterSetID                      uint32
	DependentSliceSegmentsEnabledFlag      bool
	OutputFlagPresentFlag                  bool
	NumExtraSliceHeaderBits                uint8
	SignDataHidingEnabledFlag              bool
	CabacInitPresentFlag                   bool
	NumRefIdxL0DefaultActiveMinus1         uint8
	NumRefIdxL1DefaultActiveMinus1         uint8
	InitQpMinus26                          int8
	ConstrainedIntraPredFlag               bool
	TransformSkipEnabledFlag               bool
	CuQpDeltaEnabledFlag                   bool
	DiffCuQpDeltaDepth                     uint
	CbQpOffset                             int8
	CrQpOffset                             int8
	SliceChromaQpOffsetsPresentFlag        bool
	WeightedPredFlag                       bool
	WeightedBipredFlag                     bool
	TransquantBypassEnabledFlag            bool
	TilesEnabledFlag                       bool
	EntropyCodingSyncEnabledFlag           bool
	NumTileColumnsMinus1                   uint
	NumTileRowsMinus1                      uint
	UniformSpacingFlag                     bool
	ColumnWidthMinus1                      []uint
	RowHeightMinus1                        []uint
	LoopFilterAcrossTilesEnabledFlag       bool
	LoopFilterAcrossSlicesEnabledFlag      bool
	DeblockingFilterControlPresentFlag     bool
	DeblockingFilterOverrideEnabledFlag    bool
	DeblockingFilterDisabledFlag           bool
	BetaOffsetDiv2                         int8
	TcOffsetDiv2                           int8
	ScalingListDataPresentFlag             bool
	ListsModificationPresentFlag           bool
	Log2ParallelMergeLevelMinus2           uint
	SliceSegmentHeaderExtensionPresentFlag bool
	ExtensionPresentFlag                   bool
	RangeExtensionFlag                     bool
	RangeExtension                         *RangeExtension
	MultilayerExtensionFlag                bool
	MultilayerExtension                    *MultilayerExtension
	// PPS 3D extension
	D3ExtensionFlag   bool
	D3Extension       *D3Extension
	SccExtensionFlag  bool
	SccExtension      *SccExtension
	Extension4bits    uint8
	ExtensionDataFlag []bool
}

type RangeExtension struct {
	Log2MaxTransformSkipBlockSizeMinus2 uint
	CrossComponentPredictionEnabledFlag bool
	ChromaQpOffsetListEnabledFlag       bool
	DiffCuChromaQpOffsetDepth           uint
	ChromaQpOffsetListLenMinus1         uint
	CbQpOffsetList                      []int8
	CrQpOffsetList                      []int8
	Log2SaoOffsetScaleLuma              uint
	Log2SaoOffsetScaleChroma            uint
}

type MultilayerExtension struct {
	PocResetInfoPresentFlag  bool
	InferScalingListFlag     bool
	ScalingListRefLayerId    uint8
	NumRefLocOffsets         uint
	RefLocOffsetLayerIds     []uint8
	RefLocOffsets            map[uint8]RefLocOffset
	ColourMappingEnabledFlag bool
	ColourMappingTable       *ColourMappingTable
}

type RefLocOffset struct {
	ScaledRefLayerOffsetPresentFlag bool
	ScaledRefLayerLeftOffset        int16
	ScaledRefLayerTopOffset         int16
	ScaledRefLayerRightOffset       int16
	ScaledRefLayerBottomOffset      int16
	RefRegionOffsetPresentFlag      bool
	RefRegionLeftOffset             int16
	RefRegionTopOffset              int16
	RefRegionRightOffset            int16
	RefRegionBottomOffset           int16
	ResamplePhaseSetPresentFlag     bool
	PhaseHorLuma                    uint8
	PhaseVerLuma                    uint8
	PhaseHorChromaPlus8             uint8
	PhaseVerChromaPlus8             uint8
}

type ColourMappingTable struct {
	NumCmRefLayersMinus1         uint8
	RefLayerId                   []uint8
	OctantDepth                  uint8
	YPartNumLog2                 uint8
	LumaBitDepthCmInputMinus8    uint
	ChromaBitDepthCmInputMinus8  uint
	LumaBitDepthCmOutputMinus8   uint
	ChromaBitDepthCmOutputMinus8 uint
	ResQuantBits                 uint8
	DeltaFlcBitsMinus1           uint8
	AdaptThresholdUDelta         int
	AdaptThresholdVDelta         int
	Octants                      map[string][4]Octant
}

type Octant struct {
	CodedResFlag bool
	CodedRes     [3]struct {
		ResCoeffQ uint
		ResCoeffR uint
		ResCoeffS bool
	}
}

type SccExtension struct {
	CurrPicRefEnabledFlag                      bool
	ResidualAdaptiveColourTransformEnabledFlag bool
	SliceActQpOffsetsPresentFlag               bool
	ActYQpOffsetPlus5                          int
	ActCbQpOffsetPlus5                         int
	ActCrQpOffsetPlus3                         int
	PalettePredictorInitializersPresentFlag    bool
	NumPalettePredictorInitializers            uint
	MonochromePaletteFlag                      bool
	LumaBitDepthEntryMinus8                    uint
	ChromaBitDepthEntryMinus8                  uint
	PalettePredictorInitializer                [][]uint
}

// D3Extension represent PPS 3D extension
type D3Extension struct {
	DltsPresentFlag              bool
	NumDepthLayersMinus1         uint8
	BitDepthForDepthLayersMinus8 uint8
	DepthLayers                  []DepthLayer
}

type DepthLayer struct {
	DltFlag                bool
	DltPredFlag            bool
	DltValFlagsPresentFlag bool
	DltValueFlag           []bool
	DeltaDlt               *DeltaDlt
}

type DeltaDlt struct {
	NumValDeltaDlt       uint
	MaxDiff              uint
	MinDiffMinus1        uint
	DeltaDltVal0         uint
	DeltaValDiffMinusMin []uint
}

// ParsePPSNALUnit - Parse AVC PPS NAL unit starting with NAL header
func ParsePPSNALUnit(data []byte, spsMap map[uint32]*SPS) (*PPS, error) {
	var err error

	pps := &PPS{}

	rd := bytes.NewReader(data)
	r := bits.NewEBSPReader(rd)
	// Note! First two bytes are NALU Header

	naluHdrBits := r.Read(16)
	naluType := GetNaluType(byte(naluHdrBits >> 8))
	if naluType != NALU_PPS {
		return nil, ErrNotPPS
	}
	pps.PicParameterSetID = uint32(r.ReadExpGolomb())
	pps.SeqParameterSetID = uint32(r.ReadExpGolomb())

	if _, ok := spsMap[pps.SeqParameterSetID]; !ok {
		return pps, fmt.Errorf("sps ID %d not found in map", pps.SeqParameterSetID)
	}

	pps.DependentSliceSegmentsEnabledFlag = r.ReadFlag()
	pps.OutputFlagPresentFlag = r.ReadFlag()
	pps.NumExtraSliceHeaderBits = uint8(r.Read(3))
	pps.SignDataHidingEnabledFlag = r.ReadFlag()
	pps.CabacInitPresentFlag = r.ReadFlag()
	// value shall be in the range of 0 to 14, inclusive
	pps.NumRefIdxL0DefaultActiveMinus1 = uint8(r.ReadExpGolomb())
	pps.NumRefIdxL1DefaultActiveMinus1 = uint8(r.ReadExpGolomb())
	// value shall be in the range of −( 26 + QpBdOffsetY ) to +25, inclusive
	pps.InitQpMinus26 = int8(r.ReadSignedGolomb())
	pps.ConstrainedIntraPredFlag = r.ReadFlag()
	pps.TransformSkipEnabledFlag = r.ReadFlag()
	pps.CuQpDeltaEnabledFlag = r.ReadFlag()
	if pps.CuQpDeltaEnabledFlag {
		pps.DiffCuQpDeltaDepth = r.ReadExpGolomb()
	}
	// values shall be in the range of −12 to +12, inclusive
	pps.CbQpOffset = int8(r.ReadSignedGolomb())
	pps.CrQpOffset = int8(r.ReadSignedGolomb())
	pps.SliceChromaQpOffsetsPresentFlag = r.ReadFlag()
	pps.WeightedPredFlag = r.ReadFlag()
	pps.WeightedBipredFlag = r.ReadFlag()
	pps.TransquantBypassEnabledFlag = r.ReadFlag()
	pps.TilesEnabledFlag = r.ReadFlag()
	pps.EntropyCodingSyncEnabledFlag = r.ReadFlag()
	if pps.TilesEnabledFlag {
		pps.NumTileColumnsMinus1 = r.ReadExpGolomb()
		pps.NumTileRowsMinus1 = r.ReadExpGolomb()
		pps.UniformSpacingFlag = r.ReadFlag()
		if !pps.UniformSpacingFlag {
			for i := uint(0); i < pps.NumTileColumnsMinus1; i++ {
				pps.ColumnWidthMinus1 = append(pps.ColumnWidthMinus1, r.ReadExpGolomb())
			}
			for i := uint(0); i < pps.NumTileRowsMinus1; i++ {
				pps.RowHeightMinus1 = append(pps.RowHeightMinus1, r.ReadExpGolomb())
			}
		}
		pps.LoopFilterAcrossTilesEnabledFlag = r.ReadFlag()
	}
	pps.LoopFilterAcrossSlicesEnabledFlag = r.ReadFlag()
	pps.DeblockingFilterControlPresentFlag = r.ReadFlag()
	if pps.DeblockingFilterControlPresentFlag {
		pps.DeblockingFilterOverrideEnabledFlag = r.ReadFlag()
		pps.DeblockingFilterDisabledFlag = r.ReadFlag()
		if !pps.DeblockingFilterDisabledFlag {
			// values shall be in the range of −6 to 6, inclusive
			pps.BetaOffsetDiv2 = int8(r.ReadSignedGolomb())
			pps.TcOffsetDiv2 = int8(r.ReadSignedGolomb())
		}
	}
	pps.ScalingListDataPresentFlag = r.ReadFlag()
	if pps.ScalingListDataPresentFlag {
		readPastScalingListData(r)
	}
	pps.ListsModificationPresentFlag = r.ReadFlag()
	pps.Log2ParallelMergeLevelMinus2 = r.ReadExpGolomb()
	pps.SliceSegmentHeaderExtensionPresentFlag = r.ReadFlag()
	pps.ExtensionPresentFlag = r.ReadFlag()
	if pps.ExtensionPresentFlag {
		pps.RangeExtensionFlag = r.ReadFlag()
		pps.MultilayerExtensionFlag = r.ReadFlag()
		pps.D3ExtensionFlag = r.ReadFlag()
		pps.SccExtensionFlag = r.ReadFlag()
		pps.Extension4bits = uint8(r.Read(4))
	}

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	if pps.RangeExtensionFlag {
		pps.RangeExtension, err = parseRangeExtension(r, pps.TransformSkipEnabledFlag)
		if err != nil {
			return pps, err
		}
	}
	if pps.MultilayerExtensionFlag {
		pps.MultilayerExtension, err = parseMultilayerExtension(r)
		if err != nil {
			return pps, err
		}
	}
	if pps.D3ExtensionFlag {
		pps.D3Extension, err = parse3dExtension(r)
		if err != nil {
			return pps, err
		}
	}
	if pps.SccExtensionFlag {
		pps.SccExtension, err = parseSccExtension(r)
		if err != nil {
			return pps, err
		}
	}
	if pps.Extension4bits > 0 {
		// Reserved for future use. Shall be empty
		var more bool
		more, err = r.MoreRbspData()
		if err != nil {
			return nil, err
		}
		for more {
			pps.ExtensionDataFlag = append(pps.ExtensionDataFlag, r.ReadFlag())
			more, err = r.MoreRbspData()
			if err != nil {
				return nil, err
			}
		}
	}
	err = r.ReadRbspTrailingBits()
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
	return pps, nil
}

func parseRangeExtension(r *bits.EBSPReader, transformSkipEnabled bool) (*RangeExtension, error) {
	ext := &RangeExtension{}
	if transformSkipEnabled {
		ext.Log2MaxTransformSkipBlockSizeMinus2 = r.ReadExpGolomb()
	}
	ext.CrossComponentPredictionEnabledFlag = r.ReadFlag()
	ext.ChromaQpOffsetListEnabledFlag = r.ReadFlag()
	if ext.ChromaQpOffsetListEnabledFlag {
		ext.DiffCuChromaQpOffsetDepth = r.ReadExpGolomb()
		ext.ChromaQpOffsetListLenMinus1 = r.ReadExpGolomb()
		for i := uint(0); i <= ext.ChromaQpOffsetListLenMinus1; i++ {
			// values shall be in the range of −12 to +12, inclusive
			ext.CbQpOffsetList = append(ext.CbQpOffsetList, int8(r.ReadSignedGolomb()))
			ext.CrQpOffsetList = append(ext.CrQpOffsetList, int8(r.ReadSignedGolomb()))
		}
	}
	ext.Log2SaoOffsetScaleLuma = r.ReadExpGolomb()
	ext.Log2SaoOffsetScaleChroma = r.ReadExpGolomb()

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	return ext, nil
}

func parseMultilayerExtension(r *bits.EBSPReader) (*MultilayerExtension, error) {
	ext := &MultilayerExtension{}
	ext.PocResetInfoPresentFlag = r.ReadFlag()
	ext.InferScalingListFlag = r.ReadFlag()
	if ext.InferScalingListFlag {
		ext.ScalingListRefLayerId = uint8(r.Read(6))
	}
	ext.NumRefLocOffsets = r.ReadExpGolomb()
	ext.RefLocOffsets = make(map[uint8]RefLocOffset, int(ext.NumRefLocOffsets))
	for i := uint(0); i < ext.NumRefLocOffsets; i++ {
		ext.RefLocOffsetLayerIds = append(ext.RefLocOffsetLayerIds, uint8(r.Read(6)))

		off := RefLocOffset{}
		off.ScaledRefLayerOffsetPresentFlag = r.ReadFlag()
		if off.ScaledRefLayerOffsetPresentFlag {
			// value shall be in the range of −2^14 to 2^14 − 1, inclusive
			off.ScaledRefLayerLeftOffset = int16(r.ReadSignedGolomb())
			off.ScaledRefLayerTopOffset = int16(r.ReadSignedGolomb())
			off.ScaledRefLayerRightOffset = int16(r.ReadSignedGolomb())
			off.ScaledRefLayerBottomOffset = int16(r.ReadSignedGolomb())
		}
		off.RefRegionOffsetPresentFlag = r.ReadFlag()
		if off.RefRegionOffsetPresentFlag {
			// value shall be in the range of −2^14 to 2^14 − 1, inclusive
			off.RefRegionLeftOffset = int16(r.ReadSignedGolomb())
			off.RefRegionTopOffset = int16(r.ReadSignedGolomb())
			off.RefRegionRightOffset = int16(r.ReadSignedGolomb())
			off.RefRegionBottomOffset = int16(r.ReadSignedGolomb())
		}
		off.ResamplePhaseSetPresentFlag = r.ReadFlag()
		if off.ResamplePhaseSetPresentFlag {
			// value shall be in the range of 0 to 31, inclusive
			off.PhaseHorLuma = uint8(r.ReadExpGolomb())
			off.PhaseVerLuma = uint8(r.ReadExpGolomb())
			// value shall be in the range of 0 to 63, inclusive
			off.PhaseHorChromaPlus8 = uint8(r.ReadExpGolomb())
			off.PhaseVerChromaPlus8 = uint8(r.ReadExpGolomb())
		}
		ext.RefLocOffsets[ext.RefLocOffsetLayerIds[i]] = off
	}
	ext.ColourMappingEnabledFlag = r.ReadFlag()
	if ext.ColourMappingEnabledFlag {
		var err error
		ext.ColourMappingTable, err = parseColourMappingTable(r)
		if err != nil {
			return ext, err
		}
	}
	if r.AccError() != nil {
		return nil, r.AccError()
	}

	return ext, nil
}

func parseColourMappingTable(r *bits.EBSPReader) (*ColourMappingTable, error) {
	cm := &ColourMappingTable{}
	// value shall be in the range of 0 to 61, inclusive
	cm.NumCmRefLayersMinus1 = uint8(r.ReadExpGolomb())
	for i := uint8(0); i <= cm.NumCmRefLayersMinus1; i++ {
		cm.RefLayerId = append(cm.RefLayerId, uint8(r.Read(6)))
	}
	cm.OctantDepth = uint8(r.Read(2))
	cm.YPartNumLog2 = uint8(r.Read(2))
	cm.LumaBitDepthCmInputMinus8 = r.ReadExpGolomb()
	cm.ChromaBitDepthCmInputMinus8 = r.ReadExpGolomb()
	cm.LumaBitDepthCmOutputMinus8 = r.ReadExpGolomb()
	cm.ChromaBitDepthCmOutputMinus8 = r.ReadExpGolomb()
	cm.ResQuantBits = uint8(r.Read(2))
	cm.DeltaFlcBitsMinus1 = uint8(r.Read(2))
	if cm.OctantDepth == 1 {
		cm.AdaptThresholdUDelta = r.ReadSignedGolomb()
		cm.AdaptThresholdVDelta = r.ReadSignedGolomb()
	}

	//Max( 0, ( 10 + BitDepthCmInputY − BitDepthCmOutputY − cm_res_quant_bits − ( cm_delta_flc_bits_minus1 + 1 ) ) )
	//BitDepthCmInputY = 8 + luma_bit_depth_cm_input_minus8
	//BitDepthCmOutputY = 8 + luma_bit_depth_cm_output_minus8
	resLsBits := 10 + int(cm.LumaBitDepthCmInputMinus8+8) -
		int(cm.LumaBitDepthCmOutputMinus8+8) - int(cm.ResQuantBits) - int(cm.DeltaFlcBitsMinus1+1)
	if resLsBits < 0 {
		resLsBits = 0
	}

	var err error
	cm.Octants, err = parseColourMappingOctants(r, uint(cm.OctantDepth), 1<<cm.YPartNumLog2, resLsBits,
		0, 0, 0, 0, 1<<cm.OctantDepth)
	if err != nil {
		return cm, err
	}

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	return cm, nil
}

func parseColourMappingOctants(r *bits.EBSPReader, octantDepth uint, partNumY uint, resLsBits int,
	inpDepth, idxY, idxCb, idxCr, inpLength uint) (map[string][4]Octant, error) {
	var octs map[string][4]Octant

	var splitOctantFlag bool
	if inpDepth < octantDepth {
		splitOctantFlag = r.ReadFlag()
	}
	if splitOctantFlag {
		for k := uint(0); k < 2; k++ {
			for m := uint(0); m < 2; m++ {
				for n := uint(0); n < 2; n++ {
					var err error
					octs, err = parseColourMappingOctants(r, octantDepth, partNumY, resLsBits,
						inpDepth+1, idxY+partNumY*k*inpLength/2, idxCb+m*inpLength/2, idxCr+n*inpLength/2, inpLength/2)
					if err != nil {
						return octs, err
					}
				}
			}
		}
	} else {
		octs = make(map[string][4]Octant, partNumY)
		for i := uint(0); i < partNumY; i++ {
			// A map is used instead of the 5-dimensional array in the standard pseudo-code
			// Key represent [ idxShiftY ][ idxCb ][ idxCr ] with idxShiftY variable part
			key := makeKeyOctant(idxY+(i<<(octantDepth-inpDepth)), idxCb, idxCr)
			var oct [4]Octant
			for j := 0; j < 4; j++ {
				oct[j].CodedResFlag = r.ReadFlag()
				if oct[j].CodedResFlag {
					for c := 0; c < 3; c++ {
						oct[j].CodedRes[c].ResCoeffQ = r.ReadExpGolomb()
						oct[j].CodedRes[c].ResCoeffR = r.Read(resLsBits)
						if oct[j].CodedRes[c].ResCoeffQ != 0 || oct[j].CodedRes[c].ResCoeffR != 0 {
							oct[j].CodedRes[c].ResCoeffS = r.ReadFlag()
						}
					}
				}
			}
			octs[key] = oct
		}
	}

	if r.AccError() != nil {
		return octs, r.AccError()
	}

	return octs, nil
}

func makeKeyOctant(idxShiftY, idxCb, idxCr uint) string {
	return fmt.Sprintf("%d-%d-%d", idxShiftY, idxCb, idxCr)
}

func parseSccExtension(r *bits.EBSPReader) (*SccExtension, error) {
	ext := &SccExtension{}
	ext.CurrPicRefEnabledFlag = r.ReadFlag()
	ext.ResidualAdaptiveColourTransformEnabledFlag = r.ReadFlag()
	if ext.ResidualAdaptiveColourTransformEnabledFlag {
		ext.SliceActQpOffsetsPresentFlag = r.ReadFlag()
		ext.ActYQpOffsetPlus5 = r.ReadSignedGolomb()
		ext.ActCbQpOffsetPlus5 = r.ReadSignedGolomb()
		ext.ActCrQpOffsetPlus3 = r.ReadSignedGolomb()
	}
	ext.PalettePredictorInitializersPresentFlag = r.ReadFlag()
	if ext.PalettePredictorInitializersPresentFlag {
		ext.NumPalettePredictorInitializers = r.ReadExpGolomb()
		if ext.NumPalettePredictorInitializers > 0 {
			ext.MonochromePaletteFlag = r.ReadFlag()
			ext.LumaBitDepthEntryMinus8 = r.ReadExpGolomb()
			numComps := 1
			if !ext.MonochromePaletteFlag {
				numComps = 3
				ext.ChromaBitDepthEntryMinus8 = r.ReadExpGolomb()
			}
			ext.PalettePredictorInitializer = make([][]uint, numComps)
			// Fill luma
			for i := uint(0); i < ext.NumPalettePredictorInitializers; i++ {
				ext.PalettePredictorInitializer[0] =
					append(ext.PalettePredictorInitializer[0], r.Read(int(ext.LumaBitDepthEntryMinus8+8)))
			}
			// Fill chroma if any
			for comp := 1; comp < numComps; comp++ {
				for i := uint(0); i < ext.NumPalettePredictorInitializers; i++ {
					ext.PalettePredictorInitializer[comp] =
						append(ext.PalettePredictorInitializer[comp], r.Read(int(ext.ChromaBitDepthEntryMinus8+8)))
				}
			}
		}
	}

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	return ext, nil
}

func parse3dExtension(r *bits.EBSPReader) (*D3Extension, error) {
	ext := &D3Extension{}
	ext.DltsPresentFlag = r.ReadFlag()
	if ext.DltsPresentFlag {
		ext.NumDepthLayersMinus1 = uint8(r.Read(6))
		ext.BitDepthForDepthLayersMinus8 = uint8(r.Read(4))
		for i := uint8(0); i <= ext.NumDepthLayersMinus1; i++ {
			layer := DepthLayer{}
			layer.DltFlag = r.ReadFlag()
			if layer.DltFlag {
				layer.DltPredFlag = r.ReadFlag()
				if !layer.DltPredFlag {
					layer.DltValFlagsPresentFlag = r.ReadFlag()
				}
				if layer.DltValFlagsPresentFlag {
					// variable depthMaxValue is set equal to ( 1 << ( pps_bit_depth_for_depth_layers_minus8 + 8 ) ) − 1
					depthMaxValue := (1 << (ext.BitDepthForDepthLayersMinus8 + 8)) - 1
					for j := 0; j <= depthMaxValue; j++ {
						layer.DltValueFlag = append(layer.DltValueFlag, r.ReadFlag())
					}
				} else {
					var err error
					layer.DeltaDlt, err = parseDeltaDlt(r, int(ext.BitDepthForDepthLayersMinus8+8))
					if err != nil {
						return ext, err
					}
				}
			}
			ext.DepthLayers = append(ext.DepthLayers, layer)
		}
	}

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	return ext, nil
}

func parseDeltaDlt(r *bits.EBSPReader, BitDepthForDepthLayers int) (*DeltaDlt, error) {
	dd := &DeltaDlt{}
	dd.NumValDeltaDlt = r.Read(BitDepthForDepthLayers)
	if dd.NumValDeltaDlt > 0 {
		if dd.NumValDeltaDlt > 1 {
			dd.MaxDiff = r.Read(BitDepthForDepthLayers)
		}
		if dd.NumValDeltaDlt > 2 && dd.MaxDiff > 0 {
			dd.MinDiffMinus1 = r.Read(bits.CeilLog2(dd.MaxDiff + 1))
		} else {
			dd.MinDiffMinus1 = dd.MaxDiff - 1
		}
		dd.DeltaDltVal0 = r.Read(BitDepthForDepthLayers)
		if dd.MaxDiff > (dd.MinDiffMinus1 + 1) {
			for k := uint(1); k < dd.NumValDeltaDlt; k++ {
				// variable minDiff is set equal to ( min_diff_minus1 + 1 )
				// length of delta_val_diff_minus_min[ k ] syntax element is Ceil( Log2( max_diff − minDiff + 1 ) ) bits
				dd.DeltaValDiffMinusMin =
					append(dd.DeltaValDiffMinusMin, r.Read(bits.CeilLog2(dd.MaxDiff-(dd.MinDiffMinus1+1)+1)))
			}
		}
	}

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	return dd, nil
}
