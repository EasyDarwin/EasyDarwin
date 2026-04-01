package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

const (
	PrftTimeEncoderInput       = 0
	PrftTimeEncoderOutput      = 1
	PrftTimeMoofFinalized      = 2
	PrftTimeMoofWritten        = 4
	PrftTimeArbitraryConsitent = 8
	PrftTimeCaptured           = 24
)

var PrftFlagsInterpretation = map[uint32]string{
	PrftTimeEncoderInput:       "time_encoder_input",
	PrftTimeEncoderOutput:      "time_encoder_output",
	PrftTimeMoofFinalized:      "time_moof_finalized",
	PrftTimeMoofWritten:        "time_moof_written",
	PrftTimeArbitraryConsitent: "time_arbitrary_consistent",
	PrftTimeCaptured:           "time_captured",
}

// PrftBox - Producer Reference Box (prft)
//
// Contained in File before moof box
type PrftBox struct {
	Version          byte
	Flags            uint32
	ReferenceTrackID uint32
	NTPTimestamp     NTP64
	MediaTime        uint64
}

// CreatePrftBox creates a new PrftBox.
func CreatePrftBox(version byte, flags, refTrackID uint32, ntp NTP64, mediatime uint64) *PrftBox {
	return &PrftBox{
		Version:          version,
		Flags:            flags,
		ReferenceTrackID: refTrackID,
		NTPTimestamp:     ntp,
		MediaTime:        mediatime,
	}
}

// DecodePrft - box-specific decode
func DecodePrft(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodePrftSR(hdr, startPos, sr)
}

// DecodePrftSR - box-specific decode
func DecodePrftSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	flags := versionAndFlags & flagsMask
	refTrackID := sr.ReadUint32()
	ntp := NTP64(sr.ReadUint64())
	var mediatime uint64
	if version == 0 {
		mediatime = uint64(sr.ReadUint32())
	} else {
		mediatime = sr.ReadUint64()
	}

	p := PrftBox{
		Version:          version,
		Flags:            flags,
		ReferenceTrackID: refTrackID,
		NTPTimestamp:     ntp,
		MediaTime:        mediatime,
	}
	return &p, sr.AccError()
}

// Type - return box type
func (b *PrftBox) Type() string {
	return "prft"
}

// Size - return calculated size
func (b *PrftBox) Size() uint64 {
	return uint64(boxHeaderSize + 20 + 4*int(b.Version))
}

// Encode - write box to w
func (b *PrftBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *PrftBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(b.ReferenceTrackID)
	sw.WriteUint64(uint64(b.NTPTimestamp))
	if b.Version == 0 {
		sw.WriteUint32(uint32(b.MediaTime))
	} else {
		sw.WriteUint64(b.MediaTime)
	}
	return sw.AccError()
}

// Info - write box-specific information
func (b *PrftBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - referenceTrackID: %d", b.ReferenceTrackID)
	bd.write(" - type: %s", b.InterpretFlags())
	bd.write(" - ntpTimestamp: %s (%d)", b.NTPTimestamp, b.NTPTimestamp)
	bd.write(" - mediaTime: %d", b.MediaTime)
	return bd.err
}

// InterpretFlags - return string representation of flags.
func (b *PrftBox) InterpretFlags() string {
	if interpretation, ok := PrftFlagsInterpretation[b.Flags]; ok {
		return interpretation
	}
	return "unknown"
}
