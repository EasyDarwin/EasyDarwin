package fmp4

// CodecMJPEG is the M-JPEG codec.
type CodecMJPEG struct {
	Width  int
	Height int
}

// IsVideo implements Codec.
func (CodecMJPEG) IsVideo() bool {
	return true
}

func (*CodecMJPEG) isCodec() {}
