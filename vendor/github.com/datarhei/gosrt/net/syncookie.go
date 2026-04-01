package net

import (
	"crypto/md5"
	"encoding/binary"
	"strconv"
	"time"

	"github.com/datarhei/gosrt/rand"
)

// SYNCookie implements a syn cookie for the SRT handshake.
type SYNCookie struct {
	secret1 string
	secret2 string
	daddr   string
	counter func() int64
}

func defaultCounter() int64 {
	return time.Now().Unix() >> 6
}

// NewSYNCookie returns a SYNCookie for a destination address.
func NewSYNCookie(daddr string, counter func() int64) (*SYNCookie, error) {
	s := &SYNCookie{
		daddr:   daddr,
		counter: counter,
	}

	if s.counter == nil {
		s.counter = defaultCounter
	}

	var err error
	s.secret1, err = rand.RandomString(32, rand.AlphaNumericCharset)
	if err != nil {
		return nil, err
	}

	s.secret2, err = rand.RandomString(32, rand.AlphaNumericCharset)
	if err != nil {
		return nil, err
	}

	return s, nil
}

// Get returns the current syn cookie with a sender address.
func (s *SYNCookie) Get(saddr string) uint32 {
	return s.calculate(s.counter(), saddr)
}

// Verify verfies that two syn cookies relate.
func (s *SYNCookie) Verify(cookie uint32, saddr string) bool {
	counter := s.counter()

	if s.calculate(counter, saddr) == cookie {
		return true
	}

	if s.calculate(counter-1, saddr) == cookie {
		return true
	}

	return false
}

func (s *SYNCookie) calculate(counter int64, saddr string) uint32 {
	data := s.secret1 + s.daddr + saddr + s.secret2 + strconv.FormatInt(counter, 10)

	md5sum := md5.Sum([]byte(data))

	return binary.BigEndian.Uint32(md5sum[0:])
}
