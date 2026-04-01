package mp4

import (
	"bytes"
	"errors"
	"io"

	"github.com/abema/go-mp4/internal/bitio"
)

type ProbeInfo struct {
	MajorBrand       [4]byte
	MinorVersion     uint32
	CompatibleBrands [][4]byte
	FastStart        bool
	Timescale        uint32
	Duration         uint64
	Tracks           Tracks
	Segments         Segments
}

// Deprecated: replace with ProbeInfo
type FraProbeInfo = ProbeInfo

type Tracks []*Track

// Deprecated: replace with Track
type TrackInfo = Track

type Track struct {
	TrackID   uint32
	Timescale uint32
	Duration  uint64
	Codec     Codec
	Encrypted bool
	EditList  EditList
	Samples   Samples
	Chunks    Chunks
	AVC       *AVCDecConfigInfo
	MP4A      *MP4AInfo
}

type Codec int

const (
	CodecUnknown Codec = iota
	CodecAVC1
	CodecMP4A
)

type EditList []*EditListEntry

type EditListEntry struct {
	MediaTime       int64
	SegmentDuration uint64
}

type Samples []*Sample

type Sample struct {
	Size                  uint32
	TimeDelta             uint32
	CompositionTimeOffset int64
}

type Chunks []*Chunk

type Chunk struct {
	DataOffset      uint64
	SamplesPerChunk uint32
}

type AVCDecConfigInfo struct {
	ConfigurationVersion uint8
	Profile              uint8
	ProfileCompatibility uint8
	Level                uint8
	LengthSize           uint16
	Width                uint16
	Height               uint16
}

type MP4AInfo struct {
	OTI          uint8
	AudOTI       uint8
	ChannelCount uint16
}

type Segments []*Segment

// Deprecated: replace with Segment
type SegmentInfo = Segment

type Segment struct {
	TrackID               uint32
	MoofOffset            uint64
	BaseMediaDecodeTime   uint64
	DefaultSampleDuration uint32
	SampleCount           uint32
	Duration              uint32
	CompositionTimeOffset int32
	Size                  uint32
}

// Probe probes MP4 file
func Probe(r io.ReadSeeker) (*ProbeInfo, error) {
	probeInfo := &ProbeInfo{
		Tracks:   make([]*Track, 0, 8),
		Segments: make([]*Segment, 0, 8),
	}
	bis, err := ExtractBoxes(r, nil, []BoxPath{
		{BoxTypeFtyp()},
		{BoxTypeMoov()},
		{BoxTypeMoov(), BoxTypeMvhd()},
		{BoxTypeMoov(), BoxTypeTrak()},
		{BoxTypeMoof()},
		{BoxTypeMdat()},
	})
	if err != nil {
		return nil, err
	}
	var mdatAppeared bool
	for _, bi := range bis {
		switch bi.Type {
		case BoxTypeFtyp():
			var ftyp Ftyp
			if _, err := bi.SeekToPayload(r); err != nil {
				return nil, err
			}
			if _, err := Unmarshal(r, bi.Size-bi.HeaderSize, &ftyp, bi.Context); err != nil {
				return nil, err
			}
			probeInfo.MajorBrand = ftyp.MajorBrand
			probeInfo.MinorVersion = ftyp.MinorVersion
			probeInfo.CompatibleBrands = make([][4]byte, 0, len(ftyp.CompatibleBrands))
			for _, entry := range ftyp.CompatibleBrands {
				probeInfo.CompatibleBrands = append(probeInfo.CompatibleBrands, entry.CompatibleBrand)
			}
		case BoxTypeMoov():
			probeInfo.FastStart = !mdatAppeared
		case BoxTypeMvhd():
			var mvhd Mvhd
			if _, err := bi.SeekToPayload(r); err != nil {
				return nil, err
			}
			if _, err := Unmarshal(r, bi.Size-bi.HeaderSize, &mvhd, bi.Context); err != nil {
				return nil, err
			}
			probeInfo.Timescale = mvhd.Timescale
			if mvhd.GetVersion() == 0 {
				probeInfo.Duration = uint64(mvhd.DurationV0)
			} else {
				probeInfo.Duration = mvhd.DurationV1
			}
		case BoxTypeTrak():
			track, err := probeTrak(r, bi)
			if err != nil {
				return nil, err
			}
			probeInfo.Tracks = append(probeInfo.Tracks, track)
		case BoxTypeMoof():
			segment, err := probeMoof(r, bi)
			if err != nil {
				return nil, err
			}
			probeInfo.Segments = append(probeInfo.Segments, segment)
		case BoxTypeMdat():
			mdatAppeared = true
		}
	}
	return probeInfo, nil
}

