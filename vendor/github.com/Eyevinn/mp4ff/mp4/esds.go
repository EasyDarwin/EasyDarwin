package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// EsdsBox as used for MPEG-audio, see ISO 14496-1 Section 7.2.6.6  for DecoderConfigDescriptor
type EsdsBox struct {
	Version byte
	Flags   uint32
	ESDescriptor
}

// CreateEsdsBox - Create an EsdsBox geiven decConfig
func CreateEsdsBox(decConfig []byte) *EsdsBox {
	e := &EsdsBox{
		ESDescriptor: CreateESDescriptor(decConfig),
	}
	return e
}

// DecodeEsds - box-specific decode
func DecodeEsds(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}

	sr := bits.NewFixedSliceReader(data)
	return DecodeEsdsSR(hdr, startPos, sr)
}

// DecodeEsdsSR - box-specific decode
func DecodeEsdsSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)

	e := &EsdsBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	if hdr.Size < 12+minimalEsDescrSize {
		return nil, fmt.Errorf("too few bytes in esds box")
	}
	descSize := uint32(hdr.Size - 12)
	var err error
	e.ESDescriptor, err = DecodeESDescriptor(sr, descSize)
	if err != nil {
		return nil, fmt.Errorf("DecodeESDecriptor: %w", err)
	}
	return e, sr.AccError()
}

// Type - box type
func (e *EsdsBox) Type() string {
	return "esds"
}

// Size - calculated size of box
func (e *EsdsBox) Size() uint64 {
	return uint64(8 + 4 + e.ESDescriptor.SizeSize())
}

// Encode - write box to w
func (e *EsdsBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(e.Size()))
	err := e.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (e *EsdsBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(e, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(e.Version) << 24) + e.Flags
	sw.WriteUint32(versionAndFlags)
	err = e.ESDescriptor.EncodeSW(sw)
	if err != nil {
		return err
	}
	return sw.AccError()
}

// Info - write box-specific information
func (e *EsdsBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, e, int(e.Version), e.Flags)
	err := e.ESDescriptor.Info(bd.w, specificBoxLevels, indent+indentStep, indentStep)
	if err != nil {
		return err
	}
	return bd.err
}
