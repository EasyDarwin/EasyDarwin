package codecs

// Opus is a Opus codec.
type Opus struct {
	ChannelCount int
}

func (*Opus) isCodec() {
}
