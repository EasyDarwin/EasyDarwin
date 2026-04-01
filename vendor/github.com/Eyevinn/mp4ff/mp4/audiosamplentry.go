package mp4

import (
	"bytes"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// AudioSampleEntryBox according to ISO/IEC 14496-12
type AudioSampleEntryBox struct {
	name               string
	DataReferenceIndex uint16
	ChannelCount       uint16
	SampleSize         uint16
	SampleRate         uint16 // Integer part
	Esds               *EsdsBox
	Dac3               *Dac3Box
	Dec3               *Dec3Box
	Btrt               *BtrtBox
	Sinf               *SinfBox
	Children           []Box
}

// NewAudioSampleEntryBox - Create new empty mp4a box
func NewAudioSampleEntryBox(name string) *AudioSampleEntryBox {
	return &AudioSampleEntryBox{name: name, DataReferenceIndex: 1}
}

func makeFixed32Uint(nr uint16) uint32 {
	return uint32(nr) << 16
}

func makeUint16FromFixed32(nr uint32) uint16 {
	return uint16(nr >> 16)
}

// CreateAudioSampleEntryBox - Create new AudioSampleEntry such as mp4
func CreateAudioSampleEntryBox(name string, nrChannels, sampleSize, sampleRate uint16, child Box) *AudioSampleEntryBox {
	a := &AudioSampleEntryBox{
		name:               name,
		DataReferenceIndex: 1,
		ChannelCount:       nrChannels,
		SampleSize:         sampleSize,
		SampleRate:         sampleRate,
		Children:           nil,
	}
	if child != nil {
		a.AddChild(child)
	}
	return a
}

// AddChild - add a child box (avcC normally, but clap and pasp could be part of visual entry)
func (a *AudioSampleEntryBox) AddChild(child Box) {
	switch child.Type() {
	case "esds":
		a.Esds = child.(*EsdsBox)
	case "dac3":
		a.Dac3 = child.(*Dac3Box)
	case "dec3":
		a.Dec3 = child.(*Dec3Box)
	case "btrt":
		a.Btrt = child.(*BtrtBox)
	case "sinf":
		a.Sinf = child.(*SinfBox)
	}

	a.Children = append(a.Children, child)
}

const nrAudioSampleBytesBeforeChildren = 36

// DecodeAudioSampleEntry - decode mp4a... box
func DecodeAudioSampleEntry(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	a := NewAudioSampleEntryBox(hdr.Name)

	// 14496-12 8.5.2.2 Sample entry (8 bytes)
	sr.SkipBytes(6) // Skip 6 reserved bytes
	a.DataReferenceIndex = sr.ReadUint16()

	// 14496-12 12.2.3.2 Audio Sample entry (20 bytes)

	sr.SkipBytes(8) //  reserved == 0
	a.ChannelCount = sr.ReadUint16()
	a.SampleSize = sr.ReadUint16()
	sr.SkipBytes(4) // Predefined + reserved
	a.SampleRate = makeUint16FromFixed32(sr.ReadUint32())

	remaining := sr.RemainingBytes()
	restReader := bytes.NewReader(remaining)

	pos := startPos + nrAudioSampleBytesBeforeChildren // Size of all previous data
	for {
		box, err := DecodeBox(pos, restReader)
		if err == io.EOF {
			break
		} else if err != nil {
			return nil, err
		}
		if box != nil {
			a.AddChild(box)
			pos += box.Size()
		}
		if pos == startPos+hdr.Size {
			break
		} else if pos > startPos+hdr.Size {
			return nil, fmt.Errorf("bad size when decoding %s", hdr.Name)
		}
	}
	return a, nil
}

// DecodeAudioSampleEntry - decode mp4a... box
func DecodeAudioSampleEntrySR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	a := NewAudioSampleEntryBox(hdr.Name)

	// 14496-12 8.5.2.2 Sample entry (8 bytes)
	sr.SkipBytes(6) // Skip 6 reserved bytes
	a.DataReferenceIndex = sr.ReadUint16()

	// 14496-12 12.2.3.2 Audio Sample entry (20 bytes)

	sr.SkipBytes(8) //  reserved == 0
	a.ChannelCount = sr.ReadUint16()
	a.SampleSize = sr.ReadUint16()
	sr.SkipBytes(4) // Predefined + reserved
	a.SampleRate = makeUint16FromFixed32(sr.ReadUint32())

	pos := startPos + nrAudioSampleBytesBeforeChildren // Size of all previous data
	lastPos := startPos + hdr.Size
	for {
		if pos >= lastPos {
			break
		}
		box, err := DecodeBoxSR(pos, sr)
		if err != nil {
			return nil, err
		}
		if box != nil {
			a.AddChild(box)
			pos += box.Size()
		}
	}
	return a, sr.AccError()
}

