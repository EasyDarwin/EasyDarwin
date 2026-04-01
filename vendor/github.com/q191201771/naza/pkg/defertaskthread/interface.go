// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package defertaskthread

type TaskFn func(param ...interface{})

type DeferTaskThread interface {
	// 注意，一个thread的多个task，本应该是串行执行的语义，
	// 目前为了简单，让它们并行执行了，以后可能会发生变化
	Go(deferMs int, task TaskFn, param ...interface{})
}

func NewDeferTaskThread() DeferTaskThread {
	return &deferTaskThread{}
}
