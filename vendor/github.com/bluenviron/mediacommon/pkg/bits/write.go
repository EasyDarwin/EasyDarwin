package bits

// WriteBits writes N bits.
func WriteBits(buf []byte, pos *int, bits uint64, n int) {
	res := 8 - (*pos & 0x07)
	if n < res {
		buf[*pos>>0x03] |= byte(bits << (res - n))
		*pos += n
		return
	}

	buf[*pos>>3] |= byte(bits >> (n - res))
	*pos += res
	n -= res

	for n >= 8 {
		buf[*pos>>3] = byte(bits >> (n - 8))
		*pos += 8
		n -= 8
	}

	if n > 0 {
		buf[*pos>>3] = byte((bits & (1<<n - 1)) << (8 - n))
		*pos += n
	}
}
