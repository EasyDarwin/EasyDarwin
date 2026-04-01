package mp4

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

const (
	// boxHeaderSize - standard size + name header
	boxHeaderSize = 8
	largeSizeLen  = 8          // Length of largesize exension
	flagsMask     = 0x00ffffff // Flags for masks from full header
)

var decoders map[string]BoxDecoder

func init() {
	decoders = map[string]BoxDecoder{
		"ac-3":    DecodeAudioSampleEntry,
		"alou":    DecodeAlou,
		"av01":    DecodeVisualSampleEntry,
		"avc1":    DecodeVisualSampleEntry,
		"avc3":    DecodeVisualSampleEntry,
		"avcC":    DecodeAvcC,
		"av1C":    DecodeAv1C,
		"btrt":    DecodeBtrt,
		"cdat":    DecodeCdat,
		"cdsc":    DecodeTrefType,
		"clap":    DecodeClap,
		"cslg":    DecodeCslg,
		"co64":    DecodeCo64,
		"colr":    DecodeColr,
		"ctim":    DecodeCtim,
		"ctts":    DecodeCtts,
		"dac3":    DecodeDac3,
		"data":    DecodeData,
		"dec3":    DecodeDec3,
		"desc":    DecodeGenericContainerBox,
		"dinf":    DecodeDinf,
		"dpnd":    DecodeTrefType,
		"dref":    DecodeDref,
		"ec-3":    DecodeAudioSampleEntry,
		"elng":    DecodeElng,
		"esds":    DecodeEsds,
		"edts":    DecodeEdts,
		"elst":    DecodeElst,
		"emeb":    DecodeEmeb,
		"emib":    DecodeEmib,
		"enca":    DecodeAudioSampleEntry,
		"encv":    DecodeVisualSampleEntry,
		"emsg":    DecodeEmsg,
		"evte":    DecodeEvte,
		"font":    DecodeTrefType,
		"free":    DecodeFree,
		"frma":    DecodeFrma,
		"ftyp":    DecodeFtyp,
		"hdlr":    DecodeHdlr,
		"hev1":    DecodeVisualSampleEntry,
		"hind":    DecodeTrefType,
		"hint":    DecodeTrefType,
		"hvcC":    DecodeHvcC,
		"hvc1":    DecodeVisualSampleEntry,
		"iden":    DecodeIden,
		"ilst":    DecodeIlst,
		"iods":    DecodeUnknown,
		"ipir":    DecodeTrefType,
		"kind":    DecodeKind,
		"leva":    DecodeLeva,
		"ludt":    DecodeLudt,
		"mdat":    DecodeMdat,
		"mehd":    DecodeMehd,
		"mdhd":    DecodeMdhd,
		"mdia":    DecodeMdia,
		"meta":    DecodeMeta,
		"mfhd":    DecodeMfhd,
		"mfra":    DecodeMfra,
		"mfro":    DecodeMfro,
		"mime":    DecodeMime,
		"minf":    DecodeMinf,
		"moof":    DecodeMoof,
		"moov":    DecodeMoov,
		"mpod":    DecodeTrefType,
		"mvex":    DecodeMvex,
		"mvhd":    DecodeMvhd,
		"mp4a":    DecodeAudioSampleEntry,
		"nmhd":    DecodeNmhd,
		"pasp":    DecodePasp,
		"payl":    DecodePayl,
		"prft":    DecodePrft,
		"pssh":    DecodePssh,
		"saio":    DecodeSaio,
		"saiz":    DecodeSaiz,
		"sbgp":    DecodeSbgp,
		"schi":    DecodeSchi,
		"schm":    DecodeSchm,
		"sdtp":    DecodeSdtp,
		"senc":    DecodeSenc,
		"sgpd":    DecodeSgpd,
		"sidx":    DecodeSidx,
		"silb":    DecodeSilb,
		"sinf":    DecodeSinf,
		"skip":    DecodeFree,
		"smhd":    DecodeSmhd,
		"ssix":    DecodeSsix,
		"sthd":    DecodeSthd,
		"stbl":    DecodeStbl,
		"stco":    DecodeStco,
		"stpp":    DecodeStpp,
		"stsc":    DecodeStsc,
		"stsd":    DecodeStsd,
		"stss":    DecodeStss,
		"stsz":    DecodeStsz,
		"sttg":    DecodeSttg,
		"stts":    DecodeStts,
		"styp":    DecodeStyp,
		"subs":    DecodeSubs,
		"subt":    DecodeTrefType,
		"sync":    DecodeTrefType,
		"tenc":    DecodeTenc,
		"tfdt":    DecodeTfdt,
		"tfhd":    DecodeTfhd,
		"tfra":    DecodeTfra,
		"tkhd":    DecodeTkhd,
		"tlou":    DecodeTlou,
		"traf":    DecodeTraf,
		"trak":    DecodeTrak,
		"tref":    DecodeTref,
		"trep":    DecodeTrep,
		"trex":    DecodeTrex,
		"trun":    DecodeTrun,
		"udta":    DecodeUdta,
		"url ":    DecodeURLBox,
		"uuid":    DecodeUUIDBox,
		"vdep":    DecodeTrefType,
		"vlab":    DecodeVlab,
		"vmhd":    DecodeVmhd,
		"vplx":    DecodeTrefType,
		"vsid":    DecodeVsid,
		"vtta":    DecodeVtta,
		"vttc":    DecodeVttc,
		"vttC":    DecodeVttC,
		"vtte":    DecodeVtte,
		"wvtt":    DecodeWvtt,
		"\xa9cpy": DecodeGenericContainerBox,
		"\xa9nam": DecodeGenericContainerBox,
		"\xa9too": DecodeGenericContainerBox,
		"\xa9ART": DecodeGenericContainerBox,
	}
}

