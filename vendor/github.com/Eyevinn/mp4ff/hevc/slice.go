package hevc

import (
	"bytes"
	"errors"
	"fmt"

	"github.com/Eyevinn/mp4ff/bits"
)

// This parser based on Rec. ITU-T H.265 v5 (02/2018) and ISO/IEC 23008-2 Ed. 5
// It implements specification 7.3.6 . Annex F/I extensions aren't supported yet.

// SliceType - HEVC slice type
type SliceType uint

func (s SliceType) String() string {
	switch s {
	case SLICE_I:
		return "I"
	case SLICE_P:
		return "P"
	case SLICE_B:
		return "B"
	default:
		return ""
	}
}

// HEVC slice types
const (
	SLICE_B = SliceType(0)
	SLICE_P = SliceType(1)
	SLICE_I = SliceType(2)
)

type SliceHeader struct {
	SliceType                         SliceType
	FirstSliceSegmentInPicFlag        bool
	NoOutputOfPriorPicsFlag           bool
	PicParameterSetId                 uint32
	DependentSliceSegmentFlag         bool
	SegmentAddress                    uint
	PicOutputFlag                     bool
	ColourPlaneId                     uint8
	PicOrderCntLsb                    uint16
	ShortTermRefPicSetSpsFlag         bool
	ShortTermRefPicSet                ShortTermRPS
	ShortTermRefPicSetIdx             byte
	NumLongTermSps                    uint8
	NumLongTermPics                   uint
	LongTermRefPicSets                []LongTermRPS
	TemporalMvpEnabledFlag            bool
	SaoLumaFlag                       bool
	SaoChromaFlag                     bool
	NumRefIdxActiveOverrideFlag       bool
	NumRefIdxL0ActiveMinus1           uint8
	NumRefIdxL1ActiveMinus1           uint8
	RefPicListsModification           *RefPicListsModification
	MvdL1ZeroFlag                     bool
	CabacInitFlag                     bool
	CollocatedFromL0Flag              bool
	CollocatedRefIdx                  uint8
	PredWeightTable                   *PredWeightTable
	FiveMinusMaxNumMergeCand          uint8
	UseIntegerMvFlag                  bool
	QpDelta                           int
	CbQpOffset                        int8
	CrQpOffset                        int8
	ActYQpOffset                      int8
	ActCbQpOffset                     int8
	ActCrQpOffset                     int8
	CuChromaQpOffsetEnabledFlag       bool
	DeblockingFilterOverrideFlag      bool
	DeblockingFilterDisabledFlag      bool
	BetaOffsetDiv2                    int8
	TcOffsetDiv2                      int8
	LoopFilterAcrossSlicesEnabledFlag bool
	NumEntryPointOffsets              uint
	OffsetLenMinus1                   uint8
	EntryPointOffsetMinus1            []uint32
	SegmentHeaderExtensionLength      uint16
	SegmentHeaderExtensionDataByte    []byte
	Size                              uint32
}

type RefPicListsModification struct {
	RefPicListModificationFlagL0 bool
	ListEntryL0                  []uint8
	RefPicListModificationFlagL1 bool
	ListEntryL1                  []uint8
}

type PredWeightTable struct {
	LumaLog2WeightDenom        uint8
	DeltaChromaLog2WeightDenom int8
	WeightsL0                  []WeightingFactors
	WeightsL1                  []WeightingFactors
}

// WeightingFactors fields described in specification 7.4.7.3 (Weighted prediction parameters semantics)
type WeightingFactors struct {
	LumaWeightFlag    bool
	ChromaWeightFlag  bool
	DeltaLumaWeight   int8
	LumaOffset        int
	DeltaChromaWeight [2]int8
	DeltaChromaOffset [2]int
}

