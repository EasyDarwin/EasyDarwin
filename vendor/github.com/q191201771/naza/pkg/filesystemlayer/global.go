// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package filesystemlayer

//var global IFileSystemLayer
//
//func Config(t FslType) {
//	global = FslFactory(t)
//}
//
//func Type() FslType {
//	return global.Type()
//}
//
//func Create(name string) (IFile, error) {
//	return global.Create(name)
//}
//func Rename(oldpath string, newpath string) error {
//	return global.Rename(oldpath, newpath)
//}
//
//func MkdirAll(path string, perm uint32) error {
//	return MkdirAll(path, perm)
//}
//
//func Remove(name string) error {
//	return Remove(name)
//}
//
//func RemoveAll(path string) error {
//	return RemoveAll(path)
//}
//
//func ReadFile(filename string) ([]byte, error) {
//	return global.ReadFile(filename)
//}
//
//func WriteFile(filename string, data []byte, perm uint32) error {
//	return global.WriteFile(filename, data, perm)
//}
//
//func init() {
//	global = FslFactory(FslTypeDisk)
//}
