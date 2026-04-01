package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// TrunBox - Track Fragment Run Box (trun)
//
// Contained in :  Track Fragment Box (traf)
type TrunBox struct {
	Version          byte
	Flags            uint32
	DataOffset       int32
	firstSampleFlags uint32 // interpreted same way as SampleFlags
	Samples          []Sample
	writeOrderNr     uint32 // Used for multi trun offsets
}

const TrunDataOffsetPresentFlag uint32 = 0x01
const TrunFirstSampleFlagsPresentFlag uint32 = 0x04
const TrunSampleDurationPresentFlag uint32 = 0x100
const TrunSampleSizePresentFlag uint32 = 0x200
const TrunSampleFlagsPresentFlag uint32 = 0x400
const TrunSampleCompositionTimeOffsetPresentFlag uint32 = 0x800

// DecodeTrun - box-specific decode
func DecodeTrun(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	s := bits.NewFixedSliceReader(data)
	versionAndFlags := s.ReadUint32()
	sampleCount := s.ReadUint32()
	t := &TrunBox{
		Version: byte(versionAndFlags >> 24),
		Flags:   versionAndFlags & flagsMask,
		Samples: make([]Sample, 0, sampleCount),
	}

	if t.HasDataOffset() {
		t.DataOffset = s.ReadInt32()
	}

	if t.HasFirstSampleFlags() {
		t.firstSampleFlags = s.ReadUint32()
	}

	var i uint32
	for i = 0; i < sampleCount; i++ {
		var dur, size, flags uint32
		var cto int32
		if t.HasSampleDuration() {
			dur = s.ReadUint32()
		}
		if t.HasSampleSize() {
			size = s.ReadUint32()
		}
		if t.HasSampleFlags() {
			flags = s.ReadUint32()
		} else if t.HasFirstSampleFlags() && i == 0 {
			flags = t.firstSampleFlags
		}
		if t.HasSampleCompositionTimeOffset() {
			cto = s.ReadInt32()
		}
		t.Samples = append(t.Samples, Sample{flags, dur, size, cto})
	}

	return t, nil
}

// DecodeTrun - box-specific decode
func DecodeTrunSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	sampleCount := sr.ReadUint32()
	t := &TrunBox{
		Version: byte(versionAndFlags >> 24),
		Flags:   versionAndFlags & flagsMask,
		Samples: make([]Sample, 0, sampleCount),
	}

	if t.HasDataOffset() {
		t.DataOffset = sr.ReadInt32()
	}

	if t.HasFirstSampleFlags() {
		t.firstSampleFlags = sr.ReadUint32()
	}

	var i uint32
	for i = 0; i < sampleCount; i++ {
		var dur, size, flags uint32
		var cto int32
		if t.HasSampleDuration() {
			dur = sr.ReadUint32()
		}
		if t.HasSampleSize() {
			size = sr.ReadUint32()
		}
		if t.HasSampleFlags() {
			flags = sr.ReadUint32()
		} else if t.HasFirstSampleFlags() && i == 0 {
			flags = t.firstSampleFlags
		}
		if t.HasSampleCompositionTimeOffset() {
			cto = sr.ReadInt32()
		}
		t.Samples = append(t.Samples, Sample{flags, dur, size, cto})
	}

	return t, sr.AccError()
}

// CreateTrun - create a TrunBox for filling up with samples.
// writeOrderNr is only used for multi-trun offsets.
func CreateTrun(writeOrderNr uint32) *TrunBox {
	trun := &TrunBox{
		Version:          1,     // Signed composition_time_offset
		Flags:            0xf01, // Data offset and all sample data present
		DataOffset:       0,
		firstSampleFlags: 0,
		Samples:          nil,
		writeOrderNr:     writeOrderNr,
	}
	return trun
}

