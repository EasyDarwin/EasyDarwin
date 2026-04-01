package storage

import (
	"io"
)

type doubleWriter struct {
	w1 io.WriteSeeker
	w2 io.WriteSeeker
}

func (w *doubleWriter) Write(p []byte) (int, error) {
	_, err := w.w1.Write(p)
	if err != nil {
		return 0, err
	}

	return w.w2.Write(p)
}

func (w *doubleWriter) Seek(offset int64, whence int) (int64, error) {
	_, err := w.w1.Seek(offset, whence)
	if err != nil {
		return 0, err
	}

	return w.w2.Seek(offset, whence)
}
