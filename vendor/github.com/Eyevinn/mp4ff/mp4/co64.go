package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// Co64Box - Chunk Large Offset Box
//
// Contained in : Sample Table box (stbl)
//
// 64-bit version of StcoBox
type Co64Box struct {
	Version     byte
	Flags       uint32
	ChunkOffset []uint64
}

// DecodeCo64 - box-specific decode
func DecodeCo64(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeCo64SR(hdr, startPos, sr)
}

// DecodeCo64 - box-specific decode
func DecodeCo64SR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	nrEntries := sr.ReadUint32()
	b := &Co64Box{
		Version:     byte(versionAndFlags >> 24),
		Flags:       versionAndFlags & flagsMask,
		ChunkOffset: make([]uint64, nrEntries),
	}

	for i := uint32(0); i < nrEntries; i++ {
		b.ChunkOffset[i] = sr.ReadUint64()
	}
	return b, sr.AccError()
}

// Type - box-specific type
func (b *Co64Box) Type() string {
	return "co64"
}

// Size - box-specific size
func (b *Co64Box) Size() uint64 {
	return uint64(boxHeaderSize + 8 + len(b.ChunkOffset)*8)
}

// Encode - write box to w
func (b *Co64Box) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *Co64Box) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(uint32(len(b.ChunkOffset)))
	for i := range b.ChunkOffset {
		sw.WriteUint64(b.ChunkOffset[i])
	}
	return sw.AccError()
}

// Info - write box-specific information
func (b *Co64Box) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	if len(b.ChunkOffset) > 0 {
		bd.write(" - entryCount: %d", len(b.ChunkOffset))
	}
	level := getInfoLevel(b, specificBoxLevels)
	if level >= 1 {
		for i := range b.ChunkOffset {
			bd.write(" - entry[%d]: chunkOffset=%d", i+1, b.ChunkOffset[i])
		}
	}
	return bd.err
}

// GetOffset - get offset for 1-based chunkNr.
func (b *Co64Box) GetOffset(chunkNr int) (uint64, error) {
	if chunkNr <= 0 || chunkNr > len(b.ChunkOffset) {
		return 0, fmt.Errorf("non-valid chunkNr: %d", chunkNr)
	}
	return b.ChunkOffset[chunkNr-1], nil
}
