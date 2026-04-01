// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

// Package nazalog 日志库
package nazalog

import "errors"

// 这是一个以使用方便为主要目标的日志库，特性：
//
// * 带日志级别
// * 可选输出至控制台或文件，也可以同时输出
// * 日志文件支持按天翻转
// * 支持是否输出源码文件及行号
// * 业务日志起始位置固定，方便查看
// * 支持Assert，断言失败后的行为可配置
// * 支持全局日志对象，独立日志对象，日志对象都可以配置，相互间可以赋值
// * 支持设置前缀，并且前缀可叠加，使得可以按repo ，package，对象等维度添加不同的前缀
// * 支持标准库中的打印接口函数（但是没有适配非打印接口），方便替换标准库日志
// * 日志文件目录不存在则自动创建
//
// 目前性能和标准库log相当
//
// TODO(chef): 异步日志

var ErrLog = errors.New("naza.log:fxxk")

type Logger interface {
	Tracef(format string, v ...interface{})
	Debugf(format string, v ...interface{})
	Infof(format string, v ...interface{})
	Warnf(format string, v ...interface{})
	Errorf(format string, v ...interface{})
	Fatalf(format string, v ...interface{}) // 打印日志并退出程序
	Panicf(format string, v ...interface{})

	Trace(v ...interface{})
	Debug(v ...interface{})
	Info(v ...interface{})
	Warn(v ...interface{})
	Error(v ...interface{})
	Fatal(v ...interface{})
	Panic(v ...interface{})

	Out(level Level, calldepth int, s string)

	// Assert 断言失败后的行为由配置项Option.AssertBehavior决定
	// 注意，expected和actual的类型必须相同，比如int(1)和int32(1)是不相等的
	//
	// @param expected 期望值
	// @param actual   实际值
	// @param extInfo  期望值和实际值不相等时打印的补充信息，如果没有，可以不填
	//
	Assert(expected interface{}, actual interface{}, extInfo ...string)

	// Sync flush to disk, typically
	//
	Sync()

	// WithPrefix
	//
	// 添加前缀，新生成一个Logger对象，如果老Logger也有prefix，则老Logger依然打印老prefix，新Logger打印多个prefix。
	//
	// 返回的Logger对象是新的，底层的 core 是同一个
	//
	WithPrefix(s string) Logger

	// Output Print ... 下面这些打印接口是为兼容标准库，让某些已使用标准库日志的代码替换到nazalog方便一些
	//
	Output(calldepth int, s string) error
	Print(v ...interface{})
	Printf(format string, v ...interface{})
	Println(v ...interface{})
	Fatalln(v ...interface{})
	Panicln(v ...interface{})

	// GetOption 获取配置项
	//
	// 注意，作用是只读，非修改配置
	//
	GetOption() Option

	// Init 初始化配置
	//
	// 注意，正常情况下，应在调用 New 函数生成Logger对象时进行配置， Init 方法提供了在已有Logger对象上配置的机会，
	// 但是，出于性能考虑，操作logger对象内部成员时没有加锁，调用方需自行保证该函数不和其他函数并发调用，也即在使用Logger对象前（比如程序启动时)
	//
	Init(modOptions ...ModOption) error
}

type HookBackendOutFn func(level Level, line []byte)

type Option struct {
	Level Level `json:"level"` // 日志级别，大于等于该级别的日志才会被输出

	// 文件输出和控制台输出可同时打开
	// 控制台输出主要用做开发时调试，打开后level字段使用彩色输出
	Filename   string `json:"filename"`     // 输出日志文件名，如果为空，则不写日志文件。可包含路径，路径不存在时，将自动创建
	IsToStdout bool   `json:"is_to_stdout"` // 是否以stdout输出到控制台 TODO(chef): 再增加一个stderr的配置

	IsRotateDaily  bool `json:"is_rotate_daily"`  // 日志按天翻转
	IsRotateHourly bool `json:"is_rotate_hourly"` // 日志按小时翻滚，整点翻滚

	ShortFileFlag       bool `json:"short_file_flag"`        // 是否在每行日志尾部添加源码文件及行号的信息
	TimestampFlag       bool `json:"timestamp_flag"`         // 是否在每行日志首部添加时间戳的信息
	TimestampWithMsFlag bool `json:"timestamp_with_ms_flag"` // 时间戳是否精确到毫秒
	LevelFlag           bool `json:"level_flag"`             // 日志是否包含日志级别字段

	AssertBehavior AssertBehavior `json:"assert_behavior"` // 断言失败时的行为

	// HookBackendOutFn
	//
	// hook后端输出的日志内容。
	//
	// 业务场景：比如业务方使用了nazalog向日志文件输出日志，与之同时还想要再程序中实时获取一份日志内容。
	//
	// 每次回调一行日志。
	// 获取的是全量日志。
	// 阻塞函数。
	// 回调结束后，内部会服用回调中日志内容的内存块。
	HookBackendOutFn HookBackendOutFn
}

// 没有配置的属性，将按如下配置
var defaultOption = Option{
	Level:               LevelDebug,
	Filename:            "",
	IsToStdout:          true,
	IsRotateDaily:       false,
	ShortFileFlag:       true,
	TimestampFlag:       true,
	TimestampWithMsFlag: true,
	LevelFlag:           true,
	AssertBehavior:      AssertError,
}

type Level uint8

const (
	LevelTrace Level = iota // 0
	LevelDebug              // 1
	LevelInfo
	LevelWarn
	LevelError
	LevelFatal
	LevelPanic
	LevelLogNothing
)

func (l Level) ReadableString() string {
	switch l {
	case LevelTrace:
		return "LevelTrace"
	case LevelDebug:
		return "LevelDebug"
	case LevelInfo:
		return "LevelInfo"
	case LevelWarn:
		return "LevelWarn"
	case LevelError:
		return "LevelError"
	case LevelFatal:
		return "LevelFatal"
	case LevelPanic:
		return "LevelPanic"
	case LevelLogNothing:
		return "LevelLogNothing"
	default:
		return "unknown"
	}
}

type AssertBehavior uint8

const (
	_           AssertBehavior = iota
	AssertError                // 1
	AssertFatal
	AssertPanic
)

func (a AssertBehavior) ReadableString() string {
	switch a {
	case AssertError:
		return "AssertError"
	case AssertFatal:
		return "AssertFatal"
	case AssertPanic:
		return "AssertPanic"
	default:
		return "unknown"
	}
}

type ModOption func(option *Option)

func New(modOptions ...ModOption) (Logger, error) {
	return newLogger(modOptions...)
}
