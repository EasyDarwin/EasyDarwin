package avc

import (
	"bytes"
	"errors"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// PPS - Picture Parameter Set
type PPS struct {
	PicParameterSetID                     uint32
	SeqParameterSetID                     uint32
	EntropyCodingModeFlag                 bool
	BottomFieldPicOrderInFramePresentFlag bool
	NumSliceGroupsMinus1                  uint
	SliceGroupMapType                     uint
	RunLengthMinus1                       []uint
	TopLeft                               []uint
	BottomRight                           []uint
	SliceGroupChangeDirectionFlag         bool
	SliceGroupChangeRateMinus1            uint
	PicSizeInMapUnitsMinus1               uint
	SliceGroupID                          []uint
	NumRefIdxI0DefaultActiveMinus1        uint
	NumRefIdxI1DefaultActiveMinus1        uint
	WeightedPredFlag                      bool
	WeightedBipredIDC                     uint
	PicInitQpMinus26                      int
	PicInitQsMinus26                      int
	ChromaQpIndexOffset                   int
	DeblockingFilterControlPresentFlag    bool
	ConstrainedIntraPredFlag              bool
	RedundantPicCntPresentFlag            bool
	Transform8x8ModeFlag                  bool
	PicScalingMatrixPresentFlag           bool
	PicScalingLists                       []ScalingList
	SecondChromaQpIndexOffset             int
}

// AVC PPS errors
var (
	ErrNotPPS = errors.New("Not an PPS NAL unit")
)

// ParsePPSNALUnit - Parse AVC PPS NAL unit starting with NAL header
func ParsePPSNALUnit(data []byte, spsMap map[uint32]*SPS) (*PPS, error) {
	var err error

	pps := &PPS{}

	rd := bytes.NewReader(data)
	reader := bits.NewEBSPReader(rd)
	// Note! First byte is NAL Header

	naluHdr := reader.Read(8)
	naluType := GetNaluType(byte(naluHdr))
	if naluType != NALU_PPS {
		return nil, ErrNotPPS
	}

	pps.PicParameterSetID = uint32(reader.ReadExpGolomb())
	pps.SeqParameterSetID = uint32(reader.ReadExpGolomb())
	pps.EntropyCodingModeFlag = reader.ReadFlag()
	pps.BottomFieldPicOrderInFramePresentFlag = reader.ReadFlag()
	pps.NumSliceGroupsMinus1 = reader.ReadExpGolomb()

	if pps.NumSliceGroupsMinus1 > 0 {
		pps.SliceGroupMapType = reader.ReadExpGolomb()
		switch pps.SliceGroupMapType {
		case 0:
			for iGroup := uint(0); iGroup <= pps.NumSliceGroupsMinus1; iGroup++ {
				rl := reader.ReadExpGolomb()
				pps.RunLengthMinus1 = append(pps.RunLengthMinus1, rl)
			}
		case 2:
			for iGroup := uint(0); iGroup <= pps.NumSliceGroupsMinus1; iGroup++ {
				tl := reader.ReadExpGolomb()
				pps.TopLeft = append(pps.TopLeft, tl)
				br := reader.ReadExpGolomb()
				pps.BottomRight = append(pps.BottomRight, br)
			}
		case 3, 4, 5:
			pps.SliceGroupChangeDirectionFlag = reader.ReadFlag()
			pps.SliceGroupChangeRateMinus1 = reader.ReadExpGolomb()
		case 6:
			// slice_group_id[i] has Ceil(Log2(num_slice_groups_minus1 +1) bits)
			nrBits := bits.CeilLog2(pps.NumSliceGroupsMinus1 + 1)

			for iGroup := uint(0); iGroup <= pps.NumSliceGroupsMinus1; iGroup++ {
				sgi := reader.Read(nrBits)
				pps.SliceGroupID = append(pps.SliceGroupID, sgi)
			}
		}
	}
	pps.NumRefIdxI0DefaultActiveMinus1 = reader.ReadExpGolomb()
	pps.NumRefIdxI1DefaultActiveMinus1 = reader.ReadExpGolomb()
	pps.WeightedPredFlag = reader.ReadFlag()
	pps.WeightedBipredIDC = reader.Read(2)
	pps.PicInitQpMinus26 = reader.ReadSignedGolomb()
	pps.PicInitQsMinus26 = reader.ReadSignedGolomb()
	pps.ChromaQpIndexOffset = reader.ReadSignedGolomb()
	pps.DeblockingFilterControlPresentFlag = reader.ReadFlag()
	pps.ConstrainedIntraPredFlag = reader.ReadFlag()
	pps.RedundantPicCntPresentFlag = reader.ReadFlag()
	if !reader.IsSeeker() {
		// Cannot call MoreRbspData, so cannot parse further
		return pps, nil
	}
	moreRbsp, err := reader.MoreRbspData()
	if err != nil {
		if reader.AccError() != nil {
			return nil, reader.AccError()
		}
		return nil, err
	}

	if moreRbsp {
		pps.Transform8x8ModeFlag = reader.ReadFlag()
		pps.PicScalingMatrixPresentFlag = reader.ReadFlag()
		if pps.PicScalingMatrixPresentFlag {
			sps, ok := spsMap[pps.SeqParameterSetID]
			if !ok {
				return pps, fmt.Errorf("sps ID %d not found in map", pps.SeqParameterSetID)
			}
			nrScalingLists := 6
			if pps.Transform8x8ModeFlag {
				if sps.ChromaFormatIDC != 3 {
					nrScalingLists += 2
				} else {
					nrScalingLists += 6
				}
				pps.PicScalingLists = make([]ScalingList, nrScalingLists)

				for i := 0; i < nrScalingLists; i++ {
					picScalingPresent := reader.ReadFlag()
					if !picScalingPresent {
						pps.PicScalingLists[i] = nil
						continue
					}
					sizeOfScalingList := 16 // 4x4 for i < 6
					if i >= 6 {
						sizeOfScalingList = 64 // 8x8 for i >= 6
					}
					pps.PicScalingLists[i] = readScalingList(reader, sizeOfScalingList)
				}
			}
		}
		pps.SecondChromaQpIndexOffset = reader.ReadSignedGolomb()
	}

	err = reader.ReadRbspTrailingBits()
	if err != nil {
		if reader.AccError() != nil {
			return nil, reader.AccError()
		}
		return nil, err
	}
	if reader.AccError() != nil {
		return nil, reader.AccError()
	}
	_ = reader.Read(1)
	if reader.AccError() != io.EOF {
		return nil, fmt.Errorf("Not at end after reading rbsp_trailing_bits")
	}
	return pps, nil
}
