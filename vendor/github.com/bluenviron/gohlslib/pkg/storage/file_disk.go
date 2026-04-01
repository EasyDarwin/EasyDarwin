package storage

import (
	"fmt"
	"io"
	"os"
)

type fileDisk struct {
	fpath     string
	f         *os.File
	parts     []*partDisk
	finalSize uint64
}

func newFileDisk(fpath string) (File, error) {
	f, err := os.Create(fpath)
	if err != nil {
		return nil, err
	}

	return &fileDisk{
		fpath: fpath,
		f:     f,
	}, nil
}

// Finalize implements File.
func (s *fileDisk) Finalize() {
	if len(s.parts) > 0 {
		// set size of last part
		lastPart := s.parts[len(s.parts)-1]
		lastPart.size = uint64(len(lastPart.buffer.Bytes()))

		// save size
		s.finalSize = lastPart.offset + lastPart.size
	}

	// remove file from memory; we will use disk from now on
	for _, p := range s.parts {
		p.buffer = nil
	}

	s.f.Close()
	s.f = nil
}

// Remove implements File.
func (s *fileDisk) Remove() {
	os.Remove(s.fpath)
}

// NewPart implements File.
func (s *fileDisk) NewPart() Part {
	// set size of last part and get offset
	offset := uint64(0)
	if len(s.parts) > 0 {
		lastPart := s.parts[len(s.parts)-1]
		lastPart.size = uint64(len(lastPart.buffer.Bytes()))
		offset = lastPart.offset + lastPart.size
	}

	p := newPartDisk(s, offset)
	s.parts = append(s.parts, p)
	return p
}

// Reader implements File.
func (s *fileDisk) Reader() (io.ReadCloser, error) {
	if s.f != nil {
		return nil, fmt.Errorf("file has not been finalized yet")
	}

	return os.Open(s.fpath)
}

// Size implements File.
func (s *fileDisk) Size() uint64 {
	return s.finalSize
}
