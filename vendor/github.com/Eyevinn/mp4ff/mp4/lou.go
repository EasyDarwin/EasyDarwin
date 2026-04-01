package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// TlouBox - Track loudness info Box
//
// Contained in : Ludt Box (ludt)
type TlouBox struct {
	loudnessBaseBox
}

// TlouBox - Album loudness info Box
//
// Contained in : Ludt Box (ludt)
type AlouBox struct {
	loudnessBaseBox
}

type LoudnessBase struct {
	EQSetID                uint8
	DownmixID              uint8
	DRCSetID               uint8
	BsSamplePeakLevel      int16
	BsTruePeakLevel        int16
	MeasurementSystemForTP uint8
	ReliabilityForTP       uint8
	Measurements           []Measurement
}

type Measurement struct {
	MethodDefinition  uint8
	MethodValue       uint8
	MeasurementSystem uint8
	Reliability       uint8
}

// loudnessBaseBox according to ISO/IEC 14496-12 Section 12.2.7.2
type loudnessBaseBox struct {
	Version       byte
	Flags         uint32
	LoudnessBases []*LoudnessBase
}

func decodeLoudnessBaseBoxSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (*loudnessBaseBox, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	b := &loudnessBaseBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	var loudnessBaseCount uint8
	if b.Version >= 1 {
		loudnessInfoTypeAndCount := sr.ReadUint8()
		loudnessInfoType := (loudnessInfoTypeAndCount >> 6) & 0x3
		if loudnessInfoType != 0 {
			return nil, fmt.Errorf("loudnessInfoType %d not supported", loudnessInfoType)
		}
		loudnessBaseCount = 0x3f & loudnessInfoTypeAndCount
	} else {
		loudnessBaseCount = 1
	}
	b.LoudnessBases = make([]*LoudnessBase, 0, loudnessBaseCount)

	for a := uint8(0); a < loudnessBaseCount; a++ {
		l := &LoudnessBase{}
		if b.Version >= 1 {
			l.EQSetID = 0x3f & sr.ReadUint8()
		}
		downmixIDAndDRCSetID := sr.ReadUint16()
		l.DownmixID = uint8(downmixIDAndDRCSetID >> 6)
		l.DRCSetID = uint8(downmixIDAndDRCSetID & 0x3f)
		peakLevels := sr.ReadUint24()
		l.BsSamplePeakLevel = int16(peakLevels >> 12)
		l.BsTruePeakLevel = int16(peakLevels & 0x0fff)
		measurementSystemAndReliablityForTP := sr.ReadUint8()
		l.MeasurementSystemForTP = measurementSystemAndReliablityForTP >> 4
		l.ReliabilityForTP = measurementSystemAndReliablityForTP & 0x0f
		measurementCount := sr.ReadUint8()
		l.Measurements = make([]Measurement, 0, measurementCount)
		for i := uint8(0); i < measurementCount; i++ {
			m := Measurement{}
			m.MethodDefinition = sr.ReadUint8()
			m.MethodValue = sr.ReadUint8()
			measurementSystemAndReliablity := sr.ReadUint8()
			m.MeasurementSystem = measurementSystemAndReliablity >> 4
			m.Reliability = measurementSystemAndReliablity & 0x0f
			l.Measurements = append(l.Measurements, m)
		}
		b.LoudnessBases = append(b.LoudnessBases, l)
	}
	return b, nil
}

func (b *loudnessBaseBox) size() uint64 {
	size := 4
	if b.Version >= 1 {
		size += 1
	}
	for _, l := range b.LoudnessBases {
		if b.Version >= 1 {
			size += 8 + len(l.Measurements)*3
		} else {
			size += 7 + len(l.Measurements)*3
		}
	}
	return uint64(size)
}

func (b *loudnessBaseBox) encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.size()))
	err := b.encodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

func (b *loudnessBaseBox) encodeSW(sw bits.SliceWriter) error {
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	if b.Version >= 1 {
		sw.WriteUint8(0x3f & uint8(len(b.LoudnessBases)))
	}
	for a := 0; a < len(b.LoudnessBases); a++ {
		l := b.LoudnessBases[a]
		if b.Version >= 1 {
			sw.WriteUint8(0x3f & l.EQSetID)
		}
		downmixIDAndDRCSetID := (uint16(l.DownmixID) << 6) | uint16(0x3f&l.DRCSetID)
		sw.WriteUint16(downmixIDAndDRCSetID)
		peakLevels := (uint32(l.BsSamplePeakLevel) << 12) | uint32(0x0fff&l.BsTruePeakLevel)
		sw.WriteUint24(peakLevels)
		measurementSystemAndReliablityForTP := (l.MeasurementSystemForTP << 4) | (0x0f & l.ReliabilityForTP)
		sw.WriteUint8(measurementSystemAndReliablityForTP)
		sw.WriteUint8(uint8(len(l.Measurements)))
		for i := 0; i < len(l.Measurements); i++ {
			m := l.Measurements[i]
			sw.WriteUint8(m.MethodDefinition)
			sw.WriteUint8(m.MethodValue)
			measurementSystemAndReliablity := (m.MeasurementSystem << 4) | (0x0f & m.Reliability)
			sw.WriteUint8(measurementSystemAndReliablity)
		}
	}
	return sw.AccError()
}

