package mp4

import (
	"fmt"

	"github.com/abema/go-mp4/internal/util"
)

/*************************** ilst ****************************/

func BoxTypeIlst() BoxType { return StrToBoxType("ilst") }
func BoxTypeData() BoxType { return StrToBoxType("data") }

var ilstMetaBoxTypes = []BoxType{
	StrToBoxType("----"),
	StrToBoxType("aART"),
	StrToBoxType("akID"),
	StrToBoxType("apID"),
	StrToBoxType("atID"),
	StrToBoxType("cmID"),
	StrToBoxType("cnID"),
	StrToBoxType("covr"),
	StrToBoxType("cpil"),
	StrToBoxType("cprt"),
	StrToBoxType("desc"),
	StrToBoxType("disk"),
	StrToBoxType("egid"),
	StrToBoxType("geID"),
	StrToBoxType("gnre"),
	StrToBoxType("pcst"),
	StrToBoxType("pgap"),
	StrToBoxType("plID"),
	StrToBoxType("purd"),
	StrToBoxType("purl"),
	StrToBoxType("rtng"),
	StrToBoxType("sfID"),
	StrToBoxType("soaa"),
	StrToBoxType("soal"),
	StrToBoxType("soar"),
	StrToBoxType("soco"),
	StrToBoxType("sonm"),
	StrToBoxType("sosn"),
	StrToBoxType("stik"),
	StrToBoxType("tmpo"),
	StrToBoxType("trkn"),
	StrToBoxType("tven"),
	StrToBoxType("tves"),
	StrToBoxType("tvnn"),
	StrToBoxType("tvsh"),
	StrToBoxType("tvsn"),
	{0xA9, 'A', 'R', 'T'},
	{0xA9, 'a', 'l', 'b'},
	{0xA9, 'c', 'm', 't'},
	{0xA9, 'c', 'o', 'm'},
	{0xA9, 'd', 'a', 'y'},
	{0xA9, 'g', 'e', 'n'},
	{0xA9, 'g', 'r', 'p'},
	{0xA9, 'n', 'a', 'm'},
	{0xA9, 't', 'o', 'o'},
	{0xA9, 'w', 'r', 't'},
}

func IsIlstMetaBoxType(boxType BoxType) bool {
	for _, bt := range ilstMetaBoxTypes {
		if boxType == bt {
			return true
		}
	}
	return false
}

func init() {
	AddBoxDef(&Ilst{})
	AddBoxDefEx(&Data{}, isUnderIlstMeta)
	for _, bt := range ilstMetaBoxTypes {
		AddAnyTypeBoxDefEx(&IlstMetaContainer{}, bt, isIlstMetaContainer)
	}
	AddAnyTypeBoxDefEx(&StringData{}, StrToBoxType("mean"), isUnderIlstFreeFormat)
	AddAnyTypeBoxDefEx(&StringData{}, StrToBoxType("name"), isUnderIlstFreeFormat)
}

type Ilst struct {
	Box
}

// GetType returns the BoxType
func (*Ilst) GetType() BoxType {
	return BoxTypeIlst()
}

type IlstMetaContainer struct {
	AnyTypeBox
}

func isIlstMetaContainer(ctx Context) bool {
	return ctx.UnderIlst && !ctx.UnderIlstMeta
}

const (
	DataTypeBinary             = 0
	DataTypeStringUTF8         = 1
	DataTypeStringUTF16        = 2
	DataTypeStringMac          = 3
	DataTypeStringJPEG         = 14
	DataTypeSignedIntBigEndian = 21
	DataTypeFloat32BigEndian   = 22
	DataTypeFloat64BigEndian   = 23
)

// Data is a Value BoxType
// https://developer.apple.com/documentation/quicktime-file-format/value_atom
type Data struct {
	Box
	DataType uint32 `mp4:"0,size=32"`
	DataLang uint32 `mp4:"1,size=32"`
	Data     []byte `mp4:"2,size=8"`
}

// GetType returns the BoxType
func (*Data) GetType() BoxType {
	return BoxTypeData()
}

