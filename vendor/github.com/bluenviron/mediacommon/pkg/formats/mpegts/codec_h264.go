package mpegts

import (
	"github.com/asticode/go-astits"
)

// CodecH264 is a H264 codec.
type CodecH264 struct{}

// IsVideo implements Codec.
func (CodecH264) IsVideo() bool {
	return true
}

func (*CodecH264) isCodec() {}

func (c CodecH264) marshal(pid uint16) (*astits.PMTElementaryStream, error) {
	return &astits.PMTElementaryStream{
		ElementaryPID: pid,
		StreamType:    astits.StreamTypeH264Video,
	}, nil
}
