package avc

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
	if GetNaluType(nalu[0]) != NALU_SEI {
		return nil, ErrNotSEINalu
	}
	seiBytes := nalu[1:] // Skip NALU header
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
			var cbpDbpDelay *sei.CbpDbpDelay
			var timeOffsetLen byte = 0
			hrdParams := sps.VUI.VclHrdParameters
			if hrdParams == nil {
				hrdParams = sps.VUI.NalHrdParameters
			}
			if hrdParams != nil {
				cbpDbpDelay = &sei.CbpDbpDelay{
					CpbRemovalDelayLengthMinus1: byte(hrdParams.CpbRemovalDelayLengthMinus1),
					DpbOutputDelayLengthMinus1:  byte(hrdParams.DpbOutputDelayLengthMinus1),
				}
				timeOffsetLen = byte(hrdParams.TimeOffsetLength)
			}
			seiMsg, err = sei.DecodePicTimingAvcSEIHRD(&seiData, cbpDbpDelay, timeOffsetLen)
		default:
			seiMsg, err = sei.DecodeSEIMessage(&seiData, sei.AVC)
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
