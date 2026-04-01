package mp4

import (
	"bytes"
	"fmt"

	"github.com/google/uuid"
)

/*************************** pssh ****************************/

func BoxTypePssh() BoxType { return StrToBoxType("pssh") }

func init() {
	AddBoxDef(&Pssh{}, 0, 1)
}

// Pssh is ISOBMFF pssh box type
type Pssh struct {
	FullBox  `mp4:"0,extend"`
	SystemID [16]byte  `mp4:"1,size=8,uuid"`
	KIDCount uint32    `mp4:"2,size=32,nver=0"`
	KIDs     []PsshKID `mp4:"3,nver=0,len=dynamic,size=128"`
	DataSize int32     `mp4:"4,size=32"`
	Data     []byte    `mp4:"5,size=8,len=dynamic"`
}

type PsshKID struct {
	KID [16]byte `mp4:"0,size=8,uuid"`
}

// GetFieldLength returns length of dynamic field
func (pssh *Pssh) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "KIDs":
		return uint(pssh.KIDCount)
	case "Data":
		return uint(pssh.DataSize)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=pssh fieldName=%s", name))
}

// StringifyField returns field value as string
func (pssh *Pssh) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "KIDs":
		buf := bytes.NewBuffer(nil)
		buf.WriteString("[")
		for i, e := range pssh.KIDs {
			if i != 0 {
				buf.WriteString(", ")
			}
			buf.WriteString(uuid.UUID(e.KID).String())
		}
		buf.WriteString("]")
		return buf.String(), true

	default:
		return "", false
	}
}

// GetType returns the BoxType
func (*Pssh) GetType() BoxType {
	return BoxTypePssh()
}

/*************************** tenc ****************************/

func BoxTypeTenc() BoxType { return StrToBoxType("tenc") }

func init() {
	AddBoxDef(&Tenc{}, 0, 1)
}

// Tenc is ISOBMFF tenc box type
type Tenc struct {
	FullBox                `mp4:"0,extend"`
	Reserved               uint8    `mp4:"1,size=8,dec"`
	DefaultCryptByteBlock  uint8    `mp4:"2,size=4,dec"` // always 0 on version 0
	DefaultSkipByteBlock   uint8    `mp4:"3,size=4,dec"` // always 0 on version 0
	DefaultIsProtected     uint8    `mp4:"4,size=8,dec"`
	DefaultPerSampleIVSize uint8    `mp4:"5,size=8,dec"`
	DefaultKID             [16]byte `mp4:"6,size=8,uuid"`
	DefaultConstantIVSize  uint8    `mp4:"7,size=8,opt=dynamic,dec"`
	DefaultConstantIV      []byte   `mp4:"8,size=8,opt=dynamic,len=dynamic"`
}

func (tenc *Tenc) IsOptFieldEnabled(name string, ctx Context) bool {
	switch name {
	case "DefaultConstantIVSize", "DefaultConstantIV":
		return tenc.DefaultIsProtected == 1 && tenc.DefaultPerSampleIVSize == 0
	}
	return false
}

func (tenc *Tenc) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "DefaultConstantIV":
		return uint(tenc.DefaultConstantIVSize)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=tenc fieldName=%s", name))
}

// GetType returns the BoxType
func (*Tenc) GetType() BoxType {
	return BoxTypeTenc()
}
