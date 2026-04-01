package mp4

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"math"
	"reflect"

	"github.com/abema/go-mp4/internal/bitio"
)

const (
	anyVersion = math.MaxUint8
)

var ErrUnsupportedBoxVersion = errors.New("unsupported box version")

func readerHasSize(reader bitio.ReadSeeker, size uint64) bool {
	pre, err := reader.Seek(0, io.SeekCurrent)
	if err != nil {
		return false
	}

	end, err := reader.Seek(0, io.SeekEnd)
	if err != nil {
		return false
	}

	if uint64(end-pre) < size {
		return false
	}

	_, err = reader.Seek(pre, io.SeekStart)
	if err != nil {
		return false
	}

	return true
}

type marshaller struct {
	writer bitio.Writer
	wbits  uint64
	src    IImmutableBox
	ctx    Context
}

func Marshal(w io.Writer, src IImmutableBox, ctx Context) (n uint64, err error) {
	boxDef := src.GetType().getBoxDef(ctx)
	if boxDef == nil {
		return 0, ErrBoxInfoNotFound
	}

	v := reflect.ValueOf(src).Elem()

	m := &marshaller{
		writer: bitio.NewWriter(w),
		src:    src,
		ctx:    ctx,
	}

	if err := m.marshalStruct(v, boxDef.fields); err != nil {
		return 0, err
	}

	if m.wbits%8 != 0 {
		return 0, fmt.Errorf("box size is not multiple of 8 bits: type=%s, bits=%d", src.GetType().String(), m.wbits)
	}

	return m.wbits / 8, nil
}

func (m *marshaller) marshal(v reflect.Value, fi *fieldInstance) error {
	switch v.Type().Kind() {
	case reflect.Ptr:
		return m.marshalPtr(v, fi)
	case reflect.Struct:
		return m.marshalStruct(v, fi.children)
	case reflect.Array:
		return m.marshalArray(v, fi)
	case reflect.Slice:
		return m.marshalSlice(v, fi)
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		return m.marshalInt(v, fi)
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64, reflect.Uintptr:
		return m.marshalUint(v, fi)
	case reflect.Bool:
		return m.marshalBool(v, fi)
	case reflect.String:
		return m.marshalString(v)
	default:
		return fmt.Errorf("unsupported type: %s", v.Type().Kind())
	}
}

func (m *marshaller) marshalPtr(v reflect.Value, fi *fieldInstance) error {
	return m.marshal(v.Elem(), fi)
}

func (m *marshaller) marshalStruct(v reflect.Value, fs []*field) error {
	for _, f := range fs {
		fi := resolveFieldInstance(f, m.src, v, m.ctx)

		if !isTargetField(m.src, fi, m.ctx) {
			continue
		}

		wbits, override, err := fi.cfo.OnWriteField(f.name, m.writer, m.ctx)
		if err != nil {
			return err
		}
		m.wbits += wbits
		if override {
			continue
		}

		err = m.marshal(v.FieldByName(f.name), fi)
		if err != nil {
			return err
		}
	}

	return nil
}

func (m *marshaller) marshalArray(v reflect.Value, fi *fieldInstance) error {
	size := v.Type().Size()
	for i := 0; i < int(size)/int(v.Type().Elem().Size()); i++ {
		var err error
		err = m.marshal(v.Index(i), fi)
		if err != nil {
			return err
		}
	}
	return nil
}

func (m *marshaller) marshalSlice(v reflect.Value, fi *fieldInstance) error {
	length := uint64(v.Len())
	if fi.length != LengthUnlimited {
		if length < uint64(fi.length) {
			return fmt.Errorf("the slice has too few elements: required=%d actual=%d", fi.length, length)
		}
		length = uint64(fi.length)
	}

	elemType := v.Type().Elem()
	if elemType.Kind() == reflect.Uint8 && fi.size == 8 && m.wbits%8 == 0 {
		if _, err := io.CopyN(m.writer, bytes.NewBuffer(v.Bytes()), int64(length)); err != nil {
			return err
		}
		m.wbits += length * 8
		return nil
	}

	for i := 0; i < int(length); i++ {
		m.marshal(v.Index(i), fi)
	}
	return nil
}

