package hevc

import (
	"bytes"
	"errors"
	"fmt"

	"github.com/Eyevinn/mp4ff/sei"
)

var (
	ErrNotSEINalu = errors.New("not an SEI NAL unit")
)

// ParseSEINalu - parse SEI NAL unit (incl header) and return messages given SPS.
// Returns sei.ErrRbspTrailingBitsMissing if the NALU is missing the trailing bits.
func ParseSEINalu(nalu []byte, sps *SPS) ([]sei.SEIMessage, error) {
	switch GetNaluType(nalu[0]) {
	case NALU_SEI_PREFIX, NALU_SEI_SUFFIX:
	default:
		return nil, ErrNotSEINalu
	}
	seiBytes := nalu[2:] // Skip NALU header
	buf := bytes.NewReader(seiBytes)
	seiDatas, err := sei.ExtractSEIData(buf)
	missingRbspTrailingBits := false
	if err != nil {
		if errors.Is(err, sei.ErrRbspTrailingBitsMissing) {
			missingRbspTrailingBits = true
		} else {
			return nil, fmt.Errorf("extracting SEI data: %w", err)
		}
	}

	seiMsgs := make([]sei.SEIMessage, 0, len(seiDatas))
	var seiMsg sei.SEIMessage
	for _, seiData := range seiDatas {
		switch {
		case seiData.Type() == sei.SEIPicTimingType && sps != nil && sps.VUI != nil:
			htp := fillHEVCPicTimingParams(sps)
			seiMsg, err = sei.DecodePicTimingHevcSEI(&seiData, htp)
		default:
			seiMsg, err = sei.DecodeSEIMessage(&seiData, sei.HEVC)
		}
		if err != nil {
			return nil, fmt.Errorf("sei decode: %w", err)
		}
		seiMsgs = append(seiMsgs, seiMsg)
	}
	if missingRbspTrailingBits {
		return seiMsgs, sei.ErrRbspTrailingBitsMissing
	}
	return seiMsgs, nil
}

func fillHEVCPicTimingParams(sps *SPS) sei.HEVCPicTimingParams {
	hpt := sei.HEVCPicTimingParams{}
	if sps.VUI == nil {
		return hpt
	}
	hpt.FrameFieldInfoPresentFlag = sps.VUI.FrameFieldInfoPresentFlag
	hrd := sps.VUI.HrdParameters
	if hrd == nil {
		return hpt
	}
	hpt.CpbDpbDelaysPresentFlag = hrd.CpbDpbDelaysPresentFlag()
	hpt.SubPicHrdParamsPresentFlag = hrd.SubPicHrdParamsPresentFlag
	hpt.SubPicCpbParamsInPicTimingSeiFlag = hrd.SubPicCpbParamsInPicTimingSeiFlag
	hpt.AuCbpRemovalDelayLengthMinus1 = hrd.AuCpbRemovalDelayLengthMinus1
	hpt.DpbOutputDelayLengthMinus1 = hrd.DpbOutputDelayLengthMinus1
	hpt.DpbOutputDelayDuLengthMinus1 = hrd.DpbOutputDelayDuLengthMinus1
	hpt.DuCpbRemovalDelayIncrementLengthMinus1 = hrd.DuCpbRemovalDelayIncrementLengthMinus1
	return hpt
}
