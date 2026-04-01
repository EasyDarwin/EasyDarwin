package mpegts

import (
	"github.com/asticode/go-astits"
)

// CodecMPEG1Video is a MPEG-1/2 Video codec.
type CodecMPEG1Video struct{}

// IsVideo implements Codec.
func (CodecMPEG1Video) IsVideo() bool {
	return true
}

func (*CodecMPEG1Video) isCodec() {}

func (c CodecMPEG1Video) marshal(pid uint16) (*astits.PMTElementaryStream, error) {
	return &astits.PMTElementaryStream{
		ElementaryPID: pid,
		// we use MPEG-2 to notify readers that video can be either MPEG-1 or MPEG-2
		StreamType: astits.StreamTypeMPEG2Video,
	}, nil
}
