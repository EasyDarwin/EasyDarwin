package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// TfraBox - Track Fragment Random Access Box (tfra)
// Contained it MfraBox (mfra)
type TfraBox struct {
	Version               byte
	Flags                 uint32
	TrackID               uint32
	LengthSizeOfTrafNum   byte
	LengthSizeOfTrunNum   byte
	LengthSizeOfSampleNum byte
	Entries               []TfraEntry
}

// TfraEntry - reference as used inside TfraBox
type TfraEntry struct {
	Time         uint64
	MoofOffset   uint64
	TrafNumber   uint32
	TrunNumber   uint32
	SampleNumber uint32
}

// DecodeTfra - box-specific decode
func DecodeTfra(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeTfraSR(hdr, startPos, sr)
}

// DecodeTfraSR - box-specific decode
func DecodeTfraSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)

	b := TfraBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	b.TrackID = sr.ReadUint32()
	sizesBlock := sr.ReadUint32()
	b.LengthSizeOfTrafNum = byte((sizesBlock >> 4) & 0x3)
	b.LengthSizeOfTrunNum = byte((sizesBlock >> 2) & 0x3)
	b.LengthSizeOfSampleNum = byte(sizesBlock & 0x3)
	nrEntries := sr.ReadUint32()
	for i := uint32(0); i < nrEntries; i++ {
		te := TfraEntry{}
		if b.Version == 1 {
			te.Time = sr.ReadUint64()
			te.MoofOffset = sr.ReadUint64()
		} else {
			te.Time = uint64(sr.ReadUint32())
			te.MoofOffset = uint64(sr.ReadUint32())
		}
		switch b.LengthSizeOfTrafNum {
		case 0:
			te.TrafNumber = uint32(sr.ReadUint8())
		case 1:
			te.TrafNumber = uint32(sr.ReadUint16())
		case 2:
			te.TrafNumber = uint32(sr.ReadUint24())
		case 3:
			te.TrafNumber = sr.ReadUint32()
		}
		switch b.LengthSizeOfTrunNum {
		case 0:
			te.TrunNumber = uint32(sr.ReadUint8())
		case 1:
			te.TrunNumber = uint32(sr.ReadUint16())
		case 2:
			te.TrunNumber = uint32(sr.ReadUint24())
		case 3:
			te.TrunNumber = sr.ReadUint32()
		}
		switch b.LengthSizeOfSampleNum {
		case 0:
			te.SampleNumber = uint32(sr.ReadUint8())
		case 1:
			te.SampleNumber = uint32(sr.ReadUint16())
		case 2:
			te.SampleNumber = uint32(sr.ReadUint24())
		case 3:
			te.SampleNumber = sr.ReadUint32()
		}
		b.Entries = append(b.Entries, te)
	}
	return &b, sr.AccError()
}

// Type - return box type
func (b *TfraBox) Type() string {
	return "tfra"
}

// Size - return calculated size
func (b *TfraBox) Size() uint64 {
	// Add up all fields depending on version
	nrEntries := len(b.Entries)
	size := (boxHeaderSize + 4 + 12 + // Up to number_of_entry
		8*(1+int(b.Version))*nrEntries +
		int(1+b.LengthSizeOfTrafNum+
			1+b.LengthSizeOfTrunNum+
			1+b.LengthSizeOfSampleNum)*nrEntries)
	return uint64(size)

}

// Encode - write box to w
func (b *TfraBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *TfraBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(b.TrackID)
	sizesBlock := uint32(b.LengthSizeOfTrafNum<<4 + b.LengthSizeOfTrunNum<<2 + b.LengthSizeOfSampleNum)
	sw.WriteUint32(sizesBlock)
	sw.WriteUint32(uint32(len(b.Entries)))
	for _, e := range b.Entries {

		if b.Version == 1 {
			sw.WriteUint64(e.Time)
			sw.WriteUint64(e.MoofOffset)
		} else {
			sw.WriteUint32(uint32(e.Time))
			sw.WriteUint32(uint32(e.MoofOffset))
		}
		switch b.LengthSizeOfTrafNum {
		case 0:
			sw.WriteUint8(byte(e.TrafNumber))
		case 1:
			sw.WriteUint16(uint16(e.TrafNumber))
		case 2:
			sw.WriteUint24(uint32(e.TrafNumber))
		case 3:
			sw.WriteUint32(uint32(e.TrafNumber))
		}
		switch b.LengthSizeOfTrunNum {
		case 0:
			sw.WriteUint8(byte(e.TrunNumber))
		case 1:
			sw.WriteUint16(uint16(e.TrunNumber))
		case 2:
			sw.WriteUint24(uint32(e.TrunNumber))
		case 3:
			sw.WriteUint32(uint32(e.TrunNumber))
		}
		switch b.LengthSizeOfSampleNum {
		case 0:
			sw.WriteUint8(byte(e.SampleNumber))
		case 1:
			sw.WriteUint16(uint16(e.SampleNumber))
		case 2:
			sw.WriteUint24(uint32(e.SampleNumber))
		case 3:
			sw.WriteUint32(uint32(e.SampleNumber))
		}
	}
	return sw.AccError()
}

// Info - box-specific info. More for level 1
func (b *TfraBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - trackID: %d", b.TrackID)
	bd.write(" - nrEntries: %d", len(b.Entries))
	level := getInfoLevel(b, specificBoxLevels)
	if level >= 1 {
		for i, e := range b.Entries {
			bd.write(" - %d: time=%d moofOffset=%d trafNr=%d trunNr=%d sampleDelta=%d",
				i+1, e.Time, e.MoofOffset, e.TrafNumber, e.TrunNumber, e.SampleNumber)
		}
	}
	return bd.err
}

// FindEntry - find entry for moofStart. Return nil if not found
func (b *TfraBox) FindEntry(moofStart uint64) *TfraEntry {
	for _, e := range b.Entries {
		if uint64(e.MoofOffset) == moofStart {
			return &e
		}
	}
	return nil
}
