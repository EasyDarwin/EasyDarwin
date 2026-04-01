package mp4

import (
	"encoding/binary"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// FtypBox - File Type Box (ftyp - mandatory in full file/init segment)
type FtypBox struct {
	data []byte
}

// MajorBrand - major brand (4 chars)
func (b *FtypBox) MajorBrand() string {
	return string(b.data[:4])
}

// MinorVersion - minor version
func (b *FtypBox) MinorVersion() uint32 {
	return binary.BigEndian.Uint32(b.data[4:8])
}

// AddCompatibleBrands adds new compatible brands to Ftyp box.
func (b *FtypBox) AddCompatibleBrands(compatibleBrands []string) {
	for _, cb := range compatibleBrands {
		b.data = append(b.data, []byte(cb)...)
	}
}

// CompatibleBrands - slice of compatible brands (4 chars each)
func (b *FtypBox) CompatibleBrands() []string {
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

// CreateFtyp - Create an Ftyp box suitable for DASH/CMAF
func CreateFtyp() *FtypBox {
	return NewFtyp("cmfc", 0, []string{"dash", "iso6"})
}

// NewFtyp - new ftyp box with parameters
func NewFtyp(majorBrand string, minorVersion uint32, compatibleBrands []string) *FtypBox {
	data := make([]byte, 8+4*len(compatibleBrands))
	copy(data, []byte(majorBrand))
	binary.BigEndian.PutUint32(data[4:8], minorVersion)
	for i, cb := range compatibleBrands {
		pos := 8 + 4*i
		copy(data[pos:pos+4], []byte(cb))
	}
	return &FtypBox{data: data}
}

// DecodeFtyp - box-specific decode
func DecodeFtyp(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeFtypSR(hdr, startPos, sr)
}

// DecodeFtypSR - box-specific decode
func DecodeFtypSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &FtypBox{data: sr.ReadBytes(hdr.payloadLen())}, sr.AccError()
}

// Type - return box type
func (b *FtypBox) Type() string {
	return "ftyp"
}

// Size - return calculated size
func (b *FtypBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.data))
}

// Encode - write box to w
func (b *FtypBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *FtypBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteBytes(b.data)
	return sw.AccError()
}

// Info - write specific box info to w
func (b *FtypBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - majorBrand: %s", b.MajorBrand())
	bd.write(" - minorVersion: %d", b.MinorVersion())
	for _, cb := range b.CompatibleBrands() {
		bd.write(" - compatibleBrand: %s", cb)
	}
	return bd.err
}
