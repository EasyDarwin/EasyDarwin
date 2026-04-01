package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// ElstBox - Edit List Box (elst - optional)
//
// Contained in : Edit Box (edts)
type ElstBox struct {
	Version byte
	Flags   uint32
	Entries []ElstEntry
}

type ElstEntry struct {
	SegmentDuration   uint64
	MediaTime         int64
	MediaRateInteger  int16
	MediaRateFraction int16
}

// DecodeElst - box-specific decode
func DecodeElst(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeElstSR(hdr, startPos, sr)
}

// DecodeElstSR - box-specific decode
func DecodeElstSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	entryCount := sr.ReadUint32()
	b := &ElstBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
		Entries: make([]ElstEntry, entryCount),
	}

	if version == 1 {
		for i := 0; i < int(entryCount); i++ {
			b.Entries[i].SegmentDuration = sr.ReadUint64()
			b.Entries[i].MediaTime = sr.ReadInt64()
			b.Entries[i].MediaRateInteger = sr.ReadInt16()
			b.Entries[i].MediaRateFraction = sr.ReadInt16()
		}
	} else if version == 0 {
		for i := 0; i < int(entryCount); i++ {
			b.Entries[i].SegmentDuration = uint64(sr.ReadUint32())
			b.Entries[i].MediaTime = int64(sr.ReadInt32())
			b.Entries[i].MediaRateInteger = sr.ReadInt16()
			b.Entries[i].MediaRateFraction = sr.ReadInt16()
		}
	} else {
		return nil, fmt.Errorf("unknown version for elst")
	}
	return b, sr.AccError()
}

// Type - box type
func (b *ElstBox) Type() string {
	return "elst"
}

// Size - calculated size of box
func (b *ElstBox) Size() uint64 {
	if b.Version == 1 {
		return uint64(boxHeaderSize + 8 + len(b.Entries)*20)
	}
	return uint64(boxHeaderSize + 8 + len(b.Entries)*12) // m.Version == 0
}

// Encode - write box to w
func (b *ElstBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *ElstBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(uint32(len(b.Entries)))
	if b.Version == 1 {
		for i := range b.Entries {
			sw.WriteUint64(b.Entries[i].SegmentDuration)
			sw.WriteInt64(b.Entries[i].MediaTime)
			sw.WriteInt16(b.Entries[i].MediaRateInteger)
			sw.WriteInt16(b.Entries[i].MediaRateFraction)
		}
	} else {
		for i := range b.Entries {
			sw.WriteUint32(uint32(b.Entries[i].SegmentDuration))
			sw.WriteInt32(int32(b.Entries[i].MediaTime))
			sw.WriteInt16(b.Entries[i].MediaRateInteger)
			sw.WriteInt16(b.Entries[i].MediaRateFraction)
		}
	}
	return sw.AccError()
}

// Info - write box-specific information
func (b *ElstBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	for i := 0; i < len(b.Entries); i++ {
		bd.write("- entry[%d]: segmentDuration=%d mediaTime=%d, mediaRateInteger=%d "+
			"mediaRateFraction=%d", i+1, b.Entries[i].SegmentDuration, b.Entries[i].MediaTime,
			b.Entries[i].MediaRateInteger, b.Entries[i].MediaRateFraction)
	}
	return bd.err
}
