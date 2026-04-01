package sei

import (
	"bytes"
	"encoding/json"
	"fmt"

	"github.com/Eyevinn/mp4ff/bits"
)

// PicTimingAvcSEI carries the data of an SEI 1 PicTiming message for AVC.
// The corresponding SEI 1 for HEVC is very different.
type PicTimingAvcSEI struct {
	// CbpDbpDelay is optional and triggered by VUI HRD data
	CbpDbpDelay *CbpDbpDelay `json:"-"`
	// TimeOffsetLength is 5 bits and comes from SPS HRD if present
	TimeOffsetLength uint8        `json:"-"`
	PictStruct       uint8        `json:"pict_struct"`
	Clocks           []ClockTSAvc `json:"clocks"`
}

// CbpDbpDelay carries the optional data on CpbDpbDelay.
// This being set corresponds CpbDpbDelaysPresentFlag = true,
// which in turn is a calculated value if HRD info is present,
// i.e. NalHrdBpPresentFlag or VclHrdBpPresentFlag is set
type CbpDbpDelay struct {
	CpbRemovalDelay uint
	DpbOutputDelay  uint
	// InitialCpbRemovalDelayLengthMinus1 comes from SPS HRD and is 5 bits
	InitialCpbRemovalDelayLengthMinus1 byte
	// CpbRemovalDelayLengthMinus1 comes from SPS HRD and is 5 bits
	CpbRemovalDelayLengthMinus1 byte
	// DpbOutputDelayLengthMinus1 comes from SPS HRD and is 5 bits
	DpbOutputDelayLengthMinus1 byte
}

// DecodePicTimingAvcSEI decodes SEI message 1 TimeCode without HRD parameters.
func DecodePicTimingAvcSEI(sd *SEIData) (SEIMessage, error) {
	return DecodePicTimingAvcSEIHRD(sd, nil, 0)
}

// DecodePicTimingAvcSEIHRD decodes AVC SEI message 1 PicTiming with HRD parameters.
// cbpDbpDelay length fields must be properly set if cbpDbpDelay is not nil.
// The delay values in cbpDbpDelay will then be set by the decoder by reading the bits.
// It is assumed that pict_struct_present_flag is true, so that a 4-bit pict_struct value is present.
func DecodePicTimingAvcSEIHRD(sd *SEIData, cbpDbpDelay *CbpDbpDelay, timeOffsetLen byte) (SEIMessage, error) {
	buf := bytes.NewBuffer(sd.Payload())
	br := bits.NewReader(buf)
	var outCbDbpDelay CbpDbpDelay
	if cbpDbpDelay != nil {
		outCbDbpDelay = *cbpDbpDelay
		outCbDbpDelay.CpbRemovalDelay = uint(br.Read(int(cbpDbpDelay.CpbRemovalDelayLengthMinus1) + 1))
		outCbDbpDelay.DpbOutputDelay = uint(br.Read(int(cbpDbpDelay.DpbOutputDelayLengthMinus1) + 1))
	}

	pictStruct := uint8(br.Read(4))
	var numClockTS int
	switch {
	case pictStruct <= 2:
		numClockTS = 1
	case pictStruct <= 4:
		numClockTS = 2
	case pictStruct <= 8:
		numClockTS = 3
	default:
		return nil, fmt.Errorf("unknown pict_struct value %d", pictStruct)
	}
	tc := PicTimingAvcSEI{
		PictStruct: pictStruct,
		Clocks:     make([]ClockTSAvc, 0, numClockTS),
	}
	if cbpDbpDelay != nil {
		tc.CbpDbpDelay = &outCbDbpDelay
	}
	for i := 0; i < numClockTS; i++ {
		c := DecodeClockTSAvc(br, timeOffsetLen)
		tc.Clocks = append(tc.Clocks, c)
	}
	tc.TimeOffsetLength = timeOffsetLen
	return &tc, br.AccError()
}

// Type returns the SEI payload type.
func (s *PicTimingAvcSEI) Type() uint {
	return SEIPicTimingType
}

// Payload returns the SEI raw rbsp payload.
func (s *PicTimingAvcSEI) Payload() []byte {
	sw := bits.NewFixedSliceWriter(int(s.Size()))
	if s.CbpDbpDelay != nil {
		sw.WriteBits(uint(s.CbpDbpDelay.CpbRemovalDelay), int(s.CbpDbpDelay.CpbRemovalDelayLengthMinus1)+1)
		sw.WriteBits(uint(s.CbpDbpDelay.DpbOutputDelay), int(s.CbpDbpDelay.DpbOutputDelayLengthMinus1)+1)
	}
	sw.WriteBits(uint(s.PictStruct), 4)
	for _, c := range s.Clocks {
		c.WriteToSliceWriter(sw)
	}
	sw.FlushBits()
	return sw.Bytes()
}

// String returns string representation of PicTiming SEI1.
func (s *PicTimingAvcSEI) String() string {
	msgType := SEIType(s.Type())
	msg := fmt.Sprintf("%s, size=%d, time=%s", msgType, s.Size(), s.Clocks[0].String())
	if len(s.Clocks) > 1 {
		for i := 1; i < len(s.Clocks); i++ {
			msg += fmt.Sprintf(", time=%s", s.Clocks[i].String())
		}
	}
	return msg
}

// Size is size in bytes of raw SEI message rbsp payload.
func (s *PicTimingAvcSEI) Size() uint {
	nrBits := 0
	if s.CbpDbpDelay != nil {
		nrBits += int(s.CbpDbpDelay.CpbRemovalDelayLengthMinus1) + 1
		nrBits += int(s.CbpDbpDelay.DpbOutputDelayLengthMinus1) + 1
	}
	nrBits += 4 // pict_struct
	for _, c := range s.Clocks {
		nrBits += c.NrBits()
	}
	return uint((nrBits + 7) / 8)
}

