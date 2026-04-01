// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package httpflv

import (
	"os"
)

type FlvFileReader struct {
	fp               *os.File
	hasReadFlvHeader bool
}

func (ffr *FlvFileReader) Open(filename string) (err error) {
	ffr.fp, err = os.Open(filename)
	return
}

func (ffr *FlvFileReader) ReadFlvHeader() ([]byte, error) {
	ffr.hasReadFlvHeader = true

	flvHeader := make([]byte, flvHeaderSize)
	_, err := ffr.fp.Read(flvHeader)
	return flvHeader, err
}

func (ffr *FlvFileReader) ReadTag() (Tag, error) {
	// lazy read flv header
	if !ffr.hasReadFlvHeader {
		_, _ = ffr.ReadFlvHeader()
	}

	return ReadTag(ffr.fp)
}

func (ffr *FlvFileReader) Dispose() {
	if ffr.fp != nil {
		_ = ffr.fp.Close()
	}
}
