package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// EdtsBox - Edit Box (edts - optional)
//
// Contained in: Track Box ("trak")
//
// The edit box maps the presentation timeline to the media-time line
type EdtsBox struct {
	Elst     []*ElstBox
	Children []Box
}

// DecodeEdts - box-specific decode
func DecodeEdts(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	l, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	e := &EdtsBox{}
	e.Children = l
	for _, b := range l {
		switch b.Type() {
		case "elst":
			e.Elst = append(e.Elst, b.(*ElstBox))
		default:
			return nil, fmt.Errorf("Box of type %s in edts", b.Type())
		}
	}
	return e, nil
}

// DecodeEdtsSR - box-specific decode
func DecodeEdtsSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	e := &EdtsBox{}
	e.Children = children
	for _, b := range children {
		switch b.Type() {
		case "elst":
			e.Elst = append(e.Elst, b.(*ElstBox))
		default:
			return nil, fmt.Errorf("Box of type %s in edts", b.Type())
		}
	}
	return e, sr.AccError()
}

// AddChild - Add a child box and update EntryCount
func (e *EdtsBox) AddChild(child Box) {
	e.Children = append(e.Children, child)
}

// Type - box type
func (b *EdtsBox) Type() string {
	return "edts"
}

// Size - calculated size of box
func (b *EdtsBox) Size() uint64 {
	return containerSize(b.Children)
}

// GetChildren - list of child boxes
func (b *EdtsBox) GetChildren() []Box {
	return b.Children
}

// Encode - write edts container to w
func (b *EdtsBox) Encode(w io.Writer) error {
	return EncodeContainer(b, w)
}

// EncodeSW - write edts container to sw
func (b *EdtsBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(b, sw)
}

// Info - write box-specific information
func (b *EdtsBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(b, w, specificBoxLevels, indent, indentStep)
}