func (b *loudnessBaseBox) info(realBox Box, w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, realBox, int(b.Version), 0)
	bd.write(" - LoudnessBaseCount: %d", len(b.LoudnessBases))
	level := getInfoLevel(realBox, specificBoxLevels)
	if level >= 1 {
		loudnessIndent := "     "
		for a, l := range b.LoudnessBases {
			bd.write(" - loudnessBase[%d]:", a+1)
			if b.Version == 1 {
				bd.write(loudnessIndent+"EQSetID=%d", l.EQSetID)
			}
			bd.write(loudnessIndent+"DownmixID=%d", l.DownmixID)
			bd.write(loudnessIndent+"DRCSetID=%d", l.DRCSetID)
			bd.write(loudnessIndent+"BsSamplePeakLevel=%d", l.BsSamplePeakLevel)
			bd.write(loudnessIndent+"BsTruePeakLevel=%d", l.BsTruePeakLevel)
			bd.write(loudnessIndent+"MeasurementSystemForTP=%d", l.MeasurementSystemForTP)
			bd.write(loudnessIndent+"ReliabilityForTP=%d", l.ReliabilityForTP)
			bd.write(loudnessIndent+"MeasurementCount=%d", len(l.Measurements))
			for i := 0; i < len(l.Measurements); i++ {
				msg := fmt.Sprintf(loudnessIndent+" - measurement[%d]: ", i+1)
				msg += fmt.Sprintf("MethodDefinition=%d ", l.Measurements[i].MethodDefinition)
				msg += fmt.Sprintf("MethodValue=%d ", l.Measurements[i].MethodValue)
				msg += fmt.Sprintf("MeasurementSystem=%d ", l.Measurements[i].MeasurementSystem)
				msg += fmt.Sprintf("Reliability=%d ", l.Measurements[i].Reliability)
				bd.write(msg)
			}
		}
	}
	return bd.err
}

// DecodeTlou - box-specific decode
func DecodeTlou(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeTlouSR(hdr, startPos, sr)
}

// DecodeTlouSR - box-specific decode
func DecodeTlouSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	loudnessBaseBox, err := decodeLoudnessBaseBoxSR(hdr, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	return &TlouBox{loudnessBaseBox: *loudnessBaseBox}, nil
}

// Encode - write tlou container to w
func (b *TlouBox) Encode(w io.Writer) error {
	err := EncodeHeader(b, w)
	if err != nil {
		return err
	}
	return b.loudnessBaseBox.encode(w)
}

// Encode - write tlou container to sw
func (b *TlouBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	return b.loudnessBaseBox.encodeSW(sw)
}

// Type - return box type
func (b *TlouBox) Type() string {
	return "tlou"
}

// Size - calculated size of box
func (b *TlouBox) Size() uint64 {
	return b.loudnessBaseBox.size() + boxHeaderSize
}

// Info - write box-specific information
func (b *TlouBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return b.loudnessBaseBox.info(b, w, specificBoxLevels, indent, indentStep)
}

// DecodeAlou - box-specific decode
func DecodeAlou(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeAlouBoxSR(hdr, startPos, sr)
}

// DecodeAlouSR - box-specific decode
func DecodeAlouBoxSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	loudnessBaseBox, err := decodeLoudnessBaseBoxSR(hdr, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	return &AlouBox{loudnessBaseBox: *loudnessBaseBox}, nil
}

// Encode - write alou container to w
func (b *AlouBox) Encode(w io.Writer) error {
	err := EncodeHeader(b, w)
	if err != nil {
		return err
	}
	return b.loudnessBaseBox.encode(w)
}

// Encode - write alou container to sw
func (b *AlouBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	return b.loudnessBaseBox.encodeSW(sw)
}

// Type - return box type
func (b *AlouBox) Type() string {
	return "alou"
}

// Size - calculated size of box
func (b *AlouBox) Size() uint64 {
	return b.loudnessBaseBox.size() + boxHeaderSize
}

// Info - write box-specific information
func (b *AlouBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return b.loudnessBaseBox.info(b, w, specificBoxLevels, indent, indentStep)
}
