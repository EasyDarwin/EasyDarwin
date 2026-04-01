// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazalog

import (
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"sync"
	"time"

	"github.com/q191201771/naza/pkg/mock"

	"github.com/q191201771/naza/pkg/nazacolor"

	"github.com/q191201771/naza/pkg/nazareflect"

	"github.com/q191201771/naza/pkg/fake"
)

var _ Logger = new(logger)

var Clock = mock.NewStdClock()

const (
	levelTraceString = "TRACE "
	levelDebugString = "DEBUG "
	levelInfoString  = " INFO "
	levelWarnString  = " WARN "
	levelErrorString = "ERROR "
	levelFatalString = "FATAL "
	levelPanicString = "PANIC "

	levelTraceColorString = nazacolor.SimplePrefixGreen + levelTraceString + nazacolor.SimpleSuffix
	levelDebugColorString = nazacolor.SimplePrefixBlue + levelDebugString + nazacolor.SimpleSuffix
	levelInfoColorString  = nazacolor.SimplePrefixCyan + levelInfoString + nazacolor.SimpleSuffix
	levelWarnColorString  = nazacolor.SimplePrefixYellow + levelWarnString + nazacolor.SimpleSuffix
	levelErrorColorString = nazacolor.SimplePrefixRed + levelErrorString + nazacolor.SimpleSuffix
	levelFatalColorString = nazacolor.SimplePrefixRed + levelFatalString + nazacolor.SimpleSuffix
	levelPanicColorString = nazacolor.SimplePrefixRed + levelPanicString + nazacolor.SimpleSuffix
)

var (
	levelToString = map[Level]string{
		LevelTrace: levelTraceString,
		LevelDebug: levelDebugString,
		LevelInfo:  levelInfoString,
		LevelWarn:  levelWarnString,
		LevelError: levelErrorString,
		LevelFatal: levelFatalString,
		LevelPanic: levelPanicString,
	}
	levelToColorString = map[Level]string{
		LevelTrace: levelTraceColorString,
		LevelDebug: levelDebugColorString,
		LevelInfo:  levelInfoColorString,
		LevelWarn:  levelWarnColorString,
		LevelError: levelErrorColorString,
		LevelFatal: levelFatalColorString,
		LevelPanic: levelPanicColorString,
	}
)

type logger struct {
	prefixs []string
	core    *core
}

type core struct {
	option Option

	m             sync.Mutex
	fp            *os.File
	console       *os.File
	buf           bytes.Buffer // TODO(chef): [refactor] 是否需要使用nazabytes.Buffer
	currRoundTime time.Time
}

func (l *logger) Tracef(format string, v ...interface{}) {
	l.Out(LevelTrace, 2, fmt.Sprintf(format, v...))
}

func (l *logger) Debugf(format string, v ...interface{}) {
	l.Out(LevelDebug, 2, fmt.Sprintf(format, v...))
}

func (l *logger) Infof(format string, v ...interface{}) {
	l.Out(LevelInfo, 2, fmt.Sprintf(format, v...))
}

func (l *logger) Warnf(format string, v ...interface{}) {
	l.Out(LevelWarn, 2, fmt.Sprintf(format, v...))
}

func (l *logger) Errorf(format string, v ...interface{}) {
	l.Out(LevelError, 2, fmt.Sprintf(format, v...))
}

func (l *logger) Fatalf(format string, v ...interface{}) {
	l.Out(LevelFatal, 2, fmt.Sprintf(format, v...))
	fake.Os_Exit(1)
}

func (l *logger) Panicf(format string, v ...interface{}) {
	l.Out(LevelPanic, 2, fmt.Sprintf(format, v...))
	panic(fmt.Sprintf(format, v...))
}

func (l *logger) Trace(v ...interface{}) {
	l.Out(LevelTrace, 2, fmt.Sprint(v...))
}

func (l *logger) Debug(v ...interface{}) {
	l.Out(LevelDebug, 2, fmt.Sprint(v...))
}

func (l *logger) Info(v ...interface{}) {
	l.Out(LevelInfo, 2, fmt.Sprint(v...))
}

func (l *logger) Warn(v ...interface{}) {
	l.Out(LevelWarn, 2, fmt.Sprint(v...))
}

func (l *logger) Error(v ...interface{}) {
	l.Out(LevelError, 2, fmt.Sprint(v...))
}

