package mp4

import (
	"encoding/binary"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// TrepBox - Track Extension Properties Box (trep)
// Contained in mvex
type TrepBox struct {
	Version  byte
	Flags    uint32
	TrackID  uint32
	Children []Box
}

// AddChild - Add a child box and update SampleCount
func (b *TrepBox) AddChild(child Box) {
	b.Children = append(b.Children, child)
}

// DecodeTrep - box-specific decode
func DecodeTrep(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeTrepSR(hdr, startPos, sr)
}

// DecodeTrepSR - box-specific decode
func DecodeTrepSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	trackID := sr.ReadUint32()
	b := TrepBox{
		Version: byte(versionAndFlags >> 24),
		Flags:   versionAndFlags & flagsMask,
		TrackID: trackID,
	}
	//Note higher startPos below since not simple container
	children, err := DecodeContainerChildrenSR(hdr, startPos+16, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	b.Children = make([]Box, 0, len(children))
	for _, c := range children {
		b.AddChild(c)
	}
	return &b, nil
}

// Type - box-specific type
func (b *TrepBox) Type() string {
	return "trep"
}

// Size - box-specific type
func (b *TrepBox) Size() uint64 {
	return containerSize(b.Children) + 8
}

// Encode - box-specific encode of stsd - not a usual container
func (b *TrepBox) Encode(w io.Writer) error {
	err := EncodeHeader(b, w)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	err = binary.Write(w, binary.BigEndian, versionAndFlags)
	if err != nil {
		return err
	}
	err = binary.Write(w, binary.BigEndian, b.TrackID)
	if err != nil {
		return err
	}
	for _, c := range b.Children {
		err = c.Encode(w)
		if err != nil {
			return err
		}
	}
	return nil
}

// EncodeSW- box-specific encode of stsd - not a usual container
func (b *TrepBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(b.TrackID)
	for _, c := range b.Children {
		err = c.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return nil
}

// Info - write box-specific information
func (b *TrepBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	if bd.err != nil {
		return bd.err
	}
	bd.write(" - trackID: %d", b.TrackID)
	var err error
	for _, c := range b.Children {
		err = c.Info(w, specificBoxLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	return err
}
