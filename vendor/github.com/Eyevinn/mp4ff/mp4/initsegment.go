package mp4

import (
	"bytes"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/aac"
	"github.com/Eyevinn/mp4ff/avc"
	"github.com/Eyevinn/mp4ff/bits"
	"github.com/Eyevinn/mp4ff/hevc"
)

// InitSegment - MP4/CMAF init segment
type InitSegment struct {
	MediaType string
	Ftyp      *FtypBox
	Moov      *MoovBox
	Children  []Box // All top-level boxes in order
}

// NewMP4Init - Create MP4Init
func NewMP4Init() *InitSegment {
	return &InitSegment{
		Children: []Box{},
	}
}

// AddChild - Add a top-level box to InitSegment
func (s *InitSegment) AddChild(b Box) {
	switch b.Type() {
	case "ftyp":
		s.Ftyp = b.(*FtypBox)
	case "moov":
		s.Moov = b.(*MoovBox)
	}
	s.Children = append(s.Children, b)
}

// Size - size of init segment
func (s *InitSegment) Size() uint64 {
	var size uint64 = 0
	for _, box := range s.Children {
		size += box.Size()
	}
	return size
}

// Encode - encode an initsegment to a Writer
func (s *InitSegment) Encode(w io.Writer) error {
	for _, b := range s.Children {
		err := b.Encode(w)
		if err != nil {
			return err
		}
	}
	return nil
}

// EncodeSW - encode an initsegment to a SliceWriter
func (s *InitSegment) EncodeSW(sw bits.SliceWriter) error {
	for _, b := range s.Children {
		err := b.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return nil
}

// Info - write box tree with indent for each level
func (s *InitSegment) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	for _, box := range s.Children {
		err := box.Info(w, specificBoxLevels, indent, indentStep)
		if err != nil {
			return err
		}
	}
	return nil
}

// CreateEmptyInit - create an init segment for fragmented files
func CreateEmptyInit() *InitSegment {
	initSeg := NewMP4Init()
	initSeg.AddChild(CreateFtyp())
	moov := NewMoovBox()
	initSeg.AddChild(moov)
	mvhd := CreateMvhd()
	moov.AddChild(mvhd)
	mvex := NewMvexBox()
	moov.AddChild(mvex)
	return initSeg
}

// AddEmptyTrack - add trak + trex box with appropriate trackID value
func (s *InitSegment) AddEmptyTrack(timeScale uint32, mediaType, language string) {
	moov := s.Moov
	trackID := uint32(len(moov.Traks) + 1)
	moov.Mvhd.NextTrackID = trackID + 1
	newTrak := CreateEmptyTrak(trackID, timeScale, mediaType, language)
	moov.AddChild(newTrak)
	moov.Mvex.AddChild(CreateTrex(trackID))
}

// CreateEmptyTrak - create a full trak-tree for an empty (fragmented) track with no samples or stsd content
func CreateEmptyTrak(trackID, timeScale uint32, mediaType, language string) *TrakBox {
	/*  Built tree like
	- trak
	+ tkhd (trakID, flags, width, height)
	+ mdia
	  - mdhd (track Timescale, language (3letters))
	  - hdlr (hdlr showing mediaType)
	  - elng (only if language is not 3 letters)
	  - minf
		+ vmhd/smhd etc (media header box)
		+ dinf
		  - dref
			+ url
		+ stbl
		  - stsd
			+ empty on purpose
		  - stts
		  - stsc
		  - stsz
		  - stco
	*/
	trak := &TrakBox{}
	tkhd := CreateTkhd()
	tkhd.TrackID = trackID
	if mediaType == "audio" {
		tkhd.Volume = 0x0100 // Fixed 16 value 1.0
	}
	trak.AddChild(tkhd)

	mdia := &MdiaBox{}
	trak.AddChild(mdia)
	mdhd := &MdhdBox{}
	mdhd.Timescale = timeScale
	mdia.AddChild(mdhd)
	hdlr, err := CreateHdlr(mediaType)
	if err != nil {
		panic(fmt.Sprintf("mediaType %s not supported", mediaType))
	}
	mdia.AddChild(hdlr)
	if len(language) == 3 {
		mdhd.SetLanguage(language)
	} else {
		mdhd.SetLanguage("und")
		elng := CreateElng(language)
		mdia.AddChild(elng)
	}
	minf := NewMinfBox()
	mdia.AddChild(minf)
	switch mediaType {
	case "video":
		minf.AddChild(CreateVmhd())
	case "audio":
		minf.AddChild(CreateSmhd())
	case "subtitle", "subtitles", "stpp":
		minf.AddChild(&SthdBox{})
	case "text", "wvtt":
		minf.AddChild(&NmhdBox{})
	default:
		minf.AddChild(&NmhdBox{})
	}
	dinf := &DinfBox{}
	dinf.AddChild(CreateDref())
	minf.AddChild(dinf)
	stbl := NewStblBox()
	minf.AddChild(stbl)
	stsd := NewStsdBox()
	stbl.AddChild(stsd)
	stbl.AddChild(&SttsBox{})
	stbl.AddChild(&StscBox{})
	stbl.AddChild(&StszBox{})
	stbl.AddChild(&StcoBox{})
	return trak
}