// AddSampleDefaultValues - add values from tfhd and trex boxes if needed
// Return total duration
func (t *TrunBox) AddSampleDefaultValues(tfhd *TfhdBox, trex *TrexBox) (totalDur uint64) {

	var defaultSampleDuration uint32
	var defaultSampleSize uint32
	var defaultSampleFlags uint32

	if tfhd.HasDefaultSampleDuration() {
		defaultSampleDuration = tfhd.DefaultSampleDuration
	} else if trex != nil {
		defaultSampleDuration = trex.DefaultSampleDuration
	}
	if tfhd.HasDefaultSampleSize() {
		defaultSampleSize = tfhd.DefaultSampleSize
	} else if trex != nil {
		defaultSampleSize = trex.DefaultSampleSize
	}
	if tfhd.HasDefaultSampleFlags() {
		defaultSampleFlags = tfhd.DefaultSampleFlags
	} else if trex != nil {
		defaultSampleFlags = trex.DefaultSampleFlags
	}
	var i uint32
	totalDur = 0
	for i = 0; i < t.SampleCount(); i++ {
		if !t.HasSampleDuration() {
			t.Samples[i].Dur = defaultSampleDuration
		}
		totalDur += uint64(t.Samples[i].Dur)
		if !t.HasSampleSize() {
			t.Samples[i].Size = defaultSampleSize
		}
		if !t.HasSampleFlags() {
			if i > 0 || !t.HasFirstSampleFlags() {
				t.Samples[i].Flags = defaultSampleFlags
			}
		}
	}
	return totalDur
}

// FirstSampleFlags - return firstSampleFlags and indicator if present
func (t *TrunBox) FirstSampleFlags() (flags uint32, present bool) {
	return t.firstSampleFlags, t.Flags&TrunFirstSampleFlagsPresentFlag != 0
}

// SetFirstSampleFlags - set firstSampleFlags and bit indicating its presence
func (t *TrunBox) SetFirstSampleFlags(flags uint32) {
	t.firstSampleFlags = flags
	t.Flags |= TrunFirstSampleFlagsPresentFlag
}

// RemoveFirstSampleFlags - remove firstSampleFlags and its indicator
func (t *TrunBox) RemoveFirstSampleFlags() {
	t.firstSampleFlags = 0
	t.Flags &= ^TrunFirstSampleFlagsPresentFlag
}

// SampleCount - return how many samples are defined
func (t *TrunBox) SampleCount() uint32 {
	return uint32(len(t.Samples))
}

// HasDataOffset - interpreted dataOffsetPresent flag
func (t *TrunBox) HasDataOffset() bool {
	return t.Flags&TrunDataOffsetPresentFlag != 0
}

// HasFirstSampleFlags - interpreted firstSampleFlagsPresent flag
func (t *TrunBox) HasFirstSampleFlags() bool {
	return t.Flags&TrunFirstSampleFlagsPresentFlag != 0
}

// HasSampleDuration - interpreted sampleDurationPresent flag
func (t *TrunBox) HasSampleDuration() bool {
	return t.Flags&TrunSampleDurationPresentFlag != 0
}

// HasSampleFlags - interpreted sampleFlagsPresent flag
func (t *TrunBox) HasSampleFlags() bool {
	return t.Flags&TrunSampleFlagsPresentFlag != 0
}

// HasSampleSize - interpreted sampleSizePresent flag
func (t *TrunBox) HasSampleSize() bool {
	return t.Flags&TrunSampleSizePresentFlag != 0
}

// HasSampleCompositionTimeOffset - interpreted sampleCompositionTimeOffset flag
func (t *TrunBox) HasSampleCompositionTimeOffset() bool {
	return t.Flags&TrunSampleCompositionTimeOffsetPresentFlag != 0
}

// Type - return box type
func (t *TrunBox) Type() string {
	return "trun"
}

// Size - return calculated size
func (t *TrunBox) Size() uint64 {
	sz := boxHeaderSize + 8 // flags + entrycCount
	if t.HasDataOffset() {
		sz += 4
	}
	if t.HasFirstSampleFlags() {
		sz += 4
	}
	bytesPerSample := 0
	if t.HasSampleDuration() {
		bytesPerSample += 4
	}
	if t.HasSampleSize() {
		bytesPerSample += 4
	}
	if t.HasSampleFlags() {
		bytesPerSample += 4
	}
	if t.HasSampleCompositionTimeOffset() {
		bytesPerSample += 4
	}
	sz += int(t.SampleCount()) * bytesPerSample
	return uint64(sz)
}