func (l *logger) Fatal(v ...interface{}) {
	l.Out(LevelFatal, 2, fmt.Sprint(v...))
	fake.Os_Exit(1)
}

func (l *logger) Panic(v ...interface{}) {
	l.Out(LevelPanic, 2, fmt.Sprint(v...))
	panic(fmt.Sprint(v...))
}

func (l *logger) Output(calldepth int, s string) error {
	l.Out(LevelInfo, calldepth, s)
	return nil
}

func (l *logger) Print(v ...interface{}) {
	l.Out(LevelInfo, 2, fmt.Sprint(v...))
}

func (l *logger) Printf(format string, v ...interface{}) {
	l.Out(LevelInfo, 2, fmt.Sprintf(format, v...))
}

func (l *logger) Println(v ...interface{}) {
	l.Out(LevelInfo, 2, fmt.Sprint(v...))
}

func (l *logger) Fatalln(v ...interface{}) {
	l.Out(LevelInfo, 2, fmt.Sprint(v...))
	fake.Os_Exit(1)
}

func (l *logger) Panicln(v ...interface{}) {
	l.Out(LevelInfo, 2, fmt.Sprint(v...))
	panic(fmt.Sprint(v...))
}

func (l *logger) Assert(expected interface{}, actual interface{}, extInfo ...string) {
	if !nazareflect.Equal(expected, actual) {
		var v string
		if len(extInfo) == 0 {
			v = fmt.Sprintf("assert failed. excepted=%+v, but actual=%+v", expected, actual)
		} else {
			v = fmt.Sprintf("assert failed. excepted=%+v, but actual=%+v, extInfo=%s", expected, actual, extInfo)
		}
		switch l.core.option.AssertBehavior {
		case AssertError:
			l.Out(LevelError, 2, v)
		case AssertFatal:
			l.Out(LevelFatal, 2, v)
			fake.Os_Exit(1)
		case AssertPanic:
			l.Out(LevelPanic, 2, v)
			panic(v)
		}
	}
}