// SetAVCDescriptor - Set AVC SampleDescriptor based on SPS and PPS
func (t *TrakBox) SetAVCDescriptor(sampleDescriptorType string, spsNALUs, ppsNALUs [][]byte, includePS bool) error {
	if sampleDescriptorType != "avc1" && sampleDescriptorType != "avc3" {
		return fmt.Errorf("sampleDescriptorType %s not allowed", sampleDescriptorType)
	}
	if sampleDescriptorType == "avc1" && !includePS {
		return fmt.Errorf("cannot make avc1 descriptor without parameter sets")
	}
	avcSPS, err := avc.ParseSPSNALUnit(spsNALUs[0], false)
	if err != nil {
		return fmt.Errorf("could not parse SPS NALU: %w", err)
	}
	t.Tkhd.Width = Fixed32(avcSPS.Width << 16)   // This is display width
	t.Tkhd.Height = Fixed32(avcSPS.Height << 16) // This is display height
	stsd := t.Mdia.Minf.Stbl.Stsd

	avcC, err := CreateAvcC(spsNALUs, ppsNALUs, includePS)
	if err != nil {
		return err
	}
	width, height := uint16(avcSPS.Width), uint16(avcSPS.Height)
	avcx := CreateVisualSampleEntryBox(sampleDescriptorType, width, height, avcC)
	stsd.AddChild(avcx)
	return nil
}

// SetHEVCDescriptor sets HEVC SampleDescriptor based on descriptorType, VPS, SPS, PPS and SEI.
func (t *TrakBox) SetHEVCDescriptor(sampleDescriptorType string, vpsNALUs, spsNALUs, ppsNALUs, seiNALUs [][]byte, includePS bool) error {
	if sampleDescriptorType != "hvc1" && sampleDescriptorType != "hev1" {
		return fmt.Errorf("sampleDescriptorType %s not allowed", sampleDescriptorType)
	}
	hevcSPS, err := hevc.ParseSPSNALUnit(spsNALUs[0])
	if err != nil {
		return fmt.Errorf("could not parse SPS NALU: %w", err)
	}
	width, height := hevcSPS.ImageSize()
	t.Tkhd.Width = Fixed32(width << 16)   // This is display width
	t.Tkhd.Height = Fixed32(height << 16) // This is display height
	stsd := t.Mdia.Minf.Stbl.Stsd

	// hvc1 must include parameter sets (PS) and they must be complete
	// hev1 may include PS and they may not be complete
	// here we choose to include PS in both cases
	completePS := sampleDescriptorType == "hvc1"
	if sampleDescriptorType == "hvc1" && !includePS {
		return fmt.Errorf("must include parameter sets for hvc1")
	}
	hvcC, err := CreateHvcC(vpsNALUs, spsNALUs, ppsNALUs, completePS, completePS, completePS, includePS)
	if len(seiNALUs) > 0 {
		hvcC.AddNaluArrays([]hevc.NaluArray{hevc.NewNaluArray(completePS, hevc.NALU_SEI_PREFIX, seiNALUs)})
	}
	if err != nil {
		return err
	}
	avcx := CreateVisualSampleEntryBox(sampleDescriptorType, uint16(width), uint16(height), hvcC)
	stsd.AddChild(avcx)
	return nil
}

// GetMediaType - should return video or audio (at present)
func (s *InitSegment) GetMediaType() string {
	switch s.Moov.Trak.Mdia.Hdlr.HandlerType {
	case "soun":
		return "audio"
	case "vide":
		return "video"
	default:
		return "unknown"
	}
}

