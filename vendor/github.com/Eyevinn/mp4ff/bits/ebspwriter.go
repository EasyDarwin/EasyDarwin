package bits

import (
	"io"
)

// EBSPWriter write bits and insert start-code emulation prevention bytes as necessary.
// Ceases writing at first error.
// The first error can later be checked with AccError().
type EBSPWriter struct {
	wr  io.Writer // underlying writer
	err error     // The first error caused by any write operation
	out []byte    // Slice of length 1 to avoid allocation at output

	n   int  // current number of bits
	v   uint // current accumulated value
	nr0 int  // Number preceding zero bytes

}

// NewEBSPWriter - returns a new Writer
func NewEBSPWriter(w io.Writer) *EBSPWriter {
	return &EBSPWriter{
		wr:  w,
		out: make([]byte, 1),
	}
}

// Write - write n bits from bits and save error state
func (w *EBSPWriter) Write(bits uint, n int) {
	if w.err != nil {
		return
	}
	w.v <<= uint(n)
	w.v |= bits & Mask(n)
	w.n += n
	for w.n >= 8 {
		b := (w.v >> (uint(w.n) - 8)) & Mask(8)
		if w.nr0 == 2 && b <= 3 {
			w.out[0] = 0x3 // start code emulation prevention
			_, err := w.wr.Write(w.out)
			if err != nil {
				w.err = err
				return
			}
			w.nr0 = 0
		}
		w.out[0] = uint8(b)
		_, err := w.wr.Write(w.out)
		if err != nil {
			w.err = err
			return
		}
		if b == 0 {
			w.nr0++
		} else {
			w.nr0 = 0
		}
		w.n -= 8
	}
	w.v &= Mask(8)
}

// WriteExpGolomb - write an exponential Golomb code
func (w *EBSPWriter) WriteExpGolomb(nr uint) {
	offset := uint(0)
	prefixLen := uint(0)
	delta := uint(0)
	max := uint(0)
	for {
		if nr <= max {
			delta = nr - offset
			break
		}
		offset += 1 << prefixLen
		prefixLen++
		max = offset + (1 << prefixLen) - 1
	}
	w.Write(1, int(prefixLen+1))
	if prefixLen > 0 {
		w.Write(delta, int(prefixLen))
	}
}

// WriteSEIValue insert 0xFF until value is less than 255. Used in SEI payload type and size.
func (w *EBSPWriter) WriteSEIValue(val uint) {
	for {
		if val >= 255 {
			w.Write(0xff, 8)
			val -= 255
			continue
		}
		w.Write(val, 8)
		break
	}
}

// WriteRbspTrailingBits - write rbsp trailing bits (a 1 followed by zeros to a byte boundary)
func (w *EBSPWriter) WriteRbspTrailingBits() {
	w.Write(1, 1)
	w.StuffByteWithZeros()
}

// StuffByteWithZeros - write zero bits until byte boundary (0-7bits)
func (w *EBSPWriter) StuffByteWithZeros() {
	if w.n > 0 {
		w.Write(0, 8-w.n)
	}
}

// AccError - return accumulated error
func (w *EBSPWriter) AccError() error {
	return w.err
}

// NrBitsInBuffer - number bits written in buffer byte
func (w *EBSPWriter) NrBitsInBuffer() uint {
	return uint(w.n)
}

// BitsInBuffer - n bits written in buffer byte, not written to underlying writer
func (w *EBSPWriter) BitsInBuffer() (bits, n uint) {
	return w.v, uint(w.n)
}
