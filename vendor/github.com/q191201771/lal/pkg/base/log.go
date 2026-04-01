// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

import (
	"fmt"

	"github.com/q191201771/naza/pkg/nazalog"
)

type LogDump struct {
	log         nazalog.Logger
	debugMaxNum int

	debugCount int
}

// NewLogDump
//
// @param debugMaxNum: 日志最小级别为debug时，使用debug打印日志次数的阈值
func NewLogDump(log nazalog.Logger, debugMaxNum int) LogDump {
	return LogDump{
		log:         log,
		debugMaxNum: debugMaxNum,
	}
}

func (ld *LogDump) ShouldDump() bool {
	switch ld.log.GetOption().Level {
	case nazalog.LevelTrace:
		return true
	case nazalog.LevelDebug:
		if ld.debugCount >= ld.debugMaxNum {
			return false
		}
		ld.debugCount++
		return true
	}
	return false
}

// Outf
//
// 调用之前需调用 ShouldDump
// 将 ShouldDump 独立出来的目的是避免不需要打印日志时， Outf 调用前构造实参的开销，比如
// ld.Outf("hex=%s", hex.Dump(buf))
// 这个hex.Dump调用
func (ld *LogDump) Outf(format string, v ...interface{}) {
	ld.log.Out(ld.log.GetOption().Level, 3, fmt.Sprintf(format, v...))
}