func (m *marshaller) marshalInt(v reflect.Value, fi *fieldInstance) error {
	signed := v.Int()

	if fi.is(fieldVarint) {
		return errors.New("signed varint is unsupported")
	}

	signBit := signed < 0
	val := uint64(signed)
	for i := uint(0); i < fi.size; i += 8 {
		v := val
		size := uint(8)
		if fi.size > i+8 {
			v = v >> (fi.size - (i + 8))
		} else if fi.size < i+8 {
			size = fi.size - i
		}

		// set sign bit
		if i == 0 {
			if signBit {
				v |= 0x1 << (size - 1)
			} else {
				v &= 0x1<<(size-1) - 1
			}
		}

		if err := m.writer.WriteBits([]byte{byte(v)}, size); err != nil {
			return err
		}
		m.wbits += uint64(size)
	}

	return nil
}

func (m *marshaller) marshalUint(v reflect.Value, fi *fieldInstance) error {
	val := v.Uint()

	if fi.is(fieldVarint) {
		m.writeUvarint(val)
		return nil
	}

	for i := uint(0); i < fi.size; i += 8 {
		v := val
		size := uint(8)
		if fi.size > i+8 {
			v = v >> (fi.size - (i + 8))
		} else if fi.size < i+8 {
			size = fi.size - i
		}
		if err := m.writer.WriteBits([]byte{byte(v)}, size); err != nil {
			return err
		}
		m.wbits += uint64(size)
	}

	return nil
}

func (m *marshaller) marshalBool(v reflect.Value, fi *fieldInstance) error {
	var val byte
	if v.Bool() {
		val = 0xff
	} else {
		val = 0x00
	}
	if err := m.writer.WriteBits([]byte{val}, fi.size); err != nil {
		return err
	}
	m.wbits += uint64(fi.size)
	return nil
}

func (m *marshaller) marshalString(v reflect.Value) error {
	data := []byte(v.String())
	for _, b := range data {
		if err := m.writer.WriteBits([]byte{b}, 8); err != nil {
			return err
		}
		m.wbits += 8
	}
	// null character
	if err := m.writer.WriteBits([]byte{0x00}, 8); err != nil {
		return err
	}
	m.wbits += 8
	return nil
}

func (m *marshaller) writeUvarint(u uint64) error {
	for i := 21; i > 0; i -= 7 {
		if err := m.writer.WriteBits([]byte{(byte(u >> uint(i))) | 0x80}, 8); err != nil {
			return err
		}
		m.wbits += 8
	}

	if err := m.writer.WriteBits([]byte{byte(u) & 0x7f}, 8); err != nil {
		return err
	}
	m.wbits += 8

	return nil
}

type unmarshaller struct {
	reader bitio.ReadSeeker
	dst    IBox
	size   uint64
	rbits  uint64
	ctx    Context
}

func UnmarshalAny(r io.ReadSeeker, boxType BoxType, payloadSize uint64, ctx Context) (box IBox, n uint64, err error) {
	dst, err := boxType.New(ctx)
	if err != nil {
		return nil, 0, err
	}
	n, err = Unmarshal(r, payloadSize, dst, ctx)
	return dst, n, err
}

func Unmarshal(r io.ReadSeeker, payloadSize uint64, dst IBox, ctx Context) (n uint64, err error) {
	boxDef := dst.GetType().getBoxDef(ctx)
	if boxDef == nil {
		return 0, ErrBoxInfoNotFound
	}

	v := reflect.ValueOf(dst).Elem()

	dst.SetVersion(anyVersion)

	u := &unmarshaller{
		reader: bitio.NewReadSeeker(r),
		dst:    dst,
		size:   payloadSize,
		ctx:    ctx,
	}

	if n, override, err := dst.BeforeUnmarshal(r, payloadSize, u.ctx); err != nil {
		return 0, err
	} else if override {
		return n, nil
	} else {
		u.rbits = n * 8
	}

	sn, err := r.Seek(0, io.SeekCurrent)
	if err != nil {
		return 0, err
	}

	if err := u.unmarshalStruct(v, boxDef.fields); err != nil {
		if err == ErrUnsupportedBoxVersion {
			r.Seek(sn, io.SeekStart)
		}
		return 0, err
	}

	if u.rbits%8 != 0 {
		return 0, fmt.Errorf("box size is not multiple of 8 bits: type=%s, size=%d, bits=%d", dst.GetType().String(), u.size, u.rbits)
	}

	if u.rbits > u.size*8 {
		return 0, fmt.Errorf("overrun error: type=%s, size=%d, bits=%d", dst.GetType().String(), u.size, u.rbits)
	}

	return u.rbits / 8, nil
}

