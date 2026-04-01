package mp4

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// StsdBox - Sample Description Box (stsd - manatory)
// See ISO/IEC 14496-12 Section 8.5.2.2
// Full Box + SampleCount
// All Children are sampleEntries
type StsdBox struct {
	Version     byte
	Flags       uint32
	SampleCount uint32
	// AvcX is a pointer to box with name avc1 or avc3
	AvcX *VisualSampleEntryBox
	// HvcX is a pointer to a box with name hvc1 or hev1
	HvcX *VisualSampleEntryBox
	// Av01 is a pointer to a box with name av01
	Av01 *VisualSampleEntryBox
	// Encv is a pointer to a box with name encv
	Encv *VisualSampleEntryBox
	// Mp4a is a pointer to a box with name mp4a
	Mp4a *AudioSampleEntryBox
	// AC3 is a pointer to a box with name ac-3
	AC3 *AudioSampleEntryBox
	// EC3 is a pointer to a box with name ec-3
	EC3 *AudioSampleEntryBox
	// Enca is a pointer to a box with name enca
	Enca *AudioSampleEntryBox
	// Wvtt is a pointer to a WvttBox
	Wvtt *WvttBox
	// Stpp is a pointer to a StppBox
	Stpp *StppBox
	// Evte is a pointer to an EvteBox
	Evte     *EvteBox
	Children []Box
}

// NewStsdBox - Generate a new empty stsd box
func NewStsdBox() *StsdBox {
	return &StsdBox{}
}

// AddChild - Add a child box, set relevant pointer, and update SampleCount
func (s *StsdBox) AddChild(box Box) {
	switch box.Type() {
	case "avc1", "avc3":
		s.AvcX = box.(*VisualSampleEntryBox)
	case "hvc1", "hev1":
		s.HvcX = box.(*VisualSampleEntryBox)
	case "encv":
		s.Encv = box.(*VisualSampleEntryBox)
	case "av01":
		s.Av01 = box.(*VisualSampleEntryBox)
	case "mp4a":
		s.Mp4a = box.(*AudioSampleEntryBox)
	case "ac-3":
		s.AC3 = box.(*AudioSampleEntryBox)
	case "ec-3":
		s.EC3 = box.(*AudioSampleEntryBox)
	case "enca":
		s.Enca = box.(*AudioSampleEntryBox)
	case "wvtt":
		s.Wvtt = box.(*WvttBox)
	case "stpp":
		s.Stpp = box.(*StppBox)
	case "evte":
		s.Evte = box.(*EvteBox)
	}
	s.Children = append(s.Children, box)
	s.SampleCount++
}

// GetSampleDescription - get one of multiple descriptions
func (s *StsdBox) GetSampleDescription(index int) (Box, error) {
	if index >= len(s.Children) {
		return nil, fmt.Errorf("beyond limit of sample descriptors")
	}
	return s.Children[index], nil
}

// DecodeStsd - box-specific decode
func DecodeStsd(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	var versionAndFlags, sampleCount uint32
	err := binary.Read(r, binary.BigEndian, &versionAndFlags)
	if err != nil {
		return nil, err
	}
	err = binary.Read(r, binary.BigEndian, &sampleCount)
	if err != nil {
		return nil, err
	}
	// Note higher startPos below since not simple container
	children, err := DecodeContainerChildren(hdr, startPos+16, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	if len(children) != int(sampleCount) {
		return nil, fmt.Errorf("stsd sample count  mismatch")
	}
	stsd := &StsdBox{
		Version:     byte(versionAndFlags >> 24),
		Flags:       versionAndFlags & flagsMask,
		SampleCount: 0,
	}
	for _, box := range children {
		stsd.AddChild(box)
	}
	if stsd.SampleCount != sampleCount {
		return nil, fmt.Errorf("stsd sample count mismatch")
	}
	return stsd, nil
}

// DecodeStsdSR - box-specific decode
func DecodeStsdSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	sampleCount := sr.ReadUint32()
	// Note higher startPos below since not simple container
	children, err := DecodeContainerChildrenSR(hdr, startPos+16, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	if len(children) != int(sampleCount) {
		return nil, fmt.Errorf("stsd sample count  mismatch")
	}
	stsd := StsdBox{
		Version:     byte(versionAndFlags >> 24),
		Flags:       versionAndFlags & flagsMask,
		SampleCount: 0, // set by  AddChild
		Children:    make([]Box, 0, len(children)),
	}
	for _, box := range children {
		stsd.AddChild(box)
	}
	if stsd.SampleCount != sampleCount {
		return nil, fmt.Errorf("stsd sample count mismatch")
	}
	return &stsd, nil
}

// Type - box-specific type
func (s *StsdBox) Type() string {
	return "stsd"
}

// Size - box-specific type
func (s *StsdBox) Size() uint64 {
	return containerSize(s.Children) + 8
}

// Encode - box-specific encode of stsd - not a usual container
func (s *StsdBox) Encode(w io.Writer) error {
	err := EncodeHeader(s, w)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(s.Version) << 24) + s.Flags
	err = binary.Write(w, binary.BigEndian, versionAndFlags)
	if err != nil {
		return err
	}
	err = binary.Write(w, binary.BigEndian, s.SampleCount)
	if err != nil {
		return err
	}
	for _, b := range s.Children {
		err = b.Encode(w)
		if err != nil {
			return err
		}
	}
	return nil
}

// EncodeSW - box-specific encode of stsd - not a usual container
func (s *StsdBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(s, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(s.Version) << 24) + s.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(s.SampleCount)
	for _, c := range s.Children {
		err = c.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return nil
}

// Info - write box-specific information
func (s *StsdBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, s, int(s.Version), s.Flags)
	if bd.err != nil {
		return bd.err
	}
	var err error
	for _, c := range s.Children {
		err = c.Info(w, specificBoxLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	return err
}

// GetBtrt returns the first BtrtBox found in StsdBox children.
func (s *StsdBox) GetBtrt() *BtrtBox {
	for _, c := range s.Children {
		switch child := c.(type) {
		case *VisualSampleEntryBox:
			return child.Btrt
		case *AudioSampleEntryBox:
			return child.Btrt
		case *WvttBox:
			return child.Btrt
		case *StppBox:
			return child.Btrt
		case *EvteBox:
			return child.Btrt
		}
	}
	return nil
}
