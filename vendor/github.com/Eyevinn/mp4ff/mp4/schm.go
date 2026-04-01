package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// SchmBox - Scheme Type Box
type SchmBox struct {
	Version       byte
	Flags         uint32
	SchemeType    string // 4CC represented as uint32
	SchemeVersion uint32
	SchemeURI     string // Absolute null-terminated URL
}

// DecodeSchm - box-specific decode
func DecodeSchm(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSchmSR(hdr, startPos, sr)
}

// DecodeSchmSR - box-specific decode
func DecodeSchmSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)

	b := SchmBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	b.SchemeType = sr.ReadFixedLengthString(4)
	b.SchemeVersion = sr.ReadUint32()
	if b.Flags&0x01 != 0 {
		b.SchemeURI = sr.ReadZeroTerminatedString(hdr.payloadLen())
	}
	return &b, sr.AccError()
}

// Type - return box type
func (b *SchmBox) Type() string {
	return "schm"
}

// Size - return calculated size
func (b *SchmBox) Size() uint64 {
	size := uint64(20)
	if b.Flags&0x01 != 0 {
		size += uint64(len(b.SchemeURI) + 1)
	}
	return size
}

// Encode - write box to w
func (b *SchmBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SchmBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteString(b.SchemeType, false)
	sw.WriteUint32(b.SchemeVersion)
	if b.Flags&0x01 != 0 {
		sw.WriteString(b.SchemeURI, true)
	}
	return sw.AccError()
}

// Info - write box info to w
func (b *SchmBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) (err error) {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - schemeType: %s", b.SchemeType)
	bd.write(" - schemeVersion: %d  (%d.%d)", b.SchemeVersion, b.SchemeVersion>>16, b.SchemeVersion&0xffff)
	if b.Flags&0x01 != 0 {
		bd.write(" - schemeURI: %q", b.SchemeURI)
	}
	return bd.err
}
