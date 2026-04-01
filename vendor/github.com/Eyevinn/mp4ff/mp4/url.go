package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// URLBox - DataEntryUrlBox ('url ')
//
// Contained in : DrefBox (dref
type URLBox struct {
	Version           byte
	Flags             uint32
	Location          string // Zero-terminated string
	NoLocation        bool
	NoZeroTermination bool
}

const dataIsSelfContainedFlag = 0x000001

// DecodeURLBox - box-specific decode
func DecodeURLBox(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeURLBoxSR(hdr, startPos, sr)
}

// DecodeURLBoxSR - box-specific decode
func DecodeURLBoxSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	flags := versionAndFlags & flagsMask
	maxLen := hdr.payloadLen() - 4
	b := URLBox{
		Version:    version,
		Flags:      flags,
		NoLocation: true,
	}
	if maxLen > 0 {
		b.NoLocation = false
		var ok bool
		b.Location, ok = sr.ReadPossiblyZeroTerminatedString(maxLen)
		b.NoZeroTermination = !ok
		if len(b.Location) == int(maxLen) {
			b.NoZeroTermination = true
		}
	}
	return &b, sr.AccError()
}

// CreateURLBox - Create a self-referencing URL box
func CreateURLBox() *URLBox {
	return &URLBox{
		Version:           0,
		Flags:             dataIsSelfContainedFlag,
		Location:          "",
		NoLocation:        true,
		NoZeroTermination: false,
	}
}

// Type - return box type
func (b *URLBox) Type() string {
	return "url "
}

// Size - return calculated size
func (b *URLBox) Size() uint64 {
	size := uint64(boxHeaderSize + 4)
	if !b.NoLocation {
		size += uint64(len(b.Location) + 1)
		if b.NoZeroTermination {
			size--
		}
	}
	return size
}

// Encode - write box to w
func (b *URLBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *URLBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	if !b.NoLocation {
		sw.WriteString(b.Location, !b.NoZeroTermination)
	}
	return sw.AccError()
}

// Info - write specific box information
func (b *URLBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	if b.NoLocation {
		return bd.err
	}
	bd.write(" - location: %q", b.Location)
	if b.NoZeroTermination {
		bd.write(" - Warning: no zero termination")
	}
	return bd.err
}
