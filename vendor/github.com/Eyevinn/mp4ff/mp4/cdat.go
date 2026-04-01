package mp4

import (
	"encoding/hex"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// CdatBox - Closed Captioning Sample Data according to QuickTime spec:
// https://developer.apple.com/library/archive/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html#//apple_ref/doc/uid/TP40000939-CH205-SW87
type CdatBox struct {
	Data []byte
}

// DecodeCdat - box-specific decode
func DecodeCdat(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	b := &CdatBox{
		Data: data,
	}
	return b, nil
}

// DecodeCdat - box-specific decode
func DecodeCdatSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	b := &CdatBox{
		Data: sr.ReadBytes(hdr.payloadLen()),
	}
	return b, sr.AccError()
}

// Type - box type
func (b *CdatBox) Type() string {
	return "cdat"
}

// Size - calculated size of box
func (b *CdatBox) Size() uint64 {
	return uint64(boxHeaderSize + len(b.Data))
}

// Encode - write box to w
func (b *CdatBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *CdatBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteBytes(b.Data)
	return sw.AccError()
}

// Info - write specific box information
func (b *CdatBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - data: %s", hex.EncodeToString(b.Data))
	return bd.err
}
