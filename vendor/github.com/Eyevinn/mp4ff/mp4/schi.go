package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// SchiBox -  Schema Information Box
type SchiBox struct {
	Tenc     *TencBox
	Children []Box
}

// AddChild - Add a child box
func (b *SchiBox) AddChild(child Box) {
	switch box := child.(type) {
	case *TencBox:
		b.Tenc = box
	}
	b.Children = append(b.Children, child)
}

// DecodeSchi - box-specific decode
func DecodeSchi(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	b := &SchiBox{}
	for _, child := range children {
		b.AddChild(child)
	}
	return b, nil
}

// DecodeSchiSR - box-specific decode
func DecodeSchiSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	b := &SchiBox{}
	for _, child := range children {
		b.AddChild(child)
	}
	return b, nil
}

// Type - box type
func (b *SchiBox) Type() string {
	return "schi"
}

// Size - calculated size of box
func (b *SchiBox) Size() uint64 {
	return containerSize(b.Children)
}

// GetChildren - list of child boxes
func (b *SchiBox) GetChildren() []Box {
	return b.Children
}

// Encode - write minf container to w
func (b *SchiBox) Encode(w io.Writer) error {
	return EncodeContainer(b, w)
}

// Encode - write minf container to sw
func (b *SchiBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(b, sw)
}

// Info - write box-specific information
func (b *SchiBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(b, w, specificBoxLevels, indent, indentStep)
}
