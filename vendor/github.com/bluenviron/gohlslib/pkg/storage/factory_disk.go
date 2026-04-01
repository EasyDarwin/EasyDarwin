package storage

import (
	"path/filepath"
)

type factoryDisk struct {
	dirPath string
}

// NewFactoryDisk allocates a disk-backed factory.
func NewFactoryDisk(dirPath string) Factory {
	return &factoryDisk{
		dirPath: dirPath,
	}
}

// NewFile implements Factory.
func (s *factoryDisk) NewFile(fileName string) (File, error) {
	return newFileDisk(filepath.Join(s.dirPath, fileName))
}
