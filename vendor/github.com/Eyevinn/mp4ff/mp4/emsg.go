package mp4

import (
	"encoding/hex"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// EmsgBox - DASHEventMessageBox as defined in ISO/IEC 23009-1
type EmsgBox struct {
	Version               byte
	Flags                 uint32
	TimeScale             uint32
	PresentationTimeDelta uint32
	PresentationTime      uint64
	EventDuration         uint32
	ID                    uint32
	SchemeIDURI           string
	Value                 string
	MessageData           []byte
}

// DecodeEmsg - box-specific decode
func DecodeEmsg(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeEmsgSR(hdr, startPos, sr)
}

// DecodeEmsgSR - box-specific decode
func DecodeEmsgSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	initPos := sr.GetPos()
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	b := &EmsgBox{
		Version:     version,
		Flags:       versionAndFlags & flagsMask,
		MessageData: nil,
	}
	if version == 1 {
		b.TimeScale = sr.ReadUint32()
		b.PresentationTime = sr.ReadUint64()
		b.EventDuration = sr.ReadUint32()
		b.ID = sr.ReadUint32()
		maxLen := hdr.payloadLen() - (sr.GetPos() - initPos) - 1
		b.SchemeIDURI = sr.ReadZeroTerminatedString(maxLen)
		maxLen = hdr.payloadLen() - (sr.GetPos() - initPos)
		b.Value = sr.ReadZeroTerminatedString(maxLen)
	} else if version == 0 {
		maxLen := hdr.payloadLen() - (sr.GetPos() - initPos) - 17
		b.SchemeIDURI = sr.ReadZeroTerminatedString(maxLen)
		maxLen = hdr.payloadLen() - (sr.GetPos() - initPos) - 16
		b.Value = sr.ReadZeroTerminatedString(maxLen)
		b.TimeScale = sr.ReadUint32()
		b.PresentationTimeDelta = sr.ReadUint32()
		b.EventDuration = sr.ReadUint32()
		b.ID = sr.ReadUint32()
	} else {
		return nil, fmt.Errorf("unknown version %d for emsg", version)
	}

	currPos := sr.GetPos()
	nrBytesRead := currPos - initPos + boxHeaderSize
	remainingBytes := int(hdr.Size) - nrBytesRead

	if remainingBytes > 0 {
		b.MessageData = sr.ReadBytes(remainingBytes)
	}

	return b, sr.AccError()
}

// Type - box type
func (b *EmsgBox) Type() string {
	return "emsg"
}

// Size - calculated size of box
func (b *EmsgBox) Size() uint64 {
	if b.Version == 1 {
		return uint64(boxHeaderSize + 4 + 4 + 8 + 4 + 4 + len(b.SchemeIDURI) + 1 + len(b.Value) + 1 + len(b.MessageData))
	}
	return uint64(boxHeaderSize + 4 + len(b.SchemeIDURI) + 1 + len(b.Value) + 1 + 4 + 4 + 4 + 4 + len(b.MessageData)) // m.Version == 0
}

// Encode - write box to w
func (b *EmsgBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *EmsgBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	if b.Version == 1 {
		sw.WriteUint32(b.TimeScale)
		sw.WriteUint64(b.PresentationTime)
		sw.WriteUint32(b.EventDuration)
		sw.WriteUint32(b.ID)
		sw.WriteString(b.SchemeIDURI, true)
		sw.WriteString(b.Value, true)
	} else {
		sw.WriteString(b.SchemeIDURI, true)
		sw.WriteString(b.Value, true)
		sw.WriteUint32(b.TimeScale)
		sw.WriteUint32(b.PresentationTimeDelta)
		sw.WriteUint32(b.EventDuration)
		sw.WriteUint32(b.ID)
	}
	if len(b.MessageData) > 0 {
		sw.WriteBytes(b.MessageData)
	}
	return sw.AccError()
}

// Info - write box-specific information
func (b *EmsgBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - timeScale: %d", b.TimeScale)
	if b.Version > 0 {
		bd.write(" - presentationTime: %d", b.PresentationTime)
	}
	bd.write(" - eventDuration: %d", b.EventDuration)
	bd.write(" - id: %d", b.ID)
	bd.write(" - schedIdURI: %s", b.SchemeIDURI)
	bd.write(" - value: %s", b.Value)
	if b.Version == 0 {
		bd.write(" - presentationTimeDelta: %d", b.PresentationTimeDelta)
	}
	level := getInfoLevel(b, specificBoxLevels)
	msgDataLen := len(b.MessageData)

	if msgDataLen > 0 {
		if level > 0 {
			bd.write(" - messageData size=%d: %s", msgDataLen, hex.EncodeToString(b.MessageData))
		} else {
			bd.write(" - messageData size=%d", msgDataLen)
		}
	}

	return bd.err
}
