// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

// Package bininfo
//
// 将编译时源码的git版本信息（当前tag，commit log的sha值和commit message，是否有未提交的修改），编译时间，Go版本，编译、运行平台打入程序中
// 编译时传入这些信息的方式见 naza 的编译脚本： https://github.com/q191201771/naza/blob/master/build.sh
package bininfo

import (
	"fmt"
	"runtime"
	"strings"
)

var (
	// 初始化为 unknown，如果编译时没有传入这些值，则为 unknown
	GitTag         = "unknown"
	GitCommitLog   = "unknown"
	GitStatus      = "unknown"
	BuildTime      = "unknown"
	BuildGoVersion = "unknown"
)

// 返回单行格式
func StringifySingleLine() string {
	return fmt.Sprintf("GitTag=%s. GitCommitLog=%s. GitStatus=%s. BuildTime=%s. GoVersion=%s. runtime=%s/%s.",
		GitTag, GitCommitLog, GitStatus, BuildTime, BuildGoVersion, runtime.GOOS, runtime.GOARCH)
}

// 返回多行格式
func StringifyMultiLine() string {
	return fmt.Sprintf("GitTag=%s\nGitCommitLog=%s\nGitStatus=%s\nBuildTime=%s\nGoVersion=%s\nruntime=%s/%s\n",
		GitTag, GitCommitLog, GitStatus, BuildTime, BuildGoVersion, runtime.GOOS, runtime.GOARCH)
}

// 对一些值做美化处理
func beauty() {
	if GitStatus == "" {
		// GitStatus 为空时，说明本地源码与最近的 commit 记录一致，无修改
		// 为它赋一个特殊值
		GitStatus = "cleanly"
	} else {
		// 将多行结果合并为一行
		GitStatus = strings.Replace(strings.Replace(GitStatus, "\r\n", " |", -1), "\n", " |", -1)
	}
}

func init() {
	beauty()
}