// Type - return box type
func (a *AudioSampleEntryBox) Type() string {
	return a.name
}

// SetType sets the type (name) of the box
func (a *AudioSampleEntryBox) SetType(name string) {
	a.name = name
}

// Size - return calculated size
func (a *AudioSampleEntryBox) Size() uint64 {
	totalSize := uint64(nrAudioSampleBytesBeforeChildren)
	for _, child := range a.Children {
		totalSize += child.Size()
	}
	return totalSize
}

// Encode - write box to w
func (a *AudioSampleEntryBox) Encode(w io.Writer) error {
	err := EncodeHeader(a, w)
	if err != nil {
		return err
	}
	buf := makebuf(a)
	sw := bits.NewFixedSliceWriterFromSlice(buf)
	sw.WriteZeroBytes(6)
	sw.WriteUint16(a.DataReferenceIndex)
	sw.WriteZeroBytes(8) // pre_defined and reserved
	sw.WriteUint16(a.ChannelCount)
	sw.WriteUint16(a.SampleSize)
	sw.WriteZeroBytes(4)                          // Pre-defined and reserved
	sw.WriteUint32(makeFixed32Uint(a.SampleRate)) // nrAudioSampleBytesBeforeChildren bytes this far

	_, err = w.Write(buf[:sw.Offset()]) // Only write written bytes
	if err != nil {
		return err
	}

	// Next output child boxes in order
	for _, child := range a.Children {
		err = child.Encode(w)
		if err != nil {
			return err
		}
	}
	return err
}

// Encode - write box to sw
func (a *AudioSampleEntryBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(a, sw)
	if err != nil {
		return err
	}
	sw.WriteZeroBytes(6)
	sw.WriteUint16(a.DataReferenceIndex)
	sw.WriteZeroBytes(8) // pre_defined and reserved
	sw.WriteUint16(a.ChannelCount)
	sw.WriteUint16(a.SampleSize)
	sw.WriteZeroBytes(4)                          // Pre-defined and reserved
	sw.WriteUint32(makeFixed32Uint(a.SampleRate)) // nrAudioSampleBytesBeforeChildren bytes this far

	// Next output child boxes in order
	for _, child := range a.Children {
		err = child.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return err
}

// Info - write box info to w
func (a *AudioSampleEntryBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, a, -1, 0)
	if bd.err != nil {
		return bd.err
	}
	var err error
	for _, child := range a.Children {
		err = child.Info(w, specificBoxLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	return nil
}

// RemoveEncryption - remove sinf box and set type to unencrypted type
func (a *AudioSampleEntryBox) RemoveEncryption() (*SinfBox, error) {
	if a.name != "enca" {
		return nil, fmt.Errorf("is not encrypted: %s", a.name)
	}
	sinf := a.Sinf
	if sinf == nil {
		return nil, fmt.Errorf("does not have sinf box")
	}
	for i := range a.Children {
		if a.Children[i].Type() == "sinf" {
			a.Children = append(a.Children[:i], a.Children[i+1:]...)
			a.Sinf = nil
			break
		}
	}
	a.name = sinf.Frma.DataFormat
	return sinf, nil
}
