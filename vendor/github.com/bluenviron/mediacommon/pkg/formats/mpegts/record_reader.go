package mpegts

import (
	"errors"
	"io"
)

const (
	recordReaderMaxSize = 1 * 1024 * 1024
)

type recordReader struct {
	r    io.Reader
	buf  []byte
	size int
}

func (r *recordReader) Read(p []byte) (int, error) {
	n, err := r.r.Read(p)

	if (r.size + n) > recordReaderMaxSize {
		return 0, errors.New("max buffer size exceeded")
	}

	r.buf = append(r.buf, p[:n]...)
	r.size += n
	return n, err
}
