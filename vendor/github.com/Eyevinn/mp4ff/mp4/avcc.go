package mp4

import (
	"encoding/hex"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/avc"
	"github.com/Eyevinn/mp4ff/bits"
)

// AvcCBox - AVCConfigurationBox (ISO/IEC 14496-15 5.4.2.1.2 and 5.3.3.1.2)
// Contains one AVCDecoderConfigurationRecord
type AvcCBox struct {
	avc.DecConfRec
}

// CreateAvcC - Create an avcC box based on SPS and PPS
func CreateAvcC(spsNALUs [][]byte, ppsNALUs [][]byte, includePS bool) (*AvcCBox, error) {
	avcDecConfRec, err := avc.CreateAVCDecConfRec(spsNALUs, ppsNALUs, includePS)
	if err != nil {
		return nil, fmt.Errorf("CreateAvcDecDecConfRec: %w", err)
	}

	return &AvcCBox{*avcDecConfRec}, nil
}

// DecodeAvcC - box-specific decode
func DecodeAvcC(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	avcDecConfRec, err := avc.DecodeAVCDecConfRec(data)
	if err != nil {
		return nil, err
	}
	return &AvcCBox{avcDecConfRec}, nil
}

// DecodeAvcCSR - box-specific decode
func DecodeAvcCSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	avcDecConfRec, err := avc.DecodeAVCDecConfRec(sr.ReadBytes(hdr.payloadLen()))
	if err != nil {
		return nil, err
	}
	return &AvcCBox{avcDecConfRec}, nil
}

// Type - return box type
func (a *AvcCBox) Type() string {
	return "avcC"
}

// Size - return calculated size
func (a *AvcCBox) Size() uint64 {
	return uint64(boxHeaderSize + a.DecConfRec.Size())
}

// Encode - write box to w
func (a *AvcCBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(a.Size()))
	err := a.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (a *AvcCBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(a, sw)
	if err != nil {
		return err
	}
	return a.DecConfRec.EncodeSW(sw)
}

// Info - write box-specific information
func (a *AvcCBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, a, -1, 0)
	bd.write(" - AVCProfileIndication: %d", a.AVCProfileIndication)
	bd.write(" - profileCompatibility: %02x", a.ProfileCompatibility)
	bd.write(" - AVCLevelIndication: %d", a.AVCLevelIndication)
	for _, sps := range a.SPSnalus {
		bd.write(" - SPS: %s", hex.EncodeToString(sps))
	}
	for _, pps := range a.PPSnalus {
		bd.write(" - PPS: %s", hex.EncodeToString(pps))
	}
	return bd.err
}
