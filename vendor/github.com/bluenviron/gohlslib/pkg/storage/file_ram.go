package storage

import (
	"fmt"
	"io"
)

type fileRAM struct {
	finalized bool
	parts     []*partRAM
	finalSize uint64
}

func newFileRAM() File {
	return &fileRAM{}
}

// Finalize implements File.
func (s *fileRAM) Finalize() {
	s.finalized = true

	for _, part := range s.parts {
		s.finalSize += uint64(len(part.buffer.Bytes()))
	}
}

// Remove implements File.
func (s *fileRAM) Remove() {
}

// NewPart implements File.
func (s *fileRAM) NewPart() Part {
	p := newPartRAM()
	s.parts = append(s.parts, p)
	return p
}

// Reader implements File.
func (s *fileRAM) Reader() (io.ReadCloser, error) {
	if !s.finalized {
		return nil, fmt.Errorf("file has not been finalized yet")
	}

	return io.NopCloser(&ramFileReader{
		parts: s.parts,
	}), nil
}

// Size implements File.
func (s *fileRAM) Size() uint64 {
	return s.finalSize
}
