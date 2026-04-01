package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// CttsBox - Composition Time to Sample Box (ctts - optional)
//
// Contained in: Sample Table Box (stbl)
type CttsBox struct {
	Version byte
	Flags   uint32
	// EndSampleNr - number (1-based) of last sample in chunk. Starts with 0 for index 0
	EndSampleNr []uint32
	// SampleOffeset - offset of first sample in chunk.
	SampleOffset []int32 // int32 to handle version 1
}

// DecodeCtts - box-specific decode
func DecodeCtts(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeCttsSR(hdr, startPos, sr)
}

// DecodeCttsSR - box-specific decode
func DecodeCttsSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	entryCount := sr.ReadUint32()

	b := &CttsBox{
		Version:      byte(versionAndFlags >> 24),
		Flags:        versionAndFlags & flagsMask,
		EndSampleNr:  make([]uint32, entryCount+1),
		SampleOffset: make([]int32, entryCount),
	}

	var endSampleNr uint32 = 0
	b.EndSampleNr[0] = endSampleNr
	for i := 0; i < int(entryCount); i++ {
		endSampleNr += sr.ReadUint32() // Adding sampleCount
		b.EndSampleNr[i+1] = endSampleNr
		b.SampleOffset[i] = sr.ReadInt32()
	}
	return b, sr.AccError()
}

// Type - box type
func (b *CttsBox) Type() string {
	return "ctts"
}

// Size - calculated size of box
func (b *CttsBox) Size() uint64 {
	return uint64(boxHeaderSize + 8 + len(b.SampleOffset)*8)
}

// Encode - write box to w
func (b *CttsBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *CttsBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(uint32(len(b.SampleOffset)))
	for i := 0; i < b.NrSampleCount(); i++ {
		sampleCount := b.EndSampleNr[i+1] - b.EndSampleNr[i]
		sw.WriteUint32(sampleCount)
		sw.WriteInt32(b.SampleOffset[i])
	}
	return sw.AccError()
}

// NrSampleCount - the number of SampleCount entries in box
func (b *CttsBox) NrSampleCount() int {
	return len(b.SampleOffset)
}

// SampleCount - return sample count i (zero-based)
func (b *CttsBox) SampleCount(i int) uint32 {
	return b.EndSampleNr[i+1] - b.EndSampleNr[i]

}

// AddSampleCountsAndOffsets - populate this box with data. Need the same number of entries in both
func (b *CttsBox) AddSampleCountsAndOffset(counts []uint32, offsets []int32) error {
	if len(counts) != len(offsets) {
		return fmt.Errorf("not same number of sampleCounts %d and sampleOffsets %d", len(counts), len(offsets))
	}
	b.SampleOffset = append(b.SampleOffset, offsets...)
	if len(b.EndSampleNr) == 0 {
		b.EndSampleNr = append(b.EndSampleNr, 0)
	}
	endSampleNr := b.EndSampleNr[len(b.EndSampleNr)-1]
	for i := 0; i < len(counts); i++ {
		endSampleNr += counts[i]
		b.EndSampleNr = append(b.EndSampleNr, endSampleNr)
	}
	return nil
}

// GetCompositionTimeOffset - composition time offset for (one-based) sampleNr in track timescale
func (b *CttsBox) GetCompositionTimeOffset(sampleNr uint32) int32 {
	if sampleNr == 0 {
		// This is bad index input. Should never happen
		panic("CttsBox.GetCompositionTimeOffset called with sampleNr == 0, although one-based")
	}
	// The following is essentially the sort.Search() code specialized to this case
	i, j := 0, len(b.EndSampleNr)
	for i < j {
		h := int(uint(i+j) >> 1) // avoid overflow when computing h
		// i ≤ h < j
		if b.EndSampleNr[h] < sampleNr {
			i = h + 1
		} else {
			j = h
		}
	}
	return b.SampleOffset[i-1]
}

// Info - get all info with specificBoxLevels ctts:1 or higher
func (b *CttsBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - sampleCount: %d", b.NrSampleCount())
	if getInfoLevel(b, specificBoxLevels) > 0 {
		for i := 0; i < b.NrSampleCount(); i++ {
			bd.write(" - entry[%d]: sampleCount=%d sampleOffset=%d", i+1, b.SampleCount(i), b.SampleOffset[i])
		}
	}
	return bd.err
}
