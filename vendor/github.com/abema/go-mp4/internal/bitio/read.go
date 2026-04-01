package bitio

import "io"

type Reader interface {
	io.Reader

	// alignment:
	//  |-1-byte-block-|--------------|--------------|--------------|
	//  |<-offset->|<-------------------width---------------------->|
	ReadBits(width uint) (data []byte, err error)

	ReadBit() (bit bool, err error)
}

type ReadSeeker interface {
	Reader
	io.Seeker
}

type reader struct {
	reader io.Reader
	octet  byte
	width  uint
}

func NewReader(r io.Reader) Reader {
	return &reader{reader: r}
}

func (r *reader) Read(p []byte) (n int, err error) {
	if r.width != 0 {
		return 0, ErrInvalidAlignment
	}
	return r.reader.Read(p)
}

func (r *reader) ReadBits(size uint) ([]byte, error) {
	bytes := (size + 7) / 8
	data := make([]byte, bytes)
	offset := (bytes * 8) - (size)

	for i := uint(0); i < size; i++ {
		bit, err := r.ReadBit()
		if err != nil {
			return nil, err
		}

		byteIdx := (offset + i) / 8
		bitIdx := 7 - (offset+i)%8
		if bit {
			data[byteIdx] |= 0x1 << bitIdx
		}
	}

	return data, nil
}

func (r *reader) ReadBit() (bool, error) {
	if r.width == 0 {
		buf := make([]byte, 1)
		if n, err := r.reader.Read(buf); err != nil {
			return false, err
		} else if n != 1 {
			return false, ErrDiscouragedReader
		}
		r.octet = buf[0]
		r.width = 8
	}

	r.width--
	return (r.octet>>r.width)&0x01 != 0, nil
}

type readSeeker struct {
	reader
	seeker io.Seeker
}

func NewReadSeeker(r io.ReadSeeker) ReadSeeker {
	return &readSeeker{
		reader: reader{reader: r},
		seeker: r,
	}
}

func (r *readSeeker) Seek(offset int64, whence int) (int64, error) {
	if whence == io.SeekCurrent && r.reader.width != 0 {
		return 0, ErrInvalidAlignment
	}
	n, err := r.seeker.Seek(offset, whence)
	if err != nil {
		return n, err
	}
	r.reader.width = 0
	return n, nil
}
