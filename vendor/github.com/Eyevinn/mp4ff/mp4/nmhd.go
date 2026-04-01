package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// NmhdBox - Null Media Header Box (nmhd - often used instead of sthd for subtitle tracks)
type NmhdBox struct {
	Version byte
	Flags   uint32
}

// DecodeNmhd - box-specific decode
func DecodeNmhd(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeNmhdSR(hdr, startPos, sr)
}

// DecodeNmhdSR - box-specific decode
func DecodeNmhdSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {

	versionAndFlags := sr.ReadUint32()
	sb := &NmhdBox{
		Version: byte(versionAndFlags >> 24),
		Flags:   versionAndFlags & flagsMask,
	}
	return sb, sr.AccError()
}

// Type - box-specific type
func (b *NmhdBox) Type() string {
	return "nmhd"
}

// Size - calculated size of box
func (b *NmhdBox) Size() uint64 {
	return boxHeaderSize + 4 // FullBox
}

// Encode - write box to w
func (b *NmhdBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *NmhdBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	return sw.AccError()
}

// Info - write box-specific information
func (b *NmhdBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	return bd.err
}
