package mpegts

import (
	"github.com/asticode/go-astits"
)

// CodecUnsupported is an unsupported codec.
type CodecUnsupported struct{}

// IsVideo implements Codec.
func (CodecUnsupported) IsVideo() bool {
	return false
}

func (*CodecUnsupported) isCodec() {}

func (c CodecUnsupported) marshal(uint16) (*astits.PMTElementaryStream, error) {
	panic("this should not happen")
}
