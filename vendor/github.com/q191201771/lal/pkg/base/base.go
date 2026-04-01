// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

// Package base 提供被其他多个package依赖的基础内容，自身不依赖任何package
package base

import (
	"os"
	"strings"
	"time"

	"github.com/q191201771/naza/pkg/bininfo"
)

// TODO chef: 考虑部分内容放入关联的协议package的子package中

var startTime string

// ReadableNowTime 当前时间，可读字符串形式
func ReadableNowTime() string {
	return time.Now().Format("2006-01-02 15:04:05.999")
}

func GetWd() string {
	dir, _ := os.Getwd()
	return dir
}

func LogoutStartInfo() {
	Log.Infof("     start: %s", startTime)
	Log.Infof("        wd: %s", GetWd())
	Log.Infof("      args: %s", strings.Join(os.Args, " "))
	Log.Infof("   bininfo: %s", bininfo.StringifySingleLine())
	Log.Infof("   version: %s", LalFullInfo)
	Log.Infof("    github: %s", LalGithubSite)
	Log.Infof("       doc: %s", LalDocSite)
}

func init() {
	startTime = ReadableNowTime()
}
