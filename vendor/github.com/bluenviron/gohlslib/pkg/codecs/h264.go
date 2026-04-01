package codecs

// H264 is a H264 codec.
type H264 struct {
	SPS []byte
	PPS []byte
}

func (*H264) isCodec() {
}
