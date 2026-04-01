package fmp4

import (
	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg4audio"
)

// CodecMPEG4Audio is a MPEG-4 Audio codec.
type CodecMPEG4Audio struct {
	mpeg4audio.Config
}

// IsVideo implements Codec.
func (CodecMPEG4Audio) IsVideo() bool {
	return false
}

func (*CodecMPEG4Audio) isCodec() {}
