package mp4

import (
	"encoding/hex"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// EvteBox - EventMessageSampleEntry box as defined in ISO/IEC 23001-18 Section 7.2
type EvteBox struct {
	Btrt               *BtrtBox
	Silb               *SilbBox
	Children           []Box
	DataReferenceIndex uint16
}

// DecodeEvte - Decode EventMessageSampleEntry (evte)
func DecodeEvte(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeEvteSR(hdr, startPos, sr)
}

// DecodeEvteSR - Decode EventMessageSampleEntry (evte)
func DecodeEvteSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	payloadLen := hdr.payloadLen()
	b := EvteBox{}
	// 14496-12 8.5.2.2 Sample entry (8 bytes)
	initPos := sr.GetPos()
	sr.SkipBytes(6) // Skip 6 reserved bytes
	b.DataReferenceIndex = sr.ReadUint16()
	if err := sr.AccError(); err != nil {
		return nil, err
	}
	pos := startPos + uint64(hdr.Hdrlen+sr.GetPos()-initPos)
	remainingBytes := func(sr bits.SliceReader, initPos, payloadLen int) int {
		return payloadLen - (sr.GetPos() - initPos)
	}

	for {
		rest := remainingBytes(sr, initPos, payloadLen)
		if rest <= 0 {
			break
		}
		box, err := DecodeBoxSR(pos, sr)
		if err != nil {
			return nil, err
		}
		if box != nil {
			b.AddChild(box)
			pos += box.Size()
		} else {
			return nil, fmt.Errorf("no evte child")
		}
	}
	return &b, sr.AccError()
}

// AddChild - add a child box (should only be btrt and silb)
func (b *EvteBox) AddChild(child Box) {
	switch box := child.(type) {
	case *BtrtBox:
		b.Btrt = box
	case *SilbBox:
		b.Silb = box
	default:
		// Other box
	}
	b.Children = append(b.Children, child)
}

func (b *EvteBox) Type() string {
	return "evte"
}

func (b *EvteBox) Size() uint64 {
	size := uint64(boxHeaderSize + 8)
	for _, child := range b.Children {
		size += child.Size()
	}
	return size
}

