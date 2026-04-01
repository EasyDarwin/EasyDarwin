package util

import (
	"strconv"
	"strings"
	"unicode"
)

func FormatSignedFixedFloat1616(val int32) string {
	if val&0xffff == 0 {
		return strconv.Itoa(int(val >> 16))
	} else {
		return strconv.FormatFloat(float64(val)/(1<<16), 'f', 5, 64)
	}
}

func FormatUnsignedFixedFloat1616(val uint32) string {
	if val&0xffff == 0 {
		return strconv.Itoa(int(val >> 16))
	} else {
		return strconv.FormatFloat(float64(val)/(1<<16), 'f', 5, 64)
	}
}

func FormatSignedFixedFloat88(val int16) string {
	if val&0xff == 0 {
		return strconv.Itoa(int(val >> 8))
	} else {
		return strconv.FormatFloat(float64(val)/(1<<8), 'f', 3, 32)
	}
}

func EscapeUnprintable(r rune) rune {
	if unicode.IsGraphic(r) {
		return r
	}
	return rune('.')
}

func EscapeUnprintables(src string) string {
	return strings.Map(EscapeUnprintable, src)
}