// RemoveBoxDecoder removes the decode of boxType. It will be treated as unknown instead.
//
// This is a global change, so use with care.
func RemoveBoxDecoder(boxType string) {
	delete(decoders, boxType)
	delete(decodersSR, boxType)
}

// SetBoxDecoder sets decoder functions for a specific boxType.
//
// This is a global change, so use with care.
func SetBoxDecoder(boxType string, dec BoxDecoder, decSR BoxDecoderSR) {
	decoders[boxType] = dec
	decodersSR[boxType] = decSR
}

// BoxHeader - 8 or 16 bytes depending on size
type BoxHeader struct {
	Name   string
	Size   uint64
	Hdrlen int
}

func (b BoxHeader) payloadLen() int {
	return int(b.Size) - b.Hdrlen
}

// DecodeHeader decodes a box header (size + box type + possible largeSize)
func DecodeHeader(r io.Reader) (BoxHeader, error) {
	buf := make([]byte, boxHeaderSize)
	_, err := io.ReadFull(r, buf)
	if err != nil {
		return BoxHeader{}, err
	}
	size := uint64(binary.BigEndian.Uint32(buf[0:4]))
	headerLen := boxHeaderSize
	if size == 1 {
		buf := make([]byte, largeSizeLen)
		_, err = io.ReadFull(r, buf)
		if err != nil {
			return BoxHeader{}, err
		}
		size = binary.BigEndian.Uint64(buf)
		headerLen += largeSizeLen
	} else if size == 0 {
		return BoxHeader{}, fmt.Errorf("Size 0, meaning to end of file, not supported")
	}
	return BoxHeader{string(buf[4:8]), size, headerLen}, nil
}

// EncodeHeader - encode a box header to a writer
func EncodeHeader(b Box, w io.Writer) error {
	boxType, boxSize := b.Type(), b.Size()
	if boxSize >= 1<<32 {
		return fmt.Errorf("Box size %d is too big for normal 4-byte size field", boxSize)
	}
	buf := make([]byte, boxHeaderSize)
	binary.BigEndian.PutUint32(buf, uint32(boxSize))
	strtobuf(buf[4:], boxType, 4)
	_, err := w.Write(buf)
	return err
}

// EncodeHeaderWithSize - encode a box header to a writer and allow for largeSize
func EncodeHeaderWithSize(boxType string, boxSize uint64, largeSize bool, w io.Writer) error {
	if !largeSize && boxSize >= 1<<32 {
		return fmt.Errorf("Box size %d is too big for normal 4-byte size field", boxSize)
	}
	headerSize := boxHeaderSize
	if largeSize {
		headerSize += 8
	}
	buf := make([]byte, headerSize)
	if !largeSize {
		binary.BigEndian.PutUint32(buf, uint32(boxSize))
		strtobuf(buf[4:], boxType, 4)
	} else {
		binary.BigEndian.PutUint32(buf, 1) // signals large size
		strtobuf(buf[4:], boxType, 4)
		binary.BigEndian.PutUint64(buf[8:], boxSize)
	}
	_, err := w.Write(buf)
	return err
}

// EncodeHeaderSW - encode a box header to a SliceWriter
func EncodeHeaderSW(b Box, sw bits.SliceWriter) error {
	boxType, boxSize := b.Type(), b.Size()
	if boxSize >= 1<<32 {
		return fmt.Errorf("Box size %d is too big for normal 4-byte size field", boxSize)
	}
	sw.WriteUint32(uint32(boxSize))
	sw.WriteString(boxType, false)
	return nil
}

