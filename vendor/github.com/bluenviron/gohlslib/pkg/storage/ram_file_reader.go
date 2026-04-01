package storage

import (
	"io"
)

type ramFileReader struct {
	parts   []*partRAM
	curPart int
	curPos  int
}

func (r *ramFileReader) Read(p []byte) (int, error) {
	n := 0
	lenp := len(p)

	for {
		if r.curPart >= len(r.parts) {
			return n, io.EOF
		}

		curp := r.parts[r.curPart]
		buf := curp.buffer.Bytes()

		copied := copy(p[n:], buf[r.curPos:])
		r.curPos += copied
		n += copied

		if r.curPos == len(buf) {
			r.curPart++
			r.curPos = 0
		}

		if n == lenp {
			return n, nil
		}
	}
}