func (u *unmarshaller) unmarshal(v reflect.Value, fi *fieldInstance) error {
	var err error
	switch v.Type().Kind() {
	case reflect.Ptr:
		err = u.unmarshalPtr(v, fi)
	case reflect.Struct:
		err = u.unmarshalStructInternal(v, fi)
	case reflect.Array:
		err = u.unmarshalArray(v, fi)
	case reflect.Slice:
		err = u.unmarshalSlice(v, fi)
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		err = u.unmarshalInt(v, fi)
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64, reflect.Uintptr:
		err = u.unmarshalUint(v, fi)
	case reflect.Bool:
		err = u.unmarshalBool(v, fi)
	case reflect.String:
		err = u.unmarshalString(v, fi)
	default:
		return fmt.Errorf("unsupported type: %s", v.Type().Kind())
	}
	return err
}

func (u *unmarshaller) unmarshalPtr(v reflect.Value, fi *fieldInstance) error {
	v.Set(reflect.New(v.Type().Elem()))
	return u.unmarshal(v.Elem(), fi)
}

func (u *unmarshaller) unmarshalStructInternal(v reflect.Value, fi *fieldInstance) error {
	if fi.size != 0 && fi.size%8 == 0 {
		u2 := *u
		u2.size = uint64(fi.size / 8)
		u2.rbits = 0
		if err := u2.unmarshalStruct(v, fi.children); err != nil {
			return err
		}
		u.rbits += u2.rbits
		if u2.rbits != uint64(fi.size) {
			return errors.New("invalid alignment")
		}
		return nil
	}

	return u.unmarshalStruct(v, fi.children)
}

func (u *unmarshaller) unmarshalStruct(v reflect.Value, fs []*field) error {
	for _, f := range fs {
		fi := resolveFieldInstance(f, u.dst, v, u.ctx)

		if !isTargetField(u.dst, fi, u.ctx) {
			continue
		}

		rbits, override, err := fi.cfo.OnReadField(f.name, u.reader, u.size*8-u.rbits, u.ctx)
		if err != nil {
			return err
		}
		u.rbits += rbits
		if override {
			continue
		}

		err = u.unmarshal(v.FieldByName(f.name), fi)
		if err != nil {
			return err
		}

		if v.FieldByName(f.name).Type() == reflect.TypeOf(FullBox{}) && !u.dst.GetType().IsSupportedVersion(u.dst.GetVersion(), u.ctx) {
			return ErrUnsupportedBoxVersion
		}
	}

	return nil
}

func (u *unmarshaller) unmarshalArray(v reflect.Value, fi *fieldInstance) error {
	size := v.Type().Size()
	for i := 0; i < int(size)/int(v.Type().Elem().Size()); i++ {
		var err error
		err = u.unmarshal(v.Index(i), fi)
		if err != nil {
			return err
		}
	}
	return nil
}

func (u *unmarshaller) unmarshalSlice(v reflect.Value, fi *fieldInstance) error {
	var slice reflect.Value
	elemType := v.Type().Elem()

	length := uint64(fi.length)
	if fi.length == LengthUnlimited {
		if fi.size != 0 {
			left := (u.size)*8 - u.rbits
			if left%uint64(fi.size) != 0 {
				return errors.New("invalid alignment")
			}
			length = left / uint64(fi.size)
		} else {
			length = 0
		}
	}

	if u.rbits%8 == 0 && elemType.Kind() == reflect.Uint8 && fi.size == 8 {
		totalSize := length * uint64(fi.size) / 8

		if !readerHasSize(u.reader, totalSize) {
			return fmt.Errorf("not enough bits")
		}

		buf := bytes.NewBuffer(make([]byte, 0, totalSize))
		if _, err := io.CopyN(buf, u.reader, int64(totalSize)); err != nil {
			return err
		}
		slice = reflect.ValueOf(buf.Bytes())
		u.rbits += uint64(totalSize) * 8

	} else {
		slice = reflect.MakeSlice(v.Type(), 0, 0)
		for i := 0; ; i++ {
			if fi.length != LengthUnlimited && uint(i) >= fi.length {
				break
			}
			if fi.length == LengthUnlimited && u.rbits >= u.size*8 {
				break
			}
			slice = reflect.Append(slice, reflect.Zero(elemType))
			if err := u.unmarshal(slice.Index(i), fi); err != nil {
				return err
			}
			if u.rbits > u.size*8 {
				return fmt.Errorf("failed to read array completely: fieldName=\"%s\"", fi.name)
			}
		}
	}

	v.Set(slice)
	return nil
}

