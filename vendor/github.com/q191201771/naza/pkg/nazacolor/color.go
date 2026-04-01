// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazacolor

import "fmt"

// 示例：  \033[22;31;42mHello\033[0m
// \033[    固定前缀
// 22       格式，非出题
// 31       前景色，红色
// 42       背景色，绿色
// m        固定前缀的结束
// Hello    需要打印的值
// \033[0m  固定后缀
//
// 注意，格式，前景色，背景色三个属性可选择性设置，用分号`;`风格。并且，由于这三个属性的取值范围没有交叉，所以理论上顺序可以不固定

// Format 格式
//
// TODO(chef): 添加其他值：0（默认值）、1（高亮）、22（非粗体）、4（下划线）、24（非下划线）、 5（闪烁）、25（非闪烁）、7（反显）、27（非反显）
type Format int

const (
	FormatNonBold Format = 22
)

// FgColor 前景色(字体颜色)
type FgColor int

const (
	FgBlack  FgColor = 30
	FgRed    FgColor = 31
	FgGreen  FgColor = 32
	FgYellow FgColor = 33
	FgBlue   FgColor = 34
	FgCyan   FgColor = 36
	FgWhite  FgColor = 37

	// TODO(chef): high intensity高亮高对比颜色，起始由30变成90
)

// BgColor 背景色
type BgColor int

const (
	BgBlack  BgColor = 40
	BgRed    BgColor = 41
	BgGreen  BgColor = 42
	BgYellow BgColor = 43
	BgBlue   BgColor = 44
	BgCyan   BgColor = 46
	BgWhite  BgColor = 47

	// TODO(chef): high intensity高亮高对比颜色，起始由40变成100
)

const (
	SimplePrefixBlack  = "\033[22;30m" // 黑
	SimplePrefixRed    = "\033[22;31m" // 红
	SimplePrefixGreen  = "\033[22;32m" // 浅绿，亮绿
	SimplePrefixYellow = "\033[22;33m" // 黄
	SimplePrefixBlue   = "\033[22;34m" // 深蓝
	SimplePrefixCyan   = "\033[22;36m" // 青，暗绿
	SimplePrefixWhite  = "\033[22;37m" // 白

	SimpleSuffix = suffix
)

const (
	prefix = "\033["
	suffix = "\033[0m"
)

// Wrap 分别设置样式，前景色(字体颜色)，背景色
func Wrap(v string, format Format, fg FgColor, bg BgColor) string {
	return fmt.Sprintf("%s%d;%d;%dm%s%s", prefix, format, fg, bg, v, suffix)
}

// WrapWithFgColor 只设置前景色(字体颜色)
func WrapWithFgColor(v string, fg FgColor) string {
	return fmt.Sprintf("%s%d;%dm%s%s", prefix, FormatNonBold, fg, v, suffix)
}

// WrapBlack 将前景色(字体颜色)设置为黑色
func WrapBlack(v string) string {
	return WrapWithFgColor(v, FgBlack)
}

// WrapRed 将前景色(字体颜色)设置为红色
func WrapRed(v string) string {
	return WrapWithFgColor(v, FgRed)
}

func WrapGreen(v string) string {
	return WrapWithFgColor(v, FgGreen)
}
func WrapYellow(v string) string {
	return WrapWithFgColor(v, FgYellow)
}
func WrapBlue(v string) string {
	return WrapWithFgColor(v, FgBlue)
}
func WrapCyan(v string) string {
	return WrapWithFgColor(v, FgCyan)
}
func WrapWhite(v string) string {
	return WrapWithFgColor(v, FgWhite)
}