func isUnderIlstMeta(ctx Context) bool {
	return ctx.UnderIlstMeta
}

// StringifyField returns field value as string
func (data *Data) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "DataType":
		switch data.DataType {
		case DataTypeBinary:
			return "BINARY", true
		case DataTypeStringUTF8:
			return "UTF8", true
		case DataTypeStringUTF16:
			return "UTF16", true
		case DataTypeStringMac:
			return "MAC_STR", true
		case DataTypeStringJPEG:
			return "JPEG", true
		case DataTypeSignedIntBigEndian:
			return "INT", true
		case DataTypeFloat32BigEndian:
			return "FLOAT32", true
		case DataTypeFloat64BigEndian:
			return "FLOAT64", true
		}
	case "Data":
		switch data.DataType {
		case DataTypeStringUTF8:
			return fmt.Sprintf("\"%s\"", util.EscapeUnprintables(string(data.Data))), true
		}
	}
	return "", false
}

type StringData struct {
	AnyTypeBox
	Data []byte `mp4:"0,size=8"`
}

// StringifyField returns field value as string
func (sd *StringData) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	if name == "Data" {
		return fmt.Sprintf("\"%s\"", util.EscapeUnprintables(string(sd.Data))), true
	}
	return "", false
}

/*************************** numbered items ****************************/

// Item is a numbered item under an item list atom
// https://developer.apple.com/documentation/quicktime-file-format/metadata_item_list_atom/item_list
type Item struct {
	AnyTypeBox
	Version  uint8   `mp4:"0,size=8"`
	Flags    [3]byte `mp4:"1,size=8"`
	ItemName []byte  `mp4:"2,size=8,len=4"`
	Data     Data    `mp4:"3"`
}

// StringifyField returns field value as string
func (i *Item) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "ItemName":
		return fmt.Sprintf("\"%s\"", util.EscapeUnprintables(string(i.ItemName))), true
	}
	return "", false
}

func isUnderIlstFreeFormat(ctx Context) bool {
	return ctx.UnderIlstFreeMeta
}

func BoxTypeKeys() BoxType { return StrToBoxType("keys") }

func init() {
	AddBoxDef(&Keys{})
}

/*************************** keys ****************************/

// Keys is the Keys BoxType
// https://developer.apple.com/documentation/quicktime-file-format/metadata_item_keys_atom
type Keys struct {
	FullBox    `mp4:"0,extend"`
	EntryCount int32 `mp4:"1,size=32"`
	Entries    []Key `mp4:"2,len=dynamic"`
}

// GetType implements the IBox interface and returns the BoxType
func (*Keys) GetType() BoxType {
	return BoxTypeKeys()
}

// GetFieldLength implements the ICustomFieldObject interface and returns the length of dynamic fields
func (k *Keys) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Entries":
		return uint(k.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=keys fieldName=%s", name))
}

/*************************** key ****************************/

// Key is a key value field in the Keys BoxType
// https://developer.apple.com/documentation/quicktime-file-format/metadata_item_keys_atom/key_value_key_size-8
type Key struct {
	BaseCustomFieldObject
	KeySize      int32  `mp4:"0,size=32"`
	KeyNamespace []byte `mp4:"1,size=8,len=4"`
	KeyValue     []byte `mp4:"2,size=8,len=dynamic"`
}

// GetFieldLength implements the ICustomFieldObject interface and returns the length of dynamic fields
func (k *Key) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "KeyValue":
		// sizeOf(KeySize)+sizeOf(KeyNamespace) = 8 bytes
		return uint(k.KeySize) - 8
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=key fieldName=%s", name))
}

// StringifyField returns field value as string
func (k *Key) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "KeyNamespace":
		return fmt.Sprintf("\"%s\"", util.EscapeUnprintables(string(k.KeyNamespace))), true
	case "KeyValue":
		return fmt.Sprintf("\"%s\"", util.EscapeUnprintables(string(k.KeyValue))), true
	}
	return "", false
}