func ParseSliceHeader(nalu []byte, spsMap map[uint32]*SPS, ppsMap map[uint32]*PPS) (*SliceHeader, error) {
	sh := &SliceHeader{}

	buf := bytes.NewBuffer(nalu)
	r := bits.NewEBSPReader(buf)

	naluHdrBits := r.Read(16)
	naluType := GetNaluType(byte(naluHdrBits >> 8))
	sh.FirstSliceSegmentInPicFlag = r.ReadFlag()
	if naluType >= NALU_BLA_W_LP && naluType <= NALU_IRAP_VCL23 {
		sh.NoOutputOfPriorPicsFlag = r.ReadFlag()
	}
	sh.PicParameterSetId = uint32(r.ReadExpGolomb())
	pps, ok := ppsMap[sh.PicParameterSetId]
	if !ok {
		return nil, fmt.Errorf("pps ID %d unknown", sh.PicParameterSetId)
	}
	sps, ok := spsMap[pps.SeqParameterSetID]
	if !ok {
		return nil, fmt.Errorf("sps ID %d unknown", pps.SeqParameterSetID)
	}

	if !sh.FirstSliceSegmentInPicFlag {
		if pps.DependentSliceSegmentsEnabledFlag {
			sh.DependentSliceSegmentFlag = r.ReadFlag()
		}
		/*
			Pseudocode from standard:

			MinCbLog2SizeY = log2_min_luma_coding_block_size_minus3 + 3
			CtbLog2SizeY = MinCbLog2SizeY + log2_diff_max_min_luma_coding_block_size
			CtbSizeY = 1 << CtbLog2SizeY
			PicWidthInCtbsY = Ceil( pic_width_in_luma_samples ÷ CtbSizeY )
			PicHeightInCtbsY = Ceil( pic_height_in_luma_samples ÷ CtbSizeY )
			PicSizeInCtbsY = PicWidthInCtbsY * PicHeightInCtbsY
		*/
		CtbSizeY := uint(1 << (sps.Log2MinLumaCodingBlockSizeMinus3 + 3 + sps.Log2DiffMaxMinLumaCodingBlockSize))
		PicSizeInCtbsY := ceilDiv(uint(sps.PicWidthInLumaSamples), CtbSizeY) *
			ceilDiv(uint(sps.PicHeightInLumaSamples), CtbSizeY)
		sh.SegmentAddress = r.Read(bits.CeilLog2(PicSizeInCtbsY))
	}

	if !sh.DependentSliceSegmentFlag {
		/*
				Pseudocode from standard:

				NumPicTotalCurr = 0
				if( nal_unit_type != IDR_W_RADL && nal_unit_type != IDR_N_LP ) {
					for( i = 0; i < NumNegativePics[ CurrRpsIdx ]; i++ ) if( UsedByCurrPicS0[ CurrRpsIdx ][ i ] )
						NumPicTotalCurr++
					for( i = 0; i < NumPositivePics[ CurrRpsIdx ]; i++ ) if( UsedByCurrPicS1[ CurrRpsIdx ][ i ] )
			    		NumPicTotalCurr++
					for( i = 0; i < num_long_term_sps + num_long_term_pics; i++ ) if( UsedByCurrPicLt[ i ] )
						NumPicTotalCurr++
				}
				if( pps_curr_pic_ref_enabled_flag )
					NumPicTotalCurr++
				NumPicTotalCurr += NumActiveRefLayerPics
		*/
		var NumPicTotalCurr uint8

		// The variable ChromaArrayType is derived as equal to 0 when separate_colour_plane_flag is equal to 1
		// and chroma_format_idc is equal to 3.
		ChromaArrayType := sps.ChromaFormatIDC
		if sps.SeparateColourPlaneFlag && sps.ChromaFormatIDC == 3 {
			ChromaArrayType = 0
		}

		// Decoders shall ignore the presence and value of slice_reserved_flag[ i ]
		for i := uint8(0); i < pps.NumExtraSliceHeaderBits; i++ {
			_ = r.ReadFlag()
		}
		sh.SliceType = SliceType(r.ReadExpGolomb())
		if pps.OutputFlagPresentFlag {
			sh.PicOutputFlag = r.ReadFlag()
		}
		if sps.SeparateColourPlaneFlag {
			sh.ColourPlaneId = uint8(r.Read(2))
		}
		if naluType != NALU_IDR_W_RADL && naluType != NALU_IDR_N_LP {
			// value of log2_max_pic_order_cnt_lsb_minus4 shall be in the range of 0 to 12, inclusive
			sh.PicOrderCntLsb = uint16(r.Read(int(sps.Log2MaxPicOrderCntLsbMinus4 + 4)))
			sh.ShortTermRefPicSetSpsFlag = r.ReadFlag()

			if !sh.ShortTermRefPicSetSpsFlag {
				sh.ShortTermRefPicSet = parseShortTermRPS(r, sps.NumShortTermRefPicSets,
					sps.NumShortTermRefPicSets, sps)
				if r.AccError() != nil {
					return sh, r.AccError()
				}
			} else if sps.NumShortTermRefPicSets > 1 {
				sh.ShortTermRefPicSetIdx = byte(r.Read(bits.CeilLog2(uint(sps.NumShortTermRefPicSets))))
				if int(sh.ShortTermRefPicSetIdx) >= len(sps.ShortTermRefPicSets) {
					return sh, fmt.Errorf("short_term_ref_pic_set_idx > num_short_term_ref_pic_sets")
				}
				sh.ShortTermRefPicSet = sps.ShortTermRefPicSets[sh.ShortTermRefPicSetIdx]
			}
			NumPicTotalCurr += sh.ShortTermRefPicSet.countInUsePics()

			if sps.LongTermRefPicsPresentFlag {
				if sps.NumLongTermRefPics > 0 {
					// value shall be in the range of 0 to num_long_term_ref_pics_sps, inclusive
					sh.NumLongTermSps = uint8(r.ReadExpGolomb())
				}
				sh.NumLongTermPics = r.ReadExpGolomb()
				for i := uint(0); i < uint(sh.NumLongTermSps)+sh.NumLongTermPics; i++ {
					var lt LongTermRPS
					if i < uint(sh.NumLongTermSps) {
						if sps.NumLongTermRefPics > 1 {
							LtIdxSps := r.Read(bits.CeilLog2(uint(sps.NumLongTermRefPics)))
							if int(LtIdxSps) >= len(sps.LongTermRefPicSets) {
								return sh, fmt.Errorf("lt_idx_sps > num_long_term_ref_pics_sps")
							}
							lt = sps.LongTermRefPicSets[LtIdxSps]
						}
					} else {
						lt.PocLsbLt = uint16(r.Read(int(sps.Log2MaxPicOrderCntLsbMinus4 + 4)))
						lt.UsedByCurrPicLtFlag = r.ReadFlag()
					}
					if lt.UsedByCurrPicLtFlag {
						NumPicTotalCurr++
					}
					lt.DeltaPocMsbPresentFlag = r.ReadFlag()
					if lt.DeltaPocMsbPresentFlag {
						lt.DeltaPocMsbCycleLt = r.ReadExpGolomb()
					}
					sh.LongTermRefPicSets = append(sh.LongTermRefPicSets, lt)
				}
			}
			if sps.SpsTemporalMvpEnabledFlag {
				sh.TemporalMvpEnabledFlag = r.ReadFlag()
			}
		}
		if sps.SampleAdaptiveOffsetEnabledFlag {
			sh.SaoLumaFlag = r.ReadFlag()
			if ChromaArrayType != 0 {
				sh.SaoChromaFlag = r.ReadFlag()
			}
		}
		if sh.SliceType == SLICE_P || sh.SliceType == SLICE_B {
			sh.NumRefIdxActiveOverrideFlag = r.ReadFlag()
			// When the current slice is a P or B slice and num_ref_idx_l0_active_minus1 is not present,
			// num_ref_idx_l0_active_minus1 is inferred to be equal to num_ref_idx_l0_default_active_minus1.
			sh.NumRefIdxL0ActiveMinus1 = pps.NumRefIdxL0DefaultActiveMinus1
			sh.NumRefIdxL1ActiveMinus1 = pps.NumRefIdxL1DefaultActiveMinus1
			// 0 specifies that the syntax elements num_ref_idx_l0_active_minus1 and num_ref_idx_l1_active_minus1 are not present.
			if sh.NumRefIdxActiveOverrideFlag {
				// value shall be in the range of 0 to 14, inclusive
				sh.NumRefIdxL0ActiveMinus1 = uint8(r.ReadExpGolomb())
				if sh.SliceType == SLICE_B {
					sh.NumRefIdxL1ActiveMinus1 = uint8(r.ReadExpGolomb())
				}
			}

			if pps.ListsModificationPresentFlag {
				if pps.SccExtension != nil && pps.SccExtension.CurrPicRefEnabledFlag {
					NumPicTotalCurr++
				}
				if NumPicTotalCurr > 1 {
					var err error
					sh.RefPicListsModification, err = parseRefPicListsModification(r, sh.SliceType,
						sh.NumRefIdxL0ActiveMinus1, sh.NumRefIdxL1ActiveMinus1, NumPicTotalCurr)
					if err != nil {
						return sh, err
					}
				}
			}
			if sh.SliceType == SLICE_B {
				sh.MvdL1ZeroFlag = r.ReadFlag()
			}
			if pps.CabacInitPresentFlag {
				sh.CabacInitFlag = r.ReadFlag()
			}
			if sh.TemporalMvpEnabledFlag {
				if sh.SliceType == SLICE_B {
					sh.CollocatedFromL0Flag = r.ReadFlag()
				}
				if (sh.CollocatedFromL0Flag && sh.NumRefIdxL0ActiveMinus1 > 0) ||
					(!sh.CollocatedFromL0Flag && sh.NumRefIdxL1ActiveMinus1 > 0) {
					// value shall be in the range of 0 to num_ref_idx_l0_active_minus1, inclusive
					sh.CollocatedRefIdx = uint8(r.ReadExpGolomb())
				}
			}
			if (pps.WeightedPredFlag && sh.SliceType == SLICE_P) ||
				(pps.WeightedBipredFlag && sh.SliceType == SLICE_B) {
				var err error
				sh.PredWeightTable, err = parsePredWeightTable(r, sh.SliceType,
					sh.NumRefIdxL0ActiveMinus1, sh.NumRefIdxL1ActiveMinus1, ChromaArrayType)
				if err != nil {
					return sh, err
				}
			}
			// MaxNumMergeCand = 5 − five_minus_max_num_merge_cand
			// value of MaxNumMergeCand shall be in the range of 1 to 5, inclusive
			sh.FiveMinusMaxNumMergeCand = uint8(r.ReadExpGolomb())
			if sps.SccExtension != nil && sps.SccExtension.MotionVectorResolutionControlIdc == 2 {
				sh.UseIntegerMvFlag = r.ReadFlag()
			}
		}
		sh.QpDelta = r.ReadSignedGolomb()
		if pps.SliceChromaQpOffsetsPresentFlag {
			// values shall be in the range of −12 to +12, inclusive
			sh.CbQpOffset = int8(r.ReadSignedGolomb())
			sh.CrQpOffset = int8(r.ReadSignedGolomb())
		}
		if pps.SccExtension != nil && pps.SccExtension.SliceActQpOffsetsPresentFlag {
			// values shall be in the range of −12 to +12, inclusive
			sh.ActYQpOffset = int8(r.ReadSignedGolomb())
			sh.ActCbQpOffset = int8(r.ReadSignedGolomb())
			sh.ActCrQpOffset = int8(r.ReadSignedGolomb())
		}
		if pps.RangeExtension != nil && pps.RangeExtension.ChromaQpOffsetListEnabledFlag {
			sh.CuChromaQpOffsetEnabledFlag = r.ReadFlag()
		}
		if pps.DeblockingFilterOverrideEnabledFlag {
			sh.DeblockingFilterOverrideFlag = r.ReadFlag()
		}
		if sh.DeblockingFilterOverrideFlag {
			sh.DeblockingFilterDisabledFlag = r.ReadFlag()
			if !sh.DeblockingFilterDisabledFlag {
				// values shall both be in the range of −6 to 6, inclusive
				sh.BetaOffsetDiv2 = int8(r.ReadSignedGolomb())
				sh.TcOffsetDiv2 = int8(r.ReadSignedGolomb())
			}
		}
		if pps.LoopFilterAcrossSlicesEnabledFlag &&
			(sh.SaoLumaFlag || sh.SaoChromaFlag || !sh.DeblockingFilterDisabledFlag) {
			sh.LoopFilterAcrossSlicesEnabledFlag = r.ReadFlag()
		}
	}
	if pps.TilesEnabledFlag || pps.EntropyCodingSyncEnabledFlag {
		sh.NumEntryPointOffsets = r.ReadExpGolomb()
		if sh.NumEntryPointOffsets > 0 {
			// value shall be in the range of 0 to 31, inclusive
			sh.OffsetLenMinus1 = uint8(r.ReadExpGolomb())
			if sh.NumEntryPointOffsets > 0 {
				sh.EntryPointOffsetMinus1 = make([]uint32, sh.NumEntryPointOffsets)
				for i := uint(0); i < sh.NumEntryPointOffsets; i++ {
					sh.EntryPointOffsetMinus1[i] = uint32(r.Read(int(sh.OffsetLenMinus1 + 1)))
				}
			}
		}
	}
	if pps.SliceSegmentHeaderExtensionPresentFlag {
		// value shall be in the range of 0 to 256, inclusive
		sh.SegmentHeaderExtensionLength = uint16(r.ReadExpGolomb())
		if sh.SegmentHeaderExtensionLength > 0 {
			sh.SegmentHeaderExtensionDataByte = make([]byte, sh.SegmentHeaderExtensionLength)
			for i := uint16(0); i < sh.SegmentHeaderExtensionLength; i++ {
				sh.SegmentHeaderExtensionDataByte[i] = byte(r.Read(8))
			}
		}
	}

	if !r.ReadFlag() {
		return sh, errors.New("alignment bit is not equal to one")
	}
	for r.NrBitsReadInCurrentByte() < 8 {
		if r.ReadFlag() {
			return sh, errors.New("bit after alignment is not equal to zero")
		}
	}

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	// compute the size in bytes. last byte is always aligned
	sh.Size = uint32(r.NrBytesRead())

	return sh, nil
}

