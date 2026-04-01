// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazajson

import (
	"encoding/json"
	"errors"
	"reflect"
	"strings"
)

var ErrJson = errors.New("nazajson: fxxk")

type Json struct {
	//raw []byte
	m map[string]interface{}
}

func New(raw []byte) (Json, error) {
	var j Json
	err := j.Init(raw)
	return j, err
}

func (j *Json) Init(raw []byte) error {
	return json.Unmarshal(raw, &j.m)
}

// 判断 json 中某个字段是否存在
// @param path 支持多级格式，用句号`.`分隔，比如 log.level
func (j *Json) Exist(path string) bool {
	return exist(j.m, path)
}

// ---------------------------------------------------------------------------------------------------------------------

// CollectNotExistFields
//
// @param data         json字符串
// @param v            对应的结构体变量，或者结构体指针变量。注意，并不要求是反序列化赋值后的结构体变量，内部主要是获取字段、tag等信息
// @param ignorePrefix 可选参数，可填入不需要收集的字段的前缀。
//                     注意，这里只做简单字符串匹配，比如填入`a`，那么所有以`a`开头的全部会过滤掉（不光是`a.[xxx]`，还包含`ab.`，`ac.`等等）
//                     以上语义以后可能会发送变化，建议使用方在字段名字相似的情况下，使用完成的字段名称
//
// @return 返回所有不存在的json字段组成的数组
//
func CollectNotExistFields(data []byte, v interface{}, ignorePrefix ...string) ([]string, error) {
	j, err := New(data)
	if err != nil {
		return nil, err
	}
	typ := reflect.TypeOf(v)
	ret, err := collectNotExistFields(j, "", typ, 0)
	if err != nil || len(ignorePrefix) == 0 {
		return ret, err
	}

	var filterRet []string
	for _, notExistField := range ret {
		shouldIgnore := false
		for _, ignorePrefixItem := range ignorePrefix {
			if strings.HasPrefix(notExistField, ignorePrefixItem) {
				shouldIgnore = true
				break
			}
		}
		if !shouldIgnore {
			filterRet = append(filterRet, notExistField)
		}
	}
	return filterRet, nil
}

// ---------------------------------------------------------------------------------------------------------------------

// @param prefix     判断json是否存在的路径前缀，如果没有，设置为""
//                   注意，路径可以是多级，但是最后的字符不需要`.`
// @param debugDepth 递归调用层级，调试时使用
//
func collectNotExistFields(j Json, prefix string, typ reflect.Type, debugDepth int) (notExists []string, err error) {
	//nazalog.Debugf("[%d] > collectNotExistFields. typ=%+v", debugDepth, typ)
	if typ.Kind() == reflect.Ptr {
		typ = typ.Elem()
	}
	if typ.Kind() != reflect.Struct {
		return nil, ErrJson
	}

	for i := 0; i < typ.NumField(); i++ {
		field := typ.Field(i)
		//nazalog.Debugf("[%d] iterate field=%+v", debugDepth, field)
		if field.Anonymous {
			anonyNotExists, err := collectNotExistFields(j, prefix, field.Type, debugDepth+1)
			if err != nil {
				return anonyNotExists, err
			}
			notExists = append(notExists, anonyNotExists...)
		}
		// json的字段名
		jk, ok := field.Tag.Lookup("json")
		if !ok {
			// 字段没有json tag，直接忽略
			continue
		}
		if prefix != "" {
			jk = prefix + "." + jk
		}
		ok = j.Exist(jk)
		if !ok {
			// 注意，如果这一层的字段是结构体类型，且不存在，那么该结构体的所有字段都将不存在
			// 此时有多种收集方式
			// 我们的做法是选择不收集结构体自身，而是收集该结构体的所有字段
			if field.Type.Kind() != reflect.Struct {
				notExists = append(notExists, jk)
			}
		}
		//nazalog.Debugf("[%d] check exist result. ok=%+v, prefix=%s, jk=%s", debugDepth, ok, prefix, jk)

		if field.Type.Kind() == reflect.Struct {
			subNotExists, err := collectNotExistFields(j, jk, field.Type, debugDepth+1)
			if err != nil {
				return subNotExists, err
			}
			notExists = append(notExists, subNotExists...)
		}
	}

	//nazalog.Debugf("[%d] end. collect=%+v", debugDepth, notExists)
	return
}

// 判断`m`的`path`是否存在
func exist(m map[string]interface{}, path string) bool {
	ps := strings.Split(path, ".")

	if len(ps) > 1 {
		v, ok := m[ps[0]]
		if !ok {
			return false
		}
		mm, ok := v.(map[string]interface{})
		if !ok {
			return false
		}
		return exist(mm, ps[1])
	}

	_, ok := m[ps[0]]
	return ok
}
