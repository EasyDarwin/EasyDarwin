package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// StszBox - Sample Size Box (stsz - mandatory)
//
// Contained in : Sample Table box (stbl)
//
// For each track, either stsz of the more compact stz2 must be present. stz2 variant is not supported.
//
// This table lists the size of each sample. If all samples have the same size, it can be defined in the
// SampleUniformSize attribute.
type StszBox struct {
	Version           byte
	Flags             uint32
	SampleUniformSize uint32
	SampleNumber      uint32
	SampleSize        []uint32
}

// DecodeStsz - box-specific decode
func DecodeStsz(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeStszSR(hdr, startPos, sr)
}

// DecodeStszSR - box-specific decode
func DecodeStszSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()

	b := StszBox{
		Version:           byte(versionAndFlags >> 24),
		Flags:             versionAndFlags & flagsMask,
		SampleUniformSize: sr.ReadUint32(),
		SampleNumber:      sr.ReadUint32(),
	}
	if b.SampleUniformSize == 0 {
		b.SampleSize = make([]uint32, b.SampleNumber)
		for i := 0; i < int(b.SampleNumber); i++ {
			b.SampleSize[i] = sr.ReadUint32()
		}
	}
	return &b, sr.AccError()
}

// Type - box-specific type
func (b *StszBox) Type() string {
	return "stsz"
}

// Size - box-specific size
func (b *StszBox) Size() uint64 {
	if b.SampleUniformSize > 0 {
		return uint64(boxHeaderSize + 12)
	}
	return uint64(boxHeaderSize + 12 + b.SampleNumber*4)
}

// Encode - write box to w
func (b *StszBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *StszBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(b.SampleUniformSize)
	if len(b.SampleSize) == 0 {
		sw.WriteUint32(b.SampleNumber)
	} else {
		sw.WriteUint32(uint32(len(b.SampleSize)))
		for i := range b.SampleSize {
			sw.WriteUint32(b.SampleSize[i])
		}
	}
	return sw.AccError()
}

// Info - write box-specific information
func (b *StszBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	if b.SampleNumber == 0 { // No samples
		return bd.err
	}
	if len(b.SampleSize) == 0 {
		bd.write(" - sampleSize: %d", b.SampleUniformSize)
		bd.write(" - sampleCount: %d", b.SampleNumber)
	} else {
		bd.write(" - sampleCount: %d", b.SampleNumber)
	}
	level := getInfoLevel(b, specificBoxLevels)
	if level >= 1 {
		for i := range b.SampleSize {
			bd.write(" - sample[%d] size=%d", i+1, b.SampleSize[i])
		}
	}
	return bd.err
}

// GetNrSamples - get number of sampples
func (b *StszBox) GetNrSamples() uint32 {
	if len(b.SampleSize) == 0 {
		return b.SampleNumber
	}
	return uint32(len(b.SampleSize))
}

// GetSampleSize returns the size (in bytes) of a sample
func (b *StszBox) GetSampleSize(i int) uint32 {
	if i > len(b.SampleSize) { // One-based
		return b.SampleUniformSize
	}
	return b.SampleSize[i-1]
}

// GetTotalSampleSize - get total size of a range [startNr, endNr] of samples
func (b *StszBox) GetTotalSampleSize(startNr, endNr uint32) (uint64, error) {
	if startNr <= 0 || endNr > b.SampleNumber {
		return 0, fmt.Errorf("startNr or calculated endNr outside range 1-%d", b.SampleNumber)
	}
	if endNr < startNr {
		return 0, nil
	}
	if b.SampleUniformSize != 0 {
		nrSamples := uint64(endNr - startNr + 1)
		return nrSamples * uint64(b.SampleUniformSize), nil
	}
	size := uint64(0)
	for nr := startNr; nr <= endNr; nr++ {
		size += uint64(b.SampleSize[nr-1]) // 1-based numbers
	}
	return size, nil
}