// ProbeFra probes fragmented MP4 file
// Deprecated: replace with Probe
func ProbeFra(r io.ReadSeeker) (*FraProbeInfo, error) {
	probeInfo, err := Probe(r)
	return (*FraProbeInfo)(probeInfo), err
}

func probeTrak(r io.ReadSeeker, bi *BoxInfo) (*Track, error) {
	track := new(Track)

	bips, err := ExtractBoxesWithPayload(r, bi, []BoxPath{
		{BoxTypeTkhd()},
		{BoxTypeEdts(), BoxTypeElst()},
		{BoxTypeMdia(), BoxTypeMdhd()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsd(), BoxTypeAvc1()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsd(), BoxTypeAvc1(), BoxTypeAvcC()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsd(), BoxTypeEncv()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsd(), BoxTypeEncv(), BoxTypeAvcC()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsd(), BoxTypeMp4a()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsd(), BoxTypeMp4a(), BoxTypeEsds()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsd(), BoxTypeMp4a(), BoxTypeWave(), BoxTypeEsds()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsd(), BoxTypeEnca()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsd(), BoxTypeEnca(), BoxTypeEsds()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStco()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeCo64()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStts()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeCtts()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsc()},
		{BoxTypeMdia(), BoxTypeMinf(), BoxTypeStbl(), BoxTypeStsz()},
	})
	if err != nil {
		return nil, err
	}
	var tkhd *Tkhd
	var elst *Elst
	var mdhd *Mdhd
	var avc1 *VisualSampleEntry
	var avcC *AVCDecoderConfiguration
	var audioSampleEntry *AudioSampleEntry
	var esds *Esds
	var stco *Stco
	var stts *Stts
	var stsc *Stsc
	var ctts *Ctts
	var stsz *Stsz
	var co64 *Co64
	for _, bip := range bips {
		switch bip.Info.Type {
		case BoxTypeTkhd():
			tkhd = bip.Payload.(*Tkhd)
		case BoxTypeElst():
			elst = bip.Payload.(*Elst)
		case BoxTypeMdhd():
			mdhd = bip.Payload.(*Mdhd)
		case BoxTypeAvc1():
			track.Codec = CodecAVC1
			avc1 = bip.Payload.(*VisualSampleEntry)
		case BoxTypeAvcC():
			avcC = bip.Payload.(*AVCDecoderConfiguration)
		case BoxTypeEncv():
			track.Codec = CodecAVC1
			track.Encrypted = true
		case BoxTypeMp4a():
			track.Codec = CodecMP4A
			audioSampleEntry = bip.Payload.(*AudioSampleEntry)
		case BoxTypeEnca():
			track.Codec = CodecMP4A
			track.Encrypted = true
			audioSampleEntry = bip.Payload.(*AudioSampleEntry)
		case BoxTypeEsds():
			esds = bip.Payload.(*Esds)
		case BoxTypeStco():
			stco = bip.Payload.(*Stco)
		case BoxTypeStts():
			stts = bip.Payload.(*Stts)
		case BoxTypeStsc():
			stsc = bip.Payload.(*Stsc)
		case BoxTypeCtts():
			ctts = bip.Payload.(*Ctts)
		case BoxTypeStsz():
			stsz = bip.Payload.(*Stsz)
		case BoxTypeCo64():
			co64 = bip.Payload.(*Co64)
		}
	}

	if tkhd == nil {
		return nil, errors.New("tkhd box not found")
	}
	track.TrackID = tkhd.TrackID

	if elst != nil {
		editList := make([]*EditListEntry, 0, len(elst.Entries))
		for i := range elst.Entries {
			editList = append(editList, &EditListEntry{
				MediaTime:       elst.GetMediaTime(i),
				SegmentDuration: elst.GetSegmentDuration(i),
			})
		}
		track.EditList = editList
	}

	if mdhd == nil {
		return nil, errors.New("mdhd box not found")
	}
	track.Timescale = mdhd.Timescale
	track.Duration = mdhd.GetDuration()

	if avc1 != nil && avcC != nil {
		track.AVC = &AVCDecConfigInfo{
			ConfigurationVersion: avcC.ConfigurationVersion,
			Profile:              avcC.Profile,
			ProfileCompatibility: avcC.ProfileCompatibility,
			Level:                avcC.Level,
			LengthSize:           uint16(avcC.LengthSizeMinusOne) + 1,
			Width:                avc1.Width,
			Height:               avc1.Height,
		}
	}

	if audioSampleEntry != nil && esds != nil {
		oti, audOTI, err := detectAACProfile(esds)
		if err != nil {
			return nil, err
		}
		track.MP4A = &MP4AInfo{
			OTI:          oti,
			AudOTI:       audOTI,
			ChannelCount: audioSampleEntry.ChannelCount,
		}
	}

	track.Chunks = make([]*Chunk, 0)
	if stco != nil {
		for _, offset := range stco.ChunkOffset {
			track.Chunks = append(track.Chunks, &Chunk{
				DataOffset: uint64(offset),
			})
		}
	} else if co64 != nil {
		for _, offset := range co64.ChunkOffset {
			track.Chunks = append(track.Chunks, &Chunk{
				DataOffset: offset,
			})
		}
	} else {
		return nil, errors.New("stco/co64 box not found")
	}

	if stts == nil {
		return nil, errors.New("stts box not found")
	}
	track.Samples = make([]*Sample, 0)
	for _, entry := range stts.Entries {
		for i := uint32(0); i < entry.SampleCount; i++ {
			track.Samples = append(track.Samples, &Sample{
				TimeDelta: entry.SampleDelta,
			})
		}
	}

	if stsc == nil {
		return nil, errors.New("stsc box not found")
	}
	for si, entry := range stsc.Entries {
		end := uint32(len(track.Chunks))
		if si != len(stsc.Entries)-1 && stsc.Entries[si+1].FirstChunk-1 < end {
			end = stsc.Entries[si+1].FirstChunk - 1
		}
		for ci := entry.FirstChunk - 1; ci < end; ci++ {
			track.Chunks[ci].SamplesPerChunk = entry.SamplesPerChunk
		}
	}

	if ctts != nil {
		var si uint32
		for ci, entry := range ctts.Entries {
			for i := uint32(0); i < entry.SampleCount; i++ {
				if si >= uint32(len(track.Samples)) {
					break
				}
				track.Samples[si].CompositionTimeOffset = ctts.GetSampleOffset(ci)
				si++
			}
		}
	}

	if stsz != nil {
		for i := 0; i < len(stsz.EntrySize) && i < len(track.Samples); i++ {
			track.Samples[i].Size = stsz.EntrySize[i]
		}
	}

	return track, nil
}

