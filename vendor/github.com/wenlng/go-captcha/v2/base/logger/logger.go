/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package logger

import (
	"fmt"
	"log"
	"os"
)

const (
	color_red = uint8(iota + 91)
	color_green
	color_yellow
	color_blue
)

// Logger .
type Logger interface {
	Infof(format string, v ...interface{})
	Errorf(format string, v ...interface{})
	Warnf(format string, v ...interface{})
	Debugf(format string, v ...interface{})
}

func New() Logger {
	l := &logx{l: log.New(os.Stderr, "", log.Ldate|log.Lmicroseconds)}
	return l
}

var _ Logger = (*logx)(nil)

var Logx = New()

// logx .
type logx struct {
	l *log.Logger
}

// Infof .
func (l *logx) Infof(format string, v ...interface{}) {
	l.output("INFO GO-CAPTCHA: %s", fmt.Sprintf("\x1b[%dm"+format+"\x1b[0m", color_green, v))
}

// Errorf .
func (l *logx) Errorf(format string, v ...interface{}) {
	l.output("ERROR GO-CAPTCHA: %s", fmt.Sprintf("\x1b[%dm"+format+"\x1b[0m", color_red, v))
}

// Warnf .
func (l *logx) Warnf(format string, v ...interface{}) {
	l.output("WARN GO-CAPTCHA: %s", fmt.Sprintf("\x1b[%dm"+format+"\x1b[0m", color_yellow, v))
}

// Debugf .
func (l *logx) Debugf(format string, v ...interface{}) {
	l.output("DEBUG GO-CAPTCHA: %s", fmt.Sprintf("\x1b[%dm"+format+"\x1b[0m", color_blue, v))
}

// output .
func (l *logx) output(format string, v ...interface{}) {
	if len(v) == 0 {
		l.l.Print(format)
		return
	}
	l.l.Printf(format, v...)
}
