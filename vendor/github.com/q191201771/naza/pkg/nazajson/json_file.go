// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazajson

import (
	"encoding/json"
	"io/ioutil"
)

func MarshalJsonFile(in interface{}, filename string) error {
	b, err := json.Marshal(in)
	if err != nil {
		return err
	}
	err = ioutil.WriteFile(filename, b, 0755)
	return err
}

// UnmarshalJsonFile
//
// @param filenames json文件名，支持传入多个文件名，按先后顺序读取第一个存在的文件
//
func UnmarshalJsonFile(out interface{}, filenames ...string) (err error) {
	var b []byte
	for i := range filenames {
		b, err = ioutil.ReadFile(filenames[i])
		if err == nil {
			break
		}
	}
	if err != nil {
		return err
	}
	return json.Unmarshal(b, out)
}
