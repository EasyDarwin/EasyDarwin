// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package httpts

import "github.com/q191201771/naza/pkg/nazalog"

var (
	SubSessionWriteChanSize  = 1024
	SubSessionWriteTimeoutMs = 10000

	Log = nazalog.GetGlobalLogger()
)