func detectAACProfile(esds *Esds) (oti, audOTI uint8, err error) {
	configDscr := findDescriptorByTag(esds.Descriptors, DecoderConfigDescrTag)
	if configDscr == nil || configDscr.DecoderConfigDescriptor == nil {
		return 0, 0, nil
	}
	if configDscr.DecoderConfigDescriptor.ObjectTypeIndication != 0x40 {
		return configDscr.DecoderConfigDescriptor.ObjectTypeIndication, 0, nil
	}

	specificDscr := findDescriptorByTag(esds.Descriptors, DecSpecificInfoTag)
	if specificDscr == nil {
		return 0, 0, errors.New("DecoderSpecificationInfoDescriptor not found")
	}

	r := bitio.NewReader(bytes.NewReader(specificDscr.Data))
	remaining := len(specificDscr.Data) * 8

	// audio object type
	audioObjectType, read, err := getAudioObjectType(r)
	if err != nil {
		return 0, 0, err
	}
	remaining -= read

	// sampling frequency index
	samplingFrequencyIndex, err := r.ReadBits(4)
	if err != nil {
		return 0, 0, err
	}
	remaining -= 4
	if samplingFrequencyIndex[0] == 0x0f {
		if _, err = r.ReadBits(24); err != nil {
			return 0, 0, err
		}
		remaining -= 24
	}

	if audioObjectType == 2 && remaining >= 20 {
		if _, err = r.ReadBits(4); err != nil {
			return 0, 0, err
		}
		remaining -= 4
		syncExtensionType, err := r.ReadBits(11)
		if err != nil {
			return 0, 0, err
		}
		remaining -= 11
		if syncExtensionType[0] == 0x2 && syncExtensionType[1] == 0xb7 {
			extAudioObjectType, _, err := getAudioObjectType(r)
			if err != nil {
				return 0, 0, err
			}
			if extAudioObjectType == 5 || extAudioObjectType == 22 {
				sbr, err := r.ReadBits(1)
				if err != nil {
					return 0, 0, err
				}
				remaining--
				if sbr[0] != 0 {
					if extAudioObjectType == 5 {
						sfi, err := r.ReadBits(4)
						if err != nil {
							return 0, 0, err
						}
						remaining -= 4
						if sfi[0] == 0xf {
							if _, err := r.ReadBits(24); err != nil {
								return 0, 0, err
							}
							remaining -= 24
						}
						if remaining >= 12 {
							syncExtensionType, err := r.ReadBits(11)
							if err != nil {
								return 0, 0, err
							}
							if syncExtensionType[0] == 0x5 && syncExtensionType[1] == 0x48 {
								ps, err := r.ReadBits(1)
								if err != nil {
									return 0, 0, err
								}
								if ps[0] != 0 {
									return 0x40, 29, nil
								}
							}
						}
					}
					return 0x40, 5, nil
				}
			}
		}
	}
	return 0x40, audioObjectType, nil
}

