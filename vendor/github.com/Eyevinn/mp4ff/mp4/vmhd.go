package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// VmhdBox - Video Media Header Box (vhmd - mandatory for video tracks)
//
// Contained in : Media Information Box (minf)
type VmhdBox struct {
	Version      byte
	Flags        uint32
	GraphicsMode uint16
	OpColor      [3]uint16
}

// CreateVmhd - Create Video Media Header Box
func CreateVmhd() *VmhdBox {
	// Flags should be 0x000001 according to ISO/IEC 14496-12 Sec.12.1.2.1
	return &VmhdBox{Flags: 0x000001}
}

// DecodeVmhd - box-specific decode
func DecodeVmhd(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeVmhdSR(hdr, startPos, sr)
}

// DecodeVmhdSR - box-specific decode
func DecodeVmhdSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	b := VmhdBox{
		Version:      byte(versionAndFlags >> 24),
		Flags:        versionAndFlags & flagsMask,
		GraphicsMode: sr.ReadUint16(),
	}
	for i := 0; i < 3; i++ {
		b.OpColor[i] = sr.ReadUint16()
	}
	return &b, sr.AccError()
}

// Type - box-specific type
func (b *VmhdBox) Type() string {
	return "vmhd"
}

// Size - calculated size of box
func (b *VmhdBox) Size() uint64 {
	return boxHeaderSize + 12
}

// Encode - write box to w
func (b *VmhdBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *VmhdBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint16(b.GraphicsMode)
	for i := 0; i < 3; i++ {
		sw.WriteUint16(b.OpColor[i])
	}
	return sw.AccError()
}

// Info - write box-specific information
func (b *VmhdBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	return bd.err
}
