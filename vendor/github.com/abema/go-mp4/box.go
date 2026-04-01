package mp4

import (
	"errors"
	"io"
	"math"

	"github.com/abema/go-mp4/internal/bitio"
)

const LengthUnlimited = math.MaxUint32

type ICustomFieldObject interface {
	// GetFieldSize returns size of dynamic field
	GetFieldSize(name string, ctx Context) uint

	// GetFieldLength returns length of dynamic field
	GetFieldLength(name string, ctx Context) uint

	// IsOptFieldEnabled check whether if the optional field is enabled
	IsOptFieldEnabled(name string, ctx Context) bool

	// StringifyField returns field value as string
	StringifyField(name string, indent string, depth int, ctx Context) (string, bool)

	IsPString(name string, bytes []byte, remainingSize uint64, ctx Context) bool

	BeforeUnmarshal(r io.ReadSeeker, size uint64, ctx Context) (n uint64, override bool, err error)

	OnReadField(name string, r bitio.ReadSeeker, leftBits uint64, ctx Context) (rbits uint64, override bool, err error)

	OnWriteField(name string, w bitio.Writer, ctx Context) (wbits uint64, override bool, err error)
}

type BaseCustomFieldObject struct {
}

// GetFieldSize returns size of dynamic field
func (box *BaseCustomFieldObject) GetFieldSize(string, Context) uint {
	panic(errors.New("GetFieldSize not implemented"))
}

// GetFieldLength returns length of dynamic field
func (box *BaseCustomFieldObject) GetFieldLength(string, Context) uint {
	panic(errors.New("GetFieldLength not implemented"))
}

// IsOptFieldEnabled check whether if the optional field is enabled
func (box *BaseCustomFieldObject) IsOptFieldEnabled(string, Context) bool {
	return false
}

// StringifyField returns field value as string
func (box *BaseCustomFieldObject) StringifyField(string, string, int, Context) (string, bool) {
	return "", false
}

func (*BaseCustomFieldObject) IsPString(name string, bytes []byte, remainingSize uint64, ctx Context) bool {
	return true
}

func (*BaseCustomFieldObject) BeforeUnmarshal(io.ReadSeeker, uint64, Context) (uint64, bool, error) {
	return 0, false, nil
}

func (*BaseCustomFieldObject) OnReadField(string, bitio.ReadSeeker, uint64, Context) (uint64, bool, error) {
	return 0, false, nil
}

func (*BaseCustomFieldObject) OnWriteField(string, bitio.Writer, Context) (uint64, bool, error) {
	return 0, false, nil
}

// IImmutableBox is common interface of box
type IImmutableBox interface {
	ICustomFieldObject

	// GetVersion returns the box version
	GetVersion() uint8

	// GetFlags returns the flags
	GetFlags() uint32

	// CheckFlag checks the flag status
	CheckFlag(uint32) bool

	// GetType returns the BoxType
	GetType() BoxType
}

// IBox is common interface of box
type IBox interface {
	IImmutableBox

	// SetVersion sets the box version
	SetVersion(uint8)

	// SetFlags sets the flags
	SetFlags(uint32)

	// AddFlag adds the flag
	AddFlag(uint32)

	// RemoveFlag removes the flag
	RemoveFlag(uint32)
}

type Box struct {
	BaseCustomFieldObject
}

// GetVersion returns the box version
func (box *Box) GetVersion() uint8 {
	return 0
}

// SetVersion sets the box version
func (box *Box) SetVersion(uint8) {
}

// GetFlags returns the flags
func (box *Box) GetFlags() uint32 {
	return 0x000000
}

// CheckFlag checks the flag status
func (box *Box) CheckFlag(flag uint32) bool {
	return true
}

// SetFlags sets the flags
func (box *Box) SetFlags(uint32) {
}

// AddFlag adds the flag
func (box *Box) AddFlag(flag uint32) {
}

// RemoveFlag removes the flag
func (box *Box) RemoveFlag(flag uint32) {
}

// FullBox is ISOBMFF FullBox
type FullBox struct {
	BaseCustomFieldObject
	Version uint8   `mp4:"0,size=8"`
	Flags   [3]byte `mp4:"1,size=8"`
}

// GetVersion returns the box version
func (box *FullBox) GetVersion() uint8 {
	return box.Version
}

// SetVersion sets the box version
func (box *FullBox) SetVersion(version uint8) {
	box.Version = version
}

// GetFlags returns the flags
func (box *FullBox) GetFlags() uint32 {
	flag := uint32(box.Flags[0]) << 16
	flag ^= uint32(box.Flags[1]) << 8
	flag ^= uint32(box.Flags[2])
	return flag
}

// CheckFlag checks the flag status
func (box *FullBox) CheckFlag(flag uint32) bool {
	return box.GetFlags()&flag != 0
}

// SetFlags sets the flags
func (box *FullBox) SetFlags(flags uint32) {
	box.Flags[0] = byte(flags >> 16)
	box.Flags[1] = byte(flags >> 8)
	box.Flags[2] = byte(flags)
}

// AddFlag adds the flag
func (box *FullBox) AddFlag(flag uint32) {
	box.SetFlags(box.GetFlags() | flag)
}

// RemoveFlag removes the flag
func (box *FullBox) RemoveFlag(flag uint32) {
	box.SetFlags(box.GetFlags() & (^flag))
}
