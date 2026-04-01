package mp4

import (
	"encoding/hex"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// UnknownBox - box that we don't know how to parse
type UnknownBox struct {
	name       string
	size       uint64
	notDecoded []byte
}

// DecodeUnknown - decode an unknown box
func DecodeUnknown(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeUnknownSR(hdr, startPos, sr)
}

// DecodeUnknown - decode an unknown box
func DecodeUnknownSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &UnknownBox{hdr.Name, hdr.Size, sr.ReadBytes(hdr.payloadLen())}, sr.AccError()
}

// Type - return box type
func (b *UnknownBox) Type() string {
	return b.name
}

// Size - return calculated size
func (b *UnknownBox) Size() uint64 {
	return b.size
}

// Encode - write box to w
func (b *UnknownBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *UnknownBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteBytes(b.notDecoded)
	return sw.AccError()
}

// Info - write box-specific information
func (b *UnknownBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - not implemented or unknown box")
	level := getInfoLevel(b, specificBoxLevels)
	if level > 0 {
		bd.write(" - %s", hex.EncodeToString(b.notDecoded))
	}

	return bd.err
}
