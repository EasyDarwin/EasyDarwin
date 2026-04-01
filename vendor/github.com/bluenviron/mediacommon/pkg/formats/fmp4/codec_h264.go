package fmp4

// CodecH264 is the H264 codec.
type CodecH264 struct {
	SPS []byte
	PPS []byte
}

// IsVideo implements Codec.
func (CodecH264) IsVideo() bool {
	return true
}

func (*CodecH264) isCodec() {}
