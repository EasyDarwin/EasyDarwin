// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package fake

import (
	"bytes"
	"errors"
)

type WriterType uint8

const (
	WriterTypeDoNothing WriterType = iota
	WriterTypeReturnError
	WriterTypeIntoBuffer
)

var (
	ErrFakeWriter = errors.New("naza.fake: a fake writer error")
)

type Writer struct {
	t     WriterType
	ts    map[uint32]WriterType
	count uint32
	B     bytes.Buffer
}

func NewWriter(t WriterType) *Writer {
	return &Writer{
		t: t,
	}
}

// 为某些写操作指定特定的类型，次数从 0 开始计数
func (w *Writer) SetSpecificType(ts map[uint32]WriterType) {
	w.ts = ts
}

func (w *Writer) Write(b []byte) (int, error) {
	t, exist := w.ts[w.count]
	w.count++
	if !exist {
		t = w.t
	}
	switch t {
	case WriterTypeDoNothing:
		return len(b), nil
	case WriterTypeReturnError:
		return 0, ErrFakeWriter
	}

	return w.B.Write(b)
}
