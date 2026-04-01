package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MdiaBox - Media Box (mdia)
//
// Contained in : Track Box (trak)
// Contains all information about the media data.
type MdiaBox struct {
	Mdhd     *MdhdBox
	Hdlr     *HdlrBox
	Elng     *ElngBox
	Minf     *MinfBox
	Children []Box
}

// NewMdiaBox - Generate a new empty mdia box
func NewMdiaBox() *MdiaBox {
	return &MdiaBox{}
}

// AddChild - Add a child box
func (m *MdiaBox) AddChild(box Box) {
	switch box.Type() {
	case "mdhd":
		m.Mdhd = box.(*MdhdBox)
	case "hdlr":
		m.Hdlr = box.(*HdlrBox)
	case "elng":
		m.Elng = box.(*ElngBox)
	case "minf":
		m.Minf = box.(*MinfBox)
	}
	m.Children = append(m.Children, box)
}

// DecodeMdia - box-specific decode
func DecodeMdia(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	l, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	m := NewMdiaBox()
	for _, b := range l {
		m.AddChild(b)
	}
	return m, nil
}

// DecodeMdiaSR - box-specific decode
func DecodeMdiaSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	m := NewMdiaBox()
	for _, c := range children {
		m.AddChild(c)
	}
	return m, nil
}

// Type - return box type
func (m *MdiaBox) Type() string {
	return "mdia"
}

// Size - return calculated size
func (m *MdiaBox) Size() uint64 {
	return containerSize(m.Children)
}

// GetChildren - list of child boxes
func (m *MdiaBox) GetChildren() []Box {
	return m.Children
}

// EncodeSW - write mdia container to w
func (m *MdiaBox) Encode(w io.Writer) error {
	return EncodeContainer(m, w)
}

// Encode - write mdia container via sw
func (m *MdiaBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(m, sw)
}

// Info - write box-specific information
func (m *MdiaBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(m, w, specificBoxLevels, indent, indentStep)
}