func findDescriptorByTag(dscrs []Descriptor, tag int8) *Descriptor {
	for _, dscr := range dscrs {
		if dscr.Tag == tag {
			return &dscr
		}
	}
	return nil
}

func getAudioObjectType(r bitio.Reader) (byte, int, error) {
	audioObjectType, err := r.ReadBits(5)
	if err != nil {
		return 0, 0, err
	}
	if audioObjectType[0] != 0x1f {
		return audioObjectType[0], 5, nil
	}
	audioObjectType, err = r.ReadBits(6)
	if err != nil {
		return 0, 0, err
	}
	return audioObjectType[0] + 32, 11, nil
}

func probeMoof(r io.ReadSeeker, bi *BoxInfo) (*Segment, error) {
	bips, err := ExtractBoxesWithPayload(r, bi, []BoxPath{
		{BoxTypeTraf(), BoxTypeTfhd()},
		{BoxTypeTraf(), BoxTypeTfdt()},
		{BoxTypeTraf(), BoxTypeTrun()},
	})
	if err != nil {
		return nil, err
	}

	var tfhd *Tfhd
	var tfdt *Tfdt
	var trun *Trun

	segment := &Segment{
		MoofOffset: bi.Offset,
	}
	for _, bip := range bips {
		switch bip.Info.Type {
		case BoxTypeTfhd():
			tfhd = bip.Payload.(*Tfhd)
		case BoxTypeTfdt():
			tfdt = bip.Payload.(*Tfdt)
		case BoxTypeTrun():
			trun = bip.Payload.(*Trun)
		}
	}

	if tfhd == nil {
		return nil, errors.New("tfhd not found")
	}
	segment.TrackID = tfhd.TrackID
	segment.DefaultSampleDuration = tfhd.DefaultSampleDuration

	if tfdt != nil {
		if tfdt.Version == 0 {
			segment.BaseMediaDecodeTime = uint64(tfdt.BaseMediaDecodeTimeV0)
		} else {
			segment.BaseMediaDecodeTime = tfdt.BaseMediaDecodeTimeV1
		}
	}

	if trun != nil {
		segment.SampleCount = trun.SampleCount

		if trun.CheckFlag(0x000100) {
			segment.Duration = 0
			for ei := range trun.Entries {
				segment.Duration += trun.Entries[ei].SampleDuration
			}
		} else {
			segment.Duration = tfhd.DefaultSampleDuration * segment.SampleCount
		}

		if trun.CheckFlag(0x000200) {
			segment.Size = 0
			for ei := range trun.Entries {
				segment.Size += trun.Entries[ei].SampleSize
			}
		} else {
			segment.Size = tfhd.DefaultSampleSize * segment.SampleCount
		}

		var duration uint32
		for ei := range trun.Entries {
			offset := int32(duration) + int32(trun.GetSampleCompositionTimeOffset(ei))
			if ei == 0 || offset < segment.CompositionTimeOffset {
				segment.CompositionTimeOffset = offset
			}
			if trun.CheckFlag(0x000100) {
				duration += trun.Entries[ei].SampleDuration
			} else {
				duration += tfhd.DefaultSampleDuration
			}
		}
	}

	return segment, nil
}

