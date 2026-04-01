package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// Boxes needed for wvtt according to ISO/IEC 14496-30

////////////////////////////// wvtt //////////////////////////////

// WvttBox - WVTTSampleEntry (wvtt)
// Extends PlainTextSampleEntry which extends SampleEntry
type WvttBox struct {
	VttC               *VttCBox
	Vlab               *VlabBox
	Btrt               *BtrtBox
	Children           []Box
	DataReferenceIndex uint16
}

// NewWvttBox - Create new empty wvtt box
func NewWvttBox() *WvttBox {
	return &WvttBox{DataReferenceIndex: 1}
}

// AddChild - add a child box
func (b *WvttBox) AddChild(child Box) {
	switch box := child.(type) {
	case *VttCBox:
		b.VttC = box
	case *VlabBox:
		b.Vlab = box
	case *BtrtBox:
		b.Btrt = box
	default:
		// Other box
	}

	b.Children = append(b.Children, child)
}

const nrWvttBytesBeforeChildren = 16

// DecodeWvtt - Decoder wvtt Sample Entry (wvtt)
func DecodeWvtt(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeWvttSR(hdr, startPos, sr)
}

// DecodeWvttSR - Decoder wvtt Sample Entry (wvtt)
func DecodeWvttSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	w := WvttBox{}
	// 14496-12 8.5.2.2 Sample entry (8 bytes)
	sr.SkipBytes(6) // Skip 6 reserved bytes
	w.DataReferenceIndex = sr.ReadUint16()
	pos := startPos + nrWvttBytesBeforeChildren
	endPos := startPos + uint64(hdr.Hdrlen+hdr.payloadLen())
	for {
		if pos >= endPos {
			break
		}
		box, err := DecodeBoxSR(pos, sr)
		if err != nil {
			return nil, err
		}
		if box != nil {
			w.AddChild(box)
			pos += box.Size()
		} else {
			return nil, fmt.Errorf("no child of wvtt")
		}
	}
	return &w, nil
}

// Type - return box type
func (b *WvttBox) Type() string {
	return "wvtt"
}

// Size - return calculated size
func (b *WvttBox) Size() uint64 {
	totalSize := uint64(nrWvttBytesBeforeChildren)
	for _, child := range b.Children {
		totalSize += child.Size()
	}
	return totalSize
}

// Encode - write box to w
func (b *WvttBox) Encode(w io.Writer) error {
	err := EncodeHeader(b, w)
	if err != nil {
		return err
	}
	buf := makebuf(b)
	sw := bits.NewFixedSliceWriterFromSlice(buf)
	sw.WriteZeroBytes(6)
	sw.WriteUint16(b.DataReferenceIndex)

	_, err = w.Write(buf[:sw.Offset()]) // Only write written bytes
	if err != nil {
		return err
	}

	// Next output child boxes in order
	for _, child := range b.Children {
		err = child.Encode(w)
		if err != nil {
			return err
		}
	}
	return err
}

// EncodeSW - write box to w
func (b *WvttBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteZeroBytes(6)
	sw.WriteUint16(b.DataReferenceIndex)

	// Next output child boxes in order
	for _, child := range b.Children {
		err = child.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return err
}

// Info - write box-specific information
func (b *WvttBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	if bd.err != nil {
		return bd.err
	}
	var err error
	for _, child := range b.Children {
		err = child.Info(w, specificBoxLevels, indent+indentStep, indent)
		if err != nil {
			return err
		}
	}
	return nil
}

////////////////////////////// vttC //////////////////////////////

// VttCBox - WebVTTConfigurationBox (vttC)
type VttCBox struct {
	Config string
}

// DecodeVttC - box-specific decode
func DecodeVttC(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeVttCSR(hdr, startPos, sr)
}

// DecodeVttCSR - box-specific decode
func DecodeVttCSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &VttCBox{Config: sr.ReadFixedLengthString(hdr.payloadLen())}, sr.AccError()
}

// Type - box-specific type
func (b *VttCBox) Type() string {
	return "vttC"
}

// Size - calculated size of box
func (b *VttCBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.Config))
}

// Encode - write box to w
func (b *VttCBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *VttCBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteString(b.Config, false)
	return sw.AccError()
}

// Info - write box-specific information
func (b *VttCBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - config: %q", b.Config)
	return bd.err
}

////////////////////////////// vlab //////////////////////////////

// VlabBox - WebVTTSourceLabelBox (vlab)
type VlabBox struct {
	SourceLabel string
}

