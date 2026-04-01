package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

const (
	sbgpInsideOffset = 65536
)

// SbgpBox - Sample To Group Box, ISO/IEC 14496-12 6'th edition 2020 Section 8.9.2
type SbgpBox struct {
	Version                 byte
	Flags                   uint32
	GroupingType            string // uint32, but takes values such as seig
	GroupingTypeParameter   uint32
	SampleCounts            []uint32
	GroupDescriptionIndices []uint32 // Starts at 65537 inside fragment, see Section 8.9.4
}

// DecodeSbgp - box-specific decode
func DecodeSbgp(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSbgpSR(hdr, startPos, sr)
}

// DecodeSbgpSR - box-specific decode
func DecodeSbgpSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)

	b := SbgpBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	b.GroupingType = sr.ReadFixedLengthString(4)
	if b.Version == 1 {
		b.GroupingTypeParameter = sr.ReadUint32()
	}
	entryCount := int(sr.ReadUint32())
	for i := 0; i < entryCount; i++ {
		b.SampleCounts = append(b.SampleCounts, sr.ReadUint32())
		b.GroupDescriptionIndices = append(b.GroupDescriptionIndices, sr.ReadUint32())
	}
	return &b, sr.AccError()
}

// Type - return box type
func (b *SbgpBox) Type() string {
	return "sbgp"
}

// Size - return calculated size
func (b *SbgpBox) Size() uint64 {
	// Version + Flags:4
	// GroupingType: 4
	// (v1) GroupingTypeParameter: 4
	// EntryCount: 4
	// SampleCount + GroupDescriptionIndex : 8
	return uint64(boxHeaderSize + 12 + 4*int(b.Version) + 8*len(b.GroupDescriptionIndices))
}

// Encode - write box to w
func (b *SbgpBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SbgpBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteString(b.GroupingType, false)
	if b.Version == 1 {
		sw.WriteUint32(b.GroupingTypeParameter)
	}
	entryCount := len(b.SampleCounts)
	sw.WriteUint32(uint32(entryCount))
	for i := 0; i < entryCount; i++ {
		sw.WriteUint32(b.SampleCounts[i])
		sw.WriteUint32(b.GroupDescriptionIndices[i])
	}
	return sw.AccError()
}

// Info - write box info to w
func (b *SbgpBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) (err error) {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - groupingType: %s", b.GroupingType)
	if b.Version == 1 {
		bd.write(" - groupingTypeParameter: %d", b.GroupingTypeParameter)
	}
	bd.write(" - entryCount: %d", len(b.SampleCounts))
	level := getInfoLevel(b, specificBoxLevels)
	if level > 0 {
		for i := range b.SampleCounts {
			gdi := fmt.Sprintf("%d", b.GroupDescriptionIndices[i])
			if b.GroupDescriptionIndices[i] > sbgpInsideOffset {
				gdi = fmt.Sprintf("%d (index %d inside fragment)",
					b.GroupDescriptionIndices[i], b.GroupDescriptionIndices[i]-sbgpInsideOffset)
			}
			bd.write(" - entry[%d] sampleCount=%d groupDescriptionIndex=%s",
				i+1, b.SampleCounts[i], gdi)
		}
	}
	return bd.err
}
