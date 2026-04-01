package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MimeBox - MIME Box as defined in ISO/IEC 14496-12 2020 Section 12.3.3.2
type MimeBox struct {
	Version              byte
	Flags                uint32
	ContentType          string
	LacksZeroTermination bool // Handle non-compliant case as well
}

// DecodeMime - box-specific decode
func DecodeMime(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeMimeSR(hdr, startPos, sr)
}

// DecodeMimeSR - box-specific decode
func DecodeMimeSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	b := MimeBox{
		Version: byte(versionAndFlags >> 24),
		Flags:   versionAndFlags & flagsMask,
	}
	rest := sr.ReadBytes(hdr.payloadLen() - 4)
	if rest[len(rest)-1] == 0 { // zero-termination
		b.ContentType = string(rest[:len(rest)-1])
	} else {
		b.ContentType = string(rest)
		b.LacksZeroTermination = true
	}
	return &b, sr.AccError()
}

// Type - box type
func (b *MimeBox) Type() string {
	return "mime"
}

// Size - calculated size of box
func (b *MimeBox) Size() uint64 {
	size := uint64(boxHeaderSize + 4 + len(b.ContentType) + 1)
	if b.LacksZeroTermination {
		size--
	}
	return size
}

// Encode - write box to w
func (b *MimeBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *MimeBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteString(b.ContentType, !b.LacksZeroTermination)
	return sw.AccError()
}

// Info - write specific box information
func (b *MimeBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - contentType: %s", b.ContentType)
	return bd.err
}
