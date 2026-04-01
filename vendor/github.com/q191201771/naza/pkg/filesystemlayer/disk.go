// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package filesystemlayer

import (
	"io/ioutil"
	"os"
)

type FslDisk struct {
}

func (f *FslDisk) Type() FslType {
	return FslTypeDisk
}

func (f *FslDisk) Create(name string) (IFile, error) {
	return os.Create(name)
}

func (f *FslDisk) Rename(oldpath string, newpath string) error {
	return os.Rename(oldpath, newpath)
}

func (f *FslDisk) MkdirAll(path string, perm uint32) error {
	return os.MkdirAll(path, os.FileMode(perm))
}

func (f *FslDisk) Remove(name string) error {
	return os.Remove(name)
}

func (f *FslDisk) RemoveAll(path string) error {
	return os.RemoveAll(path)
}

func (f *FslDisk) ReadFile(filename string) ([]byte, error) {
	return ioutil.ReadFile(filename)
}

func (f *FslDisk) WriteFile(filename string, data []byte, perm uint32) error {
	return ioutil.WriteFile(filename, data, os.FileMode(perm))
}
