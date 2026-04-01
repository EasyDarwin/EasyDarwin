package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// SthdBox - Subtitle Media Header Box (sthd - for subtitle tracks)
type SthdBox struct {
	Version byte
	Flags   uint32
}

// DecodeSthd - box-specific decode
func DecodeSthd(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSthdSR(hdr, startPos, sr)
}

// DecodeSthdSR - box-specific decode
func DecodeSthdSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	sb := &SthdBox{
		Version: byte(versionAndFlags >> 24),
		Flags:   versionAndFlags & flagsMask,
	}
	return sb, sr.AccError()
}

// Type - box-specific type
func (b *SthdBox) Type() string {
	return "sthd"
}

// Size - calculated size of box
func (b *SthdBox) Size() uint64 {
	return boxHeaderSize + 4 // FullBox
}

// Encode - write box to w
func (b *SthdBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SthdBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	return sw.AccError()
}

// Info - write box-specific information
func (b *SthdBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	return bd.err
}
