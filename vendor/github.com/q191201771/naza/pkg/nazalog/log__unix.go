// Copyright 2024, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

//go:build linux || darwin || netbsd || freebsd || openbsd || dragonfly
// +build linux darwin netbsd freebsd openbsd dragonfly

package nazalog

func (l *logger) writeLevelStringIfNeeded(level Level) {
	if l.core.option.LevelFlag {
		if l.core.console != nil {
			l.core.buf.WriteString(levelToColorString[level])
		} else {
			l.core.buf.WriteString(levelToString[level])
		}
	}
}
