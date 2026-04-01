package storage

import (
	"io"
)

// Part is the underlying storage of a HLS part.
type Part interface {
	// Writer returns a Writer to write the part.
	Writer() io.WriteSeeker

	// Reader returns a ReadCloser to read the part.
	// Close() must always be called to avoid a memory leak.
	Reader() (io.ReadCloser, error)
}
