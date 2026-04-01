// Package seekablebuffer contains a bytes.Buffer with an additional Seek() method.
package seekablebuffer

import (
	"bytes"
	"fmt"
	"io"
)

// Buffer is a bytes.Buffer with an additional Seek() method.
type Buffer struct {
	bytes.Buffer
	pos int64
}

// Write implements io.Writer.
func (b *Buffer) Write(p []byte) (int, error) {
	n := 0

	if b.pos < int64(b.Len()) {
		n = copy(b.Bytes()[b.pos:], p)
		p = p[n:]
	}

	if len(p) > 0 {
		// Buffer.Write can't return an error.
		nn, _ := b.Buffer.Write(p) //nolint:errcheck
		n += nn
	}

	b.pos += int64(n)
	return n, nil
}

// Read implements io.Reader.
func (b *Buffer) Read(_ []byte) (int, error) {
	return 0, fmt.Errorf("unimplemented")
}

// Seek implements io.Seeker.
func (b *Buffer) Seek(offset int64, whence int) (int64, error) {
	pos2 := int64(0)

	switch whence {
	case io.SeekStart:
		pos2 = offset

	case io.SeekCurrent:
		pos2 = b.pos + offset

	case io.SeekEnd:
		pos2 = int64(b.Len()) + offset
	}

	if pos2 < 0 {
		return 0, fmt.Errorf("negative position")
	}

	b.pos = pos2

	diff := b.pos - int64(b.Len())
	if diff > 0 {
		// Buffer.Write can't return an error.
		b.Buffer.Write(make([]byte, diff)) //nolint:errcheck
	}

	return pos2, nil
}

// Reset resets the buffer state.
func (b *Buffer) Reset() {
	b.Buffer.Reset()
	b.pos = 0
}
