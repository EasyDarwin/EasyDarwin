package mp4

import (
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// UseSubSampleEncryption - flag for subsample encryption
const UseSubSampleEncryption = 0x2

// SubSamplePattern - pattern of subsample encryption
type SubSamplePattern struct {
	BytesOfClearData     uint16
	BytesOfProtectedData uint32
}

// InitializationVector (8 or 16 bytes)
type InitializationVector []byte

// SencBox - Sample Encryption Box (senc) (in trak or traf box)
// Should only be decoded after saio and saiz provide relevant offset and sizes
// Here we make a two-step decode, with first step reading, and other parsing.
// See ISO/IEC 23001-7 Section 7.2 and CMAF specification
// Full Box + SampleCount
type SencBox struct {
	Version          byte
	readButNotParsed bool
	perSampleIVSize  byte
	Flags            uint32
	SampleCount      uint32
	StartPos         uint64
	rawData          []byte                 // intermediate storage when reading
	IVs              []InitializationVector // 8 or 16 bytes if present
	SubSamples       [][]SubSamplePattern
}

// CreateSencBox - create an empty SencBox
func CreateSencBox() *SencBox {
	return &SencBox{}
}

// NewSencBox returns a SencBox with capacity for IVs and SubSamples.
func NewSencBox(ivCapacity, subSampleCapacity int) *SencBox {
	s := SencBox{}
	if ivCapacity > 0 {
		s.IVs = make([]InitializationVector, 0, ivCapacity)
	}
	if subSampleCapacity > 0 {
		s.SubSamples = make([][]SubSamplePattern, 0, subSampleCapacity)
	}
	return &s
}

// SencSample - sample in SencBox
type SencSample struct {
	IV         InitializationVector // 0,8,16 byte length
	SubSamples []SubSamplePattern
}

// AddSample - add a senc sample with possible IV and subsamples
func (s *SencBox) AddSample(sample SencSample) error {
	if len(sample.IV) != 0 {
		if s.SampleCount == 0 {
			s.perSampleIVSize = byte(len(sample.IV))
		} else {
			if len(sample.IV) != int(s.perSampleIVSize) {
				return fmt.Errorf("mix of IV lengths")
			}
		}
		if len(sample.IV) != 0 {
			s.IVs = append(s.IVs, sample.IV)
		}
	}

	if len(sample.SubSamples) > 0 {
		s.SubSamples = append(s.SubSamples, sample.SubSamples)
		s.Flags |= UseSubSampleEncryption
	}
	s.SampleCount++
	return nil
}

// DecodeSenc - box-specific decode
func DecodeSenc(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}

	versionAndFlags := binary.BigEndian.Uint32(data[0:4])
	sampleCount := binary.BigEndian.Uint32(data[4:8])

	senc := SencBox{
		Version:          byte(versionAndFlags >> 24),
		rawData:          data[8:], // After the first 8 bytes of box content
		Flags:            versionAndFlags & flagsMask,
		StartPos:         startPos,
		SampleCount:      sampleCount,
		readButNotParsed: true,
	}

	if senc.SampleCount == 0 || len(senc.rawData) == 0 {
		senc.readButNotParsed = false
		return &senc, nil
	}
	return &senc, nil
}

// DecodeSencSR - box-specific decode
func DecodeSencSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	sampleCount := sr.ReadUint32()
	senc := SencBox{
		Version:          byte(versionAndFlags >> 24),
		rawData:          sr.ReadBytes(hdr.payloadLen() - 8), // After the first 8 bytes of box content
		Flags:            versionAndFlags & flagsMask,
		StartPos:         startPos,
		SampleCount:      sampleCount,
		readButNotParsed: true,
	}

	if senc.SampleCount == 0 || len(senc.rawData) == 0 {
		senc.readButNotParsed = false
		return &senc, sr.AccError()
	}
	return &senc, sr.AccError()
}

