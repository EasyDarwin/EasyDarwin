package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// HdlrBox - Handler Reference Box (hdlr - mandatory)
//
// Contained in: Media Box (mdia) or Meta Box (meta)
//
// This box describes the type of data contained in the trak.
// Most common hnadler types are: "vide" (video track), "soun" (audio track), "subt" (subtitle track),
// "text" (text track). "meta" (timed Metadata track), clcp (Closed Captions (QuickTime))
type HdlrBox struct {
	Version              byte
	Flags                uint32
	PreDefined           uint32
	HandlerType          string
	Name                 string // Null-terminated UTF-8 string according to ISO/IEC 14496-12 Sec. 8.4.3.3
	LacksNullTermination bool   // This should be false, but we allow true as well
}

// CreateHdlr - create mediaType-specific hdlr box
func CreateHdlr(mediaOrHdlrType string) (*HdlrBox, error) {
	hdlr := &HdlrBox{}
	switch mediaOrHdlrType {
	case "video", "vide":
		hdlr.HandlerType = "vide"
		hdlr.Name = "mp4ff video handler"
	case "audio", "soun":
		hdlr.HandlerType = "soun"
		hdlr.Name = "mp4ff audio handler"
	case "subtitle", "subt":
		hdlr.HandlerType = "subt"
		hdlr.Name = "mp4ff subtitle handler"
	case "text", "wvtt":
		hdlr.HandlerType = "text"
		hdlr.Name = "mp4ff text handler"
	case "meta":
		hdlr.HandlerType = "meta"
		hdlr.Name = "mp4ff timed metadata handler"
	case "clcp":
		hdlr.HandlerType = "subt"
		hdlr.Name = "mp4ff closed captions handler"
	default:
		if len(mediaOrHdlrType) != 4 {
			return nil, fmt.Errorf("handler type is not four characters: %s", mediaOrHdlrType)
		}
		hdlr.HandlerType = mediaOrHdlrType
		hdlr.Name = fmt.Sprintf("mp4ff %s handler", mediaOrHdlrType)
	}
	return hdlr, nil
}

// DecodeHdlr - box-specific decode
func DecodeHdlr(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeHdlrSR(hdr, startPos, sr)
}

// DecodeHdlrSR - box-specific decode
func DecodeHdlrSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	h := HdlrBox{
		Version:     byte(versionAndFlags >> 24),
		Flags:       versionAndFlags & flagsMask,
		PreDefined:  sr.ReadUint32(),
		HandlerType: sr.ReadFixedLengthString(4),
	}
	sr.SkipBytes(12) // 12 bytes of zero
	nrBytesLeft := hdr.payloadLen() - 24
	if nrBytesLeft > 0 {
		bytesLeft := sr.ReadBytes(nrBytesLeft)
		lastChar := bytesLeft[len(bytesLeft)-1]
		if lastChar != 0 {
			h.LacksNullTermination = true
			h.Name = string(bytesLeft)
		} else {
			h.Name = string(bytesLeft[:len(bytesLeft)-1])
		}
	} else {
		h.LacksNullTermination = true
	}
	return &h, sr.AccError()
}

// Type - box type
func (b *HdlrBox) Type() string {
	return "hdlr"
}

// Size - calculated size of box
func (b *HdlrBox) Size() uint64 {
	size := uint64(boxHeaderSize + 24 + len(b.Name) + 1)
	if b.LacksNullTermination {
		size--
	}
	return size
}

// Encode - write box to w
func (b *HdlrBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *HdlrBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) | b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(b.PreDefined)
	sw.WriteString(b.HandlerType, false)
	sw.WriteZeroBytes(12)
	sw.WriteString(b.Name, !b.LacksNullTermination)
	return sw.AccError()
}

// Info - write box-specific information
func (b *HdlrBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - handlerType: %s", b.HandlerType)
	bd.write(" - handlerName: %q", b.Name)
	return bd.err
}