func (u *unmarshaller) unmarshalInt(v reflect.Value, fi *fieldInstance) error {
	if fi.is(fieldVarint) {
		return errors.New("signed varint is unsupported")
	}

	if fi.size == 0 {
		return fmt.Errorf("size must not be zero: %s", fi.name)
	}

	data, err := u.reader.ReadBits(fi.size)
	if err != nil {
		return err
	}
	u.rbits += uint64(fi.size)

	signBit := false
	if len(data) > 0 {
		signMask := byte(0x01) << ((fi.size - 1) % 8)
		signBit = data[0]&signMask != 0
		if signBit {
			data[0] |= ^(signMask - 1)
		}
	}

	var val uint64
	if signBit {
		val = ^uint64(0)
	}
	for i := range data {
		val <<= 8
		val |= uint64(data[i])
	}
	v.SetInt(int64(val))
	return nil
}

func (u *unmarshaller) unmarshalUint(v reflect.Value, fi *fieldInstance) error {
	if fi.is(fieldVarint) {
		val, err := u.readUvarint()
		if err != nil {
			return err
		}
		v.SetUint(val)
		return nil
	}

	if fi.size == 0 {
		return fmt.Errorf("size must not be zero: %s", fi.name)
	}

	data, err := u.reader.ReadBits(fi.size)
	if err != nil {
		return err
	}
	u.rbits += uint64(fi.size)

	val := uint64(0)
	for i := range data {
		val <<= 8
		val |= uint64(data[i])
	}
	v.SetUint(val)

	return nil
}

func (u *unmarshaller) unmarshalBool(v reflect.Value, fi *fieldInstance) error {
	if fi.size == 0 {
		return fmt.Errorf("size must not be zero: %s", fi.name)
	}

	data, err := u.reader.ReadBits(fi.size)
	if err != nil {
		return err
	}
	u.rbits += uint64(fi.size)

	val := false
	for _, b := range data {
		val = val || (b != byte(0))
	}
	v.SetBool(val)

	return nil
}

func (u *unmarshaller) unmarshalString(v reflect.Value, fi *fieldInstance) error {
	switch fi.strType {
	case stringType_C:
		return u.unmarshalStringC(v)
	case stringType_C_P:
		return u.unmarshalStringCP(v, fi)
	default:
		return fmt.Errorf("unknown string type: %d", fi.strType)
	}
}

func (u *unmarshaller) unmarshalStringC(v reflect.Value) error {
	data := make([]byte, 0, 16)
	for {
		if u.rbits >= u.size*8 {
			break
		}

		c, err := u.reader.ReadBits(8)
		if err != nil {
			return err
		}
		u.rbits += 8

		if c[0] == 0 {
			break // null character
		}

		data = append(data, c[0])
	}
	v.SetString(string(data))

	return nil
}

func (u *unmarshaller) unmarshalStringCP(v reflect.Value, fi *fieldInstance) error {
	if ok, err := u.tryReadPString(v, fi); err != nil {
		return err
	} else if ok {
		return nil
	}
	return u.unmarshalStringC(v)
}

func (u *unmarshaller) tryReadPString(v reflect.Value, fi *fieldInstance) (ok bool, err error) {
	remainingSize := (u.size*8 - u.rbits) / 8
	if remainingSize < 2 {
		return false, nil
	}

	offset, err := u.reader.Seek(0, io.SeekCurrent)
	if err != nil {
		return false, err
	}
	defer func() {
		if err == nil && !ok {
			_, err = u.reader.Seek(offset, io.SeekStart)
		}
	}()

	buf0 := make([]byte, 1)
	if _, err := io.ReadFull(u.reader, buf0); err != nil {
		return false, err
	}
	remainingSize--
	plen := buf0[0]
	if uint64(plen) > remainingSize {
		return false, nil
	}
	buf := make([]byte, int(plen))
	if _, err := io.ReadFull(u.reader, buf); err != nil {
		return false, err
	}
	remainingSize -= uint64(plen)
	if fi.cfo.IsPString(fi.name, buf, remainingSize, u.ctx) {
		u.rbits += uint64(len(buf)+1) * 8
		v.SetString(string(buf))
		return true, nil
	}
	return false, nil
}

func (u *unmarshaller) readUvarint() (uint64, error) {
	var val uint64
	for {
		octet, err := u.reader.ReadBits(8)
		if err != nil {
			return 0, err
		}
		u.rbits += 8

		val = (val << 7) + uint64(octet[0]&0x7f)

		if octet[0]&0x80 == 0 {
			return val, nil
		}
	}
}
