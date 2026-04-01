package mp4

import (
	"encoding/hex"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

/* Elementary Stream Descriptors are defined in ISO/IEC 14496-1.
The full spec looks like, below, but we don't parse all that stuff.
*/

const (
	// Following Table 1 of Class Tags for descriptors in ISO/IEC 14496-1. There are more types
	ObjectDescrTag        = 1
	InitialObjectDescrTag = 2
	ES_DescrTag           = 3
	DecoderConfigDescrTag = 4
	DecSpecificInfoTag    = 5
	SLConfigDescrTag      = 6

	minimalEsDescrSize = 25
)

func TagType(tag byte) string {
	switch tag {
	case ObjectDescrTag:
		return "tag=1 Object"
	case InitialObjectDescrTag:
		return "tag=2 InitialObject"
	case ES_DescrTag:
		return "tag=3 ES"
	case DecoderConfigDescrTag:
		return "tag=4 DecoderConfig"
	case DecSpecificInfoTag:
		return "tag=5 DecoderSpecificInfo"
	case SLConfigDescrTag:
		return "tag=6 SLConfig"
	default:
		return fmt.Sprintf("tag=%d Unknown", tag)
	}
}

type Descriptor interface {
	// Tag - descriptor tag. Fixed for each descriptor type
	Tag() byte
	// Type is string describing Tag, making descriptor fullfill boxLike interface
	Type() string
	// Size - size of descriptor, excluding tag byte and size field
	Size() uint64
	// SizeSize - size of descriptor including tag byte and size field
	SizeSize() uint64
	// EncodeSW - Write descriptor to slice writer
	EncodeSW(sw bits.SliceWriter) error
	// Info - write information about descriptor
	//   Higher levels give more details. 0 is default
	//   indent is indent at this box level.
	//   indentStep is how much to indent at each level
	Info(w io.Writer, specificLevels, indent, indentStep string) error
}

/*
ESDescriptor is defined in ISO/IEC 14496-1 7.2.6.5

	class ES_Descriptor extends BaseDescriptor : bit(8) tag=ES_DescrTag {
	  bit(16) ES_ID;
	  bit(1) streamDependenceFlag;
	  bit(1) URL_Flag;
	  bit(1) OCRstreamFlag;
	  bit(5) streamPriority;
	  if (streamDependenceFlag)
	    bit(16) dependsOn_ES_ID;
	  if (URL_Flag) {
	    bit(8) URLlength;
	    bit(8) URLstring[URLlength];
	  }
	  if (OCRstreamFlag)
	    bit(16) OCR_ES_Id;
	  DecoderConfigDescriptor decConfigDescr;
	  if (ODProfileLevelIndication==0x01) //no SL extension.
	  {
	    SLConfigDescriptor slConfigDescr;
	  } else  { // SL extension is possible.
	    SLConfigDescriptor slConfigDescr;
	  }
	  IPI_DescrPointer ipiPtr[0 .. 1];
	  IP_IdentificationDataSet ipIDS[0 .. 255];
	  IPMP_DescriptorPointer ipmpDescrPtr[0 .. 255];
	  LanguageDescriptor langDescr[0 .. 255];
	  QoS_Descriptor qosDescr[0 .. 1];
	  RegistrationDescriptor regDescr[0 .. 1];
	  ExtensionDescriptor extDescr[0 .. 255];
	}
*/
type ESDescriptor struct {
	EsID                uint16
	DependsOnEsID       uint16
	OCResID             uint16
	FlagsAndPriority    byte
	sizeFieldSizeMinus1 byte
	URLString           string
	DecConfigDescriptor *DecoderConfigDescriptor
	SLConfigDescriptor  *SLConfigDescriptor
	OtherDescriptors    []Descriptor
	UnknownData         []byte // Data, probably erronous, that we don't understand
}

func DecodeDescriptor(sr bits.SliceReader, maxNrBytes int) (Descriptor, error) {
	if maxNrBytes < 2 {
		return nil, fmt.Errorf("descriptor size %d too small", maxNrBytes)
	}
	tag := sr.ReadUint8()
	if sr.AccError() != nil {
		return nil, sr.AccError()
	}
	switch tag {
	case ES_DescrTag:
		return nil, fmt.Errorf("use DecodeESDescriptor instead")
	case DecoderConfigDescrTag:
		return DecodeDecoderConfigDescriptor(tag, sr, maxNrBytes)
	case DecSpecificInfoTag:
		return DecodeDecSpecificInfoDescriptor(tag, sr, maxNrBytes)
	case SLConfigDescrTag:
		return DecodeSLConfigDescriptor(tag, sr, maxNrBytes)
	default:
		return DecodeRawDescriptor(tag, sr, maxNrBytes)
	}
}

func DecodeESDescriptor(sr bits.SliceReader, descSize uint32) (ESDescriptor, error) {
	ed := ESDescriptor{}
	tag := sr.ReadUint8()
	if tag != ES_DescrTag {
		return ed, fmt.Errorf("got tag %d instead of ESDescriptorTag %d", tag, ES_DescrTag)
	}

	sizeFieldSizeMinus1, size, err := readSizeSize(sr)
	if err != nil {
		return ed, err
	}
	ed.sizeFieldSizeMinus1 = sizeFieldSizeMinus1
	dataStart := sr.GetPos()
	ed.EsID = sr.ReadUint16()
	ed.FlagsAndPriority = sr.ReadUint8()
	streamDependenceFlag := ed.FlagsAndPriority >> 7
	urlFlag := (ed.FlagsAndPriority >> 6) & 0x1
	ocrStreamFlag := (ed.FlagsAndPriority >> 5) & 0x1
	// streamPriority := ed.FlagsAndPriority & 0x1f

	if streamDependenceFlag == 1 {
		ed.DependsOnEsID = sr.ReadUint16()
	}
	if urlFlag == 1 {
		urlLen := sr.ReadUint8()
		ed.URLString = sr.ReadFixedLengthString(int(urlLen))
	}
	if ocrStreamFlag == 1 {
		ed.OCResID = sr.ReadUint16()
	}
	currPos := sr.GetPos()
	nrBytesLeft := int(size) - (currPos - dataStart)
	desc, err := DecodeDescriptor(sr, nrBytesLeft)
	if err != nil {
		return ed, err
	}
	var ok bool
	ed.DecConfigDescriptor, ok = desc.(*DecoderConfigDescriptor)
	if !ok {
		return ed, fmt.Errorf("expected DecoderConfigDescriptor")
	}
	currPos = sr.GetPos()
	nrBytesLeft = int(size) - (currPos - dataStart)
	desc, err = DecodeDescriptor(sr, nrBytesLeft)
	if err != nil {
		sr.SetPos(currPos)
		ed.UnknownData = sr.ReadBytes(nrBytesLeft)
		return ed, nil
	}
	ed.SLConfigDescriptor, ok = desc.(*SLConfigDescriptor)
	if !ok {
		ed.OtherDescriptors = append(ed.OtherDescriptors, desc)
	}
	for {
		currPos = sr.GetPos()
		nrBytesLeft := int(size) - (currPos - dataStart)
		if nrBytesLeft == 0 {
			break
		}
		if nrBytesLeft < 0 {
			return ed, fmt.Errorf("read too far in ESDescriptor")
		}
		desc, err := DecodeDescriptor(sr, nrBytesLeft)
		if err != nil {
			sr.SetPos(currPos)
			ed.UnknownData = sr.ReadBytes(nrBytesLeft)
			return ed, nil
		}
		ed.OtherDescriptors = append(ed.OtherDescriptors, desc)
	}
	if size != ed.Size() {
		return ed, fmt.Errorf("read size %d differs from calculated size %d", size, ed.Size())
	}
	return ed, sr.AccError()
}

func (e *ESDescriptor) Tag() byte {
	return ES_DescrTag
}

func (e *ESDescriptor) Type() string {
	return TagType(e.Tag())
}

// Size is size of payload after tag and size field
func (e *ESDescriptor) Size() uint64 {
	var size uint64 = 2 + 1
	streamDependenceFlag := e.FlagsAndPriority >> 7
	urlFlag := (e.FlagsAndPriority >> 6) & 0x1
	ocrStreamFlag := (e.FlagsAndPriority >> 5) & 0x1
	if streamDependenceFlag == 1 {
		size += 2
	}
	if urlFlag == 1 {
		size += 1 + uint64(len(e.URLString))
	}
	if ocrStreamFlag == 1 {
		size += 2
	}
	if e.DecConfigDescriptor != nil {
		size += e.DecConfigDescriptor.SizeSize()
	}
	if e.SLConfigDescriptor != nil {
		size += e.SLConfigDescriptor.SizeSize()
	}
	for _, od := range e.OtherDescriptors {
		size += od.SizeSize()
	}
	size += uint64(len(e.UnknownData))
	return size
}

// SizeSize is size of size field.
func (e *ESDescriptor) SizeSize() uint64 {
	return 1 + uint64(e.sizeFieldSizeMinus1) + 1 + e.Size()
}

func (e *ESDescriptor) EncodeSW(sw bits.SliceWriter) error {
	sw.WriteBits(uint(e.Tag()), 8)
	writeDescriptorSize(sw, e.Size(), e.sizeFieldSizeMinus1)
	sw.WriteUint16(e.EsID)
	sw.WriteUint8(e.FlagsAndPriority)
	streamDependenceFlag := e.FlagsAndPriority >> 7
	urlFlag := (e.FlagsAndPriority >> 6) & 0x1
	ocrStreamFlag := (e.FlagsAndPriority >> 5) & 0x1
	// streamPriority := ed.FlagsAndPriority & 0x1f
	if streamDependenceFlag == 1 {
		sw.WriteUint16(e.DependsOnEsID)
	}
	if urlFlag == 1 {
		sw.WriteUint8(byte(len(e.URLString)))
		sw.WriteString(e.URLString, false /* no zero-termination */)
	}
	if ocrStreamFlag == 1 {
		sw.WriteUint16(e.OCResID)
	}
	if e.DecConfigDescriptor == nil {
		return fmt.Errorf("missing DecoderConfigDescriptor")
	}
	err := e.DecConfigDescriptor.EncodeSW(sw)
	if err != nil {
		return err
	}
	if e.SLConfigDescriptor != nil {
		err = e.SLConfigDescriptor.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	for _, od := range e.OtherDescriptors {
		err = od.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	if len(e.UnknownData) > 0 {
		sw.WriteBytes(e.UnknownData)
	}
	return sw.AccError()
}

func (e *ESDescriptor) Info(w io.Writer, specificLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, e, infoVersionDescriptor, 0)
	level := getInfoLevel(e, specificLevels)
	if level > 0 {
		bd.write(" - EsID: %d", e.EsID)
		bd.write(" - DependsOnEsID: %d", e.DependsOnEsID)
		bd.write(" - OCResID: %d", e.OCResID)
		bd.write(" - FlagsAndPriority: %d", e.FlagsAndPriority)
		bd.write(" - URLString: %s", e.URLString)
	}
	if e.DecConfigDescriptor != nil {
		err := e.DecConfigDescriptor.Info(w, specificLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	} else {
		bd.write(" - Missing DecoderConfigDescriptor")
	}
	if e.SLConfigDescriptor != nil {
		err := e.SLConfigDescriptor.Info(w, specificLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	} else {
		bd.write(" - Missing SLConfigDescriptor")
	}
	for _, od := range e.OtherDescriptors {
		err := od.Info(w, specificLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	if len(e.UnknownData) > 0 {
		bd.write(" - UnknownData (%dB): %s", len(e.UnknownData), hex.EncodeToString(e.UnknownData))
	}
	return bd.err
}

// DecoderConfigDescriptor is defined in ISO/IEC 14496-1 Section 7.2.6.6.1
//
//	class DecoderConfigDescriptor extends BaseDescriptor : bit(8) tag=DecoderConfigDescrTag {
//	  bit(8) objectTypeIndication;
//	  bit(6) streamType;
//	  bit(1) upStream;
//	  const bit(1) reserved=1;
//	  bit(24) bufferSizeDB;
//	  bit(32) maxBitrate;
//	  bit(32) avgBitrate;
//	  DecoderSpecificInfo decSpecificInfo[0 .. 1];
//	  profileLevelIndicationIndexDescriptor profileLevelIndicationIndexDescr [0..255];
//	}
type DecoderConfigDescriptor struct {
	ObjectType          byte
	StreamType          byte
	sizeFieldSizeMinus1 byte
	BufferSizeDB        uint32
	MaxBitrate          uint32
	AvgBitrate          uint32
	DecSpecificInfo     *DecSpecificInfoDescriptor
	OtherDescriptors    []Descriptor
	UnknownData         []byte // Data, probably erronous, that we don't understand
}

func exceedsMaxNrBytes(sizeFieldSizeMinus1 byte, size uint64, maxNrBytes int) bool {
	return 1+uint64(sizeFieldSizeMinus1)+1+size > uint64(maxNrBytes)
}

func DecodeDecoderConfigDescriptor(tag byte, sr bits.SliceReader, maxNrBytes int) (Descriptor, error) {
	dd := DecoderConfigDescriptor{}
	if tag != DecoderConfigDescrTag {
		return nil, fmt.Errorf("got tag %d instead of DecoderConfigDescrTag %d", tag, DecoderConfigDescrTag)
	}
	sizeFieldSizeMinus1, size, err := readSizeSize(sr)
	if err != nil {
		return nil, err
	}
	dd.sizeFieldSizeMinus1 = sizeFieldSizeMinus1
	if exceedsMaxNrBytes(sizeFieldSizeMinus1, size, maxNrBytes) {
		return nil, fmt.Errorf("DecoderConfigDescriptor size %d exceeds maxNrBytes %d", size, maxNrBytes)
	}
	dataStart := sr.GetPos()
	dd.ObjectType = sr.ReadUint8()

	streamTypeAndBufferSizeDB := sr.ReadUint32()
	dd.StreamType = byte(streamTypeAndBufferSizeDB >> 24)
	dd.BufferSizeDB = streamTypeAndBufferSizeDB & 0xffffff
	dd.MaxBitrate = sr.ReadUint32()
	dd.AvgBitrate = sr.ReadUint32()

	currPos := sr.GetPos()
	nrBytesLeft := int(size) - (currPos - dataStart)
	if nrBytesLeft == 0 {
		return &dd, nil
	}
	desc, err := DecodeDescriptor(sr, nrBytesLeft)
	if err != nil {
		return nil, fmt.Errorf("failed to decode descriptor: %w", err)
	}
	var ok bool
	dd.DecSpecificInfo, ok = desc.(*DecSpecificInfoDescriptor)
	if !ok { // The optional decoderSpeicificInfo is not present
		dd.OtherDescriptors = append(dd.OtherDescriptors, desc)
	}
	for {
		currPos := sr.GetPos()
		nrBytesLeft := int(size) - (currPos - dataStart)
		if nrBytesLeft == 0 {
			break
		}
		if nrBytesLeft < 0 {
			return nil, fmt.Errorf("read too far in DecoderConfigDescriptor")
		}
		desc, err := DecodeDescriptor(sr, nrBytesLeft)
		if err != nil {
			sr.SetPos(currPos)
			dd.UnknownData = sr.ReadBytes(nrBytesLeft)
			return &dd, nil
		}
		dd.OtherDescriptors = append(dd.OtherDescriptors, desc)
	}
	return &dd, nil
}

func (d *DecoderConfigDescriptor) Tag() byte {
	return DecoderConfigDescrTag
}

func (d *DecoderConfigDescriptor) Type() string {
	return TagType(d.Tag())
}

func (d *DecoderConfigDescriptor) Size() uint64 {
	size := uint64(13)
	if d.DecSpecificInfo != nil {
		size += d.DecSpecificInfo.SizeSize()
	}
	for _, od := range d.OtherDescriptors {
		size += od.SizeSize()
	}
	size += uint64(len(d.UnknownData))
	return uint64(size)
}

func (d *DecoderConfigDescriptor) SizeSize() uint64 {
	return 1 + uint64(d.sizeFieldSizeMinus1) + 1 + d.Size()
}

func (d *DecoderConfigDescriptor) EncodeSW(sw bits.SliceWriter) error {
	sw.WriteBits(uint(d.Tag()), 8)
	writeDescriptorSize(sw, d.Size(), d.sizeFieldSizeMinus1)
	sw.WriteUint8(d.ObjectType)
	streamTypeAndBufferSizeDB := (uint32(d.StreamType) << 24) | d.BufferSizeDB
	sw.WriteUint32(streamTypeAndBufferSizeDB)
	sw.WriteUint32(d.MaxBitrate)
	sw.WriteUint32(d.AvgBitrate)
	if d.DecSpecificInfo != nil {
		err := d.DecSpecificInfo.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	for _, desc := range d.OtherDescriptors {
		err := desc.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	if len(d.UnknownData) > 0 {
		sw.WriteBytes(d.UnknownData)
	}
	return sw.AccError()
}

func (d *DecoderConfigDescriptor) Info(w io.Writer, specificLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, d, infoVersionDescriptor, 0)
	level := getInfoLevel(d, specificLevels)
	if level > 0 {
		bd.write(" - ObjectType: %d", d.ObjectType)
		bd.write(" - StreamType: %d", d.StreamType)
	}
	bd.write(" - BufferSizeDB: %d", d.BufferSizeDB)
	bd.write(" - MaxBitrate: %d", d.MaxBitrate)
	bd.write(" - AvgBitrate: %d", d.AvgBitrate)
	if d.DecSpecificInfo != nil {
		err := d.DecSpecificInfo.Info(w, specificLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	for _, od := range d.OtherDescriptors {
		err := od.Info(w, specificLevels, indent+indentStep, indentStep)
		if err != nil {
			return err
		}
	}
	if len(d.UnknownData) > 0 {
		bd.write(" - UnknownData (%dB): %s", len(d.UnknownData), hex.EncodeToString(d.UnknownData))
	}
	return bd.err
}

// DecSpecificInfoDescriptor is a generic DecoderSpecificInfoDescriptor.
//
// The meaning of the MPEG-4 audio descriptor is defined in  ISO/IEC 14496-3 Section 1.6.2.1.

type DecSpecificInfoDescriptor struct {
	sizeFieldSizeMinus1 byte
	DecConfig           []byte
}

func DecodeDecSpecificInfoDescriptor(tag byte, sr bits.SliceReader, maxNrBytes int) (Descriptor, error) {
	dd := DecSpecificInfoDescriptor{}
	if tag != DecSpecificInfoTag {
		return nil, fmt.Errorf("got tag %d instead of DecSpecificInfoTag %d", tag, DecSpecificInfoTag)
	}

	sizeFieldSizeMinus1, size, err := readSizeSize(sr)
	if err != nil {
		return nil, err
	}
	if exceedsMaxNrBytes(sizeFieldSizeMinus1, size, maxNrBytes) {
		return nil, fmt.Errorf("DecSpecificInfoDescriptor size %d exceeds maxNrBytes %d", size, maxNrBytes)
	}
	dd.sizeFieldSizeMinus1 = sizeFieldSizeMinus1

	dataStart := sr.GetPos()
	dd.DecConfig = sr.ReadBytes(int(size))
	bytesLeft := int(size) - (sr.GetPos() - dataStart)
	if bytesLeft > 0 {
		return nil, fmt.Errorf("DecSpecificInfoDescriptor has %d bytes left", bytesLeft)
	}
	return &dd, sr.AccError()
}

func (d *DecSpecificInfoDescriptor) Tag() byte {
	return DecSpecificInfoTag
}

func (d *DecSpecificInfoDescriptor) Type() string {
	return TagType(d.Tag())
}

func (d *DecSpecificInfoDescriptor) Size() uint64 {
	return uint64(len(d.DecConfig))
}

func (d *DecSpecificInfoDescriptor) SizeSize() uint64 {
	return 1 + uint64(d.sizeFieldSizeMinus1) + 1 + d.Size()
}

func (d *DecSpecificInfoDescriptor) EncodeSW(sw bits.SliceWriter) error {
	sw.WriteBits(uint(d.Tag()), 8)
	writeDescriptorSize(sw, d.Size(), d.sizeFieldSizeMinus1)
	sw.WriteBytes(d.DecConfig)
	return sw.AccError()
}

func (d *DecSpecificInfoDescriptor) Info(w io.Writer, specificLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, d, infoVersionDescriptor, 0)
	bd.write(" - DecConfig (%dB): %s", len(d.DecConfig), hex.EncodeToString(d.DecConfig))
	return bd.err
}

type SLConfigDescriptor struct {
	sizeFieldSizeMinus1 byte
	ConfigValue         byte
	MoreData            []byte
}

func DecodeSLConfigDescriptor(tag byte, sr bits.SliceReader, maxNrBytes int) (Descriptor, error) {
	d := SLConfigDescriptor{}
	if tag != SLConfigDescrTag {
		return nil, fmt.Errorf("got tag %d instead of SLConfigDescrTag %d", tag, SLConfigDescrTag)
	}
	sizeFieldSizeMinus1, size, err := readSizeSize(sr)
	if err != nil {
		return nil, err
	}
	if exceedsMaxNrBytes(sizeFieldSizeMinus1, size, maxNrBytes) {
		return nil, fmt.Errorf("DecodeSLConfigDescriptor size %d exceeds maxNrBytes %d", size, maxNrBytes)
	}
	d.sizeFieldSizeMinus1 = sizeFieldSizeMinus1

	d.ConfigValue = sr.ReadUint8()
	if size > 1 {
		d.MoreData = sr.ReadBytes(int(size - 1))
	}
	return &d, sr.AccError()
}

func (d *SLConfigDescriptor) Tag() byte {
	return SLConfigDescrTag
}

func (d *SLConfigDescriptor) Type() string {
	return TagType(d.Tag())
}

func (d *SLConfigDescriptor) Size() uint64 {
	return uint64(1 + len(d.MoreData))
}

func (d *SLConfigDescriptor) SizeSize() uint64 {
	return 1 + uint64(d.sizeFieldSizeMinus1) + 1 + d.Size()
}

func (d *SLConfigDescriptor) EncodeSW(sw bits.SliceWriter) error {
	sw.WriteBits(uint(d.Tag()), 8)
	writeDescriptorSize(sw, d.Size(), d.sizeFieldSizeMinus1)
	sw.WriteUint8(d.ConfigValue)
	if len(d.MoreData) > 0 {
		sw.WriteBytes(d.MoreData)
	}
	return sw.AccError()
}

func (d *SLConfigDescriptor) Info(w io.Writer, specificLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, d, infoVersionDescriptor, 0)
	level := getInfoLevel(d, specificLevels)
	if level > 0 {
		bd.write(" - ConfigValue: %d", d.ConfigValue)
		if len(d.MoreData) > 0 {
			bd.write(" - MoreData: (%dB) %s", len(d.MoreData), hex.EncodeToString(d.MoreData))
		}
	}
	return bd.err
}

// RawDescriptor - raw representation of any descriptor
type RawDescriptor struct {
	tag                 byte
	sizeFieldSizeMinus1 byte
	data                []byte
}

func DecodeRawDescriptor(tag byte, sr bits.SliceReader, maxNrBytes int) (Descriptor, error) {
	sizeFieldSizeMinus1, size, err := readSizeSize(sr)
	if err != nil {
		return nil, err
	}
	if exceedsMaxNrBytes(sizeFieldSizeMinus1, size, maxNrBytes) {
		return nil, fmt.Errorf("DecRawDescriptor size %d exceeds maxNrBytes %d", size, maxNrBytes)
	}
	d := RawDescriptor{
		tag:                 tag,
		sizeFieldSizeMinus1: sizeFieldSizeMinus1,
	}
	d.data = sr.ReadBytes(int(size))
	return &d, sr.AccError()
}

func CreateRawDescriptor(tag, sizeFieldSizeMinus1 byte, data []byte) (RawDescriptor, error) {
	return RawDescriptor{
		tag:                 tag,
		sizeFieldSizeMinus1: sizeFieldSizeMinus1,
		data:                data}, nil
}

func (s *RawDescriptor) Tag() byte {
	return s.tag
}

func (d *RawDescriptor) Type() string {
	return TagType(d.Tag())
}

func (d *RawDescriptor) Size() uint64 {
	return uint64(len(d.data))
}

func (d *RawDescriptor) SizeSize() uint64 {
	return 1 + uint64(d.sizeFieldSizeMinus1) + 1 + d.Size()
}

func (d *RawDescriptor) EncodeSW(sw bits.SliceWriter) error {
	sw.WriteBits(uint(d.tag), 8)
	writeDescriptorSize(sw, d.Size(), d.sizeFieldSizeMinus1)
	sw.WriteBytes(d.data)
	return sw.AccError()
}

func (d *RawDescriptor) Info(w io.Writer, specificLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, d, infoVersionDescriptor, 0)
	level := getInfoLevel(d, specificLevels)
	if level > 0 {
		bd.write(" - data (%dB): %s", len(d.data), hex.EncodeToString(d.data))
	}
	return bd.err
}

// CreateESDescriptor creats an ESDescriptor with a DecoderConfigDescriptor for audio.
func CreateESDescriptor(decConfig []byte) ESDescriptor {
	e := ESDescriptor{
		EsID: 0x01,
		DecConfigDescriptor: &DecoderConfigDescriptor{
			ObjectType: 0x40, // Audio ISO/IEC 14496-3,
			StreamType: 0x15, // 0x5 << 2 + 0x01 (audioType + upstreamFlag + reserved)
			DecSpecificInfo: &DecSpecificInfoDescriptor{
				DecConfig: decConfig,
			},
		},
		SLConfigDescriptor: &SLConfigDescriptor{
			ConfigValue: 0x02,
		},
	}
	return e
}

// readTagAndSize - get size by accumulate 7 bits from each byte. MSB = 1 indicates more bytes.
// Defined in ISO 14496-1 Section 8.3.3
func readSizeSize(sr bits.SliceReader) (sizeFieldSizeMinus1 byte, size uint64, err error) {
	tmp := sr.ReadUint8()
	sizeOfInstance := uint64(tmp & 0x7f)
	for {
		if (tmp >> 7) == 0 {
			break // Last byte of size field
		}
		tmp = sr.ReadUint8()
		sizeFieldSizeMinus1++
		sizeOfInstance = sizeOfInstance<<7 | uint64(tmp&0x7f)
	}
	return sizeFieldSizeMinus1, sizeOfInstance, sr.AccError()
}

// writeDescriptorSize - write descriptor size 7-bit at a time in as many bytes as prescribed
func writeDescriptorSize(sw bits.SliceWriter, size uint64, sizeFieldSizeMinus1 byte) {
	for pos := int(sizeFieldSizeMinus1); pos >= 0; pos-- {
		value := byte(size>>uint32(7*pos)) & 0x7f
		if pos > 0 {
			value |= 0x80
		}
		sw.WriteBits(uint(value), 8)
	}
}
