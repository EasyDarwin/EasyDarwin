package estring

import (
	"crypto/md5"
	"encoding/base64"
	"encoding/hex"
	"fmt"
	"math/rand"
	"regexp"
	"strconv"
	"strings"
	"time"
	"unicode"

	"github.com/teris-io/shortid"
)

func ParseUInt(content string) uint {
	v, error := strconv.Atoi(content)
	if error != nil {
		return 0
	}
	return uint(v)
}
func ParseInt(content string) int {
	v, error := strconv.Atoi(content)
	if error != nil {
		return 0
	}
	return v
}
func ParseBool(content string) bool {
	v, error := strconv.ParseBool(content)
	if error != nil {
		return false
	}
	return v
}

// 将字符串加密成 md5
func String2md5(str string) string {
	data := []byte(str)
	has := md5.Sum(data)
	return fmt.Sprintf("%x", has) //将[]byte转成16进制
}

// RandomString 在数字、大写字母、小写字母范围内生成num位的随机字符串
func RandomString(length int) string {
	// 48 ~ 57 数字
	// 65 ~ 90 A ~ Z
	// 97 ~ 122 a ~ z
	// 一共62个字符，在0~61进行随机，小于10时，在数字范围随机，
	// 小于36在大写范围内随机，其他在小写范围随机
	rand.Seed(time.Now().UnixNano())
	result := make([]string, 0, length)
	for i := 0; i < length; i++ {
		t := rand.Intn(62)
		if t < 10 {
			result = append(result, strconv.Itoa(rand.Intn(10)))
		} else if t < 36 {
			result = append(result, string(rand.Intn(26)+65))
		} else {
			result = append(result, string(rand.Intn(26)+97))
		}
	}
	return strings.Join(result, "")
}

func Substr(str string, start, length int) string {
	rs := []rune(str)
	rl := len(rs)
	end := 0

	if start < 0 {
		start = rl - 1 + start
	}
	end = start + length

	if start > end {
		start, end = end, start
	}

	if start < 0 {
		start = 0
	}
	if start > rl {
		start = rl
	}
	if end < 0 {
		end = 0
	}
	if end > rl {
		end = rl
	}
	return string(rs[start:end])
}

// MD5 String
func MD5(str string) string {
	encoder := md5.New()
	encoder.Write([]byte(str))
	return hex.EncodeToString(encoder.Sum(nil))
}

func ShortID() string {
	return strings.Replace(shortid.MustGenerate(), "-", "M", -1)
}

const (
	base64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
)

var coder = base64.NewEncoding(base64Table)

func Base64Encode(src string) string {
	return coder.EncodeToString([]byte(src))
}

func Base64Decode(src string) string {
	r, _ := coder.DecodeString(src)
	return string(r)
}

// IsChineseChar 判断是否包含中文字符
func IsChineseChar(str string) bool {
	for _, r := range str {
		if unicode.Is(unicode.Scripts["Han"], r) || (regexp.MustCompile("[\u3002\uff1b\uff0c\uff1a\u201c\u201d\uff08\uff09\u3001\uff1f\u300a\u300b]").MatchString(string(r))) {
			return true
		}
	}
	return false
}

// FormatPath 格式化地址格式
func FormatPath(path string) string {
	return strings.Replace(path, "\\", "/", -1)
}

// Contains judge is contain substr
func Contains(src []string, substr string) bool {
	if strings.Contains(fmt.Sprintf("|%s|", strings.Join(src, "|")), fmt.Sprintf("|%s|", substr)) {
		return true
	}
	return false
}
