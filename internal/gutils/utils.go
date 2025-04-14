package gutils

import (
	"flag"
	"math/rand"
	"os"
	"path/filepath"
)

var ConfigDir = flag.String("conf", "./configs", "config directory, eg: -conf /configs/")

func Abs(path string) (string, error) {
	if filepath.IsAbs(path) {
		return filepath.Clean(path), nil
	}
	bin, err := os.Executable()
	if err != nil {
		return "", err
	}
	return filepath.Join(filepath.Dir(bin), path), nil
}
func GenerateRandomString(length int) string {
	const letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	b := make([]byte, length)
	for i := range b {
		b[i] = letters[rand.Intn(len(letters))]
	}
	return string(b)
}