func (l *logger) Out(level Level, calldepth int, s string) {
	if l.core.option.Level > level {
		return
	}

	now := Clock.Now()

	var file string
	var line int
	if l.core.option.ShortFileFlag {
		_, file, line, _ = runtime.Caller(calldepth)

	}

	l.core.m.Lock()

	l.core.buf.Reset()

	if l.core.option.TimestampFlag {
		writeTime(&l.core.buf, now, l.core.option.TimestampWithMsFlag)
	}

	l.writeLevelStringIfNeeded(level)

	if l.prefixs != nil {
		for _, s := range l.prefixs {
			l.core.buf.WriteString("[")
			l.core.buf.WriteString(s)
			l.core.buf.WriteString("] ")
		}
	}

	l.core.buf.WriteString(s)

	if file != "" && line > 0 {
		l.core.buf.WriteString(" - ")

		short := file
		for i := len(file) - 1; i > 0; i-- {
			if file[i] == '/' {
				short = file[i+1:]
				break
			}
		}
		file = short

		l.core.buf.WriteString(file)
		l.core.buf.WriteByte(':')
		itoa(&l.core.buf, line, -1)
	}

	if l.core.buf.Len() == 0 || l.core.buf.Bytes()[l.core.buf.Len()-1] != '\n' {
		l.core.buf.WriteByte('\n')
	}

	// 输出至控制台
	if l.core.console != nil {
		_, _ = l.core.console.Write(l.core.buf.Bytes())
		if level == LevelFatal || level == LevelPanic {
			_ = l.core.console.Sync()
		}
	}

	// 输出至日志文件
	if l.core.fp != nil {

		// 同时满足条件时，翻滚一次就够了
		rotateFlag := false
		var backupName string

		if l.core.option.IsRotateHourly && now.Hour() != l.core.currRoundTime.Hour() {
			backupName = l.core.option.Filename + "." + l.core.currRoundTime.Format("2006010215")
			rotateFlag = true
		}

		if !rotateFlag && l.core.option.IsRotateDaily && now.Day() != l.core.currRoundTime.Day() {
			backupName = l.core.option.Filename + "." + l.core.currRoundTime.Format("20060102")
			rotateFlag = true
		}

		if rotateFlag {
			err := l.core.fp.Close()
			// 忽略关闭的错误
			_ = err

			err = os.Rename(l.core.option.Filename, backupName)
			if err != nil {
				// windows会走这个逻辑分支
				// TODO(chef): 应判断具体的错误值 202302

				l.core.fp, err = os.OpenFile(l.core.option.Filename, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
			} else {
				l.core.fp, err = os.Create(l.core.option.Filename)
			}
			if err != nil {
				_, _ = fmt.Fprintf(os.Stderr, "reopen error. err=%+v, fp=%+v, filename=%s, backupName=%s, now=%s, curr=%s",
					err, l.core.fp, l.core.option.Filename, backupName, now.String(), l.core.currRoundTime.String())
				return
			}

			l.core.currRoundTime = now
		}

		_, _ = l.core.fp.Write(l.core.buf.Bytes())
		if level == LevelFatal || level == LevelPanic {
			_ = l.core.fp.Sync()
		}
	}

	// 输出至hook
	if l.core.option.HookBackendOutFn != nil {
		l.core.option.HookBackendOutFn(level, l.core.buf.Bytes())
	}

	l.core.m.Unlock()
}

func (l *logger) Sync() {
	l.core.m.Lock()
	defer l.core.m.Unlock()

	if l.core.console != nil {
		_ = l.core.console.Sync()
	}
	if l.core.fp != nil {
		_ = l.core.fp.Sync()
	}
}

func (l *logger) WithPrefix(s string) Logger {
	var prefixs []string
	if l.prefixs != nil {
		prefixs = make([]string, len(l.prefixs))
		copy(prefixs, l.prefixs)
	}
	prefixs = append(prefixs, s)
	ll := &logger{
		prefixs: prefixs,
		core:    l.core,
	}
	return ll
}

func (l *logger) GetOption() Option {
	return l.core.option
}

func (l *logger) Init(modOptions ...ModOption) error {
	var err error

	l.core.currRoundTime = time.Now()
	l.core.option = defaultOption

	for _, fn := range modOptions {
		fn(&l.core.option)
	}

	if err = validate(l.core.option); err != nil {
		return err
	}
	if l.core.option.Filename != "" {
		dir := filepath.Dir(l.core.option.Filename)
		if err = os.MkdirAll(dir, 0777); err != nil {
			return err
		}
		if l.core.fp, err = os.OpenFile(l.core.option.Filename, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666); err != nil {
			return err
		}
	}
	if l.core.option.IsToStdout {
		l.core.console = os.Stdout
	} else {
		l.core.console = nil
	}

	return nil
}

// ---------------------------------------------------------------------------------------------------------------------

func newLogger(modOptions ...ModOption) (*logger, error) {
	l := &logger{
		core: &core{},
	}
	if err := l.Init(modOptions...); err != nil {
		return nil, err
	}
	return l, nil
}

func validate(option Option) error {
	if option.Level < LevelTrace || option.Level > LevelLogNothing {
		return ErrLog
	}
	if option.AssertBehavior < AssertError || option.AssertBehavior > AssertPanic {
		return ErrLog
	}
	return nil
}

func writeTime(buf *bytes.Buffer, t time.Time, withMs bool) {
	year, month, day := t.Date()
	itoa(buf, year, 4)
	buf.WriteByte('/')
	itoa(buf, int(month), 2)
	buf.WriteByte('/')
	itoa(buf, day, 2)
	buf.WriteByte(' ')

	hour, min, sec := t.Clock()
	itoa(buf, hour, 2)
	buf.WriteByte(':')
	itoa(buf, min, 2)
	buf.WriteByte(':')
	itoa(buf, sec, 2)

	if withMs {
		buf.WriteByte('.')
		itoa(buf, t.Nanosecond()/1e3, 6)
	}

	buf.WriteByte(' ')
}

// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// @NOTICE 该函数拷贝自 Go 标准库 /src/log/log.go: func itoa(buf *[]byte, i int, wid int)
// Cheap integer to fixed-width decimal ASCII. Give a negative width to avoid zero-padding.
func itoa(buf *bytes.Buffer, i int, wid int) {
	// Assemble decimal in reverse order.
	var b [20]byte
	bp := len(b) - 1
	for i >= 10 || wid > 1 {
		wid--
		q := i / 10
		b[bp] = byte('0' + i - q*10)
		bp--
		i = q
	}
	// i < 10
	b[bp] = byte('0' + i)
	buf.Write(b[bp:])
}
