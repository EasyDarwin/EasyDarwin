package helper

import (
	"crypto/md5"
	"encoding/hex"
	"fmt"
	"os"
	"path"
)

// GetPWD .
func GetPWD() string {
	path, err := os.Getwd()
	if err != nil {
		return ""
	}
	return path
}

// StringToMD5 MD5
func StringToMD5(src string) string {
	m := md5.New()
	m.Write([]byte(src))
	res := hex.EncodeToString(m.Sum(nil))
	return res
}

func CopyFileTo(srcPath, destPath string) error {
	b, err := os.ReadFile(srcPath)
	if err != nil {
		fmt.Printf("read file fail, %v", b)
	}

	var file *os.File
	err = os.MkdirAll(path.Dir(destPath), os.ModePerm)
	if err != nil {
		return err
	}

	if _, err = os.Stat(destPath); os.IsNotExist(err) {
		file, err = os.Create(destPath)
	} else {
		file, err = os.OpenFile(destPath, os.O_RDWR, 0666)
	}
	_, err = file.Write(b)
	return err
}
