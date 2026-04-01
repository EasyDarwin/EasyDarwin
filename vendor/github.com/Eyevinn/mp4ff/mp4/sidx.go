package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

/*

Definition according to ISO/IEC 14496-12 Section 8.16.3.2
aligned(8) class SegmentIndexBox extends FullBox(‘sidx’, version, 0) {
	unsigned int(32) reference_ID;
	unsigned int(32) timescale;
	if (version==0) {
		unsigned int(32) earliest_presentation_time;
		unsigned int(32) first_offset;
	} else {
		unsigned int(64) earliest_presentation_time;
		unsigned int(64) first_offset;
	}
	unsigned int(16) reserved = 0;
	unsigned int(16) reference_count;
	for(i=1; i <= reference_count; i++) {
		bit (1)           reference_type;
		unsigned int(31)  referenced_size;
		unsigned int(32)  subsegment_duration;
		bit(1)            starts_with_SAP;
		unsigned int(3)   SAP_type;
		unsigned int(28)  SAP_delta_time;
    }
}
*/

// SidxBox - SegmentIndexBox
type SidxBox struct {
	Version                  byte
	Flags                    uint32
	ReferenceID              uint32
	Timescale                uint32
	EarliestPresentationTime uint64
	// FirstOffset is offset of first media segment relative to AnchorPoint
	FirstOffset uint64
	// AnchorPoint is first byte offset after SidxBox
	AnchorPoint uint64
	SidxRefs    []SidxRef
}

// SidxRef - reference as used inside SidxBox
type SidxRef struct {
	ReferencedSize     uint32
	SubSegmentDuration uint32
	SAPDeltaTime       uint32
	ReferenceType      uint8 // 1-bit
	StartsWithSAP      uint8 // 1-bit
	SAPType            uint8
}

// DecodeSidx - box-specific decode
func DecodeSidx(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSidxSR(hdr, startPos, sr)
}

// DecodeSidxSR - box-specific decode
func DecodeSidxSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)

	b := &SidxBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	b.ReferenceID = sr.ReadUint32()
	b.Timescale = sr.ReadUint32()
	if version == 0 {
		b.EarliestPresentationTime = uint64(sr.ReadUint32())
		b.FirstOffset = uint64(sr.ReadUint32())
	} else {
		b.EarliestPresentationTime = sr.ReadUint64()
		b.FirstOffset = sr.ReadUint64()
	}
	b.AnchorPoint = startPos + b.FirstOffset + hdr.Size
	sr.SkipBytes(2)
	refCount := sr.ReadUint16()
	for i := 0; i < int(refCount); i++ {
		ref := SidxRef{}
		work := sr.ReadUint32()
		ref.ReferenceType = uint8(work >> 31)
		ref.ReferencedSize = work & 0x7fffffff
		ref.SubSegmentDuration = sr.ReadUint32()
		work = sr.ReadUint32()
		ref.StartsWithSAP = uint8(work >> 31)
		ref.SAPType = uint8((work >> 28) & 0x07)
		ref.SAPDeltaTime = work & 0x0fffffff
		b.SidxRefs = append(b.SidxRefs, ref)
	}
	return b, sr.AccError()
}

// CreateSidx - Create a new TfdtBox with baseMediaDecodeTime
func CreateSidx(baseMediaDecodeTime uint64) *SidxBox {
	var version byte = 0
	if baseMediaDecodeTime >= 4294967296 {
		version = 1
	}
	return &SidxBox{
		Version: version,
		Flags:   0,
	}
}

// Type - return box type
func (b *SidxBox) Type() string {
	return "sidx"
}

// Size - return calculated size
func (b *SidxBox) Size() uint64 {
	// Add up all fields depending on version
	return uint64(boxHeaderSize + 4 + 20 + 8*int(b.Version) + len(b.SidxRefs)*12)
}

// Encode - write box to w
func (b *SidxBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SidxBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(b.ReferenceID)
	sw.WriteUint32(b.Timescale)
	if b.Version == 0 {
		sw.WriteUint32(uint32(b.EarliestPresentationTime))
		sw.WriteUint32(uint32(b.FirstOffset))
	} else {
		sw.WriteUint64(b.EarliestPresentationTime)
		sw.WriteUint64(b.FirstOffset)
	}
	sw.WriteUint16(0) // Reserved
	sw.WriteUint16(uint16(len(b.SidxRefs)))
	for _, ref := range b.SidxRefs {
		sw.WriteUint32(uint32(ref.ReferenceType)<<31 | ref.ReferencedSize)
		sw.WriteUint32(ref.SubSegmentDuration)
		sw.WriteUint32((uint32(ref.StartsWithSAP) << 31) | (uint32(ref.SAPType) << 28) |
			ref.SAPDeltaTime)
	}
	return sw.AccError()
}

// Info - more info for level 1
func (b *SidxBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - referenceID: %d", b.ReferenceID)
	bd.write(" - timeScale: %d", b.Timescale)
	bd.write(" - earliestPresentationTime: %d", b.EarliestPresentationTime)
	bd.write(" - firstOffset: %d", b.FirstOffset)
	level := getInfoLevel(b, specificBoxLevels)
	if level >= 1 {
		for i, ref := range b.SidxRefs {
			bd.write(" - reference[%d]: type=%d size=%d subSegmentDuration=%d startsWithSAP=%d SAPType=%d SAPDeltaTime=%d",
				i+1, ref.ReferenceType, ref.ReferencedSize, ref.SubSegmentDuration, ref.StartsWithSAP, ref.SAPType, ref.SAPDeltaTime)
		}
	}
	return bd.err
}
