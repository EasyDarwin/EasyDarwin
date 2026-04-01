// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package filesystemlayer

import (
	"errors"
	"fmt"
	"os"
	"strings"
	"sync"
)

var ErrNotFound = errors.New("naza filesystemlayer: not found")

type FslMemory struct {
	mu    sync.Mutex
	files map[string]*file // key filename
}

type file struct {
	buf []byte
}

func NewFslMemory() *FslMemory {
	return &FslMemory{
		files: make(map[string]*file),
	}
}

func (f *FslMemory) Type() FslType {
	return FslTypeMemory
}

func (f *FslMemory) Create(name string) (IFile, error) {
	return f.openFile(name, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0666)
}

func (f *FslMemory) Rename(oldpath string, newpath string) error {
	f.mu.Lock()
	defer f.mu.Unlock()
	fi, exist := f.files[oldpath]
	if !exist {
		return ErrNotFound
	}
	delete(f.files, oldpath)
	f.files[newpath] = fi
	return nil
}

func (f *FslMemory) MkdirAll(path string, perm uint32) error {
	return nil
}

func (f *FslMemory) Remove(name string) error {
	f.mu.Lock()
	defer f.mu.Unlock()
	_, exist := f.files[name]
	if !exist {
		return ErrNotFound
	}
	delete(f.files, name)
	return nil
}

func (f *FslMemory) RemoveAll(path string) error {
	if !os.IsPathSeparator(path[len(path)-1]) {
		path = fmt.Sprintf("%s%c", path, os.PathSeparator)
	}
	f.mu.Lock()
	defer f.mu.Unlock()
	files := make(map[string]*file)
	for k, v := range f.files {
		if !strings.HasPrefix(k, path) {
			files[k] = v
		}
	}
	f.files = files

	return nil
}

func (f *FslMemory) ReadFile(filename string) ([]byte, error) {
	f.mu.Lock()
	defer f.mu.Unlock()
	fi, exist := f.files[filename]
	if !exist {
		return nil, ErrNotFound
	}
	return fi.clone(), nil
}

func (f *FslMemory) WriteFile(filename string, data []byte, perm uint32) error {
	fi, err := f.openFile(filename, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, perm)
	if err != nil {
		return err
	}
	_, err = fi.Write(data)
	if err1 := fi.Close(); err == nil {
		err = err1
	}
	return err
}

// ---------------------------------------------------------------------------------------------------------------------

func (f *FslMemory) openFile(name string, flag int, perm uint32) (IFile, error) {
	f.mu.Lock()
	defer f.mu.Unlock()
	fi, ok := f.files[name]
	if !ok {
		fi = &file{}
		f.files[name] = fi
		return fi, nil
	}

	fi.truncate()
	return fi, nil
}

// ---------------------------------------------------------------------------------------------------------------------

func (f *file) Write(b []byte) (n int, err error) {
	f.buf = append(f.buf, b...)
	return len(b), nil
}

func (f *file) Close() error {
	return nil
}

func (f *file) truncate() {
	f.buf = nil
}

func (f *file) clone() []byte {
	if f.buf == nil {
		return nil
	}
	b := make([]byte, len(f.buf))
	copy(b, f.buf)
	return b
}
