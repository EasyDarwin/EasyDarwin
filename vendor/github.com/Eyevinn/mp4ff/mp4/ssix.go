package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

/*
Definition according to ISO/IEC 14496-12 Section 8.16.4.2
aligned(8) class SubsegmentIndexBox extends FullBox('ssix', 0, 0) {
  unsigned int(32) subsegment_count;
  for(i=1; i <= subsegment_count; i++){
    unsigned int(32) range_count;
    for (j=1; j <= range_count; j++) {
	  unsigned int(8) level;
	  unsigned int(24) range_size;
    }
  }
}
*/

// SsixBox - Subsegment Index Box according to ISO/IEC 14496-12 Section 8.16.4.2
type SsixBox struct {
	Version     byte
	Flags       uint32
	SubSegments []SubSegment
}

// SubSegment - subsegment data for SsixBox
type SubSegment struct {
	Ranges []SubSegmentRange
}

// SubSegmentRange - range data for SubSegment
type SubSegmentRange uint32

// Level - return level
func (s SubSegmentRange) Level() uint8 {
	return uint8(s >> 24)
}

// RangeSize - return range size
func (s SubSegmentRange) RangeSize() uint32 {
	return uint32(s & 0x00ffffff)
}

// NewSubSegmentRange - create new SubSegmentRange
func NewSubSegmentRange(level uint8, rangeSize uint32) SubSegmentRange {
	return SubSegmentRange(uint32(level)<<24 | rangeSize)
}

// DecodeSsix - box-specific decode
func DecodeSsix(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSsixSR(hdr, startPos, sr)
}

// DecodeSsixSR - box-specific decode
func DecodeSsixSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)

	b := &SsixBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	subSegmentCount := sr.ReadUint32()
	sizeLeft := hdr.Size - 16
	if subSegmentCount > uint32(sizeLeft/8) {
		return nil, fmt.Errorf("too many subsegments: %d", subSegmentCount)
	}
	b.SubSegments = make([]SubSegment, subSegmentCount)
	for i := 0; i < int(subSegmentCount); i++ {
		rangeCount := sr.ReadUint32()
		sizeLeft -= 4
		if rangeCount > uint32(sizeLeft/4) {
			return nil, fmt.Errorf("too many ranges: %d", rangeCount)
		}
		subSeg := SubSegment{
			Ranges: make([]SubSegmentRange, rangeCount),
		}
		for j := 0; j < int(rangeCount); j++ {
			subSeg.Ranges[j] = SubSegmentRange(sr.ReadUint32())
		}
		b.SubSegments[i] = subSeg
	}
	return b, sr.AccError()
}

// Type - return box type
func (b *SsixBox) Type() string {
	return "ssix"
}

// Size - return calculated size
func (b *SsixBox) Size() uint64 {
	// Add up all fields depending on version
	size := uint64(boxHeaderSize + 4 + 4)
	for _, ss := range b.SubSegments {
		size += 4 + uint64(len(ss.Ranges))*4
	}
	return size
}

// Encode - write box to w
func (b *SsixBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SsixBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(uint32(len(b.SubSegments)))
	for _, ss := range b.SubSegments {
		sw.WriteUint32(uint32(len(ss.Ranges)))
		for _, sr := range ss.Ranges {
			sw.WriteUint32(uint32(sr))
		}
	}
	return sw.AccError()
}

// Info - more info for level 1
func (b *SsixBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - subSegmentCount: %d", len(b.SubSegments))
	level := getInfoLevel(b, specificBoxLevels)
	if level >= 1 {
		for i, ss := range b.SubSegments {
			for j, rng := range ss.Ranges {
				bd.write(" - subSegment[%d] range[%d]: level=%d rangeSize=%d", i+1, j+1, rng.Level(), rng.RangeSize())
			}
		}
	}
	return bd.err
}
