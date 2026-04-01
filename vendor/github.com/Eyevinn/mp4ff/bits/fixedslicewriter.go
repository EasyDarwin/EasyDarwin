package bits

import "encoding/binary"

// FixedSliceWriter - write numbers to a fixed []byte slice
type FixedSliceWriter struct {
	accError error
	buf      []byte
	off      int
	n        int  // current number of bits
	v        uint // current accumulated value for bits
}

// NewFixedSliceWriter - create writer around slice.
// The slice will not grow, but stay the same size.
// If too much data is written, there will be
// an accumuluated error. Can be retrieved with AccError()
func NewFixedSliceWriterFromSlice(data []byte) *FixedSliceWriter {
	return &FixedSliceWriter{
		buf:      data,
		off:      0,
		n:        0,
		v:        0,
		accError: nil,
	}
}

// NewSliceWriter - create slice writer with fixed size.
func NewFixedSliceWriter(size int) *FixedSliceWriter {
	return &FixedSliceWriter{
		buf:      make([]byte, size),
		off:      0,
		n:        0,
		v:        0,
		accError: nil,
	}
}

// Len - length of FixedSliceWriter buffer written. Same as Offset()
func (sw *FixedSliceWriter) Len() int {
	return sw.off
}

// Capacity - max length of FixedSliceWriter buffer
func (sw *FixedSliceWriter) Capacity() int {
	return len(sw.buf)
}

// Offset - offset for writing in FixedSliceWriter buffer
func (sw *FixedSliceWriter) Offset() int {
	return sw.off
}

// Bytes - return buf up to what's written
func (sw *FixedSliceWriter) Bytes() []byte {
	return sw.buf[:sw.off]
}

// AccError - return accumulated erro
func (sw *FixedSliceWriter) AccError() error {
	return sw.accError
}

// WriteUint8 - write byte to slice
func (sw *FixedSliceWriter) WriteUint8(n byte) {
	if sw.off+1 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	sw.buf[sw.off] = n
	sw.off++
}

// WriteUint16 - write uint16 to slice
func (sw *FixedSliceWriter) WriteUint16(n uint16) {
	if sw.off+2 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	binary.BigEndian.PutUint16(sw.buf[sw.off:], n)
	sw.off += 2
}

// WriteInt16 - write int16 to slice
func (sw *FixedSliceWriter) WriteInt16(n int16) {
	if sw.off+2 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	binary.BigEndian.PutUint16(sw.buf[sw.off:], uint16(n))
	sw.off += 2
}

// WriteUint24 - write uint24 to slice
func (sw *FixedSliceWriter) WriteUint24(n uint32) {
	if sw.off+3 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	sw.WriteUint8(byte(n >> 16))
	sw.WriteUint16(uint16(n & 0xffff))
}

// WriteUint32 - write uint32 to slice
func (sw *FixedSliceWriter) WriteUint32(n uint32) {
	if sw.off+4 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	binary.BigEndian.PutUint32(sw.buf[sw.off:], n)
	sw.off += 4
}

// WriteInt32 - write int32 to slice
func (sw *FixedSliceWriter) WriteInt32(n int32) {
	if sw.off+4 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	binary.BigEndian.PutUint32(sw.buf[sw.off:], uint32(n))
	sw.off += 4
}

// WriteUint48 - write uint48
func (sw *FixedSliceWriter) WriteUint48(u uint64) {
	if sw.off+6 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	msb := uint16(u >> 32)
	binary.BigEndian.PutUint16(sw.buf[sw.off:], msb)
	sw.off += 2

	lsb := uint32(u & 0xffffffff)
	binary.BigEndian.PutUint32(sw.buf[sw.off:], lsb)
	sw.off += 4
}

// WriteUint64 - write uint64 to slice
func (sw *FixedSliceWriter) WriteUint64(n uint64) {
	if sw.off+8 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	binary.BigEndian.PutUint64(sw.buf[sw.off:], n)
	sw.off += 8
}

// WriteInt64 - write int64 to slice
func (sw *FixedSliceWriter) WriteInt64(n int64) {
	if sw.off+8 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	binary.BigEndian.PutUint64(sw.buf[sw.off:], uint64(n))
	sw.off += 8
}

// WriteString - write string to slice with or without zero end
func (sw *FixedSliceWriter) WriteString(s string, addZeroEnd bool) {
	nrNew := len(s)
	if addZeroEnd {
		nrNew++
	}
	if sw.off+nrNew > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	copy(sw.buf[sw.off:sw.off+len(s)], s)
	sw.off += len(s)
	if addZeroEnd {
		sw.buf[sw.off] = 0
		sw.off++
	}
}

// WriteZeroBytes - write n byte of zeroes
func (sw *FixedSliceWriter) WriteZeroBytes(n int) {
	if sw.off+n > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	for i := 0; i < n; i++ {
		sw.buf[sw.off] = 0
		sw.off++
	}
}

// WriteBytes - write []byte
func (sw *FixedSliceWriter) WriteBytes(byteSlice []byte) {
	if sw.off+len(byteSlice) > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	copy(sw.buf[sw.off:sw.off+len(byteSlice)], byteSlice)
	sw.off += len(byteSlice)
}

// WriteUnityMatrix - write a unity matrix for mvhd or tkhd
func (sw *FixedSliceWriter) WriteUnityMatrix() {
	if sw.off+36 > len(sw.buf) {
		sw.accError = ErrSliceWrite
		return
	}
	sw.WriteUint32(0x00010000) // = 1 fixed 16.16
	sw.WriteUint32(0)
	sw.WriteUint32(0)
	sw.WriteUint32(0)
	sw.WriteUint32(0x00010000) // = 1 fixed 16.16
	sw.WriteUint32(0)
	sw.WriteUint32(0)
	sw.WriteUint32(0)
	sw.WriteUint32(0x40000000) // = 1 fixed 2.30
}

func (sw *FixedSliceWriter) WriteBits(bits uint, n int) {
	if sw.accError != nil {
		return
	}
	sw.v <<= uint(n)
	sw.v |= bits & Mask(n)
	sw.n += n
	for sw.n >= 8 {
		b := byte((sw.v >> (uint(sw.n) - 8)) & Mask(8))
		sw.WriteUint8(b)
		sw.n -= 8
	}
	sw.v &= Mask(8)
}

// WriteFlag writes a flag as 1 bit.
func (sw *FixedSliceWriter) WriteFlag(f bool) {
	bit := uint(0)
	if f {
		bit = 1
	}
	sw.WriteBits(bit, 1)
}

// FlushBits - write remaining bits to the underlying .Writer.
// bits will be left-shifted and zeros appended to fill up a byte.
func (sw *FixedSliceWriter) FlushBits() {
	if sw.accError != nil {
		return
	}
	if sw.n != 0 {
		b := byte((sw.v << (8 - uint(sw.n))) & Mask(8))
		sw.WriteUint8(b)
	}
}
