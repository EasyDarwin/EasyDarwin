package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// SdtpBox - Sample Dependency Box (sdtp - optional)
//
// ISO/IEC 14496-12 Ed. 6 2020 Section 8.6.4
// Contained in Sample Table Box (stbl)
//
// Table to determine whether a sample depends or is depended on by other samples
type SdtpBox struct {
	Version byte
	Flags   uint32
	Entries []SdtpEntry
}

// SdtpEntry (uint8)
//
// ISO/IEC 14496-12 Ed. 6 2020 Section 8.6.4.2
type SdtpEntry uint8

// NewSdtpEntry - make new SdtpEntry from 2-bit parameters
func NewSdtpEntry(isLeading, sampleDependsOn, sampleDependedOn, hasRedundancy uint8) SdtpEntry {
	return SdtpEntry(isLeading<<6 | sampleDependedOn<<4 | sampleDependedOn<<2 | hasRedundancy)
}

// IsLeading (bits 0-1)
// 0: Leading unknown
// 1: Has dependency before referenced I-picture (not decodable)
// 2: Not a leading sample
// 3: Has no dependency before referenced I-picture (decodable)
func (entry SdtpEntry) IsLeading() uint8 {
	return (uint8(entry) >> 6) & 3
}

// SampleDependsOn (bits 2-3)
// 0: Dependency is unknown
// 1: Depends on others (not an I-picture)
// 2: Does not depend on others (I-picture)
// 3: Reservced
func (entry SdtpEntry) SampleDependsOn() uint8 {
	return (uint8(entry) >> 4) & 3
}

// SampleIsDependedOn (bits 4-5)
// 0: Dependency unknown
// 1: Other samples may depend on this (not disposable)
// 2: No other samples depend on this (disposable)
// 3: Reserved
func (entry SdtpEntry) SampleIsDependedOn() uint8 {
	return (uint8(entry) >> 2) & 3
}

// SampleHasRedundancy (bits 6-7)
// 0: Redundant coding unknown
// 1: Redundant coding in this sample
// 2: No redundant coding in this sample
// 3: Reserved
func (entry SdtpEntry) SampleHasRedundancy() uint8 {
	return uint8(entry) & 3
}

// CreateSdtpBox - create a new SdtpBox
func CreateSdtpBox(entries []SdtpEntry) *SdtpBox {
	return &SdtpBox{
		Entries: entries,
	}
}

// DecodeSdtp - box-specific decode
func DecodeSdtp(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSdtpSR(hdr, startPos, sr)
}

// DecodeSdtpSR - box-specific decode
func DecodeSdtpSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	flags := versionAndFlags & flagsMask

	// Supposed to get count from stsz. Use rest of payload
	entries := make([]SdtpEntry, hdr.payloadLen()-4)
	for i := range entries {
		entries[i] = SdtpEntry(sr.ReadUint8())
	}

	return &SdtpBox{
		Version: version,
		Flags:   flags,
		Entries: entries,
	}, sr.AccError()
}

// Type - return box type
func (b *SdtpBox) Type() string {
	return "sdtp"
}

// Size - return calculated size
func (b *SdtpBox) Size() uint64 {
	return uint64(boxHeaderSize + 4 + len(b.Entries))
}

// Encode - write box to w
func (b *SdtpBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SdtpBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)

	for _, entry := range b.Entries {
		sw.WriteUint8(uint8(entry))
	}

	return sw.AccError()
}

// Info - write box-specific information
func (b *SdtpBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	level := getInfoLevel(b, specificBoxLevels)
	if level >= 1 {
		for i, entry := range b.Entries {
			bd.write(" - entry[%d]: isLeading=%d dependsOn=%d isDependedOn=%d hasRedundancy=%d",
				i+1, entry.IsLeading(), entry.SampleDependsOn(), entry.SampleIsDependedOn(), entry.SampleHasRedundancy())
		}
	}
	return bd.err
}