// DecodeVlab - box-specific decode
func DecodeVlab(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeVlabSR(hdr, startPos, sr)
}

// DecodeVlabSR - box-specific decode
func DecodeVlabSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &VlabBox{SourceLabel: sr.ReadFixedLengthString(hdr.payloadLen())}, sr.AccError()
}

// Type - box-specific type
func (b *VlabBox) Type() string {
	return "vlab"
}

// Size - calculated size of box
func (b *VlabBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.SourceLabel))
}

// Encode - write box to w
func (b *VlabBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *VlabBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteString(b.SourceLabel, false)
	return sw.AccError()
}

// Info - write box-specific information
func (b *VlabBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - sourceLabel: %s", b.SourceLabel)
	return bd.err
}

// wvtt Sample boxes
// A sample is either one vtte box or one or more vttc or vta boxes

////////////////////////////// vtte //////////////////////////////

// VtteBox - VTTEmptyBox (vtte)
type VtteBox struct {
}

// Type - box-specific type
func (b *VtteBox) Type() string {
	return "vtte"
}

// DecodeVtte - box-specific decode
func DecodeVtte(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	return &VtteBox{}, nil
}

// DecodeVtteSR - box-specific decode
func DecodeVtteSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &VtteBox{}, nil
}

// Size - calculated size of box
func (b *VtteBox) Size() uint64 {
	return uint64(boxHeaderSize)
}

// Encode - write box to w
func (b *VtteBox) Encode(w io.Writer) error {
	return EncodeHeader(b, w)
}

// EncodeSW - box-specific encode to slicewriter
func (b *VtteBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeHeaderSW(b, sw)
}

// Info - write box-specific information
func (b *VtteBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	return bd.err
}

////////////////////////////// vttc //////////////////////////////

// VttcBox - VTTCueBox (vttc)
type VttcBox struct {
	Vsid     *VsidBox
	Iden     *IdenBox
	Ctim     *CtimBox
	Sttg     *SttgBox
	Payl     *PaylBox
	Children []Box
}

// AddChild - Add a child box
func (b *VttcBox) AddChild(child Box) {

	switch box := child.(type) {
	case *VsidBox:
		b.Vsid = box
	case *IdenBox:
		b.Iden = box
	case *CtimBox:
		b.Ctim = box
	case *SttgBox:
		b.Sttg = box
	case *PaylBox:
		b.Payl = box
	default:
		// Type outside ISO/IEC 14496-30 spec
	}
	b.Children = append(b.Children, child)
}

// DecodeVttc - box-specific decode
func DecodeVttc(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	b := VttcBox{}
	for _, child := range children {
		b.AddChild(child)
	}
	return &b, nil
}

// DecodeVttcSR - box-specific decode
func DecodeVttcSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	b := VttcBox{Children: make([]Box, 0, len(children))}
	for _, c := range children {
		b.AddChild(c)
	}
	return &b, nil
}

// Type - return box type
func (b *VttcBox) Type() string {
	return "vttc"
}

// Size - return calculated size
func (b *VttcBox) Size() uint64 {
	return containerSize(b.Children)
}

// GetChildren - list of child boxes
func (b *VttcBox) GetChildren() []Box {
	return b.Children
}

// Encode - write mvex container to w
func (b *VttcBox) Encode(w io.Writer) error {
	return EncodeContainer(b, w)
}

// Encode - write vttc container to sw
func (b *VttcBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(b, sw)
}

// Info - write box-specific information
func (b *VttcBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(b, w, specificBoxLevels, indent, indentStep)
}

////////////////////////////// vsid //////////////////////////////

// VsidBox - CueSourceIDBox (vsid)
type VsidBox struct {
	SourceID uint32
}

// DecodeVsid - box-specific decode
func DecodeVsid(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeVsidSR(hdr, startPos, sr)
}

// DecodeVsidSR - box-specific decode
func DecodeVsidSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &VsidBox{SourceID: sr.ReadUint32()}, sr.AccError()
}

// Type - box-specific type
func (b *VsidBox) Type() string {
	return "vsid"
}

// Size - calculated size of box
func (b *VsidBox) Size() uint64 {
	return uint64(boxHeaderSize + 4) // len of uint32
}

// Encode - write box to w
func (b *VsidBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *VsidBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteUint32(b.SourceID)
	return sw.AccError()
}

// Info - write box-specific information
func (b *VsidBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - sourceID: %d", b.SourceID)
	return bd.err
}

