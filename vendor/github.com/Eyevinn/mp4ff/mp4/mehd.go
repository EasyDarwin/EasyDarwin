package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MehdBox - Movie Extends Header Box
// Optional, provides overall duration of a fragmented movie
type MehdBox struct {
	Version          byte
	Flags            uint32
	FragmentDuration int64
}

// DecodeMehd - box-specific decode
func DecodeMehd(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeMehdSR(hdr, startPos, sr)
}

// DecodeMehdSR - box-specific decode
func DecodeMehdSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	b := &MehdBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	if version == 0 {
		b.FragmentDuration = int64(sr.ReadInt32())
	} else {
		b.FragmentDuration = sr.ReadInt64()
	}
	return b, sr.AccError()
}

// Type - return box type
func (b *MehdBox) Type() string {
	return "mehd"
}

// Size - return calculated size
func (b *MehdBox) Size() uint64 {
	size := uint64(boxHeaderSize) + 4
	if b.Version == 0 {
		size += 4
	} else {
		size += 8
	}
	return size
}

// Encode - write box to w
func (b *MehdBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *MehdBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	if b.Version == 0 {
		sw.WriteUint32(uint32(b.FragmentDuration))

	} else {
		sw.WriteUint64(uint64(b.FragmentDuration))
	}
	return sw.AccError()
}

// Info - write box-specific information
func (b *MehdBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) (err error) {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - fragmentDuration: %d", b.FragmentDuration)
	return bd.err
}
