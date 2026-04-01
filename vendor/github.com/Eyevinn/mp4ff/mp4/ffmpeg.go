package mp4

// ffmpeg boxes according to https://kdenlive.org/en/project/adding-meta-data-to-mp4-video
import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// CTooBox - ©too box defines the ffmpeg encoding tool information
type CTooBox struct {
	Children []Box
}

// DataBox - data box used by ffmpeg for providing information.
type DataBox struct {
	Data []byte
}

// DecodeData - decode Data (from mov_write_string_data_tag in movenc.c in ffmpeg)
func DecodeData(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeDataSR(hdr, startPos, sr)
}

// DecodeDataSR - decode Data (from mov_write_string_data_tag in movenc.c in ffmpeg)
func DecodeDataSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	_ = sr.ReadUint32() // Should be 1
	_ = sr.ReadUint32() // Should be 0
	return &DataBox{sr.ReadBytes(hdr.payloadLen() - 8)}, sr.AccError()
}

// Type - box type
func (b *DataBox) Type() string {
	return "data"
}

// Size - calculated size of box
func (b *DataBox) Size() uint64 {
	return uint64(boxHeaderSize + 8 + len(b.Data))
}

// Encode - write box to w
func (b *DataBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *DataBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteUint32(0x00000001)
	sw.WriteUint32(0x00000000)
	sw.WriteBytes(b.Data)
	return sw.AccError()
}

// Info - box-specific Info
func (b *DataBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - data: %s", string(b.Data))
	return bd.err
}
