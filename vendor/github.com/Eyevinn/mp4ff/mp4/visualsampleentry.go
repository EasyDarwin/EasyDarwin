package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
	"github.com/Eyevinn/mp4ff/hevc"
)

// VisualSampleEntryBox Video Sample Description box (avc1/avc3/hvc1/hev1...)
type VisualSampleEntryBox struct {
	name               string
	DataReferenceIndex uint16
	Width              uint16
	Height             uint16
	Horizresolution    uint32
	Vertresolution     uint32
	FrameCount         uint16
	CompressorName     string
	AvcC               *AvcCBox
	HvcC               *HvcCBox
	Av1C               *Av1CBox
	Btrt               *BtrtBox
	Clap               *ClapBox
	Pasp               *PaspBox
	Sinf               *SinfBox
	Children           []Box
}

// NewVisualSampleEntryBox creates new empty box with an appropriate name such as avc1
func NewVisualSampleEntryBox(name string) *VisualSampleEntryBox {
	b := &VisualSampleEntryBox{}
	b.name = name
	return b
}

// CreateVisualSampleEntryBox creates a new VisualSampleEntry such as avc1, avc3, hev1, hvc1
func CreateVisualSampleEntryBox(name string, width, height uint16, sampleEntry Box) *VisualSampleEntryBox {
	b := &VisualSampleEntryBox{
		name:               name,
		DataReferenceIndex: 1,
		Width:              width,
		Height:             height,
		Horizresolution:    0x00480000, // 72dpi
		Vertresolution:     0x00480000, // 72dpi
		FrameCount:         1,
		CompressorName:     "mp4ff video packager",
		Children:           []Box{},
	}
	if sampleEntry != nil {
		b.AddChild(sampleEntry)
	}
	return b
}

// AddChild adds a child box and sets pointer to common types
func (b *VisualSampleEntryBox) AddChild(child Box) {
	switch box := child.(type) {
	case *AvcCBox:
		b.AvcC = box
	case *HvcCBox:
		b.HvcC = box
	case *Av1CBox:
		b.Av1C = box
	case *BtrtBox:
		b.Btrt = box
	case *ClapBox:
		b.Clap = box
	case *PaspBox:
		b.Pasp = box
	case *SinfBox:
		b.Sinf = box
	}

	b.Children = append(b.Children, child)
}

// DecodeVisualSampleEntry decodes avc1/avc3/hvc1/hev1 box
func DecodeVisualSampleEntry(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeVisualSampleEntrySR(hdr, startPos, sr)
}

// DecodeVisualSampleEntrySR decodes avc1/avc3/hvc1/hev1 box
func DecodeVisualSampleEntrySR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	b := VisualSampleEntryBox{name: hdr.Name}

	// 14496-12 8.5.2.2 Sample entry (8 bytes)
	sr.SkipBytes(6) // Skip 6 reserved bytes
	b.DataReferenceIndex = sr.ReadUint16()

	// 14496-12 12.1.3.2 Visual Sample entry (70 bytes)

	sr.SkipBytes(4)  // pre_defined and reserved == 0
	sr.SkipBytes(12) // 3 x 32 bits pre_defined == 0
	b.Width = sr.ReadUint16()
	b.Height = sr.ReadUint16()

	b.Horizresolution = sr.ReadUint32()
	b.Vertresolution = sr.ReadUint32()

	sr.ReadUint32()                // reserved
	b.FrameCount = sr.ReadUint16() // Should be 1
	compressorNameLength := sr.ReadUint8()
	if compressorNameLength > 31 {
		return nil, fmt.Errorf("too long compressor name length")
	}
	b.CompressorName = sr.ReadFixedLengthString(int(compressorNameLength))
	sr.SkipBytes(int(31 - compressorNameLength))
	sr.ReadUint16() // depth == 0x0018
	sr.ReadUint16() // pre_defined == -1

	// Now there may be clap and pasp boxes
	// 14496-15  5.4.2.1.2 avcC should be inside avc1, avc3 box
	pos := startPos + 86 // Size of all previous data
	endPos := startPos + uint64(hdr.Hdrlen) + uint64(hdr.payloadLen())
	for {
		if pos >= endPos {
			break
		}
		box, err := DecodeBoxSR(pos, sr)
		if err != nil {
			return nil, fmt.Errorf("error decoding childBox of VisualSampleEntry: %w", err)
		}
		if box != nil {
			b.AddChild(box)
			pos += box.Size()
		} else {
			return nil, fmt.Errorf("not childbox of VisualSampleEntry")
		}
	}
	return &b, sr.AccError()
}

// Type returns box type
func (b *VisualSampleEntryBox) Type() string {
	return b.name
}

// SetType sets the type (name) of the box
func (b *VisualSampleEntryBox) SetType(name string) {
	b.name = name
}

