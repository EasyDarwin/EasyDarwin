// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

import "time"

// 兼容老版本的Go，不直接使用新版本Go标准库中新增的内容

// TODO(chef): [refactor] 整理到naza中 202212

func UnixMilli(t time.Time) int64 {
	return t.UnixNano() / 1e6
}
