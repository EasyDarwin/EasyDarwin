package mp4

import (
	"encoding/binary"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// StypBox - Segment Type Box (styp)
type StypBox struct {
	data []byte
}

// MajorBrand - major brand (4 chars)
func (b *StypBox) MajorBrand() string {
	return string(b.data[:4])
}

// MinorVersion - minor version
func (b *StypBox) MinorVersion() uint32 {
	return binary.BigEndian.Uint32(b.data[4:8])
}

// AddCompatibleBrands adds new compatible brands to Styp box.
func (b *StypBox) AddCompatibleBrands(compatibleBrands []string) {
	for _, cb := range compatibleBrands {
		b.data = append(b.data, []byte(cb)...)
	}
}

// CompatibleBrands - slice of compatible brands (4 chars each)
func (b *StypBox) CompatibleBrands() []string {
	nrCompatibleBrands := (len(b.data) - 8) / 4
	if nrCompatibleBrands == 0 {
		return nil
	}
	compatibleBrands := make([]string, nrCompatibleBrands)
	for i := 0; i < nrCompatibleBrands; i++ {
		pos := 8 + 4*i
		compatibleBrands[i] = string(b.data[pos : pos+4])
	}
	return compatibleBrands
}

// CreateStyp - Create an Styp box suitable for DASH/CMAF
func CreateStyp() *StypBox {
	return NewStyp("cmfs", 0, []string{"dash", "msdh"})
}

// NewStyp - new styp box with parameters
func NewStyp(majorBrand string, minorVersion uint32, compatibleBrands []string) *StypBox {
	data := make([]byte, 8+4*len(compatibleBrands))
	copy(data, []byte(majorBrand))
	binary.BigEndian.PutUint32(data[4:8], minorVersion)
	for i, cb := range compatibleBrands {
		pos := 8 + 4*i
		copy(data[pos:pos+4], []byte(cb))
	}
	return &StypBox{data: data}
}

// DecodeStyp - box-specific decode
func DecodeStyp(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	b := StypBox{data: data}
	return &b, nil
}

// DecodeStypSR - box-specific decode
func DecodeStypSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	b := StypBox{data: sr.ReadBytes(int(hdr.Size) - hdr.Hdrlen)}
	return &b, sr.AccError()
}

// Type - return box type
func (b *StypBox) Type() string {
	return "styp"
}

// Size - return calculated size
func (b *StypBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.data))
}

// Encode - write box to w
func (b *StypBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *StypBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteBytes(b.data)
	return sw.AccError()
}

// Info - write specific box info to w
func (b *StypBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - majorBrand: %s", b.MajorBrand())
	bd.write(" - minorVersion: %d", b.MinorVersion())
	for _, cb := range b.CompatibleBrands() {
		bd.write(" - compatibleBrand: %s", cb)
	}
	return bd.err
}
