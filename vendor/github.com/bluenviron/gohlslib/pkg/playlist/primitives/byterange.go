package primitives

import (
	"strconv"
	"strings"
)

// ByteRangeUnmarshal decodes a byte range.
func ByteRangeUnmarshal(v string) (uint64, *uint64, error) {
	i := strings.IndexByte(v, '@')

	if i >= 0 {
		str1, str2 := v[:i], v[i+1:]

		length, err := strconv.ParseUint(str1, 10, 64)
		if err != nil {
			return 0, nil, err
		}

		start, err := strconv.ParseUint(str2, 10, 64)
		if err != nil {
			return 0, nil, err
		}

		return length, &start, nil
	}

	length, err := strconv.ParseUint(v, 10, 64)
	if err != nil {
		return 0, nil, err
	}

	return length, nil, nil
}

// ByteRangeMarshal encodes a byte range.
func ByteRangeMarshal(length uint64, start *uint64) string {
	ret := strconv.FormatUint(length, 10)

	if start != nil {
		ret += "@" + strconv.FormatUint(*start, 10)
	}

	return ret
}
