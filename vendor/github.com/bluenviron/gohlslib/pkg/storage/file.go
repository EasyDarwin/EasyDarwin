package storage

import (
	"io"
)

// File is the underlying storage of a file.
type File interface {
	// Finalize finalizes the file, making it read-only.
	// It must always be called to avoid a memory leak.
	Finalize()

	// Remove removes the file from disk.
	Remove()

	// NewPart allocates a file part.
	NewPart() Part

	// Reader returns a ReadCloser to read the file.
	// Close() must always be called to avoid a memory leak.
	Reader() (io.ReadCloser, error)

	// Size returns the size of the file.
	Size() uint64
}
