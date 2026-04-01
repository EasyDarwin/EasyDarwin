package storage

import (
	"io"
	"os"
)

type diskPartReader struct {
	f *os.File
	r *io.LimitedReader
}

func newDiskPartReader(fpath string, offset uint64, size uint64) (io.ReadCloser, error) {
	f, err := os.Open(fpath)
	if err != nil {
		return nil, err
	}

	_, err = f.Seek(int64(offset), io.SeekStart)
	if err != nil {
		return nil, err
	}

	return &diskPartReader{
		f: f,
		r: &io.LimitedReader{
			R: f,
			N: int64(size),
		},
	}, nil
}

func (r *diskPartReader) Close() error {
	return r.f.Close()
}

func (r *diskPartReader) Read(p []byte) (int, error) {
	return r.r.Read(p)
}
