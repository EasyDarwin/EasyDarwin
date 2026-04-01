package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// StcoBox - Chunk Offset Box (stco - mandatory)
//
// Contained in : Sample Table box (stbl)
//
// The table contains the offsets (starting at the beginning of the file) for each chunk of data for the current track.
// A chunk contains samples, the table defining the allocation of samples to each chunk is stsc.
type StcoBox struct {
	Version     byte
	Flags       uint32
	ChunkOffset []uint32
}

// DecodeStco - box-specific decode
func DecodeStco(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeStcoSR(hdr, startPos, sr)
}

// DecodeStcoSR - box-specific decode
func DecodeStcoSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	entryCount := sr.ReadUint32()
	b := &StcoBox{
		Version:     byte(versionAndFlags >> 24),
		Flags:       versionAndFlags & flagsMask,
		ChunkOffset: make([]uint32, entryCount),
	}

	for i := 0; i < int(entryCount); i++ {
		b.ChunkOffset[i] = sr.ReadUint32()
	}
	return b, sr.AccError()
}

// Type - box-specific type
func (b *StcoBox) Type() string {
	return "stco"
}

// Size - box-specific size
func (b *StcoBox) Size() uint64 {
	return uint64(boxHeaderSize + 8 + len(b.ChunkOffset)*4)
}

// Encode - write box to w
func (b *StcoBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *StcoBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(uint32(len(b.ChunkOffset)))
	for i := range b.ChunkOffset {
		sw.WriteUint32(b.ChunkOffset[i])
	}
	return sw.AccError()
}

// Info - write box-specific information
func (b *StcoBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
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
func (b *StcoBox) GetOffset(chunkNr int) (uint64, error) {
	if chunkNr <= 0 || chunkNr > len(b.ChunkOffset) {
		return 0, fmt.Errorf("non-valid chunkNr: %d", chunkNr)
	}
	return uint64(b.ChunkOffset[chunkNr-1]), nil
}
