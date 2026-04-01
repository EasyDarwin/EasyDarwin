package codecs

// H265 is a H265 codec.
type H265 struct {
	VPS []byte
	SPS []byte
	PPS []byte
}

func (*H265) isCodec() {
}
