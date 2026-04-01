package fmp4

// CodecLPCM is the LPCM codec.
type CodecLPCM struct {
	LittleEndian bool
	BitDepth     int
	SampleRate   int
	ChannelCount int
}

// IsVideo implements Codec.
func (CodecLPCM) IsVideo() bool {
	return false
}

func (*CodecLPCM) isCodec() {}
