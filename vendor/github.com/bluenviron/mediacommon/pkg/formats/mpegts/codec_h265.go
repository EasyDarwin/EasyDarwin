package mpegts

import (
	"github.com/asticode/go-astits"
)

// CodecH265 is a H265 codec.
type CodecH265 struct{}

// IsVideo implements Codec.
func (CodecH265) IsVideo() bool {
	return true
}

func (*CodecH265) isCodec() {}

func (c CodecH265) marshal(pid uint16) (*astits.PMTElementaryStream, error) {
	return &astits.PMTElementaryStream{
		ElementaryPID: pid,
		StreamType:    astits.StreamTypeH265Video,
	}, nil
}
