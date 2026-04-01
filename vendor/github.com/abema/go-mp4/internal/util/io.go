package util

import (
	"bytes"
	"io"
)

func ReadString(r io.Reader) (string, error) {
	b := make([]byte, 1)
	buf := bytes.NewBuffer(nil)
	for {
		if _, err := r.Read(b); err != nil {
			return "", err
		}
		if b[0] == 0 {
			return buf.String(), nil
		}
		buf.Write(b)
	}
}

func WriteString(w io.Writer, s string) error {
	if _, err := w.Write([]byte(s)); err != nil {
		return err
	}
	if _, err := w.Write([]byte{0}); err != nil {
		return err
	}
	return nil
}
