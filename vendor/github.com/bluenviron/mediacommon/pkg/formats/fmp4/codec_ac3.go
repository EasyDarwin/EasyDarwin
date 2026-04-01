package fmp4

// CodecAC3 is the AC-3 codec.
type CodecAC3 struct {
	SampleRate   int
	ChannelCount int
	Fscod        uint8
	Bsid         uint8
	Bsmod        uint8
	Acmod        uint8
	LfeOn        bool
	BitRateCode  uint8
}

// IsVideo implements Codec.
func (CodecAC3) IsVideo() bool {
	return false
}

func (*CodecAC3) isCodec() {}