func FindIDRFrames(r io.ReadSeeker, trackInfo *TrackInfo) ([]int, error) {
	if trackInfo.AVC == nil {
		return nil, nil
	}
	lengthSize := uint32(trackInfo.AVC.LengthSize)

	var si int
	idxs := make([]int, 0, 8)
	for _, chunk := range trackInfo.Chunks {
		end := si + int(chunk.SamplesPerChunk)
		dataOffset := chunk.DataOffset
		for ; si < end && si < len(trackInfo.Samples); si++ {
			sample := trackInfo.Samples[si]
			if sample.Size == 0 {
				continue
			}
			for nalOffset := uint32(0); nalOffset+lengthSize+1 <= sample.Size; {
				if _, err := r.Seek(int64(dataOffset+uint64(nalOffset)), io.SeekStart); err != nil {
					return nil, err
				}
				data := make([]byte, lengthSize+1)
				if _, err := io.ReadFull(r, data); err != nil {
					return nil, err
				}
				var length uint32
				for i := 0; i < int(lengthSize); i++ {
					length = (length << 8) + uint32(data[i])
				}
				nalHeader := data[lengthSize]
				nalType := nalHeader & 0x1f
				if nalType == 5 {
					idxs = append(idxs, si)
					break
				}
				nalOffset += lengthSize + length
			}
			dataOffset += uint64(sample.Size)
		}
	}
	return idxs, nil
}

func (samples Samples) GetBitrate(timescale uint32) uint64 {
	var totalSize uint64
	var totalDuration uint64
	for _, sample := range samples {
		totalSize += uint64(sample.Size)
		totalDuration += uint64(sample.TimeDelta)
	}
	if totalDuration == 0 {
		return 0
	}
	return 8 * totalSize * uint64(timescale) / totalDuration
}

func (samples Samples) GetMaxBitrate(timescale uint32, timeDelta uint64) uint64 {
	if timeDelta == 0 {
		return 0
	}
	var maxBitrate uint64
	var size uint64
	var duration uint64
	var begin int
	var end int
	for end < len(samples) {
		for {
			size += uint64(samples[end].Size)
			duration += uint64(samples[end].TimeDelta)
			end++
			if duration >= timeDelta || end == len(samples) {
				break
			}
		}
		bitrate := 8 * size * uint64(timescale) / duration
		if bitrate > maxBitrate {
			maxBitrate = bitrate
		}
		for {
			size -= uint64(samples[begin].Size)
			duration -= uint64(samples[begin].TimeDelta)
			begin++
			if duration < timeDelta {
				break
			}
		}
	}
	return maxBitrate
}

func (segments Segments) GetBitrate(trackID uint32, timescale uint32) uint64 {
	var totalSize uint64
	var totalDuration uint64
	for _, segment := range segments {
		if segment.TrackID == trackID {
			totalSize += uint64(segment.Size)
			totalDuration += uint64(segment.Duration)
		}
	}
	if totalDuration == 0 {
		return 0
	}
	return 8 * totalSize * uint64(timescale) / totalDuration
}

func (segments Segments) GetMaxBitrate(trackID uint32, timescale uint32) uint64 {
	var maxBitrate uint64
	for _, segment := range segments {
		if segment.TrackID == trackID && segment.Duration != 0 {
			bitrate := 8 * uint64(segment.Size) * uint64(timescale) / uint64(segment.Duration)
			if bitrate > maxBitrate {
				maxBitrate = bitrate
			}
		}
	}
	return maxBitrate
}
