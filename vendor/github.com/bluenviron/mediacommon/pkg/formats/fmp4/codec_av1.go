package fmp4

// CodecAV1 is the AV1 codec.
type CodecAV1 struct {
	SequenceHeader []byte
}

// IsVideo implements Codec.
func (CodecAV1) IsVideo() bool {
	return true
}

func (*CodecAV1) isCodec() {}
