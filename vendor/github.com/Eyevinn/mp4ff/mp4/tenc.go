package mp4

import (
	"encoding/hex"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// TencBox - Track Encryption Box
// Defined in ISO/IEC 23001-7 Secion 8.2
type TencBox struct {
	Version                byte
	Flags                  uint32
	DefaultCryptByteBlock  byte
	DefaultSkipByteBlock   byte
	DefaultIsProtected     byte
	DefaultPerSampleIVSize byte
	DefaultKID             UUID
	// DefaultConstantIVSize  byte given by len(DefaultConstantIV)
	DefaultConstantIV []byte
}

// DecodeTenc - box-specific decode
func DecodeTenc(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeTencSR(hdr, startPos, sr)
}

// DecodeTencSR - box-specific decode
func DecodeTencSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)

	b := TencBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	_ = sr.ReadUint8() // Skip reserved == 0
	if version == 0 {
		_ = sr.ReadUint8() // Skip reserved == 0
	} else {
		infoByte := sr.ReadUint8()
		b.DefaultCryptByteBlock = infoByte >> 4
		b.DefaultSkipByteBlock = infoByte & 0x0f
	}
	b.DefaultIsProtected = sr.ReadUint8()
	b.DefaultPerSampleIVSize = sr.ReadUint8()
	b.DefaultKID = UUID(sr.ReadBytes(16))
	if b.DefaultIsProtected == 1 && b.DefaultPerSampleIVSize == 0 {
		defaultConstantIVSize := int(sr.ReadUint8())
		b.DefaultConstantIV = sr.ReadBytes(defaultConstantIVSize)
	}
	return &b, sr.AccError()
}

// Type - return box type
func (b *TencBox) Type() string {
	return "tenc"
}

// Size - return calculated size
func (b *TencBox) Size() uint64 {
	var size uint64 = 32
	if b.DefaultIsProtected == 1 && b.DefaultPerSampleIVSize == 0 {
		size += uint64(1 + len(b.DefaultConstantIV))
	}
	return size
}

// Encode - write box to w
func (b *TencBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *TencBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint8(0) // reserved
	if b.Version == 0 {
		sw.WriteUint8(0) // reserved
	} else {
		sw.WriteUint8(b.DefaultCryptByteBlock<<4 | b.DefaultSkipByteBlock)
	}
	sw.WriteUint8(b.DefaultIsProtected)
	sw.WriteUint8(b.DefaultPerSampleIVSize)
	sw.WriteBytes(b.DefaultKID)
	if b.DefaultIsProtected == 1 && b.DefaultPerSampleIVSize == 0 {
		sw.WriteUint8(byte(len(b.DefaultConstantIV)))
		sw.WriteBytes(b.DefaultConstantIV)
	}
	return sw.AccError()
}

// Info - write box info to w
func (b *TencBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) (err error) {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	if b.Version > 0 {
		bd.write(" - defaultCryptByteBlock: %d", b.DefaultCryptByteBlock)
		bd.write(" - defaultSkipByteBlock: %d", b.DefaultSkipByteBlock)
	}
	bd.write(" - defaultIsProtected: %d", b.DefaultIsProtected)
	bd.write(" - defaultPerSampleIVSize: %d", b.DefaultPerSampleIVSize)
	bd.write(" - defaultKID: %s", b.DefaultKID)
	if b.DefaultIsProtected == 1 && b.DefaultPerSampleIVSize == 0 {
		bd.write(" - defaultConstantIV: %s", hex.EncodeToString(b.DefaultConstantIV))
	}
	return bd.err
}
