package bitio

import (
	"io"
)

type Writer interface {
	io.Writer

	// alignment:
	//  |-1-byte-block-|--------------|--------------|--------------|
	//  |<-offset->|<-------------------width---------------------->|
	WriteBits(data []byte, width uint) error

	WriteBit(bit bool) error
}

type writer struct {
	writer io.Writer
	octet  byte
	width  uint
}

func NewWriter(w io.Writer) Writer {
	return &writer{writer: w}
}

func (w *writer) Write(p []byte) (n int, err error) {
	if w.width != 0 {
		return 0, ErrInvalidAlignment
	}
	return w.writer.Write(p)
}

func (w *writer) WriteBits(data []byte, width uint) error {
	length := uint(len(data)) * 8
	offset := length - width
	for i := offset; i < length; i++ {
		oi := i / 8
		if err := w.WriteBit((data[oi]>>(7-i%8))&0x01 != 0); err != nil {
			return err
		}
	}
	return nil
}

func (w *writer) WriteBit(bit bool) error {
	if bit {
		w.octet |= 0x1 << (7 - w.width)
	}
	w.width++

	if w.width == 8 {
		if _, err := w.writer.Write([]byte{w.octet}); err != nil {
			return err
		}
		w.octet = 0x00
		w.width = 0
	}
	return nil
}
