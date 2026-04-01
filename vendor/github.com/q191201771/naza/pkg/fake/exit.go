// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package fake

import "os"

var exit = os.Exit

type ExitResult struct {
	HasExit  bool
	ExitCode int
}

var exitResult ExitResult

// 正常情况下，调用 os.Exit，单元测试时，可通过调用 WithFakeExit 配置为不调用 os.Exit
func Os_Exit(code int) {
	exit(code)
}

func WithFakeOsExit(fn func()) ExitResult {
	startFakeExit()
	fn()
	stopFakeExit()
	return exitResult
}

func startFakeExit() {
	exitResult.HasExit = false
	exitResult.ExitCode = 0

	exit = func(code int) {
		exitResult.HasExit = true
		exitResult.ExitCode = code
	}
}

func stopFakeExit() {
	exit = os.Exit
}
