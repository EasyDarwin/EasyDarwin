package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// TrefBox -  // TrackReferenceBox - ISO/IEC 14496-12 Ed. 9 Sec. 8.3
type TrefBox struct {
	Children []Box
}

// AddChild - Add a child box
func (b *TrefBox) AddChild(box Box) {
	b.Children = append(b.Children, box)
}

// DecodeTref - box-specific decode
func DecodeTref(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	b := TrefBox{}
	for _, c := range children {
		b.AddChild(c)
	}
	return &b, nil
}

// DecodeTrefSR - box-specific decode
func DecodeTrefSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	b := TrefBox{}
	for _, c := range children {
		b.AddChild(c)
	}
	return &b, nil
}

// Type - box type
func (b *TrefBox) Type() string {
	return "tref"
}

// Size - calculated size of box
func (b *TrefBox) Size() uint64 {
	return containerSize(b.Children)
}

// GetChildren - list of child boxes
func (b *TrefBox) GetChildren() []Box {
	return b.Children
}

// Encode - write minf container to w
func (b *TrefBox) Encode(w io.Writer) error {
	return EncodeContainer(b, w)
}

// Encode - write minf container to sw
func (b *TrefBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(b, sw)
}

// Info - write box-specific information
func (b *TrefBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(b, w, specificBoxLevels, indent, indentStep)
}

// TrefTypeBox - TrackReferenceTypeBox - ISO/IEC 14496-12 Ed. 9 Sec. 8.3
// Name can be one of hint, cdsc, font, hind, vdep, vplx, subt (ISO/IEC 14496-12)
// dpnd, ipir, mpod, sync (ISO/IEC 14496-14)
type TrefTypeBox struct {
	Name     string
	TrackIDs []uint32
}

// DecodeTrefType - box-specific decode
func DecodeTrefType(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeTrefTypeSR(hdr, startPos, sr)
}

// DecodeTrefTypeSR - box-specific decode
func DecodeTrefTypeSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	nrIds := hdr.payloadLen() / 4
	b := TrefTypeBox{
		Name:     hdr.Name,
		TrackIDs: make([]uint32, nrIds),
	}
	for i := 0; i < nrIds; i++ {
		b.TrackIDs[i] = sr.ReadUint32()
	}
	return &b, sr.AccError()
}

// Type - box type
func (b *TrefTypeBox) Type() string {
	return b.Name
}

// Size - calculated size of box
func (b *TrefTypeBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.TrackIDs)*4)
}

// Encode - write box to w
func (t *TrefTypeBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(t.Size()))
	err := t.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// Encode - write box to sw
func (b *TrefTypeBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	for _, trackID := range b.TrackIDs {
		sw.WriteUint32(trackID)
	}
	return sw.AccError()
}

// Info - write box-specific information
func (b *TrefTypeBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	msg := " - trackIDs: "
	for _, trackID := range b.TrackIDs {
		msg += fmt.Sprintf(" %d", trackID)
	}
	bd.write(msg)
	return bd.err
}
