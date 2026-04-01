// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package filesystemlayer

// 注意，这个package并没有完整实现所有的文件操作，使用内存作为存储时，存在一些限制
// 目前只是服务于我另一个项目中的特定场景 https://github.com/q191201771/lal

var (
	DefaultDiskFileSystemLayer   IFileSystemLayer
	DefaultMemoryFileSystemLayer IFileSystemLayer
)

type IFileSystemLayer interface {
	Type() FslType

	// 创建文件
	// 原始语义：如果文件已经存在，原文件内容被清空
	Create(name string) (IFile, error)

	Rename(oldpath string, newpath string) error
	MkdirAll(path string, perm uint32) error
	Remove(name string) error
	RemoveAll(path string) error

	ReadFile(filename string) ([]byte, error)
	WriteFile(filename string, data []byte, perm uint32) error
}

type IFile interface {
	Write(b []byte) (n int, err error)
	Close() error
}

type FslType int

const (
	FslTypeDisk   FslType = 1
	FslTypeMemory         = 2
)

func FslFactory(t FslType) IFileSystemLayer {
	switch t {
	case FslTypeDisk:
		return &FslDisk{}
	case FslTypeMemory:
		return NewFslMemory()
	}
	return nil
}

func init() {
	DefaultDiskFileSystemLayer = FslFactory(FslTypeDisk)
	DefaultMemoryFileSystemLayer = FslFactory(FslTypeMemory)
}
