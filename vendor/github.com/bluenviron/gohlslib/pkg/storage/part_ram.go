package storage

import (
	"bytes"
	"io"

	"github.com/bluenviron/mediacommon/pkg/formats/fmp4/seekablebuffer"
)

type partRAM struct {
	buffer seekablebuffer.Buffer
}

func newPartRAM() *partRAM {
	return &partRAM{}
}

// Writer implements Part.
func (p *partRAM) Writer() io.WriteSeeker {
	return &p.buffer
}

// Reader implements Part.
func (p *partRAM) Reader() (io.ReadCloser, error) {
	return io.NopCloser(bytes.NewReader(p.buffer.Bytes())), nil
}
