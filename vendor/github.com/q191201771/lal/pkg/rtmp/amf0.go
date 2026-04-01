// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtmp

// amf0.go
// @pure
// 提供amf0格式的编码与解码的操作

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"github.com/q191201771/naza/pkg/nazabytes"
	"io"
	"strings"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazaerrors"

	"github.com/q191201771/naza/pkg/bele"
)

const (
	Amf0TypeMarkerNumber      = uint8(0x00)
	Amf0TypeMarkerBoolean     = uint8(0x01)
	Amf0TypeMarkerString      = uint8(0x02)
	Amf0TypeMarkerObject      = uint8(0x03)
	Amf0TypeMarkerNull        = uint8(0x05)
	Amf0TypeMarkerUndefined   = uint8(0x06)
	Amf0TypeMarkerEcmaArray   = uint8(0x08)
	Amf0TypeMarkerObjectEnd   = uint8(0x09) // end for both Object and Array
	Amf0TypeMarkerStrictArray = uint8(0x0a)
	Amf0TypeMarkerLongString  = uint8(0x0c)
	Amf0TypeMarkerUnsupported = uint8(0x0d)

	// 还没用到的类型
	//Amf0TypeMarkerMovieclip   = uint8(0x04)
	//Amf0TypeMarkerReference   = uint8(0x07)
	//Amf0TypeMarkerData        = uint8(0x0b)
	//Amf0TypeMarkerRecordset   = uint8(0x0e)
	//Amf0TypeMarkerXmlDocument = uint8(0x0f)
	//Amf0TypeMarkerTypedObject = uint8(0x10)
)

var (
	// Amf0TypeMarkerObjectEndBytes Amf0TypeMarkerArrayEndBytes:
	// object-end-type(0x00 0x00 0x09) 表示Object和EcmaArray类型的结束标识
	Amf0TypeMarkerObjectEndBytes = []byte{0, 0, Amf0TypeMarkerObjectEnd}
	Amf0TypeMarkerArrayEndBytes  = []byte{0, 0, Amf0TypeMarkerObjectEnd}
)

// ---------------------------------------------------------------------------------------------------------------------

type ObjectPair struct {
	Key   string      // Amf0TypeMarkerStrictArray类型的数据，Key为""
	Value interface{} // TODO(chef): [perf] 考虑换成泛型 202206
}

type ObjectPairArray []ObjectPair

func (o ObjectPairArray) Find(key string) interface{} {
	for _, op := range o {
		if op.Key == key {
			return op.Value
		}
	}
	return nil
}

func (o ObjectPairArray) FindString(key string) (string, error) {
	for _, op := range o {
		if op.Key == key {
			if s, ok := op.Value.(string); ok {
				return s, nil
			}
		}
	}
	return "", base.ErrAmfNotExist
}

func (o ObjectPairArray) FindNumber(key string) (int, error) {
	for _, op := range o {
		if op.Key == key {
			if s, ok := op.Value.(float64); ok {
				return int(s), nil
			}
		}
	}
	return -1, base.ErrAmfNotExist
}

func (o ObjectPairArray) DebugString() string {
	var b strings.Builder
	for _, v := range o {
		b.WriteString(fmt.Sprintf("%s: %+v\n", v.Key, v.Value))
	}
	return b.String()
}

// ---------------------------------------------------------------------------------------------------------------------

type amf0 struct{}

var Amf0 amf0

func (amf0) WriteNumber(writer io.Writer, val float64) error {
	if _, err := writer.Write([]byte{Amf0TypeMarkerNumber}); err != nil {
		return err
	}
	return bele.WriteBe(writer, val)
}

func (amf0) WriteString(writer io.Writer, val string) error {
	if len(val) < 65536 {
		if _, err := writer.Write([]byte{Amf0TypeMarkerString}); err != nil {
			return err
		}
		if err := bele.WriteBe(writer, uint16(len(val))); err != nil {
			return err
		}
	} else {
		if _, err := writer.Write([]byte{Amf0TypeMarkerLongString}); err != nil {
			return err
		}
		if err := bele.WriteBe(writer, uint32(len(val))); err != nil {
			return err
		}
	}
	_, err := writer.Write([]byte(val))
	return err
}

func (amf0) WriteNull(writer io.Writer) error {
	_, err := writer.Write([]byte{Amf0TypeMarkerNull})
	return err
}

func (amf0) WriteBoolean(writer io.Writer, b bool) error {
	if _, err := writer.Write([]byte{Amf0TypeMarkerBoolean}); err != nil {
		return err
	}
	v := uint8(0)
	if b {
		v = 1
	}
	_, err := writer.Write([]byte{v})
	return err
}