func parseRefPicListsModification(r *bits.EBSPReader, sliceType SliceType,
	refIdxL0Minus1, refIdxL1Minus1 uint8, numPicTotalCurr uint8) (*RefPicListsModification, error) {
	rplm := &RefPicListsModification{
		RefPicListModificationFlagL0: r.ReadFlag(),
	}
	if rplm.RefPicListModificationFlagL0 {
		rplm.ListEntryL0 = make([]uint8, refIdxL0Minus1+1)
		for i := uint8(0); i <= refIdxL0Minus1; i++ {
			rplm.ListEntryL0[i] = uint8(r.Read(bits.CeilLog2(uint(numPicTotalCurr))))
		}
	}
	if sliceType == SLICE_B {
		rplm.RefPicListModificationFlagL1 = r.ReadFlag()
		if rplm.RefPicListModificationFlagL1 {
			rplm.ListEntryL1 = make([]uint8, refIdxL1Minus1+1)
			for i := uint8(0); i <= refIdxL1Minus1; i++ {
				rplm.ListEntryL1[i] = uint8(r.Read(bits.CeilLog2(uint(numPicTotalCurr))))
			}
		}
	}

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	return rplm, nil
}

func parsePredWeightTable(r *bits.EBSPReader, sliceType SliceType,
	refIdxL0Minus1, refIdxL1Minus1 uint8, chromaArrayType byte) (*PredWeightTable, error) {
	pwt := &PredWeightTable{
		// value shall be in the range of 0 to 7, inclusive
		LumaLog2WeightDenom: uint8(r.ReadExpGolomb()),
	}
	if chromaArrayType != 0 {
		// ChromaLog2WeightDenom is derived to be equal to luma_log2_weight_denom + delta_chroma_log2_weight_denom
		// and the value shall be in the range of 0 to 7, inclusive
		pwt.DeltaChromaLog2WeightDenom = int8(r.ReadSignedGolomb())
	}

	pwt.WeightsL0 = make([]WeightingFactors, refIdxL0Minus1+1)
	for i := uint8(0); i <= refIdxL0Minus1; i++ {
		// Not implemented
		// if( ( pic_layer_id( RefPicList0[ i ] ) != nuh_layer_id ) | |
		//( PicOrderCnt( RefPicList0[ i ] ) != PicOrderCnt( CurrPic ) ) )
		pwt.WeightsL0[i].LumaWeightFlag = r.ReadFlag()
	}
	if chromaArrayType != 0 {
		for i := uint8(0); i <= refIdxL0Minus1; i++ {
			// Not implemented
			// if( ( pic_layer_id( RefPicList0[ i ] ) != nuh_layer_id ) | |
			//( PicOrderCnt( RefPicList0[ i ] ) != PicOrderCnt( CurrPic ) ) )
			pwt.WeightsL0[i].ChromaWeightFlag = r.ReadFlag()
		}
	}
	for i := uint8(0); i <= refIdxL0Minus1; i++ {
		if pwt.WeightsL0[i].LumaWeightFlag {
			// value shall be in the range of −128 to 127, inclusive
			pwt.WeightsL0[i].DeltaLumaWeight = int8(r.ReadSignedGolomb())
			pwt.WeightsL0[i].LumaOffset = r.ReadSignedGolomb()
		}
		if pwt.WeightsL0[i].ChromaWeightFlag {
			for j := 0; j < 2; j++ {
				// value shall be in the range of −128 to 127, inclusive
				pwt.WeightsL0[i].DeltaChromaWeight[j] = int8(r.ReadSignedGolomb())
				pwt.WeightsL0[i].DeltaChromaOffset[j] = r.ReadSignedGolomb()
			}
		}
	}
	if sliceType == SLICE_B {
		pwt.WeightsL1 = make([]WeightingFactors, refIdxL1Minus1+1)
		for i := uint8(0); i <= refIdxL1Minus1; i++ {
			// Not implemented
			// if( ( pic_layer_id( RefPicList0[ i ] ) != nuh_layer_id ) | |
			//( PicOrderCnt( RefPicList1[ i ] ) != PicOrderCnt( CurrPic ) ) )
			pwt.WeightsL1[i].LumaWeightFlag = r.ReadFlag()
		}
		if chromaArrayType != 0 {
			for i := uint8(0); i <= refIdxL1Minus1; i++ {
				// Not implemented
				// if( ( pic_layer_id( RefPicList0[ i ] ) != nuh_layer_id ) | |
				//( PicOrderCnt( RefPicList1[ i ] ) != PicOrderCnt( CurrPic ) ) )
				pwt.WeightsL1[i].ChromaWeightFlag = r.ReadFlag()
			}
		}
		for i := uint8(0); i <= refIdxL1Minus1; i++ {
			if pwt.WeightsL1[i].LumaWeightFlag {
				// value shall be in the range of −128 to 127, inclusive
				pwt.WeightsL1[i].DeltaLumaWeight = int8(r.ReadSignedGolomb())
				pwt.WeightsL1[i].LumaOffset = r.ReadSignedGolomb()
			}
			if pwt.WeightsL1[i].ChromaWeightFlag {
				for j := 0; j < 2; j++ {
					// value shall be in the range of −128 to 127, inclusive
					pwt.WeightsL1[i].DeltaChromaWeight[j] = int8(r.ReadSignedGolomb())
					pwt.WeightsL1[i].DeltaChromaOffset[j] = r.ReadSignedGolomb()
				}
			}
		}
	}

	if r.AccError() != nil {
		return nil, r.AccError()
	}

	return pwt, nil
}

func ceilDiv(a, b uint) uint {
	return (a + b - 1) / b
}
