package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// SmhdBox - Sound Media Header Box (smhd - mandatory for sound tracks)
//
// Contained in : Media Information Box (minf)
type SmhdBox struct {
	Version byte
	Flags   uint32
	Balance uint16 // should be int16
}

// CreateSmhd - Create Sound Media Header Box (all is zero)
func CreateSmhd() *SmhdBox {
	return &SmhdBox{}
}

// DecodeSmhd - box-specific decode
func DecodeSmhd(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSmhdSR(hdr, startPos, sr)
}

// DecodeSmhdSR - box-specific decode
func DecodeSmhdSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	b := SmhdBox{
		Version: byte(versionAndFlags >> 24),
		Flags:   versionAndFlags & flagsMask,
		Balance: sr.ReadUint16(),
	}
	sr.SkipBytes(2) // Reserved
	return &b, sr.AccError()
}

// Type - box type
func (b *SmhdBox) Type() string {
	return "smhd"
}

// Size - calculated size of box
func (b *SmhdBox) Size() uint64 {
	return boxHeaderSize + 8
}

// Encode - write box to w
func (b *SmhdBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SmhdBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint16(b.Balance)
	sw.WriteUint16(0) // Reserved
	return sw.AccError()
}

// Info - write box-specific information
func (b *SmhdBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	return bd.err
}
