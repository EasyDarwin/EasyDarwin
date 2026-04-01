package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// SaizBox - Sample Auxiliary Information Sizes Box (saiz)  (in stbl or traf box)
type SaizBox struct {
	Version               byte
	Flags                 uint32
	AuxInfoType           string // Used for Common Encryption Scheme (4-bytes uint32 according to spec)
	AuxInfoTypeParameter  uint32
	SampleCount           uint32
	SampleInfo            []byte
	DefaultSampleInfoSize byte
}

// DecodeSaiz - box-specific decode
func DecodeSaiz(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSaizSR(hdr, startPos, sr)
}

// DecodeSaizSR - box-specific decode
func DecodeSaizSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	b := SaizBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	if b.Flags&0x01 != 0 {
		b.AuxInfoType = sr.ReadFixedLengthString(4)
		b.AuxInfoTypeParameter = sr.ReadUint32()
	}
	b.DefaultSampleInfoSize = sr.ReadUint8()
	b.SampleCount = sr.ReadUint32()
	if b.DefaultSampleInfoSize == 0 {
		for i := uint32(0); i < b.SampleCount; i++ {
			b.SampleInfo = append(b.SampleInfo, sr.ReadUint8())
		}
	}
	return &b, sr.AccError()
}

// NewSaizBox creates a SaizBox with appropriate size allocated.
func NewSaizBox(capacity int) *SaizBox {
	return &SaizBox{
		SampleInfo: make([]byte, 0, capacity),
	}
}

// AddSampleInfo adds a sampleinfo info based on parameters provided.
// If no length field, don't update the sample field (typicall audio cbcs)
func (b *SaizBox) AddSampleInfo(iv []byte, subsamplePatterns []SubSamplePattern) {
	size := len(iv)
	if len(subsamplePatterns) > 0 {
		size += 2 + len(subsamplePatterns)*6
		b.SampleInfo = append(b.SampleInfo, byte(size))
	} else if size > 0 {
		switch b.DefaultSampleInfoSize {
		case 0:
			b.DefaultSampleInfoSize = byte(size)
		default:
			if byte(size) != b.DefaultSampleInfoSize {
				panic("inconsistent sample info size")
			}
		}
	}
	if size > 0 {
		b.SampleCount++
	}
}

// Type - return box type
func (b *SaizBox) Type() string {
	return "saiz"
}

// Size - return calculated size
func (b *SaizBox) Size() uint64 {
	size := uint64(boxHeaderSize) + 9
	if b.Flags&0x01 != 0 {
		size += 8
	}
	if b.DefaultSampleInfoSize == 0 {
		size += uint64(b.SampleCount)
	}
	return size
}

// Encode - write box to w
func (b *SaizBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SaizBox) EncodeSW(sw bits.SliceWriter) error {
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
	sw.WriteUint8(b.DefaultSampleInfoSize)
	sw.WriteUint32(b.SampleCount)
	if b.DefaultSampleInfoSize == 0 {
		for i := uint32(0); i < b.SampleCount; i++ {
			sw.WriteUint8(b.SampleInfo[i])
		}
	}
	return sw.AccError()
}

// Info - write SaizBox details. Get sampleInfo list with level >= 1
func (b *SaizBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) (err error) {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	if b.Flags&0x01 != 0 {
		bd.write(" - auxInfoType: %s", b.AuxInfoType)
		bd.write(" - auxInfoTypeParameter: %d", b.AuxInfoTypeParameter)
	}
	bd.write(" - defaultSampleInfoSize: %d", b.DefaultSampleInfoSize)
	bd.write(" - sampleCount: %d", b.SampleCount)
	level := getInfoLevel(b, specificBoxLevels)
	if level > 0 {
		if b.DefaultSampleInfoSize == 0 {
			for i := uint32(0); i < b.SampleCount; i++ {
				bd.write(" - sampleInfo[%d]=%d", i+1, b.SampleInfo[i])
			}
		}
	}
	return bd.err
}
