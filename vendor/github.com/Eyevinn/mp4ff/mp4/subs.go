package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

/*

subs definition according to ISO/IEC 14496-12 Section 8.7.7.2

aligned(8) class SubSampleInformationBox
    extends FullBox(‘subs’, version, flags) {
    unsigned int(32) entry_count;
	int i,j;
	for (i=0; i < entry_count; i++) {
		unsigned int(32) sample_delta;
		unsigned int(16) subsample_count;
		if (subsample_count > 0) {
			for (j=0; j < subsample_count; j++) {
				if(version == 1) {
					unsigned int(32) subsample_size;
				} else {
					unsigned int(16) subsample_size;
				}
				unsigned int(8) subsample_priority;
				unsigned int(8) discardable;
				unsigned int(32) codec_specific_parameters;
			}
		}
	}
}
*/

// SubsBox - SubSampleInformationBox
type SubsBox struct {
	Version byte
	Flags   uint32
	Entries []SubsEntry
}

// SubsEntry - entry in SubsBox
type SubsEntry struct {
	SampleDelta uint32
	SubSamples  []SubsSample
}

// SubsSample - sample in SubsEntry
type SubsSample struct {
	SubsampleSize           uint32
	CodecSpecificParameters uint32
	SubsamplePriority       uint8
	Discardable             uint8
}

// DecodeSubs - box-specific decode
func DecodeSubs(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSubsSR(hdr, startPos, sr)
}

// DecodeSubsSR - box-specific decode
func DecodeSubsSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)

	b := SubsBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	entryCount := sr.ReadUint32()
	for i := uint32(0); i < entryCount; i++ {
		e := SubsEntry{}
		e.SampleDelta = sr.ReadUint32()
		subsampleCount := sr.ReadUint16()
		for j := uint16(0); j < subsampleCount; j++ {
			ss := SubsSample{}
			if version == 1 {
				ss.SubsampleSize = sr.ReadUint32()
			} else {
				ss.SubsampleSize = uint32(sr.ReadUint16())
			}
			ss.SubsamplePriority = sr.ReadUint8()
			ss.Discardable = sr.ReadUint8()
			ss.CodecSpecificParameters = sr.ReadUint32()
			e.SubSamples = append(e.SubSamples, ss)
		}
		b.Entries = append(b.Entries, e)
	}
	return &b, sr.AccError()
}

// Type - return box type
func (b *SubsBox) Type() string {
	return "subs"
}

// Size - return calculated size
func (b *SubsBox) Size() uint64 {
	size := boxHeaderSize + 4 + 4 // FullBox + entry_count
	for _, e := range b.Entries {
		size += 6 // sample_delta + sub_sample_count
		//  4 entries per subsample with different lengths for
		// version 0 and 1
		if b.Version == 0 {
			size += len(e.SubSamples) * (2 + 1 + 1 + 4)
		} else {
			size += len(e.SubSamples) * (4 + 1 + 1 + 4)
		}
	}
	return uint64(size)
}

// Encode - write box to w
func (b *SubsBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SubsBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(uint32(len(b.Entries)))
	for _, e := range b.Entries {
		sw.WriteUint32(e.SampleDelta)
		sw.WriteUint16(uint16(len(e.SubSamples)))
		for _, s := range e.SubSamples {
			if b.Version == 1 {
				sw.WriteUint32(s.SubsampleSize)
			} else {
				sw.WriteUint16(uint16(s.SubsampleSize))
			}
			sw.WriteUint8(s.SubsamplePriority)
			sw.WriteUint8(s.Discardable)
			sw.WriteUint32(s.CodecSpecificParameters)
		}
	}
	return sw.AccError()
}

// Info - specificBoxLevels dump:1 gives details
func (b *SubsBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	level := getInfoLevel(b, specificBoxLevels)
	if level < 1 {
		return bd.err
	}
	for _, e := range b.Entries {
		bd.write(" - sampleDelta: %d", e.SampleDelta)
		for _, s := range e.SubSamples {
			msg := fmt.Sprintf("  > subSampleSize=%d", s.SubsampleSize)
			msg += fmt.Sprintf(" subSamplePriority=%d discardable=%d", s.SubsamplePriority, s.Discardable)
			msg += fmt.Sprintf(" codecSpecificParameters=%d", s.CodecSpecificParameters)
			bd.write(msg)
		}
	}
	return bd.err
}