func (amf0) WriteObject(writer io.Writer, opa ObjectPairArray) error {
	if _, err := writer.Write([]byte{Amf0TypeMarkerObject}); err != nil {
		return err
	}
	for i := 0; i < len(opa); i++ {
		if err := bele.WriteBe(writer, uint16(len(opa[i].Key))); err != nil {
			return err
		}
		if _, err := writer.Write([]byte(opa[i].Key)); err != nil {
			return err
		}
		switch opa[i].Value.(type) {
		case string:
			if err := Amf0.WriteString(writer, opa[i].Value.(string)); err != nil {
				return err
			}
		case int, float64:
			var numberVal float64
			if intval, ok := opa[i].Value.(int); ok {
				numberVal = float64(intval)
			} else if floatVal, ok := opa[i].Value.(float64); ok {
				numberVal = floatVal
			}
			if err := Amf0.WriteNumber(writer, numberVal); err != nil {
				return err
			}
		case bool:
			if err := Amf0.WriteBoolean(writer, opa[i].Value.(bool)); err != nil {
				return err
			}
		default:
			Log.Panicf("unknown value type. i=%d, v=%+v", i, opa[i].Value)
		}
	}
	_, err := writer.Write(Amf0TypeMarkerObjectEndBytes)
	return err
}

// ----------------------------------------------------------------------------
// read类型的方法集合
//
// 从输入参数<b>切片中读取函数名所指定的amf类型数据
// 注意，方法内部不会修改输入参数<b>切片的内容
//
// 返回值如无特殊说明，则
// 第1个参数为读取出的所指定类型的数据
// 第2个参数为读取时从<b>消耗的字节大小
// 第3个参数error，如果不等于nil，表示读取失败

