// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package hls

import (
	"github.com/q191201771/naza/pkg/filesystemlayer"
)

type Fragment struct {
	fp filesystemlayer.IFile
}

func (f *Fragment) OpenFile(filename string) (err error) {
	f.fp, err = fslCtx.Create(filename)
	if err != nil {
		return
	}
	return
}

func (f *Fragment) WriteFile(b []byte) (err error) {
	_, err = f.fp.Write(b)
	return
}

func (f *Fragment) CloseFile() error {
	return f.fp.Close()
}
