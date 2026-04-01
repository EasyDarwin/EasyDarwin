/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package helper

import (
	"errors"
	"image/color"
	"math"
	"os"
	"strconv"
	"unicode"
	"unicode/utf8"

	"github.com/wenlng/go-captcha/v2/base/random"
)

// t2x is to turn to hex
func t2x(t int64) string {
	result := strconv.FormatInt(t, 16)
	if len(result) == 1 {
		result = "0" + result
	}
	return result
}

// FormatAlpha is formatting transparent
func FormatAlpha(val float32) uint8 {
	a := math.Min(float64(val), 1)
	alpha := a * 255
	return uint8(alpha)
}

// RgbToHex is to turn RBG color to hex color
func RgbToHex(red int64, green int64, blue int64) string {
	r := t2x(red)
	g := t2x(green)
	b := t2x(blue)
	return r + g + b
}

// HexToRgb is to turn hex color to RBG color
func HexToRgb(hex string) (int64, int64, int64) {
	r, _ := strconv.ParseInt(hex[:2], 16, 10)
	g, _ := strconv.ParseInt(hex[2:4], 16, 18)
	b, _ := strconv.ParseInt(hex[4:], 16, 10)
	return r, g, b
}

var ColorHexFormatErr = errors.New("hex color must start with '#'")
var ColorInvalidErr = errors.New("hexToByte component invalid")

// ParseHexColor is to turn the hex color to RGB color
func ParseHexColor(s string) (c color.RGBA, err error) {
	c.A = 0xff
	if s[0] != '#' {
		return c, ColorHexFormatErr
	}

	hexToByte := func(b byte) byte {
		switch {
		case b >= '0' && b <= '9':
			return b - '0'
		case b >= 'a' && b <= 'f':
			return b - 'a' + 10
		case b >= 'A' && b <= 'F':
			return b - 'A' + 10
		}
		err = ColorInvalidErr
		return 0
	}

	switch len(s) {
	case 7:
		c.R = hexToByte(s[1])<<4 + hexToByte(s[2])
		c.G = hexToByte(s[3])<<4 + hexToByte(s[4])
		c.B = hexToByte(s[5])<<4 + hexToByte(s[6])

	case 4:
		c.R = hexToByte(s[1]) * 17
		c.G = hexToByte(s[2]) * 17
		c.B = hexToByte(s[3]) * 17
	default:
		err = ColorInvalidErr
	}
	return
}

// PathExists is to detect whether the file exists
func PathExists(path string) (bool, error) {
	_, err := os.Stat(path)
	if err == nil {
		return true, nil
	}
	if os.IsNotExist(err) {
		return false, nil
	}
	return false, err
}

// InArrayWithStr is the verification whether it is in the array
func InArrayWithStr(items []string, s string) bool {
	for _, item := range items {
		if item == s {
			return true
		}
	}
	return false
}

// IsChineseChar is to detect whether it is Chinese
func IsChineseChar(str string) bool {
	for _, r := range str {
		if unicode.Is(unicode.Scripts["Han"], r) {
			return true
		}
	}
	return false
}

// LenChineseChar is to calc Chinese and letter length
func LenChineseChar(str string) int {
	return utf8.RuneCountInString(str)
}

// RandIndex is to the random length range value
func RandIndex(length int) int {
	if length == 0 {
		return -1
	}

	index := random.RandInt(0, length)
	if index >= length {
		index = length - 1
	}

	return index
}
