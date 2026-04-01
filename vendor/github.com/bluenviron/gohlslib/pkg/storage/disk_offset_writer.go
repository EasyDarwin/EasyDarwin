package storage

import (
	"errors"
	"io"
)

// copy of https://cs.opensource.google/go/go/+/refs/tags/go1.20.2:src/io/io.go;l=558
// will be removed when go 1.20 is the minimum supported version.
type offsetWriter struct {
	w    io.WriterAt
	base int64 // the original offset
	off  int64 // the current offset
}

// NewoffsetWriter returns an offsetWriter that writes to w
// starting at offset off.
func newOffsetWriter(w io.WriterAt, off int64) *offsetWriter {
	return &offsetWriter{w, off, off}
}

func (o *offsetWriter) Write(p []byte) (n int, err error) {
	n, err = o.w.WriteAt(p, o.off)
	o.off += int64(n)
	return
}

func (o *offsetWriter) WriteAt(p []byte, off int64) (n int, err error) {
	off += o.base
	return o.w.WriteAt(p, off)
}

var (
	errWhence = errors.New("Seek: invalid whence")
	errOffset = errors.New("Seek: invalid offset")
)

func (o *offsetWriter) Seek(offset int64, whence int) (int64, error) {
	switch whence {
	default:
		return 0, errWhence
	case io.SeekStart:
		offset += o.base
	case io.SeekCurrent:
		offset += o.off
	}
	if offset < o.base {
		return 0, errOffset
	}
	o.off = offset
	return offset - o.base, nil
}
