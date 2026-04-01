package mp4

import (
	"errors"
	"io"
)

type Writer struct {
	writer  io.WriteSeeker
	biStack []*BoxInfo
}

func NewWriter(w io.WriteSeeker) *Writer {
	return &Writer{
		writer: w,
	}
}

func (w *Writer) Write(p []byte) (int, error) {
	return w.writer.Write(p)
}

func (w *Writer) Seek(offset int64, whence int) (int64, error) {
	return w.writer.Seek(offset, whence)
}

func (w *Writer) StartBox(bi *BoxInfo) (*BoxInfo, error) {
	bi, err := WriteBoxInfo(w.writer, bi)
	if err != nil {
		return nil, err
	}
	w.biStack = append(w.biStack, bi)
	return bi, nil
}

func (w *Writer) EndBox() (*BoxInfo, error) {
	bi := w.biStack[len(w.biStack)-1]
	w.biStack = w.biStack[:len(w.biStack)-1]
	end, err := w.writer.Seek(0, io.SeekCurrent)
	if err != nil {
		return nil, err
	}
	bi.Size = uint64(end) - bi.Offset
	if _, err = bi.SeekToStart(w.writer); err != nil {
		return nil, err
	}
	if bi2, err := WriteBoxInfo(w.writer, bi); err != nil {
		return nil, err
	} else if bi.HeaderSize != bi2.HeaderSize {
		return nil, errors.New("header size changed")
	}
	if _, err := w.writer.Seek(end, io.SeekStart); err != nil {
		return nil, err
	}
	return bi, nil
}

func (w *Writer) CopyBox(r io.ReadSeeker, bi *BoxInfo) error {
	if _, err := bi.SeekToStart(r); err != nil {
		return err
	}
	if n, err := io.CopyN(w, r, int64(bi.Size)); err != nil {
		return err
	} else if n != int64(bi.Size) {
		return errors.New("failed to copy box")
	}
	return nil
}