// ClockTSAvc carries a clock time stamp for SEI type 1.
type ClockTSAvc struct {
	CtType             byte // scan type
	NuitFieldBasedFlag bool // misspelled in AVC spec, changed in HEVC to UnitsFieldBasedFlag
	CountingType       byte
	NFrames            byte
	Hours              byte
	Minutes            byte
	Seconds            byte
	ClockTimeStampFlag bool
	FullTimeStampFlag  bool
	SecondsFlag        bool
	MinutesFlag        bool
	HoursFlag          bool
	DiscontinuityFlag  bool
	CntDroppedFlag     bool
	TimeOffsetLength   byte
	TimeOffsetValue    int
}

// String returns time stamp
func (c ClockTSAvc) String() string {
	return fmt.Sprintf("%02d:%02d:%02d:%02d offset=%d", c.Hours, c.Minutes, c.Seconds, c.NFrames, c.TimeOffsetValue)
}

func (c *ClockTSAvc) MarshalJSON() ([]byte, error) {
	return json.Marshal(struct {
		Time   string `json:"time"`
		Offset int    `json:"offset"`
	}{
		Time:   fmt.Sprintf("%02d:%02d:%02d:%02d", c.Hours, c.Minutes, c.Seconds, c.NFrames),
		Offset: c.TimeOffsetValue,
	})
}

// CreatePTClockTS creates a clock timestamp.
func CreateClockTSAvc(timeOffsetLen byte) ClockTSAvc {
	return ClockTSAvc{
		TimeOffsetLength: timeOffsetLen,
	}
}

func DecodeClockTSAvc(br *bits.Reader, timeOffsetLen byte) ClockTSAvc {
	c := CreateClockTSAvc(timeOffsetLen)
	c.ClockTimeStampFlag = br.ReadFlag()
	if c.ClockTimeStampFlag {
		c.CtType = byte(br.Read(2)) // 0 progressive, 1 interlaced, 2 unknown, 3 reserved
		c.NuitFieldBasedFlag = br.ReadFlag()
		c.CountingType = byte(br.Read(5))
		c.FullTimeStampFlag = br.ReadFlag()
		c.DiscontinuityFlag = br.ReadFlag()
		c.CntDroppedFlag = br.ReadFlag()
		c.NFrames = byte(br.Read(8))
		if c.FullTimeStampFlag {
			c.Seconds = byte(br.Read(6))
			c.Minutes = byte(br.Read(6))
			c.Hours = byte(br.Read(5))
		} else {
			c.SecondsFlag = br.ReadFlag()
			if c.SecondsFlag {
				c.Seconds = byte(br.Read(6))
				c.MinutesFlag = br.ReadFlag()
				if c.MinutesFlag {
					c.Minutes = byte(br.Read(6))
					c.HoursFlag = br.ReadFlag()
					if c.HoursFlag {
						c.Hours = byte(br.Read(5))
					}
				}
			}
		}
		if c.TimeOffsetLength > 0 {
			c.TimeOffsetValue = br.ReadSigned(int(c.TimeOffsetLength))
		}
	}
	return c
}

// NrBits returns size of PTClockTS in bits.
func (c ClockTSAvc) NrBits() int {
	nrBits := 1
	if c.ClockTimeStampFlag {
		nrBits += 2 + 1 + 5 + 1 + 1 + 1 + 8
		if c.FullTimeStampFlag {
			nrBits += 6 + 6 + 5
		} else {
			nrBits++
			if c.SecondsFlag {
				nrBits += 6 + 1
				if c.MinutesFlag {
					nrBits += 6 + 1
					if c.HoursFlag {
						nrBits += 5
					}
				}
			}
		}
		nrBits += int(c.TimeOffsetLength)
	}
	return nrBits
}

// WriteToSliceWriter writes PTClockTS to slice writer.
func (c ClockTSAvc) WriteToSliceWriter(sw bits.SliceWriter) {
	sw.WriteFlag(c.ClockTimeStampFlag)
	if c.ClockTimeStampFlag {
		sw.WriteBits(uint(c.CtType), 2)
		sw.WriteFlag(c.NuitFieldBasedFlag)
		sw.WriteBits(uint(c.CountingType), 5)
		sw.WriteFlag(c.FullTimeStampFlag)
		sw.WriteFlag(c.DiscontinuityFlag)
		sw.WriteFlag(c.CntDroppedFlag)
		sw.WriteBits(uint(c.NFrames), 8)
		if c.FullTimeStampFlag {
			sw.WriteBits(uint(c.Seconds), 6)
			sw.WriteBits(uint(c.Minutes), 6)
			sw.WriteBits(uint(c.Hours), 5)
		} else {
			sw.WriteFlag(c.SecondsFlag)
			if c.SecondsFlag {
				sw.WriteBits(uint(c.Seconds), 6)
				sw.WriteFlag(c.MinutesFlag)
				if c.MinutesFlag {
					sw.WriteBits(uint(c.Minutes), 6)
					sw.WriteFlag(c.HoursFlag)
					if c.HoursFlag {
						sw.WriteBits(uint(c.Hours), 5)
					}
				}
			}
		}
		if c.TimeOffsetLength > 0 {
			sw.WriteBits(uint(c.TimeOffsetValue), int(c.TimeOffsetLength))
		}
	}
}
