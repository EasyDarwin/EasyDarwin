package mp4

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"io"
	"strings"

	"github.com/Eyevinn/mp4ff/bits"
)

const (
	// The following UUIDs belong to Microsoft Smooth Streaming Protocol (MSS)

	// UUIDMssSm - MSS StreamManifest UUID [MS-SSTR 2.2.7.2]
	UUIDMssSm = "3c2fe51b-efee-40a3-ae815300199dc348"

	// UUIDMssLs - MSS LiveServerManifest UUID [MS-SSTR 2.2.7.3]
	UUIDMssLsm = "a5d40b30-e814-11dd-ba2f-0800200c9a66"

	// UUIDTfxd - MSS tfxd UUID [MS-SSTR 2.2.4.4]
	UUIDTfxd = "6d1d9b05-42d5-44e6-80e2-141daff757b2"

	// UUIDTfrf - MSS tfrf UUID [MS-SSTR 2.2.4.5]
	UUIDTfrf = "d4807ef2-ca39-4695-8e54-26cb9e46a79f"

	// UUIDPiffSenc - PIFF UUID for Sample Encryption Box (PIFF 1.1 spec)
	UUIDPiffSenc = "a2394f52-5a9b-4f14-a244-6c427c648df4"
)

// uuid - compact representation of UUID
type uuid [16]byte

// String - UUID-formatted string
func (u uuid) String() string {
	hexStr := hex.EncodeToString(u[:])
	return fmt.Sprintf("%s-%s-%s-%s-%s", hexStr[:8], hexStr[8:12], hexStr[12:16], hexStr[16:20], hexStr[20:])
}

// Equal - compare with other uuid
func (u uuid) Equal(a uuid) bool {
	return bytes.Equal(u[:], a[:])
}

// createUUID - create uuid from string
func createUUID(u string) (uuid, error) {
	var a uuid
	stripped := strings.ReplaceAll(u, "-", "")
	b, err := hex.DecodeString(stripped)
	if err != nil || len(b) != 16 {
		return a, fmt.Errorf("bad uuid string: %s", u)
	}
	_ = copy(a[:], b)
	return a, nil
}

// mustCreateUUID - create uuid from string. Panic for bad string
func mustCreateUUID(u string) uuid {
	b, err := createUUID(u)
	if err != nil {
		panic(err.Error())
	}
	return b
}

var (
	uuidTfxd     uuid = mustCreateUUID(UUIDTfxd)
	uuidTfrf     uuid = mustCreateUUID(UUIDTfrf)
	uuidPiffSenc uuid = mustCreateUUID(UUIDPiffSenc)
)

// UUIDBox - Used as container for MSS boxes tfxd and tfrf
// For unknown UUID, the data after the UUID is stored as UnknownPayload
type UUIDBox struct {
	uuid           uuid
	Tfxd           *TfxdData
	Tfrf           *TfrfData
	Senc           *SencBox
	StartPos       uint64
	UnknownPayload []byte
}

// UUID - Return UUID as formatted string
func (u *UUIDBox) UUID() string {
	return u.uuid.String()
}

// UUID - Set UUID from string
func (u *UUIDBox) SetUUID(uuid string) (err error) {
	u.uuid, err = createUUID(uuid)
	return err
}

// TfxdData - MSS TfxdBox data after UUID part
// Defined in MSS-SSTR v20180912 section 2.2.4.4
type TfxdData struct {
	Version                  byte
	Flags                    uint32
	FragmentAbsoluteTime     uint64
	FragmentAbsoluteDuration uint64
}

// TfrfData - MSS TfrfBox data after UUID part
// Defined in MSS-SSTR v20180912 section 2.2.4.5
type TfrfData struct {
	Version                   byte
	Flags                     uint32
	FragmentCount             byte
	FragmentAbsoluteTimes     []uint64
	FragmentAbsoluteDurations []uint64
}

// DecodeUUIDBox - decode a UUID box including tfxd or tfrf
func DecodeUUIDBox(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeUUIDBoxSR(hdr, startPos, sr)
}

// DecodeUUIDBoxSR - decode a UUID box including tfxd or tfrf
func DecodeUUIDBoxSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	b := &UUIDBox{StartPos: startPos}
	copy(b.uuid[:], sr.ReadBytes(16))
	switch b.UUID() {
	case UUIDTfxd:
		tfxd, err := decodeTfxd(sr)
		if err != nil {
			return nil, err
		}
		b.Tfxd = tfxd
	case UUIDTfrf:
		tfrf, err := decodeTfrf(sr)
		if err != nil {
			return nil, err
		}
		b.Tfrf = tfrf
	case UUIDPiffSenc:
		// This is like a SencBox except that there is no size and type. Offset and sizes must be slightly adjusted.
		subHdr := BoxHeader{"senc", hdr.Size - 16, 8}
		box, err := DecodeSencSR(subHdr, b.StartPos+16, sr)
		if err != nil {
			return nil, fmt.Errorf("failed to decode senc in UUID: %w", err)
		}
		b.Senc = box.(*SencBox)
	default:
		b.UnknownPayload = sr.ReadBytes(int(hdr.Size) - 8 - 16)
	}

	return b, sr.AccError()
}

// Type - return box type
func (b *UUIDBox) Type() string {
	return "uuid"
}

// Size - return calculated size including tfxd/tfrf
func (b *UUIDBox) Size() uint64 {
	var size uint64 = 8 + 16
	switch u := b.uuid; {
	case u.Equal(uuidTfxd):
		size += b.Tfxd.size()
	case u.Equal(uuidTfrf):
		size += b.Tfrf.size()
	case u.Equal(uuidPiffSenc):
		size += b.Senc.Size() - 8 // -8 because no header
	default:
		size += uint64(len(b.UnknownPayload))
	}
	return size
}

