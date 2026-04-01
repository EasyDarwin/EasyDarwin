package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// BtrtBox - BitRateBox - ISO/IEC 14496-12 Section 8.5.2.2
type BtrtBox struct {
	BufferSizeDB uint32
	MaxBitrate   uint32
	AvgBitrate   uint32
}

// DecodeBtrt - box-specific decode
func DecodeBtrt(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeBtrtSR(hdr, startPos, sr)
}

// DecodeBtrtSR - box-specific decode
func DecodeBtrtSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	b := &BtrtBox{
		BufferSizeDB: sr.ReadUint32(),
		MaxBitrate:   sr.ReadUint32(),
		AvgBitrate:   sr.ReadUint32(),
	}
	return b, sr.AccError()
}

// Type - return box type
func (b *BtrtBox) Type() string {
	return "btrt"
}

// Size - return calculated size
func (b *BtrtBox) Size() uint64 {
	return 20
}

// Encode - write box to w
func (b *BtrtBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *BtrtBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteUint32(b.BufferSizeDB)
	sw.WriteUint32(b.MaxBitrate)
	sw.WriteUint32(b.AvgBitrate)
	return sw.AccError()
}

// Info - write box-specific information
func (b *BtrtBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - bufferSizeDB: %d", b.BufferSizeDB)
	bd.write(" - maxBitrate: %d", b.MaxBitrate)
	bd.write(" - AvgBitrate: %d", b.AvgBitrate)
	return bd.err
}
