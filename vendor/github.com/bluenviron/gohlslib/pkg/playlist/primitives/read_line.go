package primitives

import (
	"strings"
)

// ReadLine reads a line from a string. It returns the line and the remaining string.
func ReadLine(s string) (string, string) {
	i := strings.IndexByte(s, '\n')
	if i < 0 {
		return s, ""
	}

	line, remaining := s[:i], s[i+1:]

	if len(line) != 0 && line[len(line)-1] == '\r' {
		line = line[:len(line)-1]
	}

	return line, remaining
}
