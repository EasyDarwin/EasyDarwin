package fmp4

// CodecMPEG4Video is a MPEG-4 Video codec.
type CodecMPEG4Video struct {
	Config []byte
}

// IsVideo implements Codec.
func (CodecMPEG4Video) IsVideo() bool {
	return true
}

func (*CodecMPEG4Video) isCodec() {}
