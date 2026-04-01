package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// ElngBox - Extended Language Box
// Defined in ISO/IEC 14496-12 Section 8.4.6
// It should be a full box, but was erronously implemented
// as a normal box. For backwards compatibility, the
// erronous box without full header can still be decoded.
// The method MissingFullBoxBytes() returns true if that is the case.
type ElngBox struct {
	missingFullBox bool
	Version        byte
	Flags          uint32
	Language       string
}

// MissingFullBoxBytes indicates that the box is errornously not including the 4 full box header bytes
func (b *ElngBox) MissingFullBoxBytes() bool {
	return b.missingFullBox
}

// CreateElng - Create an Extended Language Box
func CreateElng(language string) *ElngBox {
	return &ElngBox{
		Version:  0,
		Flags:    0,
		Language: language}
}

// DecodeElng - box-specific decode
func DecodeElng(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeElngSR(hdr, startPos, sr)
}

// DecodeElngSR - box-specific decode
func DecodeElngSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	b := ElngBox{}
	plLen := hdr.payloadLen()
	if plLen < 7 { // Less than 4 byte flag and version + 2 letters + 0 termination
		b.missingFullBox = true
		b.Language = string(sr.ReadZeroTerminatedString(plLen))
		return &b, nil
	}
	versionAndFlags := sr.ReadUint32()
	if versionAndFlags != 0 {
		return nil, fmt.Errorf("version and flags are not zero")
	}
	b.Version = byte(versionAndFlags >> 24)
	b.Flags = versionAndFlags & 0xffffff
	b.Language = string(sr.ReadZeroTerminatedString(plLen - 4))
	return &b, sr.AccError()
}

// Type - box type
func (b *ElngBox) Type() string {
	return "elng"
}

// Size - calculated size of box
func (b *ElngBox) Size() uint64 {
	size := uint64(boxHeaderSize + 4 + len(b.Language) + 1)
	if b.missingFullBox {
		size -= 4
	}
	return size
}

// FixMissingFullBoxBytes adds missing bytes version and flags bytes.
func (b *ElngBox) FixMissingFullBoxBytes() {
	b.missingFullBox = false
}

// Encode - write box to w
func (b *ElngBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *ElngBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	if !b.missingFullBox {
		versionAndFlags := uint32(b.Version)<<24 | b.Flags
		sw.WriteUint32(versionAndFlags)
	}
	sw.WriteString(b.Language, true)
	return sw.AccError()
}

// Info - write box-specific information
func (b *ElngBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - language: %s", b.Language)
	return bd.err
}
