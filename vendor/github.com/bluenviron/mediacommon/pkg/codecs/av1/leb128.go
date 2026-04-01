package av1

import (
	"fmt"
)

// LEB128Unmarshal decodes an unsigned integer from the LEB128 format.
// Specification: https://aomediacodec.github.io/av1-spec/#leb128
func LEB128Unmarshal(buf []byte) (uint, int, error) {
	v := uint(0)
	n := 0

	for i := 0; i < 8; i++ {
		if len(buf) == 0 {
			return 0, 0, fmt.Errorf("not enough bytes")
		}

		b := buf[0]

		v |= (uint(b&0b01111111) << (i * 7))
		n++

		if (b & 0b10000000) == 0 {
			break
		}

		buf = buf[1:]
	}

	return v, n, nil
}

// LEB128MarshalSize returns the marshal size of an unsigned integer in LEB128 format.
// Specification: https://aomediacodec.github.io/av1-spec/#leb128
func LEB128MarshalSize(v uint) int {
	n := 0

	for {
		v >>= 7
		n++

		if v <= 0 {
			break
		}
	}

	return n
}

// LEB128MarshalTo encodes an unsigned integer with the LEB128 format.
// Specification: https://aomediacodec.github.io/av1-spec/#leb128
func LEB128MarshalTo(v uint, buf []byte) int {
	n := 0

	for {
		curbyte := byte(v) & 0b01111111
		v >>= 7

		if v <= 0 {
			buf[n] = curbyte
			n++
			break
		}

		curbyte |= 0b10000000
		buf[n] = curbyte
		n++
	}

	return n
}
