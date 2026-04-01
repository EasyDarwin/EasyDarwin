package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// IlstBox - iTunes Metadata Item List Atom (ilst)
// See https://developer.apple.com/library/archive/documentation/QuickTime/QTFF/Metadata/Metadata.html
type IlstBox struct {
	Children []Box
}

// AddChild - Add a child box and update SampleCount
func (b *IlstBox) AddChild(child Box) {
	b.Children = append(b.Children, child)
}

// DecodeIlstSR - box-specific decode
func DecodeIlstSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	b := &IlstBox{}
	for _, c := range children {
		b.AddChild(c)
	}
	return b, nil
}

// DecodeIlst - box-specific decode
func DecodeIlst(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	b := &IlstBox{}
	for _, c := range children {
		b.AddChild(c)
	}
	return b, nil
}

// Type - box-specific type
func (b *IlstBox) Type() string {
	return "ilst"
}

// Size - box-specific type
func (b *IlstBox) Size() uint64 {
	return containerSize(b.Children)
}

// GetChildren - list of child boxes
func (b *IlstBox) GetChildren() []Box {
	return b.Children
}

// Encode - write ilst container to w
func (b *IlstBox) Encode(w io.Writer) error {
	return EncodeContainer(b, w)
}

// Encode - write ilst container to sw
func (b *IlstBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(b, sw)
}

// Info - write box-specific information
func (b *IlstBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(b, w, specificBoxLevels, indent, indentStep)
}