// Encode - write box to w
func (b *UUIDBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *UUIDBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteBytes(b.uuid[:])
	switch u := b.uuid; {
	case u.Equal(uuidTfxd):
		err = b.Tfxd.encode(sw)
	case u.Equal(uuidTfrf):
		err = b.Tfrf.encode(sw)
	case u.Equal(uuidPiffSenc):
		err = b.Senc.EncodeSWNoHdr(sw)
	default:
		sw.WriteBytes(b.UnknownPayload)
	}
	if err != nil {
		return err
	}
	return sw.AccError()
}

// SubType - interpret the UUID as a known sub type or unknown
func (b *UUIDBox) SubType() string {
	switch u := b.uuid; {
	case u.Equal(uuidTfxd):
		return "tfxd"
	case u.Equal(uuidTfrf):
		return "tfrf"
	case u.Equal(uuidPiffSenc):
		return "senc"
	default:
		return "unknown"
	}
}

func decodeTfxd(s bits.SliceReader) (*TfxdData, error) {
	versionAndFlags := s.ReadUint32()
	version := byte(versionAndFlags >> 24)
	var fragmentAbsoluteTime uint64
	var fragmentAbsoluteDuration uint64
	if version == 0 {
		fragmentAbsoluteTime = uint64(s.ReadUint32())
		fragmentAbsoluteDuration = uint64(s.ReadUint32())
	} else {
		fragmentAbsoluteTime = s.ReadUint64()
		fragmentAbsoluteDuration = s.ReadUint64()
	}

	t := &TfxdData{
		Version:                  version,
		Flags:                    versionAndFlags & flagsMask,
		FragmentAbsoluteTime:     fragmentAbsoluteTime,
		FragmentAbsoluteDuration: fragmentAbsoluteDuration,
	}
	return t, nil
}

func (t *TfxdData) size() uint64 {
	return 4 + 8 + 8*uint64(t.Version)
}

func (t *TfxdData) encode(sw bits.SliceWriter) error {
	versionAndFlags := (uint32(t.Version) << 24) + t.Flags
	sw.WriteUint32(versionAndFlags)
	if t.Version == 0 {
		sw.WriteUint32(uint32(t.FragmentAbsoluteTime))
		sw.WriteUint32(uint32(t.FragmentAbsoluteDuration))
	} else {
		sw.WriteUint64(t.FragmentAbsoluteTime)
		sw.WriteUint64(t.FragmentAbsoluteDuration)
	}
	return sw.AccError()
}

func decodeTfrf(s bits.SliceReader) (*TfrfData, error) {
	versionAndFlags := s.ReadUint32()
	version := byte(versionAndFlags >> 24)
	t := &TfrfData{
		Version:       version,
		Flags:         versionAndFlags & flagsMask,
		FragmentCount: s.ReadUint8(),
	}
	if t.Version == 0 {
		for i := byte(0); i < t.FragmentCount; i++ {
			t.FragmentAbsoluteTimes = append(t.FragmentAbsoluteTimes, uint64(s.ReadUint32()))
			t.FragmentAbsoluteDurations = append(t.FragmentAbsoluteDurations, uint64(s.ReadUint32()))
		}
	} else {
		for i := byte(0); i < t.FragmentCount; i++ {
			t.FragmentAbsoluteTimes = append(t.FragmentAbsoluteTimes, s.ReadUint64())
			t.FragmentAbsoluteDurations = append(t.FragmentAbsoluteDurations, s.ReadUint64())
		}
	}
	return t, nil
}

func (t *TfrfData) size() uint64 {
	return 4 + 1 + (8+8*uint64(t.Version))*uint64(t.FragmentCount)
}

func (t *TfrfData) encode(sw bits.SliceWriter) error {
	versionAndFlags := (uint32(t.Version) << 24) + t.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint8(t.FragmentCount)
	if t.Version == 0 {
		for i := byte(0); i < t.FragmentCount; i++ {
			sw.WriteUint32(uint32(t.FragmentAbsoluteTimes[i]))
			sw.WriteUint32(uint32(t.FragmentAbsoluteDurations[i]))
		}
	} else {
		for i := byte(0); i < t.FragmentCount; i++ {
			sw.WriteUint64(t.FragmentAbsoluteTimes[i])
			sw.WriteUint64(t.FragmentAbsoluteDurations[i])
		}
	}
	return sw.AccError()
}

// Info - box-specific info
func (b *UUIDBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - uuid: %s", b.uuid)
	bd.write(" - subType: %s", b.SubType())
	level := getInfoLevel(b, specificBoxLevels)
	if level > 0 {
		switch b.SubType() {
		case "tfxd":
			bd.write(" - absTime=%d absDur=%d", b.Tfxd.FragmentAbsoluteTime, b.Tfxd.FragmentAbsoluteDuration)
		case "tfrf":
			for i := 0; i < int(b.Tfrf.FragmentCount); i++ {
				bd.write(" - [%d]: absTime=%d absDur=%d", i+1, b.Tfrf.FragmentAbsoluteTimes[i], b.Tfrf.FragmentAbsoluteDurations[i])
			}
		case "senc":
			err := b.Senc.Info(w, specificBoxLevels, indent+"    ", indentStep)
			if err != nil {
				return fmt.Errorf("piff senc: %w", err)
			}
		default:
			bd.write(" - payload: %s", hex.EncodeToString(b.UnknownPayload))
		}
	}
	return bd.err
}
