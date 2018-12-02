package utils

import (
	"encoding/json"
	"regexp"
	"strings"
	"unicode"
)

type StringArray string

func (r StringArray) MarshalJSON() ([]byte, error) {
	items := []string{}
	if string(r) != "" {
		items = strings.Split(string(r), ",")
	}
	for _, item := range items {
		item = strings.TrimSpace(item)
	}
	return json.Marshal(items)
}

func Ellipsis(text string, length int) string {
	r := []rune(text)
	if len(r) > length {
		return string(r[0:length]) + "..."
	}
	return text
}

func HasChinese(str string) bool {
	for _, r := range str {
		if unicode.Is(unicode.Scripts["Han"], r) || (regexp.MustCompile("[\u3002\uff1b\uff0c\uff1a\u201c\u201d\uff08\uff09\u3001\uff1f\u300a\u300b]").MatchString(string(r))) {
			return true
		}
	}
	return false
}
