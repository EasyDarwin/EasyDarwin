package sei

import (
	"bytes"
	"fmt"

	"github.com/Eyevinn/mp4ff/bits"
)

// TimeCodeSEI carries the data of an SEI 136 TimeCode message.
type TimeCodeSEI struct {
	Clocks []ClockTS
}

// ClockTS carries a clock time stamp.
type ClockTS struct {
	TimeOffsetValue     uint32
	NFrames             uint16
	Hours               byte
	Minutes             byte
	Seconds             byte
	ClockTimeStampFlag  bool
	UnitsFieldBasedFlag bool
	FullTimeStampFlag   bool
	SecondsFlag         bool
	MinutesFlag         bool
	HoursFlag           bool
	DiscontinuityFlag   bool
	CntDroppedFlag      bool
	CountingType        byte
	TimeOffsetLength    byte
}

// String returns time stamp
func (c ClockTS) String() string {
	return fmt.Sprintf("%02d:%02d:%02d:%02d offset=%d", c.Hours, c.Minutes, c.Seconds, c.NFrames, c.TimeOffsetValue)
}

// CreateClockTS creates a clock timestamp with time parts set to -1.
func CreateClockTS() ClockTS {
	return ClockTS{}
}

func DecodeClockTS(br *bits.Reader) ClockTS {
	c := CreateClockTS()
	c.ClockTimeStampFlag = br.ReadFlag()
	if c.ClockTimeStampFlag {
		c.UnitsFieldBasedFlag = br.ReadFlag()
		c.CountingType = byte(br.Read(5))
		c.FullTimeStampFlag = br.ReadFlag()
		c.DiscontinuityFlag = br.ReadFlag()
		c.CntDroppedFlag = br.ReadFlag()
		c.NFrames = uint16(br.Read(9))
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
		c.TimeOffsetLength = byte(br.Read(5))
		if c.TimeOffsetLength > 0 {
			c.TimeOffsetValue = uint32(br.Read(int(c.TimeOffsetLength)))
		}
	}
	return c
}

// DecodeTimeCodeSEI decodes SEI message 136 TimeCode.
func DecodeTimeCodeSEI(sd *SEIData) (SEIMessage, error) {
	buf := bytes.NewBuffer(sd.Payload())
	br := bits.NewReader(buf)
	numClockTS := int(br.Read(2))
	tc := TimeCodeSEI{make([]ClockTS, 0, numClockTS)}
	for i := 0; i < numClockTS; i++ {
		c := DecodeClockTS(br)
		tc.Clocks = append(tc.Clocks, c)
	}
	return &tc, br.AccError()
}

// Type returns the SEI payload type.
func (s *TimeCodeSEI) Type() uint {
	return SEITimeCodeType
}

// Payload returns the SEI raw rbsp payload.
func (s *TimeCodeSEI) Payload() []byte {
	sw := bits.NewFixedSliceWriter(int(s.Size()))
	sw.WriteBits(uint(len(s.Clocks)), 2)
	for _, c := range s.Clocks {
		sw.WriteFlag(c.ClockTimeStampFlag)
		if c.ClockTimeStampFlag {
			sw.WriteFlag(c.UnitsFieldBasedFlag)
			sw.WriteBits(uint(c.CountingType), 5)
			sw.WriteFlag(c.FullTimeStampFlag)
			sw.WriteFlag(c.DiscontinuityFlag)
			sw.WriteFlag(c.CntDroppedFlag)
			sw.WriteBits(uint(c.NFrames), 9)
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
			sw.WriteBits(uint(c.TimeOffsetLength), 5)
			if c.TimeOffsetLength > 0 {
				sw.WriteBits(uint(c.TimeOffsetValue), int(c.TimeOffsetLength))
			}
		}
	}
	sw.WriteFlag(true) // Final 1 and then byte align
	sw.FlushBits()
	return sw.Bytes()
}

// String returns string representation of TimeCodeSEI.
func (s *TimeCodeSEI) String() string {
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
func (s *TimeCodeSEI) Size() uint {
	nrBits := 2
	for _, c := range s.Clocks {
		nrBits++
		if c.ClockTimeStampFlag {
			nrBits += 18
			if c.FullTimeStampFlag {
				nrBits += 17
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
			nrBits += 5
			nrBits += int(c.TimeOffsetLength)
		}
	}
	return uint((nrBits + 7) / 8)
}