// Encode - write box to w
func (t *TrunBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(t.Size()))
	err := t.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (t *TrunBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(t, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(t.Version) << 24) + t.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(t.SampleCount())
	if t.HasDataOffset() {
		if t.DataOffset == 0 {
			panic("trun data offset not set")
		}
		sw.WriteInt32(t.DataOffset)
	}
	if t.HasFirstSampleFlags() {
		sw.WriteUint32(t.firstSampleFlags)
	}
	var i uint32
	for i = 0; i < t.SampleCount(); i++ {
		if t.HasSampleDuration() {
			sw.WriteUint32(t.Samples[i].Dur)
		}
		if t.HasSampleSize() {
			sw.WriteUint32(t.Samples[i].Size)
		}
		if t.HasSampleFlags() {
			sw.WriteUint32(t.Samples[i].Flags)
		}
		if t.HasSampleCompositionTimeOffset() {
			sw.WriteInt32(t.Samples[i].CompositionTimeOffset)
		}

	}
	return sw.AccError()
}

// Info - specificBoxLevels trun:1 gives details
func (t *TrunBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, t, int(t.Version), t.Flags)
	bd.write(" - sampleCount: %d", t.SampleCount())
	level := getInfoLevel(t, specificBoxLevels)
	if level > 0 {
		if t.HasDataOffset() {
			bd.write(" - DataOffset: %d", t.DataOffset)
		}
		if t.HasFirstSampleFlags() {
			bd.write(" - firstSampleFlags: %08x (%s)", t.firstSampleFlags, DecodeSampleFlags(t.firstSampleFlags))
		}
		for i := 0; i < int(t.SampleCount()); i++ {
			msg := fmt.Sprintf(" - sample[%d]:", i+1)
			if t.HasSampleDuration() {
				msg += fmt.Sprintf(" dur=%d", t.Samples[i].Dur)
			}
			if t.HasSampleSize() {
				msg += fmt.Sprintf(" size=%d", t.Samples[i].Size)
			}
			if t.HasSampleFlags() {
				sampleFlags := t.Samples[i].Flags
				msg += fmt.Sprintf(" flags=%08x (%s)", sampleFlags, DecodeSampleFlags(sampleFlags))
			}
			if t.HasSampleCompositionTimeOffset() {
				msg += fmt.Sprintf(" compositionTimeOffset=%d", t.Samples[i].CompositionTimeOffset)
			}
			bd.write(msg)
		}
	}
	return bd.err
}

// GetFullSamples - get all sample data including accumulated time and binary media data
// offsetInMdat is offset in mdat data (data normally starts 8 or 16 bytes after start of mdat box)
// baseDecodeTime is decodeTime in tfdt in track timescale (timescale in mfhd)
// To fill missing individual values from tfhd and trex defaults, call trun.AddSampleDefaultValues() before this call
func (t *TrunBox) GetFullSamples(offsetInMdat uint32, baseDecodeTime uint64, mdat *MdatBox) []FullSample {
	samples := make([]FullSample, 0, t.SampleCount())
	var accDur uint64 = 0
	for _, s := range t.Samples {
		dTime := baseDecodeTime + accDur

		newSample := FullSample{
			Sample:     s,
			DecodeTime: dTime,
			Data:       mdat.Data[offsetInMdat : offsetInMdat+s.Size],
		}
		samples = append(samples, newSample)
		accDur += uint64(s.Dur)
		offsetInMdat += s.Size
	}
	return samples
}

// GetSamples - get all trun sample data
// To fill missing individual values from tfhd and trex defaults, call AddSampleDefaultValues() before this call
func (t *TrunBox) GetSamples() []Sample {
	return t.Samples
}

// GetSampleRange - get a one-based range of samples
// To fill missing individual values from tfhd and trex defaults, call AddSampleDefaultValues() before this call
func (t *TrunBox) GetSampleRange(startSampleNr, endSampleNr uint32) []Sample {
	return t.Samples[startSampleNr-1 : endSampleNr]
}

