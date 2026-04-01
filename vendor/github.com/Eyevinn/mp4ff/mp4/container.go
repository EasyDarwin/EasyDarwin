package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// ContainerBox is interface for ContainerBoxes
type ContainerBox interface {
	Type() string
	Size() uint64
	Encode(w io.Writer) error
	EncodeSW(w bits.SliceWriter) error
	GetChildren() []Box
	Info(w io.Writer, specificBoxLevels, indent, indentStep string) error
}

// GenericContainerBox is a generic container box with no special child pointers
type GenericContainerBox struct {
	name     string
	Children []Box
}

func NewGenericContainerBox(name string) *GenericContainerBox {
	return &GenericContainerBox{name: name}
}

func (b *GenericContainerBox) Type() string {
	return b.name
}

func (b *GenericContainerBox) Size() uint64 {
	return containerSize(b.Children)
}

// Encode - write GenericContainerBox to w
func (b *GenericContainerBox) Encode(w io.Writer) error {
	return EncodeContainer(b, w)
}

// Encode - write minf container to sw
func (b *GenericContainerBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(b, sw)
}

// Info - write box-specific information
func (b *GenericContainerBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(b, w, specificBoxLevels, indent, indentStep)
}

// GetChildren - list of child boxes
func (b *GenericContainerBox) GetChildren() []Box {
	return b.Children
}

// DecodeGenericContainerBox - box-specific decode
func DecodeGenericContainerBox(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	m := NewGenericContainerBox(hdr.Name)
	for _, c := range children {
		m.AddChild(c)
	}
	return m, nil
}

// DecodeGenericContainerBoxSR - box-specific decode
func DecodeGenericContainerBoxSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	m := NewGenericContainerBox(hdr.Name)
	for _, c := range children {
		m.AddChild(c)
	}
	return m, nil
}

// AddChild - Add a child box
func (b *GenericContainerBox) AddChild(child Box) {
	b.Children = append(b.Children, child)
}

func containerSize(children []Box) uint64 {
	var contentSize uint64 = 0
	for _, child := range children {
		contentSize += child.Size()
	}
	return boxHeaderSize + contentSize
}

// DecodeContainerChildren decodes a container box
func DecodeContainerChildren(hdr BoxHeader, startPos, endPos uint64, r io.Reader) ([]Box, error) {
	children := make([]Box, 0, 8)
	pos := startPos
	for {
		child, err := DecodeBox(pos, r)
		if err == io.EOF {
			return children, nil
		}
		if err != nil {
			return children, err
		}
		children = append(children, child)
		pos += child.Size()
		if pos == endPos {
			return children, nil
		} else if pos > endPos {
			msg := ""
			for _, c := range children {
				msg += fmt.Sprintf("%s:%d ", c.Type(), c.Size())
			}
			return nil, fmt.Errorf("non-matching children box sizes, parentSize=%d, %s", endPos-startPos, msg)
		}
	}
}

// DecodeContainerChildren decodes a container box
func DecodeContainerChildrenSR(hdr BoxHeader, startPos, endPos uint64, sr bits.SliceReader) ([]Box, error) {
	children := make([]Box, 0, 8) // Good initial size
	pos := startPos
	initPos := sr.GetPos()
	for {
		if pos > endPos {
			msg := ""
			for _, c := range children {
				msg += fmt.Sprintf("%s:%d ", c.Type(), c.Size())
			}
			return nil, fmt.Errorf("non-matching children box sizes, parentSize=%d, %s", endPos-startPos, msg)
		}
		if pos == endPos {
			break
		}
		child, err := DecodeBoxSR(pos, sr)
		if err != nil {
			return children, err
		}
		children = append(children, child)
		pos += child.Size()
		relPosFromSize := sr.GetPos() - initPos
		if int(pos-startPos) != relPosFromSize {
			return nil, fmt.Errorf("child %s size mismatch in %s: %d - %d", child.Type(), hdr.Name, pos-startPos, relPosFromSize)
		}
	}
	return children, nil
}

// EncodeContainer - marshal container c to w
func EncodeContainer(c ContainerBox, w io.Writer) error {
	err := EncodeHeader(c, w)
	if err != nil {
		return err
	}
	for _, child := range c.GetChildren() {
		err = child.Encode(w)
		if err != nil {
			return err
		}
	}
	return nil
}

// EncodeContainerSW - marshal container c to sw
func EncodeContainerSW(c ContainerBox, sw bits.SliceWriter) error {
	err := EncodeHeaderSW(c, sw)
	if err != nil {
		return err
	}
	for _, child := range c.GetChildren() {
		err = child.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return nil
}

// ContainerInfo - write container-box information
func ContainerInfo(c ContainerBox, w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, c, -1, 0)
	if bd.err != nil {
		return bd.err
	}
	var err error
	for _, child := range c.GetChildren() {
		err := child.Info(w, specificBoxLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	return err
}
