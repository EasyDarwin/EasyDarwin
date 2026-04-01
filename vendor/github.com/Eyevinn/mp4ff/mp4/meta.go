package mp4

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MetaBox is MPEG-4 Meta box or QuickTime meta Atom (without version and flags)

// MPEG box defined in ISO/IEC 14496-12 Ed. 6 2020 Section 8.11
//
// Note. QuickTime meta atom has no version and flags field.
// https://developer.apple.com/library/archive/documentation/QuickTime/QTFF/Metadata/Metadata.html#//apple_ref/doc/uid/TP40000939-CH1-SW10
type MetaBox struct {
	Version     byte
	Flags       uint32
	Hdlr        *HdlrBox
	Children    []Box
	isQuickTime bool // Has no version and flags
}

// IsQuickTime returns true if box is QuickTime compatible (has no version and flags)
func (m *MetaBox) IsQuickTime() bool {
	return m.isQuickTime
}

// CreateMetaBox creates a new MetaBox
func CreateMetaBox(version byte, hdlr *HdlrBox) *MetaBox {
	b := &MetaBox{
		Version: version,
		Flags:   0,
	}
	b.AddChild(hdlr)
	return b
}

// AddChild adds a child box
func (b *MetaBox) AddChild(child Box) {
	switch box := child.(type) {
	case *HdlrBox:
		b.Hdlr = box
	}
	b.Children = append(b.Children, child)
}

// DecodeMeta decodes a MetaBox in either MPEG or QuickTime version
func DecodeMeta(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeMetaSR(hdr, startPos, sr)
}

// DecodeMetaSR decodes a MetaBox in either MPEG or QuickTime version
func DecodeMetaSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	b := MetaBox{}
	lookAheadData := make([]byte, 4)
	err := sr.LookAhead(4, lookAheadData)
	if err != nil {
		return nil, fmt.Errorf("could not look ahead in Meta box")
	}
	var offset uint64 = 8
	if bytes.Equal(lookAheadData, []byte("hdlr")) {
		b.isQuickTime = true
	} else {
		//Note larger offset below since not simple container
		offset += 4
		versionAndFlags := sr.ReadUint32()
		b.Version = byte(versionAndFlags >> 24)
		b.Flags = versionAndFlags & flagsMask
	}

	children, err := DecodeContainerChildrenSR(hdr, startPos+offset, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}

	for _, child := range children {
		b.AddChild(child)
	}
	return &b, nil
}

// Type returns box type
func (b *MetaBox) Type() string {
	return "meta"
}

// Size calculates size of box
func (b *MetaBox) Size() uint64 {
	size := 4 + containerSize(b.Children)
	if b.IsQuickTime() {
		size -= 4
	}
	return size
}

// GetChildren lists child boxes
func (b *MetaBox) GetChildren() []Box {
	return b.Children
}

// Encode writes minf container to w
func (b *MetaBox) Encode(w io.Writer) error {
	err := EncodeHeader(b, w)
	if err != nil {
		return err
	}
	if !b.isQuickTime {
		versionAndFlags := (uint32(b.Version) << 24) + b.Flags
		err = binary.Write(w, binary.BigEndian, versionAndFlags)
		if err != nil {
			return err
		}
	}
	for _, b := range b.Children {
		err = b.Encode(w)
		if err != nil {
			return err
		}
	}
	return nil
}

// Encode writes minf container to sw
func (b *MetaBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	for _, c := range b.Children {
		err = c.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return nil
}

// Info writes box-specific info
func (b *MetaBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	if bd.err != nil {
		return bd.err
	}
	if b.isQuickTime {
		bd.write(" - is QuickTime meta atom")
	}
	var err error
	for _, c := range b.Children {
		err = c.Info(w, specificBoxLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	return err
}
