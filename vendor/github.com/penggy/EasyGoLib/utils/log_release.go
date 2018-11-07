// +build release

package utils

import (
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"

	rotatelogs "github.com/lestrrat-go/file-rotatelogs"
)

func Log(msg ...interface{}) {}

func Logf(format string, msg ...interface{}) {}

var rl *rotatelogs.RotateLogs

func GetLogWriter() io.Writer {
	if rl != nil {
		return rl
	}
	logDir := LogDir()
	logFile := filepath.Join(logDir, strings.ToLower(EXEName())+"-%Y%m%d.log")
	_rl, err := rotatelogs.New(logFile, rotatelogs.WithMaxAge(-1), rotatelogs.WithRotationCount(3))
	if err == nil {
		rl = _rl
		return rl
	}
	log.Println("failed to create rotatelogs", err)
	log.Println("use stdout")
	return os.Stdout
}

func CloseLogWriter() {
	log.SetOutput(os.Stdout)
	if rl != nil {
		rl.Close()
		rl = nil
	}
}
