package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// DinfBox - Data Information Box (dinf - mandatory)
//
// Contained in : Media Information Box (minf) or Meta Box (meta)
type DinfBox struct {
	Dref     *DrefBox
	Children []Box
}

// AddChild - Add a child box
func (d *DinfBox) AddChild(box Box) {

	switch box.Type() {
	case "dref":
		d.Dref = box.(*DrefBox)
	}
	d.Children = append(d.Children, box)
}

// DecodeDinf - box-specific decode
func DecodeDinf(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	l, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	d := &DinfBox{}
	for _, b := range l {
		d.AddChild(b)
	}
	return d, nil
}

// DecodeDinfSR - box-specific decode
func DecodeDinfSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	d := &DinfBox{}
	for _, c := range children {
		d.AddChild(c)
	}
	return d, nil
}

// Type - box-specific type
func (d *DinfBox) Type() string {
	return "dinf"
}

// Size - box-specific size
func (d *DinfBox) Size() uint64 {
	return containerSize(d.Children)
}

// GetChildren - list of child boxes
func (d *DinfBox) GetChildren() []Box {
	return d.Children
}

// Encode - write dinf container to w
func (d *DinfBox) Encode(w io.Writer) error {
	return EncodeContainer(d, w)
}

// EncodeSW - write container using slice writer
func (d *DinfBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(d, sw)
}

// Info - write box info to w
func (d *DinfBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(d, w, specificBoxLevels, indent, indentStep)
}
