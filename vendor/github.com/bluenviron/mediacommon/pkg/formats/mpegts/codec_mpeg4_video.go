package mpegts

import (
	"github.com/asticode/go-astits"
)

// CodecMPEG4Video is a MPEG-4 Video codec.
type CodecMPEG4Video struct{}

// IsVideo implements Codec.
func (CodecMPEG4Video) IsVideo() bool {
	return true
}

func (*CodecMPEG4Video) isCodec() {}

func (c CodecMPEG4Video) marshal(pid uint16) (*astits.PMTElementaryStream, error) {
	return &astits.PMTElementaryStream{
		ElementaryPID: pid,
		StreamType:    astits.StreamTypeMPEG4Video,
	}, nil
}
