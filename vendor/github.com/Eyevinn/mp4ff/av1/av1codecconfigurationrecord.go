package av1

import (
	"errors"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// AV1 parsing errors
var (
	ErrInvalidMarker       = errors.New("invalid marker value found in AV1CodecConfigurationRecord")
	ErrInvalidVersion      = errors.New("unsupported AV1CodecConfigurationRecord version")
	ErrNonZeroReservedBits = errors.New("non-zero reserved bits found in AV1CodecConfigurationRecord")
)

// CodecConfRec - AV1CodecConfigurationRecord
// Specified in https://github.com/AOMediaCodec/av1-isobmff/releases/tag/v1.2.0
type CodecConfRec struct {
	Version                          byte
	SeqProfile                       byte
	SeqLevelIdx0                     byte
	SeqTier0                         byte
	HighBitdepth                     byte
	TwelveBit                        byte
	MonoChrome                       byte
	ChromaSubsamplingX               byte
	ChromaSubsamplingY               byte
	ChromaSamplePosition             byte
	InitialPresentationDelayPresent  byte
	InitialPresentationDelayMinusOne byte
	ConfigOBUs                       []byte
}

// DecodeAVCDecConfRec - decode an AV1CodecConfRec
func DecodeAV1CodecConfRec(data []byte) (CodecConfRec, error) {
	av1drc := CodecConfRec{}

	Marker := data[0] >> 7
	if Marker != 1 {
		return CodecConfRec{}, ErrInvalidMarker
	}
	av1drc.Version = data[0] & 0x7F
	if av1drc.Version != 1 {
		return CodecConfRec{}, ErrInvalidVersion
	}
	av1drc.SeqProfile = data[1] >> 5
	av1drc.SeqLevelIdx0 = data[1] & 0x1F
	av1drc.SeqTier0 = data[2] >> 7
	av1drc.HighBitdepth = (data[2] >> 6) & 0x01
	av1drc.TwelveBit = (data[2] >> 5) & 0x01
	av1drc.MonoChrome = (data[2] >> 4) & 0x01
	av1drc.ChromaSubsamplingX = (data[2] >> 3) & 0x01
	av1drc.ChromaSubsamplingY = (data[2] >> 2) & 0x01
	av1drc.ChromaSamplePosition = data[2] & 0x03
	if data[3]>>5 != 0 {
		return CodecConfRec{}, ErrNonZeroReservedBits
	}
	av1drc.InitialPresentationDelayPresent = (data[3] >> 4) & 0x01
	if av1drc.InitialPresentationDelayPresent == 1 {
		av1drc.InitialPresentationDelayMinusOne = data[3] & 0x0F
	} else {
		if data[3]&0x0F != 0 {
			return CodecConfRec{}, ErrNonZeroReservedBits
		}
		av1drc.InitialPresentationDelayMinusOne = 0
	}
	if len(data) > 4 {
		av1drc.ConfigOBUs = data[4:]
	}

	return av1drc, nil
}

// Size - total size in bytes
func (a *CodecConfRec) Size() uint64 {
	return uint64(4 + len(a.ConfigOBUs))
}

// EncodeSW- write an AV1CodecConfRec to w
func (a *CodecConfRec) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(a.Size()))
	err := a.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW- write an AV1CodecConfRec to sw
func (a *CodecConfRec) EncodeSW(sw bits.SliceWriter) error {
	sw.WriteBits(1, 1)
	sw.WriteBits(uint(a.Version), 7)
	sw.WriteBits(uint(a.SeqProfile), 3)
	sw.WriteBits(uint(a.SeqLevelIdx0), 5)
	sw.WriteBits(uint(a.SeqTier0), 1)
	sw.WriteBits(uint(a.HighBitdepth), 1)
	sw.WriteBits(uint(a.TwelveBit), 1)
	sw.WriteBits(uint(a.MonoChrome), 1)
	sw.WriteBits(uint(a.ChromaSubsamplingX), 1)
	sw.WriteBits(uint(a.ChromaSubsamplingY), 1)
	sw.WriteBits(uint(a.ChromaSamplePosition), 2)
	sw.WriteBits(0, 3)
	sw.WriteBits(uint(a.InitialPresentationDelayPresent), 1)
	if a.InitialPresentationDelayPresent == 1 {
		sw.WriteBits(uint(a.InitialPresentationDelayMinusOne), 4)
	} else {
		sw.WriteBits(0, 4)
	}
	if len(a.ConfigOBUs) != 0 {
		sw.WriteBytes(a.ConfigOBUs)
	}

	return sw.AccError()
}
