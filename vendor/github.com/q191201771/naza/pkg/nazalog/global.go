// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazalog

import (
	"fmt"

	"github.com/q191201771/naza/pkg/nazareflect"

	"github.com/q191201771/naza/pkg/fake"
)

var global Logger

func Tracef(format string, v ...interface{}) {
	global.Out(LevelTrace, 2, fmt.Sprintf(format, v...))
}

func Debugf(format string, v ...interface{}) {
	global.Out(LevelDebug, 2, fmt.Sprintf(format, v...))
}

func Infof(format string, v ...interface{}) {
	global.Out(LevelInfo, 2, fmt.Sprintf(format, v...))
}

func Warnf(format string, v ...interface{}) {
	global.Out(LevelWarn, 2, fmt.Sprintf(format, v...))
}

func Errorf(format string, v ...interface{}) {
	global.Out(LevelError, 2, fmt.Sprintf(format, v...))
}

func Fatalf(format string, v ...interface{}) {
	global.Out(LevelFatal, 2, fmt.Sprintf(format, v...))
	fake.Os_Exit(1)
}

func Panicf(format string, v ...interface{}) {
	global.Out(LevelPanic, 2, fmt.Sprintf(format, v...))
	panic(fmt.Sprintf(format, v...))
}

func Trace(v ...interface{}) {
	global.Out(LevelTrace, 2, fmt.Sprint(v...))
}

func Debug(v ...interface{}) {
	global.Out(LevelDebug, 2, fmt.Sprint(v...))
}

func Info(v ...interface{}) {
	global.Out(LevelInfo, 2, fmt.Sprint(v...))
}

func Warn(v ...interface{}) {
	global.Out(LevelWarn, 2, fmt.Sprint(v...))
}

func Error(v ...interface{}) {
	global.Out(LevelError, 2, fmt.Sprint(v...))
}

func Fatal(v ...interface{}) {
	global.Out(LevelFatal, 2, fmt.Sprint(v...))
	fake.Os_Exit(1)
}

func Panic(v ...interface{}) {
	global.Out(LevelPanic, 2, fmt.Sprint(v...))
	panic(fmt.Sprint(v...))
}

func Output(calldepth int, s string) error {
	global.Out(LevelInfo, calldepth, s)
	return nil
}

func Print(v ...interface{}) {
	global.Out(LevelInfo, 2, fmt.Sprint(v...))
}

func Printf(format string, v ...interface{}) {
	global.Out(LevelInfo, 2, fmt.Sprintf(format, v...))
}
func Println(v ...interface{}) {
	global.Out(LevelInfo, 2, fmt.Sprint(v...))
}
func Fatalln(v ...interface{}) {
	global.Out(LevelInfo, 2, fmt.Sprint(v...))
	fake.Os_Exit(1)
}
func Panicln(v ...interface{}) {
	global.Out(LevelInfo, 2, fmt.Sprint(v...))
	panic(fmt.Sprint(v...))
}

func Assert(expected interface{}, actual interface{}, extInfo ...string) {
	if !nazareflect.Equal(expected, actual) {
		var v string
		if len(extInfo) == 0 {
			v = fmt.Sprintf("assert failed. excepted=%+v, but actual=%+v", expected, actual)
		} else {
			v = fmt.Sprintf("assert failed. excepted=%+v, but actual=%+v, extInfo=%s", expected, actual, extInfo)
		}
		switch global.GetOption().AssertBehavior {
		case AssertError:
			global.Out(LevelError, 2, v)
		case AssertFatal:
			global.Out(LevelFatal, 2, v)
			fake.Os_Exit(1)
		case AssertPanic:
			global.Out(LevelPanic, 2, v)
			panic(v)
		}
	}
}

func Out(level Level, calldepth int, s string) {
	global.Out(level, calldepth, s)
}

func Sync() {
	global.Sync()
}

func WithPrefix(s string) Logger {
	return global.WithPrefix(s)
}

func GetOption() Option {
	return global.GetOption()
}

// ---------------------------------------------------------------------------------------------------------------------

// GetGlobalLogger 获取全局Logger
func GetGlobalLogger() Logger {
	return global
}

// Init 初始化全局Logger
//
// 注意，全局Logger在不需要特殊配置时，可以不显示调用 Init 函数
// 注意，该方法不会修改global指针指向，而是操作global指针指向的对象
func Init(modOptions ...ModOption) error {
	return global.Init(modOptions...)
}

// SetGlobalLogger 更换全局Logger
//
// 注意，更换后，之前调用 GetGlobalLogger 获取的全局Logger和当前的全局Logger将是两个对象
//
// TODO(chef): [refactor] 在已经提供 Init 的前提下，是否应该删除掉该函数
func SetGlobalLogger(l Logger) {
	global = l
}

// ---------------------------------------------------------------------------------------------------------------------

func init() {
	global, _ = newLogger()
}
