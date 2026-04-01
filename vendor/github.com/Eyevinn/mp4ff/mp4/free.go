package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// FreeBox - Free Space Box (free or skip)
type FreeBox struct {
	Name       string
	notDecoded []byte
}

// DecodeFree - box-specific decode
func DecodeFree(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	return &FreeBox{Name: hdr.Name, notDecoded: data}, nil
}

// DecodeFreeSR - box-specific decode
func DecodeFreeSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &FreeBox{Name: hdr.Name, notDecoded: sr.ReadBytes(hdr.payloadLen())}, sr.AccError()
}

// Type - box type
func (b *FreeBox) Type() string {
	return b.Name
}

// Size - calculated size of box
func (b *FreeBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.notDecoded))
}

// Encode - write box to w
func (b *FreeBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *FreeBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteBytes(b.notDecoded)
	return sw.AccError()
}

// Info - write box-specific information
func (b *FreeBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	return bd.err
}
