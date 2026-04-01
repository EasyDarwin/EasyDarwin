package mp4

import (
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"strings"
)

var ErrBoxInfoNotFound = errors.New("box info not found")

// BoxType is mpeg box type
type BoxType [4]byte

func StrToBoxType(code string) BoxType {
	if len(code) != 4 {
		panic(fmt.Errorf("invalid box type id length: [%s]", code))
	}
	return BoxType{code[0], code[1], code[2], code[3]}
}

// Uint32ToBoxType returns a new BoxType from the provied uint32
func Uint32ToBoxType(i uint32) BoxType {
	b := make([]byte, 4)
	binary.BigEndian.PutUint32(b, i)
	return BoxType{b[0], b[1], b[2], b[3]}
}

func (boxType BoxType) String() string {
	if isPrintable(boxType[0]) && isPrintable(boxType[1]) && isPrintable(boxType[2]) && isPrintable(boxType[3]) {
		s := string([]byte{boxType[0], boxType[1], boxType[2], boxType[3]})
		s = strings.ReplaceAll(s, string([]byte{0xa9}), "(c)")
		return s
	}
	return fmt.Sprintf("0x%02x%02x%02x%02x", boxType[0], boxType[1], boxType[2], boxType[3])
}

func isASCII(c byte) bool {
	return c >= 0x20 && c <= 0x7e
}

func isPrintable(c byte) bool {
	return isASCII(c) || c == 0xa9
}

func (lhs BoxType) MatchWith(rhs BoxType) bool {
	if lhs == boxTypeAny || rhs == boxTypeAny {
		return true
	}
	return lhs == rhs
}

var boxTypeAny = BoxType{0x00, 0x00, 0x00, 0x00}

func BoxTypeAny() BoxType {
	return boxTypeAny
}

type boxDef struct {
	dataType reflect.Type
	versions []uint8
	isTarget func(Context) bool
	fields   []*field
}

var boxMap = make(map[BoxType][]boxDef, 64)

func AddBoxDef(payload IBox, versions ...uint8) {
	boxMap[payload.GetType()] = append(boxMap[payload.GetType()], boxDef{
		dataType: reflect.TypeOf(payload).Elem(),
		versions: versions,
		fields:   buildFields(payload),
	})
}

func AddBoxDefEx(payload IBox, isTarget func(Context) bool, versions ...uint8) {
	boxMap[payload.GetType()] = append(boxMap[payload.GetType()], boxDef{
		dataType: reflect.TypeOf(payload).Elem(),
		versions: versions,
		isTarget: isTarget,
		fields:   buildFields(payload),
	})
}

func AddAnyTypeBoxDef(payload IAnyType, boxType BoxType, versions ...uint8) {
	boxMap[boxType] = append(boxMap[boxType], boxDef{
		dataType: reflect.TypeOf(payload).Elem(),
		versions: versions,
		fields:   buildFields(payload),
	})
}

func AddAnyTypeBoxDefEx(payload IAnyType, boxType BoxType, isTarget func(Context) bool, versions ...uint8) {
	boxMap[boxType] = append(boxMap[boxType], boxDef{
		dataType: reflect.TypeOf(payload).Elem(),
		versions: versions,
		isTarget: isTarget,
		fields:   buildFields(payload),
	})
}

var itemBoxFields = buildFields(&Item{})

func (boxType BoxType) getBoxDef(ctx Context) *boxDef {
	boxDefs := boxMap[boxType]
	for i := len(boxDefs) - 1; i >= 0; i-- {
		boxDef := &boxDefs[i]
		if boxDef.isTarget == nil || boxDef.isTarget(ctx) {
			return boxDef
		}
	}
	if ctx.UnderIlst {
		typeID := int(binary.BigEndian.Uint32(boxType[:]))
		if typeID >= 1 && typeID <= ctx.QuickTimeKeysMetaEntryCount {
			return &boxDef{
				dataType: reflect.TypeOf(Item{}),
				isTarget: isIlstMetaContainer,
				fields:   itemBoxFields,
			}
		}
	}
	return nil
}

func (boxType BoxType) IsSupported(ctx Context) bool {
	return boxType.getBoxDef(ctx) != nil
}

func (boxType BoxType) New(ctx Context) (IBox, error) {
	boxDef := boxType.getBoxDef(ctx)
	if boxDef == nil {
		return nil, ErrBoxInfoNotFound
	}

	box, ok := reflect.New(boxDef.dataType).Interface().(IBox)
	if !ok {
		return nil, fmt.Errorf("box type not implements IBox interface: %s", boxType.String())
	}

	anyTypeBox, ok := box.(IAnyType)
	if ok {
		anyTypeBox.SetType(boxType)
	}

	return box, nil
}

func (boxType BoxType) GetSupportedVersions(ctx Context) ([]uint8, error) {
	boxDef := boxType.getBoxDef(ctx)
	if boxDef == nil {
		return nil, ErrBoxInfoNotFound
	}
	return boxDef.versions, nil
}

func (boxType BoxType) IsSupportedVersion(ver uint8, ctx Context) bool {
	boxDef := boxType.getBoxDef(ctx)
	if boxDef == nil {
		return false
	}
	if len(boxDef.versions) == 0 {
		return true
	}
	for _, sver := range boxDef.versions {
		if ver == sver {
			return true
		}
	}
	return false
}