// ParseReadBox - second phase when perSampleIVSize should be known from tenc or sgpd boxes
// if perSampleIVSize is 0, we try to find the appropriate error given data length
func (s *SencBox) ParseReadBox(perSampleIVSize byte, saiz *SaizBox) error {
	if !s.readButNotParsed {
		return fmt.Errorf("senc box already parsed")
	}
	if perSampleIVSize != 0 {
		s.perSampleIVSize = byte(perSampleIVSize)
	}
	sr := bits.NewFixedSliceReader(s.rawData)
	nrBytesLeft := uint32(sr.NrRemainingBytes())

	if s.Flags&UseSubSampleEncryption == 0 {
		// No subsamples
		if perSampleIVSize == 0 { // Infer the size
			perSampleIVSize = byte(nrBytesLeft / s.SampleCount)
			s.perSampleIVSize = perSampleIVSize
		}

		s.IVs = make([]InitializationVector, 0, s.SampleCount)
		switch perSampleIVSize {
		case 0:
			// Nothing to do
		case 8:
			for i := 0; i < int(s.SampleCount); i++ {
				s.IVs = append(s.IVs, sr.ReadBytes(8))
			}
		case 16:
			for i := 0; i < int(s.SampleCount); i++ {
				s.IVs = append(s.IVs, sr.ReadBytes(16))
			}
		default:
			return fmt.Errorf("strange derived PerSampleIVSize: %d", perSampleIVSize)
		}
		s.readButNotParsed = false
		return nil
	}
	// 6 bytes of subsamplecount per subsample and known perSampleIVSize
	// The total length for each sample should correspond to
	// sizes in saiz (defaultSampleInfoSize or SampleInfo value)
	// We don't check that though, but it could be implemented here.
	if perSampleIVSize != 0 {
		if ok := s.parseAndFillSamples(sr, perSampleIVSize); !ok {
			return fmt.Errorf("error decoding senc with perSampleIVSize = %d", perSampleIVSize)
		}
		s.readButNotParsed = false
		return nil
	}

	// Finally, 6 bytes of subsamplecount per subsample and unknown perSampleIVSize
	startPos := sr.GetPos()
	ok := false
	for perSampleIVSize := byte(0); perSampleIVSize <= 16; perSampleIVSize += 8 {
		sr.SetPos(startPos)
		ok = s.parseAndFillSamples(sr, perSampleIVSize)
		if ok {
			break // We have found a working perSampleIVSize
		}
	}
	if !ok {
		return fmt.Errorf("could not decode senc")
	}
	s.readButNotParsed = false
	return nil
}

// parseAndFillSamples - parse and fill senc samples given perSampleIVSize
func (s *SencBox) parseAndFillSamples(sr bits.SliceReader, perSampleIVSize byte) (ok bool) {
	ok = true
	s.SubSamples = make([][]SubSamplePattern, s.SampleCount)
	for i := 0; i < int(s.SampleCount); i++ {
		if perSampleIVSize > 0 {
			if sr.NrRemainingBytes() < int(perSampleIVSize) {
				ok = false
				break
			}
			s.IVs = append(s.IVs, sr.ReadBytes(int(perSampleIVSize)))
		}
		if sr.NrRemainingBytes() < 2 {
			ok = false
			break
		}
		subsampleCount := int(sr.ReadUint16())
		if sr.NrRemainingBytes() < subsampleCount*6 {
			ok = false
			break
		}
		s.SubSamples[i] = make([]SubSamplePattern, subsampleCount)
		for j := 0; j < subsampleCount; j++ {
			s.SubSamples[i][j].BytesOfClearData = sr.ReadUint16()
			s.SubSamples[i][j].BytesOfProtectedData = sr.ReadUint32()
		}
	}
	if !ok || sr.NrRemainingBytes() != 0 {
		// Cleanup the IVs and SubSamples which may have been partially set
		s.IVs = nil
		s.SubSamples = nil
		ok = false
	}
	s.perSampleIVSize = byte(perSampleIVSize)
	return ok
}

// Type - box-specific type
func (s *SencBox) Type() string {
	return "senc"
}

