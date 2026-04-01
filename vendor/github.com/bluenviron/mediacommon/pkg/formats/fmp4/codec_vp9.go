package fmp4

// CodecVP9 is the VP9 codec.
type CodecVP9 struct {
	Width             int
	Height            int
	Profile           uint8
	BitDepth          uint8
	ChromaSubsampling uint8
	ColorRange        bool
}

// IsVideo implements Codec.
func (CodecVP9) IsVideo() bool {
	return true
}

func (*CodecVP9) isCodec() {}
