package avc

import (
	"encoding/binary"
	"errors"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// AVC parsing errors
var (
	ErrCannotParseAVCExtension = errors.New("Cannot parse SPS extensions")
	ErrLengthSize              = errors.New("Can only handle 4byte NAL length size")
)

// DecConfRec - AVCDecoderConfigurationRecord
type DecConfRec struct {
	AVCProfileIndication byte
	ProfileCompatibility byte
	AVCLevelIndication   byte
	SPSnalus             [][]byte
	PPSnalus             [][]byte
	ChromaFormat         byte
	BitDepthLumaMinus1   byte
	BitDepthChromaMinus1 byte
	NumSPSExt            byte
	NoTrailingInfo       bool // To handle strange cases where trailing info is missing
}

// CreateAVCDecConfRec - extract information from sps and insert sps, pps if includePS set
func CreateAVCDecConfRec(spsNalus [][]byte, ppsNalus [][]byte, includePS bool) (*DecConfRec, error) {
	if len(spsNalus) == 0 {
		return nil, fmt.Errorf("no SPS NALU supported. Needed to extract fundamental information")
	}

	sps, err := ParseSPSNALUnit(spsNalus[0], false) // false -> parse only start of VUI
	if err != nil {
		return nil, fmt.Errorf("parse SPS nalu: %w", err)
	}

	drc := DecConfRec{
		AVCProfileIndication: byte(sps.Profile),
		ProfileCompatibility: byte(sps.ProfileCompatibility),
		AVCLevelIndication:   byte(sps.Level),
		SPSnalus:             nil,
		PPSnalus:             nil,
		ChromaFormat:         1,
		BitDepthLumaMinus1:   0,
		BitDepthChromaMinus1: 0,
		NumSPSExt:            0,
		NoTrailingInfo:       false,
	}
	if includePS {
		drc.SPSnalus = spsNalus
		drc.PPSnalus = ppsNalus
	}
	return &drc, nil
}

// DecodeAVCDecConfRec - decode an AVCDecConfRec
func DecodeAVCDecConfRec(data []byte) (DecConfRec, error) {
	configurationVersion := data[0] // Should be 1
	if configurationVersion != 1 {
		return DecConfRec{}, fmt.Errorf("AVC decoder configuration record version %d unknown",
			configurationVersion)
	}
	AVCProfileIndication := data[1]
	ProfileCompatibility := data[2]
	AVCLevelIndication := data[3]
	LengthSizeMinus1 := data[4] & 0x03 // The first 5 bits are 1
	if LengthSizeMinus1 != 0x3 {
		return DecConfRec{}, ErrLengthSize
	}
	numSPS := data[5] & 0x1f // 5 bits following 3 reserved bits
	pos := 6
	spsNALUs := make([][]byte, 0, 1)
	for i := 0; i < int(numSPS); i++ {
		naluLength := int(binary.BigEndian.Uint16(data[pos : pos+2]))
		pos += 2
		spsNALUs = append(spsNALUs, data[pos:pos+naluLength])
		pos += naluLength
	}
	ppsNALUs := make([][]byte, 0, 1)
	numPPS := data[pos]
	pos++
	for i := 0; i < int(numPPS); i++ {
		naluLength := int(binary.BigEndian.Uint16(data[pos : pos+2]))
		pos += 2
		ppsNALUs = append(ppsNALUs, data[pos:pos+naluLength])
		pos += naluLength
	}
	adcr := DecConfRec{
		AVCProfileIndication: AVCProfileIndication,
		ProfileCompatibility: ProfileCompatibility,
		AVCLevelIndication:   AVCLevelIndication,
		SPSnalus:             spsNALUs,
		PPSnalus:             ppsNALUs,
	}
	// The rest of this structure may vary
	// ISO/IEC 14496-15 2017 says that
	// Compatible extensions to this record will extend it and
	// will not change the configuration version code.
	// Readers should be prepared to ignore unrecognized
	// data beyond the definition of the data they understand
	// (e.g. after the parameter sets in this specification).

	switch AVCProfileIndication {
	case 66, 77, 88: // From ISO/IEC 14496-15 2017 Section 5.3.3.1.2
		// No extra bytes
	default:
		if pos == len(data) { // Not according to standard, but have been seen
			adcr.NoTrailingInfo = true
			return adcr, nil
		}
		adcr.ChromaFormat = data[pos] & 0x03
		adcr.BitDepthLumaMinus1 = data[pos+1] & 0x07
		adcr.BitDepthChromaMinus1 = data[pos+2] & 0x07
		adcr.NumSPSExt = data[pos+3]
		if adcr.NumSPSExt != 0 {
			return adcr, ErrCannotParseAVCExtension
		}
	}

	return adcr, nil
}

// Size - total size in bytes
func (a *DecConfRec) Size() uint64 {
	totalSize := 7
	for _, nalu := range a.SPSnalus {
		totalSize += 2 + len(nalu)
	}
	for _, nalu := range a.PPSnalus {
		totalSize += 2 + len(nalu)
	}
	switch a.AVCProfileIndication {
	case 66, 77, 88: // From ISO/IEC 14496-15 2017 Section 5.3.1.1.2
		// No extra bytes
	default:
		if !a.NoTrailingInfo {
			totalSize += 4
		}
	}
	return uint64(totalSize)
}

// Encode - write box to w
func (a *DecConfRec) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(a.Size()))
	err := a.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// Encode - write an AVCDecConfRec to w
func (a *DecConfRec) EncodeSW(sw bits.SliceWriter) error {

	var configurationVersion byte = 1
	sw.WriteUint8(configurationVersion)
	sw.WriteUint8(a.AVCProfileIndication)
	sw.WriteUint8(a.ProfileCompatibility)
	sw.WriteUint8(a.AVCLevelIndication)
	sw.WriteUint8(0xff) // Set length to 4

	var nrSPS byte = byte(len(a.SPSnalus)) | 0xe0 // Added reserved 3 bits
	sw.WriteUint8(nrSPS)
	for _, sps := range a.SPSnalus {
		var length uint16 = uint16(len(sps))
		sw.WriteUint16(length)
		sw.WriteBytes(sps)
	}
	var nrPPS byte = byte(len(a.PPSnalus))
	sw.WriteUint8(nrPPS)
	for _, pps := range a.PPSnalus {
		var length uint16 = uint16(len(pps))
		sw.WriteUint16(length)
		sw.WriteBytes(pps)
	}
	switch a.AVCProfileIndication {
	case 100, 110, 122, 144: // From ISO/IEC 14496-15 2017 Section 5.3.3.1.2
		if a.NoTrailingInfo { // Strange content, but consistent with Size()
			return sw.AccError()
		}
		sw.WriteUint8(0xfc | a.ChromaFormat)
		sw.WriteUint8(0xf8 | a.BitDepthLumaMinus1)
		sw.WriteUint8(0xf8 | a.BitDepthChromaMinus1)
		sw.WriteUint8(a.NumSPSExt)
	default:
		//Nothing more to write
	}

	return sw.AccError()
}
