package mpegts

import (
	"github.com/asticode/go-astits"
)

// CodecAC3 is an AC-3 codec.
type CodecAC3 struct {
	SampleRate   int
	ChannelCount int
}

// IsVideo implements Codec.
func (CodecAC3) IsVideo() bool {
	return true
}

func (*CodecAC3) isCodec() {}

func (c CodecAC3) marshal(pid uint16) (*astits.PMTElementaryStream, error) {
	return &astits.PMTElementaryStream{
		ElementaryPID: pid,
		StreamType:    astits.StreamTypeAC3Audio,
	}, nil
}