// Encode - write box to w via a SliceWriter
func (b *EvteBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - write box to sw
func (b *EvteBox) EncodeSW(sw bits.SliceWriter) error {
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

// Info - write specific box info to w
func (b *EvteBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - dataReferenceIndex: %d", b.DataReferenceIndex)
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

// SilbBox - Scheme Identifier Box as defined in ISO/IEC 23001-18 Section 7.3
type SilbBox struct {
	Version          uint8
	Flags            uint32
	Schemes          []SilbEntry
	OtherSchemesFlag bool
}

// SilbEntry - Scheme Identifier Box entry
type SilbEntry struct {
	SchemeIdURI    string
	Value          string
	AtLeastOneFlag bool
}

// DecodeSilb - Decode Scheme Identifier Box (silb)
func DecodeSilb(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeSilbSR(hdr, startPos, sr)

}

// DecodeSilbSR - Decode Scheme Identifier Box (silb)
func DecodeSilbSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	b := SilbBox{}
	versionAndFlags := sr.ReadUint32()
	b.Version = uint8(versionAndFlags >> 24)
	b.Flags = versionAndFlags & flagsMask
	nrSchemes := sr.ReadUint32()
	b.Schemes = make([]SilbEntry, nrSchemes)
	for i := uint32(0); i < nrSchemes; i++ {
		schemeIdURI := sr.ReadZeroTerminatedString(int(hdr.payloadLen()) - 8)
		value := sr.ReadZeroTerminatedString(int(hdr.payloadLen()) - 8 - len(schemeIdURI) - 1)
		atLeastOneFlag := sr.ReadUint8() == 1
		b.Schemes[i] = SilbEntry{
			SchemeIdURI:    schemeIdURI,
			Value:          value,
			AtLeastOneFlag: atLeastOneFlag,
		}
	}
	b.OtherSchemesFlag = sr.ReadUint8() == 1
	return &b, sr.AccError()
}

func (b *SilbBox) Type() string {
	return "silb"
}

func (b *SilbBox) Size() uint64 {
	size := uint64(boxHeaderSize + 8)
	for _, scheme := range b.Schemes {
		size += uint64(len(scheme.SchemeIdURI) + 1 + len(scheme.Value) + 1 + 1)
	}
	size += 1 // OtherSchemesFlag
	return size
}

// Encode - write box to w via a SliceWriter
func (b *SilbBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - write box to sw
func (b *SilbBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := uint32(b.Version)<<24 | b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(uint32(len(b.Schemes)))
	for _, scheme := range b.Schemes {
		sw.WriteString(scheme.SchemeIdURI, true)
		sw.WriteString(scheme.Value, true)
		if scheme.AtLeastOneFlag {
			sw.WriteUint8(1)
		} else {
			sw.WriteUint8(0)
		}
	}
	if b.OtherSchemesFlag {
		sw.WriteUint8(1)
	} else {
		sw.WriteUint8(0)
	}
	return sw.AccError()
}

// Info - write specific box info to w
func (b *SilbBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	nrEntries := len(b.Schemes)
	for i := 0; i < nrEntries; i++ {
		bd.write(" - entry[%d]: schemeIdURI=%q value=%q atLeastOneFlag=%t", i+1,
			b.Schemes[i].SchemeIdURI, b.Schemes[i].Value, b.Schemes[i].AtLeastOneFlag)
	}
	return bd.err
}

// EmibBox - EventMessageInstanceBox as defined in ISO/IEC 23001-18 Section 6.1
type EmibBox struct {
	Version               uint8
	Flags                 uint32
	PresentationTimeDelta int64
	EventDuration         uint32
	Id                    uint32
	SchemeIdURI           string
	Value                 string
	MessageData           []byte
}

// DecodeEmib - box-specific decode
func DecodeEmib(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeEmibSR(hdr, startPos, sr)
}

// DecodeEmibSR - box-specific decode
func DecodeEmibSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	b := EmibBox{}
	versionAndFlags := sr.ReadUint32()
	b.Version = uint8(versionAndFlags >> 24)
	b.Flags = versionAndFlags & flagsMask
	_ = sr.ReadUint32() // reserved
	b.PresentationTimeDelta = sr.ReadInt64()
	b.EventDuration = sr.ReadUint32()
	b.Id = sr.ReadUint32()
	b.SchemeIdURI = sr.ReadZeroTerminatedString(int(hdr.payloadLen()) - 24)
	b.Value = sr.ReadZeroTerminatedString(int(hdr.payloadLen()) - 24 - len(b.SchemeIdURI) - 1)
	b.MessageData = sr.ReadBytes(int(hdr.payloadLen()) - 24 - len(b.SchemeIdURI) - 1 - len(b.Value) - 1)
	return &b, sr.AccError()
}

func (b *EmibBox) Type() string {
	return "emib"
}

func (b *EmibBox) Size() uint64 {
	return uint64(boxHeaderSize + 24 + len(b.SchemeIdURI) + 1 + len(b.Value) + 1 + len(b.MessageData))
}

func (b *EmibBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

func (b *EmibBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := uint32(b.Version)<<24 | b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(0) // reserved
	sw.WriteInt64(b.PresentationTimeDelta)
	sw.WriteUint32(b.EventDuration)
	sw.WriteUint32(b.Id)
	sw.WriteString(b.SchemeIdURI, true)
	sw.WriteString(b.Value, true)
	sw.WriteBytes(b.MessageData)
	return sw.AccError()
}

func (b *EmibBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - presentationTimeDelta: %d", b.PresentationTimeDelta)
	bd.write(" - eventDuration: %d", b.EventDuration)
	bd.write(" - id: %d", b.Id)
	level := getInfoLevel(b, specificBoxLevels)
	if level > 0 {
		bd.write(" - schemeIdURI: %q", b.SchemeIdURI)
		bd.write(" - value: %q", b.Value)
		bd.write(" - messageData: %s", hex.EncodeToString(b.MessageData))
	}
	return bd.err
}

// EmebBox - EventMessageBox as defined in ISO/IEC 23001-18 Section 6.2
type EmebBox struct {
}

// DecodeEmeb - box-specific decode
func DecodeEmeb(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	if hdr.Size != 8 {
		return nil, fmt.Errorf("decode emeb: size %d not 8", hdr.Size)
	}
	return &EmebBox{}, nil
}

// DecodeEmebSR - box-specific decode
func DecodeEmebSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	if hdr.Size != 8 {
		return nil, fmt.Errorf("decode emeb: size %d not 8", hdr.Size)
	}
	return &EmebBox{}, nil
}

// Type - box-specific type
func (b *EmebBox) Type() string {
	return "emeb"
}

// Size - calculated size of box
func (b *EmebBox) Size() uint64 {
	return uint64(boxHeaderSize)
}

// Encode - write box to w
func (b *EmebBox) Encode(w io.Writer) error {
	return EncodeHeader(b, w)
}

// EncodeSW - box-specific encode to slicewriter
func (b *EmebBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeHeaderSW(b, sw)
}

// Info - write box-specific information
func (b *EmebBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	return bd.err
}
