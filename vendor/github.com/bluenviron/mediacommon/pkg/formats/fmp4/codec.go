package fmp4

// Codec is a fMP4 codec.
type Codec interface {
	IsVideo() bool

	isCodec()
}
