package mp4

import "fmt"

/*************************** esds ****************************/

// https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html

func BoxTypeEsds() BoxType { return StrToBoxType("esds") }

func init() {
	AddBoxDef(&Esds{}, 0)
}

const (
	ESDescrTag            = 0x03
	DecoderConfigDescrTag = 0x04
	DecSpecificInfoTag    = 0x05
	SLConfigDescrTag      = 0x06
)

// Esds is ES descripter box
type Esds struct {
	FullBox     `mp4:"0,extend"`
	Descriptors []Descriptor `mp4:"1,array"`
}

// GetType returns the BoxType
func (*Esds) GetType() BoxType {
	return BoxTypeEsds()
}

type Descriptor struct {
	BaseCustomFieldObject
	Tag                     int8                     `mp4:"0,size=8"` // must be 0x03
	Size                    uint32                   `mp4:"1,varint"`
	ESDescriptor            *ESDescriptor            `mp4:"2,extend,opt=dynamic"`
	DecoderConfigDescriptor *DecoderConfigDescriptor `mp4:"3,extend,opt=dynamic"`
	Data                    []byte                   `mp4:"4,size=8,opt=dynamic,len=dynamic"`
}

// GetFieldLength returns length of dynamic field
func (ds *Descriptor) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Data":
		return uint(ds.Size)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=esds fieldName=%s", name))
}

func (ds *Descriptor) IsOptFieldEnabled(name string, ctx Context) bool {
	switch ds.Tag {
	case ESDescrTag:
		return name == "ESDescriptor"
	case DecoderConfigDescrTag:
		return name == "DecoderConfigDescriptor"
	default:
		return name == "Data"
	}
}

// StringifyField returns field value as string
func (ds *Descriptor) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "Tag":
		switch ds.Tag {
		case ESDescrTag:
			return "ESDescr", true
		case DecoderConfigDescrTag:
			return "DecoderConfigDescr", true
		case DecSpecificInfoTag:
			return "DecSpecificInfo", true
		case SLConfigDescrTag:
			return "SLConfigDescr", true
		default:
			return "", false
		}
	default:
		return "", false
	}
}

type ESDescriptor struct {
	BaseCustomFieldObject
	ESID                 uint16 `mp4:"0,size=16"`
	StreamDependenceFlag bool   `mp4:"1,size=1"`
	UrlFlag              bool   `mp4:"2,size=1"`
	OcrStreamFlag        bool   `mp4:"3,size=1"`
	StreamPriority       int8   `mp4:"4,size=5"`
	DependsOnESID        uint16 `mp4:"5,size=16,opt=dynamic"`
	URLLength            uint8  `mp4:"6,size=8,opt=dynamic"`
	URLString            []byte `mp4:"7,size=8,len=dynamic,opt=dynamic,string"`
	OCRESID              uint16 `mp4:"8,size=16,opt=dynamic"`
}

func (esds *ESDescriptor) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "URLString":
		return uint(esds.URLLength)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=ESDescriptor fieldName=%s", name))
}

func (esds *ESDescriptor) IsOptFieldEnabled(name string, ctx Context) bool {
	switch name {
	case "DependsOnESID":
		return esds.StreamDependenceFlag
	case "URLLength", "URLString":
		return esds.UrlFlag
	case "OCRESID":
		return esds.OcrStreamFlag
	default:
		return false
	}
}

type DecoderConfigDescriptor struct {
	BaseCustomFieldObject
	ObjectTypeIndication byte   `mp4:"0,size=8"`
	StreamType           int8   `mp4:"1,size=6"`
	UpStream             bool   `mp4:"2,size=1"`
	Reserved             bool   `mp4:"3,size=1"`
	BufferSizeDB         uint32 `mp4:"4,size=24"`
	MaxBitrate           uint32 `mp4:"5,size=32"`
	AvgBitrate           uint32 `mp4:"6,size=32"`
}
