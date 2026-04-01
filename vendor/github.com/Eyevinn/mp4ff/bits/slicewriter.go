package bits

import (
	"errors"
)

var ErrSliceWrite = errors.New("overflow in SliceWriter")

type SliceWriter interface {
	Len() int
	Capacity() int
	Offset() int
	Bytes() []byte
	AccError() error
	WriteUint8(n byte)
	WriteUint16(n uint16)
	WriteInt16(n int16)
	WriteUint24(n uint32)
	WriteUint32(n uint32)
	WriteInt32(n int32)
	WriteUint48(u uint64)
	WriteUint64(n uint64)
	WriteInt64(n int64)
	WriteString(s string, addZeroEnd bool)
	WriteZeroBytes(n int)
	WriteBytes(byteSlice []byte)
	WriteUnityMatrix()
	WriteBits(bits uint, n int)
	WriteFlag(f bool)
	FlushBits()
}
