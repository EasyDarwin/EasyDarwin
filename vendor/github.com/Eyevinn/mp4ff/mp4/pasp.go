package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// PaspBox - Pixel Aspect Ratio Box, ISO/IEC 14496-12 2020 Sec. 12.1.4
type PaspBox struct {
	HSpacing uint32
	VSpacing uint32
}

// DecodePasp - box-specific decode
func DecodePasp(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodePaspSR(hdr, startPos, sr)
}

// DecodePaspSR - box-specific decode
func DecodePaspSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	pasp := &PaspBox{}
	pasp.HSpacing = sr.ReadUint32()
	pasp.VSpacing = sr.ReadUint32()
	return pasp, sr.AccError()
}

// Type - box type
func (b *PaspBox) Type() string {
	return "pasp"
}

// Size - calculated size of box
func (b *PaspBox) Size() uint64 {
	return uint64(boxHeaderSize + 8)
}

// Encode - write box to w
func (b *PaspBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *PaspBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteUint32(b.HSpacing)
	sw.WriteUint32(b.VSpacing)
	return sw.AccError()
}

// Info - write box-specific information
func (b *PaspBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - hSpacing:vSpacing: %d:%d", b.HSpacing, b.VSpacing)
	return bd.err
}