// EncodeHeaderWithSize - encode a box header to a writer and allow for largeSize
func EncodeHeaderWithSizeSW(boxType string, boxSize uint64, largeSize bool, sw bits.SliceWriter) error {
	if !largeSize && boxSize >= 1<<32 {
		return fmt.Errorf("Box size %d is too big for normal 4-byte size field", boxSize)
	}
	if !largeSize {
		sw.WriteUint32(uint32(boxSize))
		sw.WriteString(boxType, false)
	} else {
		sw.WriteUint32(1) // signals large size
		sw.WriteString(boxType, false)
		sw.WriteUint64(boxSize)
	}
	return sw.AccError()
}

// Box is the general interface to any ISOBMFF box or similar
type Box interface {
	// Type of box, normally 4 asccii characters, but is uint32 according to spec
	Type() string
	// Size of box including header and all children if any
	Size() uint64
	// Encode box to writer
	Encode(w io.Writer) error
	// Encode box to SliceWriter
	EncodeSW(sw bits.SliceWriter) error
	// Info - write box details
	//   spedificBoxLevels is a comma-separated list box:level or all:level where level >= 0.
	//   Higher levels give more details. 0 is default
	//   indent is indent at this box level.
	//   indentStep is how much to indent at each level
	Info(w io.Writer, specificBoxLevels, indent, indentStep string) error
}

// Informer - write box, segment or file details
type Informer interface {
	// Info - write details via Info method
	//   spedificBoxLevels is a comma-separated list box:level or all:level where level >= 0.
	//   Higher levels give more details. 0 is default
	//   indent is indent at this box level.
	//   indentStep is how much to indent at each level
	Info(w io.Writer, specificBoxLevels, indent, indentStep string) error
}

// BoxDecoder is function signature of the Box Decode method
type BoxDecoder func(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error)

// DecodeBox decodes a box
func DecodeBox(startPos uint64, r io.Reader) (Box, error) {
	var err error
	var b Box

	h, err := DecodeHeader(r)
	if err != nil {
		return nil, err
	}

	d, ok := decoders[h.Name]

	if !ok {
		b, err = DecodeUnknown(h, startPos, r)
	} else {
		b, err = d(h, startPos, r)
	}
	if err != nil {
		return nil, fmt.Errorf("decode %s pos %d: %w", h.Name, startPos, err)
	}

	return b, nil
}

// DecodeBoxLazyMdat decodes a box but doesn't read mdat into memory
func DecodeBoxLazyMdat(startPos uint64, r io.ReadSeeker) (Box, error) {
	var err error
	var b Box

	h, err := DecodeHeader(r)
	if err != nil {
		return nil, err
	}

	d, ok := decoders[h.Name]

	remainingLength := int64(h.Size) - int64(h.Hdrlen)

	if !ok {
		b, err = DecodeUnknown(h, startPos, r)
	} else {
		switch h.Name {
		case "mdat":
			b, err = DecodeMdatLazily(h, startPos)
			if err == nil {
				_, err = r.Seek(remainingLength, io.SeekCurrent)
			}
		default:
			b, err = d(h, startPos, r)
		}
	}
	if err != nil {
		return nil, fmt.Errorf("decode box %q: %w", h.Name, err)
	}

	return b, nil
}

// Fixed16 - An 8.8 fixed point number
type Fixed16 uint16

func (f Fixed16) String() string {
	return fmt.Sprintf("%d.%d", uint16(f)>>8, uint16(f)&0xff)
}

// Fixed32 -  A 16.16 fixed point number
type Fixed32 uint32

func (f Fixed32) String() string {
	return fmt.Sprintf("%d.%d", uint32(f)>>16, uint32(f)&0xffff)
}

func strtobuf(out []byte, in string, l int) {
	if l < len(in) {
		copy(out, in)
	} else {
		copy(out, in[0:l])
	}
}

func makebuf(b Box) []byte {
	return make([]byte, b.Size()-boxHeaderSize)
}

// readBoxBody reads complete box body. Returns error if not possible
func readBoxBody(r io.Reader, h BoxHeader) ([]byte, error) {
	bodyLen := h.Size - uint64(h.Hdrlen)
	if bodyLen == 0 {
		return nil, nil
	}
	body := make([]byte, bodyLen)
	_, err := io.ReadFull(r, body)
	if err != nil {
		return nil, err
	}
	return body, nil
}
