package mpegts

import (
	"io"
)

type playbackReader struct {
	r   io.Reader
	buf []byte
}

func (r *playbackReader) Read(p []byte) (int, error) {
	if r.buf != nil {
		n := copy(p, r.buf)

		if len(r.buf) == n {
			r.buf = nil // release buf from memory
		} else {
			r.buf = r.buf[n:]
		}

		return n, nil
	}

	return r.r.Read(p)
}
