// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package mpegts

import (
	"os"

	"github.com/q191201771/lal/pkg/base"
)

type FileWriter struct {
	fp *os.File
}

func (fw *FileWriter) Create(filename string) (err error) {
	fw.fp, err = os.Create(filename)
	return
}

func (fw *FileWriter) Write(b []byte) (err error) {
	if fw.fp == nil {
		return base.ErrFileNotExist
	}
	_, err = fw.fp.Write(b)
	return
}

func (fw *FileWriter) Dispose() error {
	if fw.fp == nil {
		return base.ErrFileNotExist
	}
	return fw.fp.Close()
}

func (fw *FileWriter) Name() string {
	if fw.fp == nil {
		return ""
	}
	return fw.fp.Name()
}
