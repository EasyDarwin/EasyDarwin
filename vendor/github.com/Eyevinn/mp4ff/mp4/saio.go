package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// SaioBox - Sample Auxiliary Information Offsets Box (saiz) (in stbl or traf box)
type SaioBox struct {
	Version              byte
	Flags                uint32
	AuxInfoType          string // Used for Common Encryption Scheme (4-bytes uint32 according to spec)
	AuxInfoTypeParameter uint32
	Offset               []int64
}

// Return a new SaioBox with one offset to be updated later
func NewSaioBox() *SaioBox {
	return &SaioBox{
		Offset: []int64{-1},
	}
}

// SetOffset sets offset for first (and only) entry
func (b *SaioBox) SetOffset(offset int64) {
	b.Offset[0] = offset
}

// DecodeSaio - box-specific decode
func DecodeSaio(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSaioSR(hdr, startPos, sr)
}

// DecodeSaioSR - box-specific decode
func DecodeSaioSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	b := SaioBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	if b.Flags&0x01 != 0 {
		b.AuxInfoType = sr.ReadFixedLengthString(4)
		b.AuxInfoTypeParameter = sr.ReadUint32()
	}
	entryCount := sr.ReadUint32()
	if version == 0 {
		for i := uint32(0); i < entryCount; i++ {
			b.Offset = append(b.Offset, int64(sr.ReadInt32()))
		}
	} else {
		for i := uint32(0); i < entryCount; i++ {
			b.Offset = append(b.Offset, sr.ReadInt64())
		}
	}
	return &b, sr.AccError()
}

// Type - return box type
func (b *SaioBox) Type() string {
	return "saio"
}

// Size - return calculated size
func (b *SaioBox) Size() uint64 {
	size := uint64(boxHeaderSize) + 8
	if b.Flags&0x01 != 0 {
		size += 8
	}
	if b.Version == 0 {
		size += 4 * uint64(len(b.Offset))
	} else {
		size += 8 * uint64(len(b.Offset))
	}
	return size
}

// Encode - write box to w
func (b *SaioBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SaioBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	if b.Flags&0x01 != 0 {
		sw.WriteString(b.AuxInfoType, false)
		sw.WriteUint32(b.AuxInfoTypeParameter)
	}
	sw.WriteUint32(uint32(len(b.Offset)))
	if b.Version == 0 {
		for i := 0; i < len(b.Offset); i++ {
			sw.WriteInt32(int32(b.Offset[i]))
		}
	} else {
		for i := 0; i < len(b.Offset); i++ {
			sw.WriteInt64(b.Offset[i])
		}
	}
	return sw.AccError()
}

// Info - write SaioBox details. Get offset list with level >= 1
func (b *SaioBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) (err error) {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	if b.Flags&0x01 != 0 {
		bd.write(" - auxInfoType: %s", b.AuxInfoType)
		bd.write(" - auxInfoTypeParameter: %d", b.AuxInfoTypeParameter)
	}
	bd.write(" - sampleCount: %d", len(b.Offset))
	level := getInfoLevel(b, specificBoxLevels)
	bd.write(" - offset[%d]=%d", 1, b.Offset[0])
	if level > 0 {
		for i := 1; i < len(b.Offset); i++ {
			bd.write(" - offset[%d]=%d", i+1, b.Offset[i])
		}
	}
	return bd.err
}