// TweakSingleTrakLive assures that there is only one track and removes any mehd box.
func (s *InitSegment) TweakSingleTrakLive() error {
	if len(s.Moov.Traks) != 1 {
		return fmt.Errorf("only one track allowed for live")
	}
	mvex := s.Moov.Mvex
	if mvex == nil {
		return fmt.Errorf("no mvex box found")

	}
	mehd := mvex.Mehd
	if mehd != nil {
		for i, c := range mvex.Children {
			if c == mehd {
				mvex.Children = append(mvex.Children[:i], mvex.Children[i+1:]...)
				mvex.Mehd = nil
				break
			}
		}
	}
	return nil
}

// SetAACDescriptor - Modify a TrakBox by adding AAC SampleDescriptor
// objType is one of AAClc, HEAACv1, HEAACv2
// For HEAAC, the samplingFrequency is the base frequency (normally 24000)
func (t *TrakBox) SetAACDescriptor(objType byte, samplingFrequency int) error {
	stsd := t.Mdia.Minf.Stbl.Stsd
	asc := &aac.AudioSpecificConfig{
		ObjectType:           objType,
		ChannelConfiguration: 2,
		SamplingFrequency:    samplingFrequency,
		ExtensionFrequency:   0,
		SBRPresentFlag:       false,
		PSPresentFlag:        false,
	}
	switch objType {
	case aac.HEAACv1:
		asc.ExtensionFrequency = 2 * samplingFrequency
		asc.SBRPresentFlag = true
	case aac.HEAACv2:
		asc.ExtensionFrequency = 2 * samplingFrequency
		asc.SBRPresentFlag = true
		asc.ChannelConfiguration = 1
		asc.PSPresentFlag = true
	}

	buf := &bytes.Buffer{}
	err := asc.Encode(buf)
	if err != nil {
		return err
	}
	ascBytes := buf.Bytes()
	esds := CreateEsdsBox(ascBytes)
	mp4a := CreateAudioSampleEntryBox("mp4a",
		uint16(asc.ChannelConfiguration),
		16, uint16(samplingFrequency), esds)
	stsd.AddChild(mp4a)
	return nil
}

// SetAC3Descriptor  - Modify a TrakBox by adding AC-3 SampleDescriptor
func (t *TrakBox) SetAC3Descriptor(dac3 *Dac3Box) error {
	stsd := t.Mdia.Minf.Stbl.Stsd
	nrChannels, _ := dac3.ChannelInfo()
	samplingFrequency := AC3SampleRates[dac3.FSCod]

	ac3 := CreateAudioSampleEntryBox("ac-3",
		uint16(nrChannels), //  Not to be used, but we set it anyway
		16, uint16(samplingFrequency), dac3)
	stsd.AddChild(ac3)
	return nil
}

// SetEC3Descriptor  - Modify a TrakBox by adding EC-3 SampleDescriptor
func (t *TrakBox) SetEC3Descriptor(dec3 *Dec3Box) error {
	stsd := t.Mdia.Minf.Stbl.Stsd
	nrChannels, _ := dec3.ChannelInfo()
	fscod := dec3.EC3Subs[0].FSCod
	samplingFrequency := AC3SampleRates[fscod]

	ec3 := CreateAudioSampleEntryBox("ec-3",
		uint16(nrChannels), //  Not to be used, but we set it anyway
		16, uint16(samplingFrequency), dec3)
	stsd.AddChild(ec3)
	return nil
}

// SetWvttDescriptor - Set wvtt descriptor with a vttC box. config should start with WEBVTT or be empty.
func (t *TrakBox) SetWvttDescriptor(config string) error {
	if config == "" {
		config = "WEBVTT"
	}
	vttC := VttCBox{Config: config}
	wvtt := WvttBox{}
	wvtt.AddChild(&vttC)
	t.Mdia.Minf.Stbl.Stsd.AddChild(&wvtt)
	return nil
}

// SetStppDescriptor - add stpp box with utf8-lists namespace, schemaLocation and auxiliaryMimeType
// The utf8-lists have space-separated items, but no zero-termination
func (t *TrakBox) SetStppDescriptor(namespace, schemaLocation, auxiliaryMimeTypes string) error {
	if namespace == "" {
		namespace = "http://www.w3.org/ns/ttml"
	}
	stpp := NewStppBox(namespace, schemaLocation, auxiliaryMimeTypes)
	t.Mdia.Minf.Stbl.Stsd.AddChild(stpp)
	return nil
}
