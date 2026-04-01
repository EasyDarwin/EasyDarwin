package bits

import (
	"encoding/binary"
	"errors"
	"fmt"
	"io"
)

// ESBPReader errors
var (
	ErrNotReadSeeker = errors.New("reader does not support Seek")
)

const (
	startCodeEmulationPreventionByte = 0x03
)

// EBSPReader reads an EBSP bitstream dropping start-code emulation bytes.
// It also supports checking for more rbsp data and reading rbsp_trailing_bits.
type EBSPReader struct {
	rd        io.Reader
	err       error
	n         int  // current number of bits
	v         uint // current accumulated value
	pos       int
	zeroCount int // Count number of zero bytes read
}

// NewEBSPReader return a new EBSP reader stopping reading at first error.
func NewEBSPReader(rd io.Reader) *EBSPReader {
	return &EBSPReader{
		rd:  rd,
		pos: -1,
	}
}

// AccError returns the accumulated error. If no error, returns nil.
func (r *EBSPReader) AccError() error {
	return r.err
}

// NrBytesRead returns how many bytes read into parser.
func (r *EBSPReader) NrBytesRead() int {
	return r.pos + 1 // Starts at -1
}

// NrBitsRead returns total number of bits read into parser.
func (r *EBSPReader) NrBitsRead() int {
	nrBits := r.NrBytesRead() * 8
	if r.NrBitsReadInCurrentByte() != 8 {
		nrBits += r.NrBitsReadInCurrentByte() - 8
	}
	return nrBits
}

// NrBitsReadInCurrentByte returns number of bits read in current byte.
func (r *EBSPReader) NrBitsReadInCurrentByte() int {
	return 8 - r.n
}

// Read reads n bits and respects and accumulates errors. If error, returns 0.
func (r *EBSPReader) Read(n int) uint {
	if r.err != nil {
		return 0
	}
	var err error
	for r.n < n {
		r.v <<= 8
		var b uint8
		err = binary.Read(r.rd, binary.BigEndian, &b)
		if err != nil {
			r.err = err
			return 0
		}
		r.pos++
		if r.zeroCount == 2 && b == startCodeEmulationPreventionByte {
			err = binary.Read(r.rd, binary.BigEndian, &b)
			if err != nil {
				r.err = err
				return 0
			}
			r.pos++
			r.zeroCount = 0
		}
		if b != 0 {
			r.zeroCount = 0
		} else {
			r.zeroCount++
		}
		r.v |= uint(b)

		r.n += 8
	}
	v := r.v >> uint(r.n-n)

	r.n -= n
	r.v &= Mask(r.n)

	return v
}

// ReadBytes read n bytes and return nil if new or accumulated error.
func (r *EBSPReader) ReadBytes(n int) []byte {
	if r.err != nil {
		return nil
	}
	payload := make([]byte, n)
	for i := 0; i < n; i++ {
		b := byte(r.Read(8))
		payload[i] = b
	}
	if r.err != nil {
		return nil
	}
	return payload
}

// ReadFlag reads 1 bit and translates a bool.
func (r *EBSPReader) ReadFlag() bool {
	return r.Read(1) == 1
}

// ReadExpGolomb reads one unsigned exponential Golomb code.
func (r *EBSPReader) ReadExpGolomb() uint {
	if r.err != nil {
		return 0
	}
	leadingZeroBits := 0

	for {
		b := r.Read(1)
		if r.err != nil {
			return 0
		}
		if b == 1 {
			break
		}
		leadingZeroBits++
	}

	var res uint = (1 << leadingZeroBits) - 1

	endBits := r.Read(leadingZeroBits)
	if r.err != nil {
		return 0
	}

	return res + endBits
}

// ReadSignedGolomb reads one signed exponential Golomb code.
func (r *EBSPReader) ReadSignedGolomb() int {
	if r.err != nil {
		return 0
	}
	unsignedGolomb := r.ReadExpGolomb()
	if r.err != nil {
		return 0
	}
	if unsignedGolomb%2 == 1 {
		return int((unsignedGolomb + 1) / 2)
	}
	return -int(unsignedGolomb / 2)
}

// IsSeeker returns tru if underluing reader supports Seek interface.
func (r *EBSPReader) IsSeeker() bool {
	_, ok := r.rd.(io.ReadSeeker)
	return ok
}

// MoreRbspData returns false if next bit is 1 and last 1-bit in fullSlice.
// Underlying reader must support ReadSeeker interface to reset after check.
// Return false, nil if underlying error.
func (r *EBSPReader) MoreRbspData() (bool, error) {
	if !r.IsSeeker() {
		return false, ErrNotReadSeeker
	}
	// Find out if next position is the last 1
	stateCopy := *r

	firstBit := r.Read(1)
	if r.err != nil {
		return false, nil
	}
	if firstBit != 1 {
		err := r.reset(stateCopy)
		if err != nil {
			return false, err
		}
		return true, nil
	}
	// If all remainging bits are zero, there is no more rbsp data
	more := false
	for {
		b := r.Read(1)
		if r.err == io.EOF {
			r.err = nil // Reset
			break
		}
		if r.err != nil {
			return false, nil
		}
		if b == 1 {
			more = true
			break
		}
	}
	err := r.reset(stateCopy)
	if err != nil {
		return false, err
	}
	return more, nil
}

// reset resets EBSPReader based on copy of previous state.
func (r *EBSPReader) reset(prevState EBSPReader) error {
	rdSeek, _ := r.rd.(io.ReadSeeker)
	_, err := rdSeek.Seek(int64(prevState.pos+1), 0)
	if err != nil {
		return err
	}
	r.n = prevState.n
	r.v = prevState.v
	r.pos = prevState.pos
	r.zeroCount = prevState.zeroCount
	return nil
}

// ReadRbspTrailingBits reads rbsp_traling_bits. Returns error if wrong pattern.
// If other error, returns nil and let AccError() provide that error.
func (r *EBSPReader) ReadRbspTrailingBits() error {
	if r.err != nil {
		return nil
	}
	firstBit := r.Read(1)
	if r.err != nil {
		return nil
	}
	if firstBit != 1 {
		return fmt.Errorf("rbspTrailingBits don't start with 1")
	}
	for {
		b := r.Read(1)
		if r.err == io.EOF {
			r.err = nil // Reset
			return nil
		}
		if r.err != nil {
			return nil
		}
		if b == 1 {
			return fmt.Errorf("another 1 in RbspTrailingBits")
		}
	}
}

// SetError sets an error if not already set.
func (r *EBSPReader) SetError(err error) {
	if r.err == nil {
		r.err = err
	}
}
