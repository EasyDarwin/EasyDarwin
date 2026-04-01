package fmp4

// CodecMPEG1Audio is a MPEG-1 Audio codec.
type CodecMPEG1Audio struct {
	SampleRate   int
	ChannelCount int
}

// IsVideo implements Codec.
func (CodecMPEG1Audio) IsVideo() bool {
	return false
}

func (*CodecMPEG1Audio) isCodec() {}
