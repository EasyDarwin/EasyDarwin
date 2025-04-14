package system

import "fmt"

const (
	Reset  = "\033[0m"  // 重置颜色
	Red    = "\033[31m" // 红色
	Green  = "\033[32m" // 绿色
	Yellow = "\033[33m" // 黄色
	Blue   = "\033[34m" // 蓝色
)

// ErrPrintf error output
func ErrPrintf(format string, a ...interface{}) {
	fmt.Printf(Red+format+Reset, a...)
}

// WarnPrintf warn output
func WarnPrintf(format string, a ...interface{}) {
	fmt.Printf(Yellow+format+Reset, a...)
}
