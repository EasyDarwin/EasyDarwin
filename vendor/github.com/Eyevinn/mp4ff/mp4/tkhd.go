package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// TkhdBox - Track Header Box (tkhd - mandatory)
//
// This box describes the track. Duration is measured in time units (according to the time scale
// defined in the movie header box). Duration is 0 for fragmented files.
//
// Volume (relevant for audio tracks) is a fixed point number (8 bits + 8 bits). Full volume is 1.0.
// Width and Height (relevant for video tracks) are fixed point numbers (16 bits + 16 bits).
// Video pixels are not necessarily square.
type TkhdBox struct {
	Version          byte
	Flags            uint32
	CreationTime     uint64
	ModificationTime uint64
	TrackID          uint32
	Duration         uint64
	Layer            int16
	AlternateGroup   int16 // should be int16
	Volume           Fixed16
	Width, Height    Fixed32
}

// CreateTkhd - create tkhd box with common settings
func CreateTkhd() *TkhdBox {
	return &TkhdBox{
		Version: 0,
		Flags:   0x000007,      // Enabled, inMovie, inPreview set
		TrackID: DefaultTrakID, // Typically just have one track
	}
}

// DecodeTkhd - box-specific decode
func DecodeTkhd(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeTkhdSR(hdr, startPos, sr)
}

// DecodeTkhdSR - box-specific decode
func DecodeTkhdSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)
	flags := versionAndFlags & flagsMask

	t := TkhdBox{
		Version: version,
		Flags:   flags,
	}

	if version == 1 {
		t.CreationTime = sr.ReadUint64()
		t.ModificationTime = sr.ReadUint64()
		t.TrackID = sr.ReadUint32()
		sr.SkipBytes(4) // Reserved = 0
		t.Duration = sr.ReadUint64()
	} else {
		t.CreationTime = uint64(sr.ReadUint32())
		t.ModificationTime = uint64(sr.ReadUint32())
		t.TrackID = sr.ReadUint32()
		sr.SkipBytes(4) // Reserved = 0
		t.Duration = uint64(sr.ReadUint32())
	}
	sr.SkipBytes(8) // Reserved 8 x 0
	t.Layer = sr.ReadInt16()
	t.AlternateGroup = sr.ReadInt16()
	t.Volume = Fixed16(sr.ReadInt16())
	sr.SkipBytes(2)
	sr.SkipBytes(36) // 3x3 matrixdata
	t.Width = Fixed32(sr.ReadUint32())
	t.Height = Fixed32(sr.ReadUint32())

	return &t, sr.AccError()
}

// Type - box type
func (b *TkhdBox) Type() string {
	return "tkhd"
}

// Size - calculated size of box
func (b *TkhdBox) Size() uint64 {
	if b.Version == 1 {
		return 104
	}
	return 92
}

// Encode - write box to w
func (b *TkhdBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *TkhdBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	if b.Version == 0 {
		sw.WriteUint32(uint32(b.CreationTime))
		sw.WriteUint32(uint32(b.ModificationTime))
		sw.WriteUint32(b.TrackID)
		sw.WriteZeroBytes(4) // Reserved
		sw.WriteUint32(uint32(b.Duration))
	} else {
		sw.WriteUint64(b.CreationTime)
		sw.WriteUint64(b.ModificationTime)
		sw.WriteUint32(b.TrackID)
		sw.WriteZeroBytes(4) // Reserved
		sw.WriteUint64(b.Duration)
	}
	sw.WriteZeroBytes(8) // Reserved
	sw.WriteInt16(b.Layer)
	sw.WriteInt16(b.AlternateGroup)
	sw.WriteUint16(uint16(b.Volume))
	sw.WriteZeroBytes(2)  // Reserved
	sw.WriteUnityMatrix() // unity matrix according to 8.3.2.2
	sw.WriteUint32(uint32(b.Width))
	sw.WriteUint32(uint32(b.Height))

	return sw.AccError()
}

// Info - write box-specific information
func (b *TkhdBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - trackID: %d", b.TrackID)
	bd.write(" - duration: %d", b.Duration)
	bd.write(" - creation time: %s", timeStr(b.CreationTime))
	bd.write(" - modification time: %s", timeStr(b.ModificationTime))
	if b.Width != 0 && b.Height != 0 { // These are Fixed32 values
		bd.write(" - Width: %s, Height: %s", b.Width, b.Height)
	}
	return bd.err
}

// CraetionTimeS returns the creation time in seconds since Jan 1, 1970
func (b *TkhdBox) CreationTimeS() int64 {
	return int64(b.CreationTime) - EpochDiffS
}

// ModificationTimeS returns the modification time in seconds since Jan 1, 1970
func (b *TkhdBox) ModificationTimeS() int64 {
	return int64(b.ModificationTime) - EpochDiffS
}

// SetCreationTimeS sets the creation time from seconds since Jan 1, 1970
func (b *TkhdBox) SetCreationTimeS(unixTimeS int64) {
	b.CreationTime = uint64(unixTimeS + EpochDiffS)
}

// SetModificationTimeS sets the modification time from seconds since Jan 1, 1970
func (b *TkhdBox) SetModificationTimeS(unixTimeS int64) {
	b.ModificationTime = uint64(unixTimeS + EpochDiffS)
}
