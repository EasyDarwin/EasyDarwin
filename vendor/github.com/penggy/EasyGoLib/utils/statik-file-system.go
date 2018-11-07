package utils

import (
	"net/http"
	"strings"
)

type StatikFileSystem struct {
	http.FileSystem
}

func (s *StatikFileSystem) Exists(prefix string, filepath string) bool {
	if p := strings.TrimPrefix(filepath, prefix); len(p) < len(filepath) {
		_, err := s.Open("/" + p)
		return err == nil
	}
	return false
}
