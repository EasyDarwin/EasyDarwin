package mp4

import (
	"fmt"
	"os"
	"reflect"
	"sort"
	"strconv"
	"strings"
)

type (
	stringType uint8
	fieldFlag  uint16
)

const (
	stringType_C stringType = iota
	stringType_C_P

	fieldString        fieldFlag = 1 << iota // 0
	fieldExtend                              // 1
	fieldDec                                 // 2
	fieldHex                                 // 3
	fieldISO639_2                            // 4
	fieldUUID                                // 5
	fieldHidden                              // 6
	fieldOptDynamic                          // 7
	fieldVarint                              // 8
	fieldSizeDynamic                         // 9
	fieldLengthDynamic                       // 10
)

type field struct {
	children []*field
	name     string
	cnst     string
	order    int
	optFlag  uint32
	nOptFlag uint32
	size     uint
	length   uint
	flags    fieldFlag
	strType  stringType
	version  uint8
	nVersion uint8
}

func (f *field) set(flag fieldFlag) {
	f.flags |= flag
}

func (f *field) is(flag fieldFlag) bool {
	return f.flags&flag != 0
}

func buildFields(box IImmutableBox) []*field {
	t := reflect.TypeOf(box).Elem()
	return buildFieldsStruct(t)
}

func buildFieldsStruct(t reflect.Type) []*field {
	fs := make([]*field, 0, 8)
	for i := 0; i < t.NumField(); i++ {
		ft := t.Field(i).Type
		tag, ok := t.Field(i).Tag.Lookup("mp4")
		if !ok {
			continue
		}
		f := buildField(t.Field(i).Name, tag)
		f.children = buildFieldsAny(ft)
		fs = append(fs, f)
	}
	sort.SliceStable(fs, func(i, j int) bool {
		return fs[i].order < fs[j].order
	})
	return fs
}

func buildFieldsAny(t reflect.Type) []*field {
	switch t.Kind() {
	case reflect.Struct:
		return buildFieldsStruct(t)
	case reflect.Ptr, reflect.Array, reflect.Slice:
		return buildFieldsAny(t.Elem())
	default:
		return nil
	}
}

func buildField(fieldName string, tag string) *field {
	f := &field{
		name: fieldName,
	}
	tagMap := parseFieldTag(tag)
	for key, val := range tagMap {
		if val != "" {
			continue
		}
		if order, err := strconv.Atoi(key); err == nil {
			f.order = order
			break
		}
	}

	if val, contained := tagMap["string"]; contained {
		f.set(fieldString)
		if val == "c_p" {
			f.strType = stringType_C_P
			fmt.Fprint(os.Stderr, "go-mp4: string=c_p tag is deprecated!! See https://github.com/abema/go-mp4/issues/76\n")
		}
	}

	if _, contained := tagMap["varint"]; contained {
		f.set(fieldVarint)
	}

	if val, contained := tagMap["opt"]; contained {
		if val == "dynamic" {
			f.set(fieldOptDynamic)
		} else {
			base := 10
			if strings.HasPrefix(val, "0x") {
				val = val[2:]
				base = 16
			}
			opt, err := strconv.ParseUint(val, base, 32)
			if err != nil {
				panic(err)
			}
			f.optFlag = uint32(opt)
		}
	}

	if val, contained := tagMap["nopt"]; contained {
		base := 10
		if strings.HasPrefix(val, "0x") {
			val = val[2:]
			base = 16
		}
		nopt, err := strconv.ParseUint(val, base, 32)
		if err != nil {
			panic(err)
		}
		f.nOptFlag = uint32(nopt)
	}

	if _, contained := tagMap["extend"]; contained {
		f.set(fieldExtend)
	}

	if _, contained := tagMap["dec"]; contained {
		f.set(fieldDec)
	}

	if _, contained := tagMap["hex"]; contained {
		f.set(fieldHex)
	}

	if _, contained := tagMap["iso639-2"]; contained {
		f.set(fieldISO639_2)
	}

	if _, contained := tagMap["uuid"]; contained {
		f.set(fieldUUID)
	}

	if _, contained := tagMap["hidden"]; contained {
		f.set(fieldHidden)
	}

	if val, contained := tagMap["const"]; contained {
		f.cnst = val
	}

	f.version = anyVersion
	if val, contained := tagMap["ver"]; contained {
		ver, err := strconv.Atoi(val)
		if err != nil {
			panic(err)
		}
		f.version = uint8(ver)
	}

	f.nVersion = anyVersion
	if val, contained := tagMap["nver"]; contained {
		ver, err := strconv.Atoi(val)
		if err != nil {
			panic(err)
		}
		f.nVersion = uint8(ver)
	}

	if val, contained := tagMap["size"]; contained {
		if val == "dynamic" {
			f.set(fieldSizeDynamic)
		} else {
			size, err := strconv.ParseUint(val, 10, 32)
			if err != nil {
				panic(err)
			}
			f.size = uint(size)
		}
	}

	f.length = LengthUnlimited
	if val, contained := tagMap["len"]; contained {
		if val == "dynamic" {
			f.set(fieldLengthDynamic)
		} else {
			l, err := strconv.ParseUint(val, 10, 32)
			if err != nil {
				panic(err)
			}
			f.length = uint(l)
		}
	}

	return f
}

func parseFieldTag(str string) map[string]string {
	tag := make(map[string]string, 8)

	list := strings.Split(str, ",")
	for _, e := range list {
		kv := strings.SplitN(e, "=", 2)
		if len(kv) == 2 {
			tag[strings.Trim(kv[0], " ")] = strings.Trim(kv[1], " ")
		} else {
			tag[strings.Trim(kv[0], " ")] = ""
		}
	}

	return tag
}

type fieldInstance struct {
	field
	cfo ICustomFieldObject
}

func resolveFieldInstance(f *field, box IImmutableBox, parent reflect.Value, ctx Context) *fieldInstance {
	fi := fieldInstance{
		field: *f,
	}

	cfo, ok := parent.Addr().Interface().(ICustomFieldObject)
	if ok {
		fi.cfo = cfo
	} else {
		fi.cfo = box
	}

	if fi.is(fieldSizeDynamic) {
		fi.size = fi.cfo.GetFieldSize(f.name, ctx)
	}

	if fi.is(fieldLengthDynamic) {
		fi.length = fi.cfo.GetFieldLength(f.name, ctx)
	}

	return &fi
}

func isTargetField(box IImmutableBox, fi *fieldInstance, ctx Context) bool {
	if box.GetVersion() != anyVersion {
		if fi.version != anyVersion && box.GetVersion() != fi.version {
			return false
		}

		if fi.nVersion != anyVersion && box.GetVersion() == fi.nVersion {
			return false
		}
	}

	if fi.optFlag != 0 && box.GetFlags()&fi.optFlag == 0 {
		return false
	}

	if fi.nOptFlag != 0 && box.GetFlags()&fi.nOptFlag != 0 {
		return false
	}

	if fi.is(fieldOptDynamic) && !fi.cfo.IsOptFieldEnabled(fi.name, ctx) {
		return false
	}

	return true
}
