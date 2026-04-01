package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MinfBox -  Media Information Box (minf - mandatory)
//
// Contained in : Media Box (mdia)
type MinfBox struct {
	Vmhd     *VmhdBox
	Smhd     *SmhdBox
	Sthd     *SthdBox
	Dinf     *DinfBox
	Stbl     *StblBox
	Children []Box
}

// NewMinfBox - Generate a new empty minf box
func NewMinfBox() *MinfBox {
	return &MinfBox{}
}

// AddChild - Add a child box
func (m *MinfBox) AddChild(child Box) {
	switch box := child.(type) {
	case *VmhdBox:
		m.Vmhd = box
	case *SmhdBox:
		m.Smhd = box
	case *SthdBox:
		m.Sthd = box
	case *DinfBox:
		m.Dinf = box
	case *StblBox:
		m.Stbl = box
	}
	m.Children = append(m.Children, child)
}

// DecodeMinf - box-specific decode
func DecodeMinf(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	m := NewMinfBox()
	for _, c := range children {
		m.AddChild(c)
	}
	return m, nil
}

// DecodeMinfSR - box-specific decode
func DecodeMinfSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	m := NewMinfBox()
	for _, c := range children {
		m.AddChild(c)
	}
	return m, nil
}

// Type - box type
func (m *MinfBox) Type() string {
	return "minf"
}

// Size - calculated size of box
func (m *MinfBox) Size() uint64 {
	return containerSize(m.Children)
}

// GetChildren - list of child boxes
func (m *MinfBox) GetChildren() []Box {
	return m.Children
}

// Encode - write minf container to w
func (m *MinfBox) Encode(w io.Writer) error {
	return EncodeContainer(m, w)
}

// Encode - write minf container to sw
func (m *MinfBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(m, sw)
}

// Info - write box-specific information
func (m *MinfBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(m, w, specificBoxLevels, indent, indentStep)
}
