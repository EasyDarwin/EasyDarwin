package codecs

import (
	"github.com/bluenviron/mediacommon/pkg/formats/fmp4"
)

// FromFMP4 imports a codec from fMP4.
func FromFMP4(in fmp4.Codec) Codec {
	switch in := in.(type) {
	case *fmp4.CodecAV1:
		return &AV1{
			SequenceHeader: in.SequenceHeader,
		}

	case *fmp4.CodecVP9:
		return &VP9{
			Width:             in.Width,
			Height:            in.Height,
			Profile:           in.Profile,
			BitDepth:          in.BitDepth,
			ChromaSubsampling: in.ChromaSubsampling,
			ColorRange:        in.ColorRange,
		}

	case *fmp4.CodecH265:
		return &H265{
			VPS: in.VPS,
			SPS: in.SPS,
			PPS: in.PPS,
		}

	case *fmp4.CodecH264:
		return &H264{
			SPS: in.SPS,
			PPS: in.PPS,
		}

	case *fmp4.CodecOpus:
		return &Opus{
			ChannelCount: in.ChannelCount,
		}

	case *fmp4.CodecMPEG4Audio:
		return &MPEG4Audio{
			Config: in.Config,
		}
	}

	return nil
}

// ToFMP4 converts a codec in its fMP4 equivalent.
func ToFMP4(in Codec) fmp4.Codec {
	switch in := in.(type) {
	case *AV1:
		return &fmp4.CodecAV1{
			SequenceHeader: in.SequenceHeader,
		}

	case *VP9:
		return &fmp4.CodecVP9{
			Width:             in.Width,
			Height:            in.Height,
			Profile:           in.Profile,
			BitDepth:          in.BitDepth,
			ChromaSubsampling: in.ChromaSubsampling,
			ColorRange:        in.ColorRange,
		}

	case *H265:
		return &fmp4.CodecH265{
			VPS: in.VPS,
			SPS: in.SPS,
			PPS: in.PPS,
		}

	case *H264:
		return &fmp4.CodecH264{
			SPS: in.SPS,
			PPS: in.PPS,
		}

	case *Opus:
		return &fmp4.CodecOpus{
			ChannelCount: in.ChannelCount,
		}

	case *MPEG4Audio:
		return &fmp4.CodecMPEG4Audio{
			Config: in.Config,
		}
	}

	return nil
}