func (amf0) ReadStringWithoutType(b []byte) (string, int, error) {
	if len(b) < 2 {
		return "", 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	l := int(bele.BeUint16(b))
	if l > len(b)-2 {
		return "", 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	return string(b[2 : 2+l]), 2 + l, nil
}

func (amf0) ReadLongStringWithoutType(b []byte) (string, int, error) {
	if len(b) < 4 {
		return "", 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	l := int(bele.BeUint32(b))
	if l > len(b)-4 {
		return "", 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	return string(b[4 : 4+l]), 4 + l, nil
}

func (amf0) ReadString(b []byte) (val string, l int, err error) {
	if len(b) < 1 {
		return "", 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	switch b[0] {
	case Amf0TypeMarkerString:
		val, l, err = Amf0.ReadStringWithoutType(b[1:])
		l++
	case Amf0TypeMarkerLongString:
		val, l, err = Amf0.ReadLongStringWithoutType(b[1:])
		l++
	default:
		err = base.NewErrAmfInvalidType(b[0])
	}
	return
}

func (amf0) ReadNumber(b []byte) (float64, int, error) {
	if len(b) < 9 {
		return 0, 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	if b[0] != Amf0TypeMarkerNumber {
		return 0, 0, base.NewErrAmfInvalidType(b[0])
	}
	return bele.BeFloat64(b[1:]), 9, nil
}

func (amf0) ReadBoolean(b []byte) (bool, int, error) {
	if len(b) < 2 {
		return false, 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	if b[0] != Amf0TypeMarkerBoolean {
		return false, 0, base.NewErrAmfInvalidType(b[0])
	}
	return b[1] != 0x0, 2, nil
}

func (amf0) ReadNull(b []byte) (int, error) {
	if len(b) < 1 {
		return 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	if b[0] != Amf0TypeMarkerNull {
		return 0, base.NewErrAmfInvalidType(b[0])
	}
	return 1, nil
}

func (amf0) ReadUndefinedOrUnsupported(b []byte) (int, error) {
	if len(b) < 1 {
		return 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	return 1, nil
}

// ReadObject
//
// @return ObjectPairArray: ...
// @return int: 读取时从 b 消耗的字节大小
// @return error: ...
func (amf0) ReadObject(b []byte) (ObjectPairArray, int, error) {
	if len(b) < 1 {
		return nil, 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	if b[0] != Amf0TypeMarkerObject {
		return nil, 0, base.NewErrAmfInvalidType(b[0])
	}

	index := 1
	var ops ObjectPairArray
	for {
		if len(b)-index >= 3 && bytes.Equal(b[index:index+3], Amf0TypeMarkerObjectEndBytes) {
			return ops, index + 3, nil
		}

		k, l, err := Amf0.ReadStringWithoutType(b[index:])
		if err != nil {
			return nil, 0, err
		}
		index += l

		var readErr error
		ops, index, readErr = Amf0.read(b, index, k, ops)
		if readErr != nil {
			return ops, index, readErr
		}
	}
}

// TODO chef: 实现WriteArray

// ReadArray Amf0TypeMarkerEcmaArray
func (amf0) ReadArray(b []byte) (ObjectPairArray, int, error) {
	if len(b) < 5 {
		return nil, 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	if b[0] != Amf0TypeMarkerEcmaArray {
		return nil, 0, base.NewErrAmfInvalidType(b[0])
	}
	count := int(bele.BeUint32(b[1:]))

	index := 5
	var ops ObjectPairArray
	for i := 0; i < count; i++ {
		k, l, err := Amf0.ReadStringWithoutType(b[index:])
		if err != nil {
			return nil, 0, err
		}
		index += l

		var readErr error
		ops, index, readErr = Amf0.read(b, index, k, ops)
		if readErr != nil {
			return ops, index, readErr
		}
	}

	if len(b)-index >= 3 && bytes.Equal(b[index:index+3], Amf0TypeMarkerArrayEndBytes) {
		index += 3
	} else {
		Log.Warn("amf ReadArray without suffix Amf0TypeMarkerObjectEndBytes.")
	}
	return ops, index, nil
}

func (amf0) ReadStrictArray(b []byte) (ObjectPairArray, int, error) {
	if len(b) < 5 {
		return nil, 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	if b[0] != Amf0TypeMarkerStrictArray {
		return nil, 0, base.NewErrAmfInvalidType(b[0])
	}
	count := int(bele.BeUint32(b[1:]))

	index := 5
	var ops ObjectPairArray
	for i := 0; i < count; i++ {
		var readErr error
		ops, index, readErr = Amf0.read(b, index, "", ops)
		if readErr != nil {
			return ops, index, readErr
		}
	}

	return ops, index, nil
}

func (amf0) ReadObjectOrArray(b []byte) (ObjectPairArray, int, error) {
	if len(b) < 1 {
		return nil, 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	switch b[0] {
	case Amf0TypeMarkerObject:
		return Amf0.ReadObject(b)
	case Amf0TypeMarkerEcmaArray:
		return Amf0.ReadArray(b)
	}
	return nil, 0, base.NewErrAmfInvalidType(b[0])
}

func (amf0) read(b []byte, index int, k string, ops ObjectPairArray) (ObjectPairArray, int, error) {
	if len(b)-index < 1 {
		return nil, 0, nazaerrors.Wrap(base.ErrAmfTooShort)
	}
	vt := b[index]
	switch vt {
	case Amf0TypeMarkerNumber:
		v, l, err := Amf0.ReadNumber(b[index:])
		if err != nil {
			return nil, 0, err
		}
		ops = append(ops, ObjectPair{k, v})
		index += l
	case Amf0TypeMarkerBoolean:
		v, l, err := Amf0.ReadBoolean(b[index:])
		if err != nil {
			return nil, 0, err
		}
		ops = append(ops, ObjectPair{k, v})
		index += l
	case Amf0TypeMarkerString:
		v, l, err := Amf0.ReadString(b[index:])
		if err != nil {
			return nil, 0, err
		}
		ops = append(ops, ObjectPair{k, v})
		index += l
	case Amf0TypeMarkerNull:
		l, err := Amf0.ReadNull(b[index:])
		if err != nil {
			return nil, 0, err
		}
		index += l
	case Amf0TypeMarkerObject:
		v, l, err := Amf0.ReadObject(b[index:])
		if err != nil {
			return nil, 0, err
		}
		ops = append(ops, ObjectPair{k, v})
		index += l
	case Amf0TypeMarkerEcmaArray:
		v, l, err := Amf0.ReadArray(b[index:])
		if err != nil {
			return nil, 0, err
		}
		ops = append(ops, ObjectPair{k, v})
		index += l
	case Amf0TypeMarkerStrictArray:
		v, l, err := Amf0.ReadStrictArray(b[index:])
		if err != nil {
			return nil, 0, err
		}
		ops = append(ops, ObjectPair{k, v})
		index += l
	case Amf0TypeMarkerUndefined, Amf0TypeMarkerUnsupported:
		l, err := Amf0.ReadUndefinedOrUnsupported(b[index:])
		if err != nil {
			return nil, 0, err
		}
		index += l
	default:
		Log.Errorf("unknown type. vt=%d, hex=%s, %s", vt, hex.Dump(nazabytes.Prefix(b, 4096)), hex.Dump(nazabytes.Prefix(b[index:], 4096)))
		return ops, index, base.NewErrAmfInvalidType(vt)
	}

	return ops, index, nil
}
