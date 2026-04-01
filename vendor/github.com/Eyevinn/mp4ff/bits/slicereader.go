package bits

import "fmt"

// SliceReader errors
var (
	ErrSliceRead = fmt.Errorf("read too far in SliceReader")
)

type SliceReader interface {
	AccError() error
	ReadUint8() byte
	ReadUint16() uint16
	ReadInt16() int16
	ReadUint24() uint32
	ReadUint32() uint32
	ReadInt32() int32
	ReadUint64() uint64
	ReadInt64() int64
	ReadFixedLengthString(n int) string
	ReadZeroTerminatedString(maxLen int) string
	ReadPossiblyZeroTerminatedString(maxLen int) (str string, ok bool)
	ReadBytes(n int) []byte
	RemainingBytes() []byte
	NrRemainingBytes() int
	SkipBytes(n int)
	SetPos(pos int)
	GetPos() int
	Length() int
	LookAhead(offset int, data []byte) error
}