// Size - return calculated size
func (b *VisualSampleEntryBox) Size() uint64 {
	totalSize := uint64(boxHeaderSize + 78)
	for _, child := range b.Children {
		totalSize += child.Size()
	}
	return totalSize
}

// Encode writes box to w
func (b *VisualSampleEntryBox) Encode(w io.Writer) error {
	err := EncodeHeader(b, w)
	if err != nil {
		return err
	}
	buf := makebuf(b)
	sw := bits.NewFixedSliceWriterFromSlice(buf)
	sw.WriteZeroBytes(6)
	sw.WriteUint16(b.DataReferenceIndex)
	sw.WriteZeroBytes(16) // pre_defined and reserved
	sw.WriteUint16(b.Width)
	sw.WriteUint16(b.Height) //36 bytes

	sw.WriteUint32(b.Horizresolution)
	sw.WriteUint32(b.Vertresolution)
	sw.WriteZeroBytes(4)
	sw.WriteUint16(b.FrameCount) //50 bytes

	compressorNameLength := byte(len(b.CompressorName))
	sw.WriteUint8(compressorNameLength)
	sw.WriteString(b.CompressorName, false)
	sw.WriteZeroBytes(int(31 - compressorNameLength))
	sw.WriteUint16(0x0018) // depth == 0x0018
	sw.WriteUint16(0xffff) // pre_defined == -1  //86 bytes

	_, err = w.Write(buf[:sw.Offset()]) // Only write  written bytes
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

// EncodeSW writes box to sw
func (b *VisualSampleEntryBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteZeroBytes(6)
	sw.WriteUint16(b.DataReferenceIndex)
	sw.WriteZeroBytes(16) // pre_defined and reserved
	sw.WriteUint16(b.Width)
	sw.WriteUint16(b.Height) //36 bytes

	sw.WriteUint32(b.Horizresolution)
	sw.WriteUint32(b.Vertresolution)
	sw.WriteZeroBytes(4)
	sw.WriteUint16(b.FrameCount) //50 bytes

	compressorNameLength := byte(len(b.CompressorName))
	sw.WriteUint8(compressorNameLength)
	sw.WriteString(b.CompressorName, false)
	sw.WriteZeroBytes(int(31 - compressorNameLength))
	sw.WriteUint16(0x0018) // depth == 0x0018
	sw.WriteUint16(0xffff) // pre_defined == -1  //86 bytes

	// Next output child boxes in order
	for _, child := range b.Children {
		err = child.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return err
}

// Info writes box-specific information
func (b *VisualSampleEntryBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - width: %d", b.Width)
	bd.write(" - height: %d", b.Height)
	bd.write(" - compressorName: %q", b.CompressorName)
	if bd.err != nil {
		return bd.err
	}
	var err error
	for _, child := range b.Children {
		err = child.Info(w, specificBoxLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	return nil
}

// RemoveEncryption removes sinf box and set type to unencrypted type
func (b *VisualSampleEntryBox) RemoveEncryption() (*SinfBox, error) {
	if b.name != "encv" {
		return nil, fmt.Errorf("is not encrypted: %s", b.name)
	}
	sinf := b.Sinf
	if sinf == nil {
		return nil, fmt.Errorf("does not have sinf box")
	}
	for i := range b.Children {
		if b.Children[i].Type() == "sinf" {
			b.Children = append(b.Children[:i], b.Children[i+1:]...)
			b.Sinf = nil
			break
		}
	}
	b.name = sinf.Frma.DataFormat
	return sinf, nil
}

// ConvertHev1ToHvc1 converts visual sample entry box type and insert VPS, SPS, and PPS parameter sets
func (b *VisualSampleEntryBox) ConvertHev1ToHvc1(vpss [][]byte, spss [][]byte, ppss [][]byte) error {
	if b.Type() != "hev1" {
		return fmt.Errorf("type is %s and not hev1", b.Type())
	}
	b.SetType("hvc1")
	b.HvcC.DecConfRec.NaluArrays = append(b.HvcC.DecConfRec.NaluArrays, hevc.NewNaluArray(true, hevc.NALU_VPS, vpss))
	b.HvcC.DecConfRec.NaluArrays = append(b.HvcC.DecConfRec.NaluArrays, hevc.NewNaluArray(true, hevc.NALU_SPS, spss))
	b.HvcC.DecConfRec.NaluArrays = append(b.HvcC.DecConfRec.NaluArrays, hevc.NewNaluArray(true, hevc.NALU_PPS, ppss))
	return nil
}

// ConvertAvc3ToAvc1 converts visual sample entry box type and insert SPS and PPS parameter sets
func (b *VisualSampleEntryBox) ConvertAvc3ToAvc1(spss [][]byte, ppss [][]byte) error {
	if b.Type() != "avc3" {
		return fmt.Errorf("type is %s and not avc3", b.Type())
	}
	b.SetType("avc1")
	b.AvcC.DecConfRec.SPSnalus = spss
	b.AvcC.DecConfRec.PPSnalus = ppss
	return nil
}
