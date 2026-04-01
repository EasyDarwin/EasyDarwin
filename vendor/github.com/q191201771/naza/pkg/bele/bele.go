// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

// Package bele 提供了大小端的转换操作
//
// be是big endian的缩写，即大端
// le是little endian的缩写，即小端
//
// assume local is `le`
package bele

import (
	"encoding/binary"
	"io"
	"math"
)

// ----- 反序列化 -----

func BeUint16(p []byte) uint16 {
	return binary.BigEndian.Uint16(p)
}

func BeUint24(p []byte) uint32 {
	return uint32(p[2]) | uint32(p[1])<<8 | uint32(p[0])<<16
}

func BeUint32(p []byte) (ret uint32) {
	return binary.BigEndian.Uint32(p)
}

func BeUint64(p []byte) (ret uint64) {
	return binary.BigEndian.Uint64(p)
}

func BeFloat64(p []byte) (ret float64) {
	a := binary.BigEndian.Uint64(p)
	return math.Float64frombits(a)
}

func LeUint32(p []byte) (ret uint32) {
	return binary.LittleEndian.Uint32(p)
}
func LeUint16(p []byte) (ret uint16) {
	return binary.LittleEndian.Uint16(p)
}
func ReadBytes(r io.Reader, n int) ([]byte, error) {
	b := make([]byte, n)
	// 原生Read函数，读不够时，会在第一次调用时读入剩余的数据，并返回读入数据的真实长度，以及nil值的error
	// 在下一次Read时，才返回EOF
	// 这里我们在第一次读不够时，就直接返回EOF。（但是也会把剩余的数据读取到）
	nn, err := r.Read(b)
	if err != nil {
		return nil, err
	}
	if nn != n {
		return b, io.EOF
	}
	return b, nil
}

func ReadString(r io.Reader, n int) (string, error) {
	b, err := ReadBytes(r, n)
	return string(b), err
}

func ReadUint8(r io.Reader) (uint8, error) {
	b, err := ReadBytes(r, 1)
	if err != nil {
		return 0, err
	}
	return b[0], nil
}

func ReadBeUint16(r io.Reader) (uint16, error) {
	b, err := ReadBytes(r, 2)
	if err != nil {
		return 0, err
	}
	return BeUint16(b), nil
}

func ReadBeUint24(r io.Reader) (uint32, error) {
	b, err := ReadBytes(r, 3)
	if err != nil {
		return 0, err
	}
	return BeUint24(b), nil
}

func ReadBeUint32(r io.Reader) (uint32, error) {
	b, err := ReadBytes(r, 4)
	if err != nil {
		return 0, err
	}
	return BeUint32(b), nil
}

func ReadBeUint64(r io.Reader) (uint64, error) {
	b, err := ReadBytes(r, 8)
	if err != nil {
		return 0, err
	}
	return BeUint64(b), nil
}

func ReadLeUint32(r io.Reader) (uint32, error) {
	b, err := ReadBytes(r, 4)
	if err != nil {
		return 0, err
	}
	return LeUint32(b), nil
}
func ReadLeUint16(r io.Reader) (uint16, error) {
	b, err := ReadBytes(r, 4)
	if err != nil {
		return 0, err
	}
	return LeUint16(b), nil
}

// ----- 序列化 -----

func BePutUint16(out []byte, in uint16) {
	binary.BigEndian.PutUint16(out, in)
}

func BePutUint24(out []byte, in uint32) {
	out[0] = byte(in >> 16)
	out[1] = byte(in >> 8)
	out[2] = byte(in)
}

func BePutUint32(out []byte, in uint32) {
	binary.BigEndian.PutUint32(out, in)
}

func BePutUint64(out []byte, in uint64) {
	binary.BigEndian.PutUint64(out, in)
}

func LePutUint32(out []byte, in uint32) {
	binary.LittleEndian.PutUint32(out, in)
}
func LePutUint16(out []byte, in uint16) {
	binary.LittleEndian.PutUint16(out, in)
}
func WriteBeUint24(writer io.Writer, in uint32) error {
	_, err := writer.Write([]byte{uint8(in >> 16), uint8(in >> 8), uint8(in & 0xFF)})
	return err
}

func WriteBe(writer io.Writer, in interface{}) error {
	return binary.Write(writer, binary.BigEndian, in)
}

func WriteLe(writer io.Writer, in interface{}) error {
	return binary.Write(writer, binary.LittleEndian, in)
}
