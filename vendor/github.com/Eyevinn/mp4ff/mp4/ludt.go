package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// LudtBox - Track loudness container
//
// Contained in : Udta Box (udta)
type LudtBox struct {
	Loudness      []*TlouBox
	AlbumLoudness []*AlouBox
	Children      []Box
}

// DecodeLudt - box-specific decode
func DecodeLudt(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	b := &LudtBox{}
	for _, c := range children {
		b.AddChild(c)
	}
	return b, nil
}

// DecodeLudtSR - box-specific decode
func DecodeLudtSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	b := &LudtBox{}
	for _, c := range children {
		b.AddChild(c)
	}
	return b, nil
}

// AddChild - add child box
func (b *LudtBox) AddChild(child Box) {
	switch box := child.(type) {
	case *TlouBox:
		b.Loudness = append(b.Loudness, box)
	case *AlouBox:
		b.AlbumLoudness = append(b.AlbumLoudness, box)
	}
	b.Children = append(b.Children, child)
}

// Size - calculated size of box
func (b *LudtBox) Size() uint64 {
	return containerSize(b.Children)
}

// Type - return box type
func (b *LudtBox) Type() string {
	return "ludt"
}

// Encode - write ludt container to w
func (b *LudtBox) Encode(w io.Writer) error {
	return EncodeContainer(b, w)
}

// Encode - write ludt container to sw
func (b *LudtBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(b, sw)
}

// GetChildren - list of child boxes
func (b *LudtBox) GetChildren() []Box {
	return b.Children
}

// Info - write box-specific information
func (b *LudtBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(b, w, specificBoxLevels, indent, indentStep)
}
