package fmp4

// CodecH265 is the H265 codec.
type CodecH265 struct {
	SPS []byte
	PPS []byte
	VPS []byte
}

// IsVideo implements Codec.
func (CodecH265) IsVideo() bool {
	return true
}

func (*CodecH265) isCodec() {}
