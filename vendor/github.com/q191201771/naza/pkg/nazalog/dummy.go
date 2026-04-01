// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazalog

var DummyLogger Logger

func init() {
	DummyLogger, _ = New(func(option *Option) {
		option.Level = LevelLogNothing
	})
}
