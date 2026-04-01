package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

const charOffset = 0x60 // According to Section 8.4.2.3 of 14496-12

// MdhdBox - Media Header Box (mdhd - mandatory)
//
// Contained in : Media Box (mdia)
//
// Timescale defines the timescale used for this track.
// Language is a ISO-639-2/T language code stored as 1bit padding + [3]int5
type MdhdBox struct {
	Version          byte // Only version 0
	Flags            uint32
	CreationTime     uint64 // Seconds since 1904-01-01
	ModificationTime uint64 // Seconds since 1904-01-01
	Timescale        uint32 // Media timescale for this track
	Duration         uint64 // Trak duration, 0 for fragmented files
	Language         uint16 // Three-letter ISO-639-2/T language code
}

// DecodeMdhd - Decode box
func DecodeMdhd(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeMdhdSR(hdr, startPos, sr)
}

// DecodeMdhd - Decode box
func DecodeMdhdSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	b := MdhdBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	if version == 1 {
		b.CreationTime = sr.ReadUint64()
		b.ModificationTime = sr.ReadUint64()
		b.Timescale = sr.ReadUint32()
		b.Duration = sr.ReadUint64()
	} else if version == 0 {
		b.CreationTime = uint64(sr.ReadUint32())
		b.ModificationTime = uint64(sr.ReadUint32())
		b.Timescale = sr.ReadUint32()
		b.Duration = uint64(sr.ReadUint32())
	} else {
		return nil, fmt.Errorf("unknown mdhd version %d", version)
	}
	b.Language = sr.ReadUint16()
	sr.SkipBytes(2)
	return &b, sr.AccError()
}

// GetLanguage - Get three-byte language string
func (m *MdhdBox) GetLanguage() string {
	a := (m.Language >> 10) & 0x1f
	b := (m.Language >> 5) & 0x1f
	c := m.Language & 0x1f
	return fmt.Sprintf("%c%c%c", a+charOffset, b+charOffset, c+charOffset)
}

// SetLanguage - Set three-byte language string
func (m *MdhdBox) SetLanguage(lang string) {
	var l uint16 = 0
	for i, c := range lang {
		l += uint16(((c - charOffset) & 0x1f) << (5 * (2 - i)))
	}
	m.Language = l
}

// Type - box type
func (m *MdhdBox) Type() string {
	return "mdhd"
}

// Size - calculated size of box
func (m *MdhdBox) Size() uint64 {
	if m.Version == 1 {
		return 44
	}
	return 32 // m.Version = 0
}

// Encode - write box to w
func (m *MdhdBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(m.Size()))
	err := m.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (m *MdhdBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(m, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(m.Version) << 24) + m.Flags
	sw.WriteUint32(versionAndFlags)
	if m.Version == 1 {
		sw.WriteUint64(m.CreationTime)
		sw.WriteUint64(m.ModificationTime)
		sw.WriteUint32(m.Timescale)
		sw.WriteUint64(m.Duration)
	} else {
		sw.WriteUint32(uint32(m.CreationTime))
		sw.WriteUint32(uint32(m.ModificationTime))
		sw.WriteUint32(m.Timescale)
		sw.WriteUint32(uint32(m.Duration))
	}
	sw.WriteUint16(m.Language)
	sw.WriteUint16(0)
	return sw.AccError()
}

// Info - write box-specific information
func (m *MdhdBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, m, int(m.Version), m.Flags)
	bd.write(" - timeScale: %d", m.Timescale)
	bd.write(" - creation time: %s", timeStr(m.CreationTime))
	bd.write(" - modification time: %s", timeStr(m.ModificationTime))
	bd.write(" - language: %s", m.GetLanguage())
	return bd.err
}

// CreationTimeS returns the creation time in seconds since Jan 1, 1970
func (b *MdhdBox) CreationTimeS() int64 {
	return int64(b.CreationTime) - EpochDiffS
}

// ModificationTimeS returns the modification time in seconds since Jan 1, 1970
func (b *MdhdBox) ModificationTimeS() int64 {
	return int64(b.ModificationTime) - EpochDiffS
}

// SetCreationTimeS sets the creation time from seconds since Jan 1, 1970
func (b *MdhdBox) SetCreationTimeS(unixTimeS int64) {
	b.CreationTime = uint64(unixTimeS + EpochDiffS)
}

// SetModificationTimeS sets the modification time from seconds since Jan 1, 1970
func (b *MdhdBox) SetModificationTimeS(unixTimeS int64) {
	b.ModificationTime = uint64(unixTimeS + EpochDiffS)
}
