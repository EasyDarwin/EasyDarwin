package codecs

// AV1 is a AV1 codec.
type AV1 struct {
	SequenceHeader []byte
}

func (*AV1) isCodec() {
}