// setSubSamplesUsedFlag - set flag if subsamples are used
func (s *SencBox) setSubSamplesUsedFlag() {
	for _, subSamples := range s.SubSamples {
		if len(subSamples) > 0 {
			s.Flags |= UseSubSampleEncryption
			break
		}
	}
}

// Size - box-specific type
func (s *SencBox) Size() uint64 {
	if s.readButNotParsed {
		return boxHeaderSize + 8 + uint64(len(s.rawData)) // read 8 bytes after header
	}
	totalSize := boxHeaderSize + 8
	perSampleIVSize := s.GetPerSampleIVSize()
	for i := 0; i < int(s.SampleCount); i++ {
		totalSize += perSampleIVSize
		if s.Flags&UseSubSampleEncryption != 0 {
			totalSize += 2 + 6*len(s.SubSamples[i])
		}
	}
	return uint64(totalSize)
}

// Encode - write box to w
func (s *SencBox) Encode(w io.Writer) error {
	// First check if subsamplencryption is to be used since it influences the box size
	s.setSubSamplesUsedFlag()
	sw := bits.NewFixedSliceWriter(int(s.Size()))
	err := s.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (s *SencBox) EncodeSW(sw bits.SliceWriter) error {
	s.setSubSamplesUsedFlag()
	err := EncodeHeaderSW(s, sw)
	if err != nil {
		return err
	}
	err = s.EncodeSWNoHdr(sw)
	return err
}

// EncodeSWNoHdr encodes without header (useful for PIFF box)
func (s *SencBox) EncodeSWNoHdr(sw bits.SliceWriter) error {
	versionAndFlags := (uint32(s.Version) << 24) + s.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(s.SampleCount)
	if s.readButNotParsed {
		sw.WriteBytes(s.rawData)
		return sw.AccError()
	}
	perSampleIVSize := s.GetPerSampleIVSize()
	for i := 0; i < int(s.SampleCount); i++ {
		if perSampleIVSize > 0 {
			sw.WriteBytes(s.IVs[i])
		}
		if s.Flags&UseSubSampleEncryption != 0 {
			sw.WriteUint16(uint16(len(s.SubSamples[i])))
			for _, subSample := range s.SubSamples[i] {
				sw.WriteUint16(subSample.BytesOfClearData)
				sw.WriteUint32(subSample.BytesOfProtectedData)
			}
		}
	}
	return sw.AccError()
}

// Info - write box-specific information
func (s *SencBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, s, int(s.Version), s.Flags)
	bd.write(" - sampleCount: %d", s.SampleCount)
	if s.readButNotParsed {
		bd.write(" - NOT YET PARSED, call ParseReadBox to parse it")
		return nil
	}
	for _, subSamples := range s.SubSamples {
		if len(subSamples) > 0 {
			s.Flags |= UseSubSampleEncryption
		}
	}
	perSampleIVSize := s.GetPerSampleIVSize()
	bd.write(" - perSampleIVSize: %d", perSampleIVSize)
	level := getInfoLevel(s, specificBoxLevels)
	if level > 0 && (perSampleIVSize > 0 || s.Flags&UseSubSampleEncryption != 0) {
		for i := 0; i < int(s.SampleCount); i++ {
			line := fmt.Sprintf(" - sample[%d]:", i+1)
			if perSampleIVSize > 0 {
				line += fmt.Sprintf(" iv=%s", hex.EncodeToString(s.IVs[i]))
			}
			bd.write(line)
			if s.Flags&UseSubSampleEncryption != 0 {
				for j, subSample := range s.SubSamples[i] {
					bd.write("   - subSample[%d]: nrBytesClear=%d nrBytesProtected=%d", j+1,
						subSample.BytesOfClearData, subSample.BytesOfProtectedData)
				}
			}
		}
	}
	return bd.err
}

// GetPerSampleIVSize - return perSampleIVSize
func (s *SencBox) GetPerSampleIVSize() int {
	return int(s.perSampleIVSize)
}
