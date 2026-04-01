package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MfhdBox - Media Fragment Header Box (mfhd)
//
// Contained in : Movie Fragment box (moof))
type MfhdBox struct {
	Version        byte
	Flags          uint32
	SequenceNumber uint32
}

// DecodeMfhd - box-specific decode
func DecodeMfhd(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	s := bits.NewFixedSliceReader(data)
	versionAndFlags := s.ReadUint32()
	version := byte(versionAndFlags >> 24)
	flags := versionAndFlags & flagsMask
	sequenceNumber := s.ReadUint32()
	return &MfhdBox{
		Version:        version,
		Flags:          flags,
		SequenceNumber: sequenceNumber,
	}, nil
}

// DecodeMfhdSR - box-specific decode
func DecodeMfhdSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	flags := versionAndFlags & flagsMask
	sequenceNumber := sr.ReadUint32()
	return &MfhdBox{
		Version:        version,
		Flags:          flags,
		SequenceNumber: sequenceNumber,
	}, sr.AccError()
}

// CreateMfhd - create an MfhdBox
func CreateMfhd(sequenceNumber uint32) *MfhdBox {
	return &MfhdBox{
		Version:        0,
		Flags:          0,
		SequenceNumber: sequenceNumber,
	}
}

// Type - box type
func (m *MfhdBox) Type() string {
	return "mfhd"
}

// Size - calculated size of box
func (m *MfhdBox) Size() uint64 {
	return boxHeaderSize + 8
}

// Encode - write box to w
func (m *MfhdBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(m.Size()))
	err := m.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (m *MfhdBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(m, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(m.Version) << 24) + m.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(m.SequenceNumber)
	return sw.AccError()
}

// Info - write box-specific information
func (m *MfhdBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, m, int(m.Version), m.Flags)
	bd.write(" - sequenceNumber: %d", m.SequenceNumber)
	return bd.err
}