// GetSampleInterval - get sample interval [startSampleNr, endSampleNr] (1-based and inclusive)
// This includes mdat data (if not lazy), in which case only offsetInMdat is given.
// baseDecodeTime is decodeTime in tfdt in track timescale (timescale from mfhd).
// To fill missing individual values from tfhd and trex defaults, call AddSampleDefaultValues() before this call.
func (t *TrunBox) GetSampleInterval(startSampleNr, endSampleNr uint32, baseDecodeTime uint64,
	mdat *MdatBox, offsetInMdat uint32) (SampleInterval, error) {
	si := SampleInterval{}
	if startSampleNr < 1 {
		return si, fmt.Errorf("startSegNr < 1")
	}
	nrSamples := uint32(len(t.Samples))
	if endSampleNr > nrSamples {
		return si, fmt.Errorf("endSampleNr=%d is greater than nr samples %d", endSampleNr, nrSamples)
	}
	decTime := baseDecodeTime
	var size uint32
	for i, s := range t.Samples {
		sampleNr := uint32(i + 1)
		if sampleNr == startSampleNr {
			si.FirstDecodeTime = decTime
		}
		if sampleNr >= startSampleNr {
			size += s.Size
			if sampleNr == endSampleNr {
				break
			}
			continue
		}
		decTime += uint64(s.Dur)
		offsetInMdat += s.Size
	}
	si.Samples = t.Samples[startSampleNr-1 : endSampleNr]
	si.OffsetInMdat = offsetInMdat
	si.Size = size
	if mdat != nil && !mdat.IsLazy() {
		si.Data = mdat.Data[si.OffsetInMdat : si.OffsetInMdat+si.Size]
	}
	return si, nil
}

// AddFullSample - add Sample part of FullSample
func (t *TrunBox) AddFullSample(s *FullSample) {
	t.Samples = append(t.Samples, s.Sample)
}

// AddSample - add a Sample
func (t *TrunBox) AddSample(s Sample) {
	t.Samples = append(t.Samples, s)
}

// AddSamples - add a a slice of Sample
func (t *TrunBox) AddSamples(s []Sample) {
	t.Samples = append(t.Samples, s...)
}

// Duration returns the total duration of all samples given defaultSampleDuration
func (t *TrunBox) Duration(defaultSampleDuration uint32) uint64 {
	if !t.HasSampleDuration() {
		return uint64(defaultSampleDuration) * uint64(t.SampleCount())
	}
	var total uint64 = 0
	for _, s := range t.Samples {
		total += uint64(s.Dur)
	}
	return total
}

// CommonSampleDuration returns the common duration if all samples have the same duration, otherwise 0.
func (t *TrunBox) CommonSampleDuration(defaultSampleDuration uint32) uint32 {
	if !t.HasSampleDuration() {
		return defaultSampleDuration
	}
	if t.SampleCount() == 0 {
		return 0
	}
	commonDur := t.Samples[0].Dur
	for i := 1; i < int(t.SampleCount()); i++ {
		if t.Samples[i].Dur != commonDur {
			return 0
		}
	}
	return commonDur
}

// GetSampleNrForRelativeTime - get sample number for exact relative time (calculated from summing durations)
func (t *TrunBox) GetSampleNrForRelativeTime(deltaTime uint64, defaultSampleDuration uint32) (uint32, error) {
	sampleCount := t.SampleCount()
	if sampleCount == 0 {
		return 0, fmt.Errorf("no samples in trun")
	}
	if !t.HasSampleDuration() {
		nr := deltaTime / uint64(defaultSampleDuration)
		if nr >= uint64(sampleCount) {
			return 0, fmt.Errorf("time %d is bigger than largest time %d", deltaTime, (sampleCount-1)*defaultSampleDuration)
		}
		if nr*uint64(defaultSampleDuration) == deltaTime {
			return uint32(nr) + 1, nil
		}
		return 0, fmt.Errorf("did not find time %d but %d", deltaTime, nr*uint64(defaultSampleDuration))
	}
	var accTime uint64
	var nr uint32
	found := false
	for _, s := range t.Samples {
		if deltaTime <= accTime {
			found = true
			break
		}
		accTime += uint64(s.Dur)
		nr++
	}
	if found && accTime == deltaTime {
		return nr + 1, nil
	}
	return 0, fmt.Errorf("did not find time %d but %d", deltaTime, accTime)
}

// SizeOfData - size of mediasamples in bytes
func (t *TrunBox) SizeOfData() (totalSize uint64) {
	for _, sample := range t.Samples {
		totalSize += uint64(sample.Size)
	}
	return totalSize
}
