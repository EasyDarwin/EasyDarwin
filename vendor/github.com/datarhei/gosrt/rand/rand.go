package rand

import "crypto/rand"

var AlphaNumericCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

// https://www.calhoun.io/creating-random-strings-in-go/
func RandomString(length int, charset string) (string, error) {
	b := make([]byte, length)
	for i := range b {
		j, err := Int63n(int64(len(charset)))
		if err != nil {
			return "", err
		}
		b[i] = charset[int(j)]
	}

	return string(b), nil
}

func Read(b []byte) (int, error) {
	return rand.Read(b)
}

func Uint32() (uint32, error) {
	var b [4]byte
	_, err := rand.Read(b[:])
	if err != nil {
		return 0, err
	}

	return uint32(b[0])<<24 | uint32(b[1])<<16 | uint32(b[2])<<8 | uint32(b[3]), nil
}

func Int63() (int64, error) {
	var b [8]byte
	_, err := rand.Read(b[:])
	if err != nil {
		return 0, err
	}

	return int64(uint64(b[0]&0b01111111)<<56 | uint64(b[1])<<48 | uint64(b[2])<<40 | uint64(b[3])<<32 |
		uint64(b[4])<<24 | uint64(b[5])<<16 | uint64(b[6])<<8 | uint64(b[7])), nil
}

// https://cs.opensource.google/go/go/+/refs/tags/go1.20.4:src/math/rand/rand.go;l=119
func Int63n(n int64) (int64, error) {
	if n&(n-1) == 0 { // n is power of two, can mask
		r, err := Int63()
		if err != nil {
			return 0, err
		}
		return r & (n - 1), nil
	}

	max := int64((1 << 63) - 1 - (1<<63)%uint64(n))

	v, err := Int63()
	if err != nil {
		return 0, err
	}

	for v > max {
		v, err = Int63()
		if err != nil {
			return 0, err
		}
	}

	return v % n, nil
}
