package bits

import (
	"encoding/binary"
	"io"
)

// ByteWriter - writer that wraps an io.Writer and accumulates error.
// Only the first error is saved, but any later calls will not panic.
type ByteWriter struct {
	w   io.Writer
	err error
}

// NewByteWriter creates accumulated error writer around io.Writer.
func NewByteWriter(w io.Writer) *ByteWriter {
	return &ByteWriter{
		w: w,
	}
}

// AccError - return accumulated error
func (a *ByteWriter) AccError() error {
	return a.err
}

// WriteUint8 - write a byte
func (a *ByteWriter) WriteUint8(b byte) {
	if a.err != nil {
		return
	}
	a.err = binary.Write(a.w, binary.BigEndian, b)
}

// WriteUint16 - write uint16
func (a *ByteWriter) WriteUint16(u uint16) {
	if a.err != nil {
		return
	}
	a.err = binary.Write(a.w, binary.BigEndian, u)
}

// WriteUint32 - write uint32
func (a *ByteWriter) WriteUint32(u uint32) {
	if a.err != nil {
		return
	}
	a.err = binary.Write(a.w, binary.BigEndian, u)
}

// WriteUint48 - write uint48
func (a *ByteWriter) WriteUint48(u uint64) {
	if a.err != nil {
		return
	}
	msb := uint16(u >> 32)
	a.err = binary.Write(a.w, binary.BigEndian, msb)
	if a.err != nil {
		return
	}
	lsb := uint32(u & 0xffffffff)
	a.err = binary.Write(a.w, binary.BigEndian, lsb)
}

// WriteUint64 - write uint64
func (a *ByteWriter) WriteUint64(u uint64) {
	if a.err != nil {
		return
	}
	a.err = binary.Write(a.w, binary.BigEndian, u)
}

// WriteSlice - write a slice
func (a *ByteWriter) WriteSlice(s []byte) {
	if a.err != nil {
		return
	}
	_, a.err = a.w.Write(s)
}
