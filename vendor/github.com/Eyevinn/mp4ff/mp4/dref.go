package mp4

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// DrefBox - Data Reference Box (dref - mandatory)
//
// Contained id: Data Information Box (dinf)
//
// Defines the location of the media data. If the data for the track is located in the same file
// it contains nothing useful.
type DrefBox struct {
	Version    byte
	Flags      uint32
	EntryCount uint32
	Children   []Box
}

// CreateDref - Create an DataReferenceBox for selfcontained content
func CreateDref() *DrefBox {
	url := CreateURLBox()
	dref := &DrefBox{}
	dref.AddChild(url)
	return dref
}

// AddChild - Add a child box and update EntryCount
func (d *DrefBox) AddChild(box Box) {
	d.Children = append(d.Children, box)
	d.EntryCount++
}

// DecodeDref - box-specific decode
func DecodeDref(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	var versionAndFlags, entryCount uint32
	err := binary.Read(r, binary.BigEndian, &versionAndFlags)
	if err != nil {
		return nil, err
	}
	err = binary.Read(r, binary.BigEndian, &entryCount)
	if err != nil {
		return nil, err
	}

	// Note higher startPos for children since not simple container.
	children, err := DecodeContainerChildren(hdr, startPos+16, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}

	dref := &DrefBox{
		Version: byte(versionAndFlags >> 24),
		Flags:   versionAndFlags & flagsMask,
	}

	for _, c := range children {
		dref.AddChild(c)
	}
	if entryCount != dref.EntryCount {
		return nil, fmt.Errorf("inconsistent entry count in Dref")
	}
	return dref, nil
}

// DecodeDrefSR - box-specific decode
func DecodeDrefSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	entryCount := sr.ReadUint32()

	// Note higher startPos for children since not simple container.
	children, err := DecodeContainerChildrenSR(hdr, startPos+16, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}

	dref := &DrefBox{
		Version:    byte(versionAndFlags >> 24),
		Flags:      versionAndFlags & flagsMask,
		EntryCount: 0, // incremented by AddChild
	}

	for _, c := range children {
		dref.AddChild(c)
	}
	if entryCount != dref.EntryCount {
		return nil, fmt.Errorf("inconsistent entry count in Dref")
	}
	return dref, sr.AccError()
}

// Type - box type
func (d *DrefBox) Type() string {
	return "dref"
}

// Size - calculated size of box
func (d *DrefBox) Size() uint64 {
	return containerSize(d.Children) + 8
}

// Encode - write dref box to w including children
func (d *DrefBox) Encode(w io.Writer) error {
	err := EncodeHeader(d, w)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(d.Version) << 24) + d.Flags
	err = binary.Write(w, binary.BigEndian, versionAndFlags)
	if err != nil {
		return err
	}
	err = binary.Write(w, binary.BigEndian, uint32(d.EntryCount))
	if err != nil {
		return err
	}
	for _, b := range d.Children {
		err = b.Encode(w)
		if err != nil {
			return err
		}
	}
	return err
}

// EncodeSW - write dref box to w including children
func (d *DrefBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(d, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(d.Version) << 24) + d.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(uint32(d.EntryCount))
	for _, b := range d.Children {
		err = b.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return err
}

// Info - write box-specific information
func (d *DrefBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, d, int(d.Version), d.Flags)
	if bd.err != nil {
		return bd.err
	}
	var err error
	for _, c := range d.Children {
		err = c.Info(w, specificBoxLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	return err
}
