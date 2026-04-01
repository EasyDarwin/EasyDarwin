package bits

import (
	"encoding/binary"
	"errors"
	"fmt"
)

// FixedSliceReader - read integers and other data from a fixed slice.
// Accumulates error, and the first error can be retrieved.
// If err != nil, 0 or empty string is returned
type FixedSliceReader struct {
	err   error
	slice []byte
	pos   int
	len   int
}

// bits.NewFixedSliceReader creates a new slice reader reading from data
func NewFixedSliceReader(data []byte) *FixedSliceReader {
	return &FixedSliceReader{
		slice: data,
		pos:   0,
		len:   len(data),
		err:   nil,
	}
}

// AccError - get accumulated error after read operations
func (s *FixedSliceReader) AccError() error {
	return s.err
}

// ReadUint8 - read uint8 from slice
func (s *FixedSliceReader) ReadUint8() byte {
	if s.err != nil {
		return 0
	}
	if s.pos > s.len-1 {
		s.err = ErrSliceRead
		return 0
	}
	res := s.slice[s.pos]
	s.pos++
	return res
}

// ReadUint16 - read uint16 from slice
func (s *FixedSliceReader) ReadUint16() uint16 {
	if s.err != nil {
		return 0
	}
	if s.pos > s.len-2 {
		s.err = ErrSliceRead
		return 0
	}
	res := binary.BigEndian.Uint16(s.slice[s.pos : s.pos+2])
	s.pos += 2
	return res
}

// ReadInt16 - read int16 from slice
func (s *FixedSliceReader) ReadInt16() int16 {
	if s.err != nil {
		return 0
	}
	if s.pos > s.len-2 {
		s.err = ErrSliceRead
		return 0
	}
	res := binary.BigEndian.Uint16(s.slice[s.pos : s.pos+2])
	s.pos += 2
	return int16(res)
}

// ReadUint24 - read uint24 from slice
func (s *FixedSliceReader) ReadUint24() uint32 {
	if s.err != nil {
		return 0
	}
	if s.pos > s.len-3 {
		s.err = ErrSliceRead
		return 0
	}
	res := uint32(binary.BigEndian.Uint16(s.slice[s.pos : s.pos+2]))
	res = res<<8 | uint32(s.slice[s.pos+2])
	s.pos += 3
	return res
}

// ReadUint32 - read uint32 from slice
func (s *FixedSliceReader) ReadUint32() uint32 {
	if s.err != nil {
		return 0
	}
	if s.pos > s.len-4 {
		s.err = ErrSliceRead
		return 0
	}
	res := binary.BigEndian.Uint32(s.slice[s.pos : s.pos+4])
	s.pos += 4
	return res
}

// ReadInt32 - read int32 from slice
func (s *FixedSliceReader) ReadInt32() int32 {
	if s.err != nil {
		return 0
	}
	if s.pos > s.len-4 {
		s.err = ErrSliceRead
		return 0
	}
	res := binary.BigEndian.Uint32(s.slice[s.pos : s.pos+4])
	s.pos += 4
	return int32(res)
}

// ReadUint64 - read uint64 from slice
func (s *FixedSliceReader) ReadUint64() uint64 {
	if s.err != nil {
		return 0
	}
	if s.pos > s.len-8 {
		s.err = ErrSliceRead
		return 0
	}
	res := binary.BigEndian.Uint64(s.slice[s.pos : s.pos+8])
	s.pos += 8
	return res
}

// ReadInt64 - read int64 from slice
func (s *FixedSliceReader) ReadInt64() int64 {
	if s.err != nil {
		return 0
	}
	if s.pos > s.len-8 {
		s.err = ErrSliceRead
		return 0
	}
	res := binary.BigEndian.Uint64(s.slice[s.pos : s.pos+8])
	s.pos += 8
	return int64(res)
}

// ReadFixedLengthString - read string of specified length n.
// Sets err and returns empty string if full length not available
func (s *FixedSliceReader) ReadFixedLengthString(n int) string {
	if s.err != nil {
		return ""
	}
	if s.pos > s.len-n {
		s.err = ErrSliceRead
		return ""
	}
	res := string(s.slice[s.pos : s.pos+n])
	s.pos += n
	return res
}

// ReadZeroTerminatedString - read string until zero byte but at most maxLen
// Set err and return empty string if no zero byte found.
func (s *FixedSliceReader) ReadZeroTerminatedString(maxLen int) string {
	if s.err != nil {
		return ""
	}
	startPos := s.pos
	maxPos := startPos + maxLen
	for {
		if s.pos >= maxPos {
			s.err = errors.New("did not find terminating zero")
			return ""
		}
		c := s.slice[s.pos]
		if c == 0 {
			str := string(s.slice[startPos:s.pos])
			s.pos++ // Next position to read
			return str
		}
		s.pos++
	}
}

// ReadPossiblyZeroTerminatedString - read string until zero byte but at most maxLen.
// If maxLen is reached and no zero-byte, return string and ok = false
func (s *FixedSliceReader) ReadPossiblyZeroTerminatedString(maxLen int) (str string, ok bool) {
	startPos := s.pos
	maxPos := startPos + maxLen
	for {
		if s.pos == maxPos {
			return string(s.slice[startPos:s.pos]), true
		}
		if s.pos > maxPos {
			s.err = errors.New("did not find terminating zero")
			return "", false
		}
		c := s.slice[s.pos]
		if c == 0 {
			str = string(s.slice[startPos:s.pos])
			s.pos++ // Next position to read
			return str, true
		}
		s.pos++
	}
}

// ReadBytes - read a slice of n bytes
// Return empty slice if n bytes not available
func (s *FixedSliceReader) ReadBytes(n int) []byte {
	if s.err != nil {
		return []byte{}
	}
	if s.pos > s.len-n {
		s.err = ErrSliceRead
		return []byte{}
	}
	res := s.slice[s.pos : s.pos+n]
	s.pos += n
	return res
}

// RemainingBytes - return remaining bytes of this slice
func (s *FixedSliceReader) RemainingBytes() []byte {
	if s.err != nil {
		return []byte{}
	}
	res := s.slice[s.pos:]
	s.pos = s.Length()
	return res
}

// NrRemainingBytes - return number of bytes remaining
func (s *FixedSliceReader) NrRemainingBytes() int {
	if s.err != nil {
		return 0
	}
	return s.Length() - s.GetPos()
}

// SkipBytes - skip passed n bytes
func (s *FixedSliceReader) SkipBytes(n int) {
	if s.err != nil {
		return
	}
	if s.pos+n > s.Length() {
		s.err = fmt.Errorf("attempt to skip bytes to pos %d beyond slice len %d", s.pos+n, s.len)
		return
	}
	s.pos += n
}

// SetPos - set read position is slice
func (s *FixedSliceReader) SetPos(pos int) {
	if pos > s.len {
		s.err = fmt.Errorf("attempt to set pos %d beyond slice len %d", pos, s.len)
		return
	}
	s.pos = pos
}

// GetPos - get read position is slice
func (s *FixedSliceReader) GetPos() int {
	return s.pos
}

// Length - get length of slice
func (s *FixedSliceReader) Length() int {
	return s.len
}

// LookAhead returns data ahead of current pos if within bounds.
func (s *FixedSliceReader) LookAhead(offset int, data []byte) error {
	if s.pos+offset+len(data) > s.len {
		return fmt.Errorf("out of bounds")
	}
	copy(data, s.slice[s.pos+offset:])
	return nil
}
