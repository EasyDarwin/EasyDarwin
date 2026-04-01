// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

// package unique 对象唯一ID
package unique

import (
	"fmt"
	"sync"

	"github.com/q191201771/naza/pkg/nazaatomic"
)

var global MultiGenerator

func GenUniqueKey(prefix string) string {
	return global.GenUniqueKey(prefix)
}

// 只管理一个需要生成unique id的generator对象
type SingleGenerator struct {
	prefix string
	id     nazaatomic.Uint64
}

// 统一管理各个需要生成unique id的generator对象
type MultiGenerator struct {
	mu        sync.Mutex
	prefix2id map[string]uint64
}

func NewSingleGenerator(prefix string) *SingleGenerator {
	return &SingleGenerator{
		prefix: prefix,
	}
}

func (si *SingleGenerator) GenUniqueKey() string {
	return fmt.Sprintf("%s%d", si.prefix, si.id.Increment())
}

func (mi *MultiGenerator) GenUniqueKey(prefix string) string {
	mi.mu.Lock()
	defer mi.mu.Unlock()
	id, ok := mi.prefix2id[prefix]
	if ok {
		id++
	} else {
		id = 1
	}
	mi.prefix2id[prefix] = id
	return fmt.Sprintf("%s%d", prefix, id)
}

func init() {
	global.prefix2id = make(map[string]uint64)
}
