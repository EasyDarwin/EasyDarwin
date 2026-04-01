// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package httpflv

import "github.com/q191201771/naza/pkg/nazalog"

var (
	SubSessionWriteChanSize  = 1024 // SubSession发送数据时channel的大小
	SubSessionWriteTimeoutMs = 10000
	FlvHeader                = []byte{0x46, 0x4c, 0x56, 0x01, 0x05, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00}

	Log = nazalog.GetGlobalLogger()
)

var readBufSize = 256 //16384 // ClientPullSession读取数据时
