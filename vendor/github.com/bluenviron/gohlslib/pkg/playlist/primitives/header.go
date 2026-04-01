package primitives

import (
	"fmt"
)

// HeaderUnmarshal decodes a header.
func HeaderUnmarshal(s string) (string, error) {
	var line string
	line, s = ReadLine(s)

	if line != "#EXTM3U" {
		return "", fmt.Errorf("M3U8 header is missing")
	}

	return s, nil
}
