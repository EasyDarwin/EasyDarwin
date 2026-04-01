package sei

import (
	"bytes"
	"fmt"

	"github.com/Eyevinn/mp4ff/bits"
)

// PicTimingHevcSEI carries the data of an SEI 1 PicTiming message for HEVC.
// The corresponding SEI 1 for AVC is very different. Time code is in SEI 136 for HEVC.
// Defined in ISO/IEC 23008-2 Ed 5. Section D.2.3 (page 372) and D.3.2.3 (page 405)
type PicTimingHevcSEI struct {
	ExternalParams                         HEVCPicTimingParams `json:"-"`
	FrameFieldInfo                         *HEVCFrameFieldInfo `json:"FrameFieldInfo,omitempty"`
	AuCpbRemovalDelayMinus1                uint32              `json:"AuCpbRemovalDelayMinus1,omitempty"`
	PicDpbOutputDelay                      uint32              `json:"PicDpbOutputDelay,omitempty"`
	PicDpbOutputDuDelay                    uint32              `json:"PicDpbOutputDuDelay,omitempty"`
	NumDecodingUnitsMinus1                 uint32              `json:"NumDecodingUnitsMinus1,omitempty"`
	DuCommonCpbRemovalDelayFlag            bool                `json:"DuCommonCpbRemovalDelayFlag,omitempty"`
	DuCommonCpbRemovalDelayIncrementMinus1 uint32              `json:"DuCommonCpbRemovalDelayIncrementMinus1,omitempty"`
	NumNalusInDuMinus1                     []uint32            `json:"NumNalusInDuMinus1,omitempty"`
	DuCpbRemovalDelayIncrementMinus1       []uint32            `json:"DuCpbRemovalDelayIncrementMinus1,omitempty"`
	payload                                []byte              `json:"-"`
}

type HEVCPicTimingParams struct {
	FrameFieldInfoPresentFlag              bool
	CpbDpbDelaysPresentFlag                bool
	SubPicHrdParamsPresentFlag             bool
	SubPicCpbParamsInPicTimingSeiFlag      bool
	AuCbpRemovalDelayLengthMinus1          uint8
	DpbOutputDelayLengthMinus1             uint8
	DpbOutputDelayDuLengthMinus1           uint8
	DuCpbRemovalDelayIncrementLengthMinus1 uint8
}

type HEVCFrameFieldInfo struct {
	PicStruct      uint8 // 4 bits
	SourceScanType uint8 // 2 bits
	DuplicateFlag  bool  `json:"DuplicateFlag,omitempty"` // 1bit
}

func DecodePicTimingHevcSEI(sd *SEIData, exPar HEVCPicTimingParams) (SEIMessage, error) {
	buf := bytes.NewBuffer(sd.Payload())
	br := bits.NewEBSPReader(buf)
	pt := PicTimingHevcSEI{
		payload: sd.Payload(),
	}
	if exPar.FrameFieldInfoPresentFlag {
		frameFieldInfo := &HEVCFrameFieldInfo{}
		frameFieldInfo.PicStruct = uint8(br.Read(4))
		frameFieldInfo.SourceScanType = uint8(br.Read(2))
		frameFieldInfo.DuplicateFlag = br.ReadFlag()
		pt.FrameFieldInfo = frameFieldInfo
	}
	if exPar.CpbDpbDelaysPresentFlag {
		pt.AuCpbRemovalDelayMinus1 = uint32(br.Read(int(exPar.AuCbpRemovalDelayLengthMinus1) + 1))
		pt.PicDpbOutputDelay = uint32(br.Read(int(exPar.DpbOutputDelayLengthMinus1) + 1))
		if exPar.SubPicHrdParamsPresentFlag {
			pt.PicDpbOutputDuDelay = uint32(br.Read(int(exPar.DpbOutputDelayDuLengthMinus1) + 1))
			if exPar.SubPicCpbParamsInPicTimingSeiFlag {
				pt.NumDecodingUnitsMinus1 = uint32(br.ReadExpGolomb())
				pt.DuCommonCpbRemovalDelayFlag = br.ReadFlag()
				if pt.DuCommonCpbRemovalDelayFlag {
					pt.DuCommonCpbRemovalDelayIncrementMinus1 = uint32(br.Read(int(exPar.DuCpbRemovalDelayIncrementLengthMinus1) + 1))
				}
				for i := uint32(0); i <= pt.NumDecodingUnitsMinus1; i++ {
					pt.NumNalusInDuMinus1[i] = uint32(br.ReadExpGolomb())
					if !pt.DuCommonCpbRemovalDelayFlag && i < pt.NumDecodingUnitsMinus1 {
						pt.DuCpbRemovalDelayIncrementMinus1[i] = uint32(br.Read(int(exPar.DuCpbRemovalDelayIncrementLengthMinus1) + 1))
					}
				}
			}
		}
	}
	return &pt, br.AccError()
}

// Type returns the SEI payload type.
func (s *PicTimingHevcSEI) Type() uint {
	return SEIPicTimingType
}

// Payload returns the SEI raw rbsp payload.
func (s *PicTimingHevcSEI) Payload() []byte {
	return s.payload
}

// String returns string representation of PicTiming SEI1.
func (s *PicTimingHevcSEI) String() string {
	msgType := SEIType(s.Type())
	msg := fmt.Sprintf("%s: ", msgType)
	if s.FrameFieldInfo != nil {
		msg += fmt.Sprintf("FrameFieldInfo: %+v, ", s.FrameFieldInfo)
	}
	return msg
}

// Size is size in bytes of raw SEI message rbsp payload.
func (s *PicTimingHevcSEI) Size() uint {
	return uint(len(s.payload))
}
