package fmp4

// CodecMPEG1Video is a MPEG-1 Video codec.
type CodecMPEG1Video struct {
	Config []byte
}

// IsVideo implements Codec.
func (CodecMPEG1Video) IsVideo() bool {
	return true
}

func (*CodecMPEG1Video) isCodec() {}