////////////////////////////// ctim //////////////////////////////

// CtimBox - CueTimeBox (ctim)
// CueCurrentTime is current time indication (for split cues)
type CtimBox struct {
	CueCurrentTime string
}

// DecodeCtim - box-specific decode
func DecodeCtim(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeCtimSR(hdr, startPos, sr)
}

// DecodeCtimSR - box-specific decode
func DecodeCtimSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &CtimBox{CueCurrentTime: sr.ReadFixedLengthString(hdr.payloadLen())}, sr.AccError()
}

// Type - box-specific type
func (b *CtimBox) Type() string {
	return "ctim"
}

// Size - calculated size of box
func (b *CtimBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.CueCurrentTime))
}

// Encode - write box to w
func (b *CtimBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *CtimBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteString(b.CueCurrentTime, false)
	return sw.AccError()
}

// Info - write box-specific information
func (b *CtimBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - cueCurrentTime: %s", b.CueCurrentTime)
	return bd.err
}

////////////////////////////// iden //////////////////////////////

// IdenBox - CueIDBox (iden)
type IdenBox struct {
	CueID string
}

// DecodeIden - box-specific decode
func DecodeIden(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeIdenSR(hdr, startPos, sr)
}

// DecodeIdenSR - box-specific decode
func DecodeIdenSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &IdenBox{CueID: sr.ReadFixedLengthString(hdr.payloadLen())}, sr.AccError()
}

// Type - box-specific type
func (b *IdenBox) Type() string {
	return "iden"
}

// Size - calculated size of box
func (b *IdenBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.CueID))
}

// Encode - write box to w
func (b *IdenBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *IdenBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteString(b.CueID, false)
	return sw.AccError()
}

// Info - write box-specific information
func (b *IdenBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - cueID: %s", b.CueID)
	return bd.err
}

////////////////////////////// sttg //////////////////////////////

// SttgBox - CueSettingsBox (sttg)
type SttgBox struct {
	Settings string
}

// DecodeSttg - box-specific decode
func DecodeSttg(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSttgSR(hdr, startPos, sr)
}

// DecodeSttgSR - box-specific decode
func DecodeSttgSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &SttgBox{Settings: sr.ReadFixedLengthString(hdr.payloadLen())}, sr.AccError()
}

// Type - box-specific type
func (b *SttgBox) Type() string {
	return "sttg"
}

// Size - calculated size of box
func (b *SttgBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.Settings))
}

// Encode - write box to w
func (b *SttgBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *SttgBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteString(b.Settings, false)
	return sw.AccError()
}

// Info - write box-specific information
func (b *SttgBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - settings: %s", b.Settings)
	return bd.err
}

////////////////////////////// payl //////////////////////////////

// PaylBox - CuePayloadBox (payl)
type PaylBox struct {
	CueText string
}

// DecodePayl - box-specific decode
func DecodePayl(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodePaylSR(hdr, startPos, sr)
}

// DecodePaylSR - box-specific decode
func DecodePaylSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &PaylBox{CueText: sr.ReadFixedLengthString(hdr.payloadLen())}, sr.AccError()
}

// Type - box-specific type
func (b *PaylBox) Type() string {
	return "payl"
}

// Size - calculated size of box
func (b *PaylBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.CueText))
}

// Encode - write box to w
func (b *PaylBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *PaylBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteString(b.CueText, false)
	return sw.AccError()
}

// Info - write box-specific information
func (b *PaylBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - cueText: %q", b.CueText)
	return bd.err
}

////////////////////////////// vtta //////////////////////////////

// VttaBox - VTTAdditionalTextBox (vtta) (corresponds to NOTE in WebVTT)
type VttaBox struct {
	CueAdditionalText string
}

// DecodeVtta - box-specific decode
func DecodeVtta(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeVttaSR(hdr, startPos, sr)
}

// DecodeVttaSR - box-specific decode
func DecodeVttaSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	return &VttaBox{CueAdditionalText: sr.ReadFixedLengthString(hdr.payloadLen())}, sr.AccError()
}

// Type - box-specific type
func (b *VttaBox) Type() string {
	return "vtta"
}

// Size - calculated size of box
func (b *VttaBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.CueAdditionalText))
}

// Encode - write box to w
func (b *VttaBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *VttaBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteString(b.CueAdditionalText, false)
	return sw.AccError()
}

// Info - write box-specific information
func (b *VttaBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - cueAdditionalText: %q", b.CueAdditionalText)
	return bd.err
}
