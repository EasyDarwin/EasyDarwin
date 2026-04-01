package codecs

import (
	"github.com/bluenviron/mediacommon/pkg/formats/mpegts"
)

// FromMPEGTS imports a codec from MPEG-TS.
func FromMPEGTS(in mpegts.Codec) Codec {
	switch in := in.(type) {
	case *mpegts.CodecH264:
		return &H264{}

	case *mpegts.CodecMPEG4Audio:
		return &MPEG4Audio{
			Config: in.Config,
		}
	}

	return nil
}

// ToMPEGTS converts a codec in its MPEG-TS equivalent.
func ToMPEGTS(in Codec) mpegts.Codec {
	switch in := in.(type) {
	case *H264:
		return &mpegts.CodecH264{}

	case *MPEG4Audio:
		return &mpegts.CodecMPEG4Audio{
			Config: in.Config,
		}
	}

	return nil
}
