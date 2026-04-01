// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

//go:build !386 && !arm && !mips && !mipsle
// +build !386,!arm,!mips,!mipsle

package nazaatomic

import "sync/atomic"

type Int64 struct {
	core int64
}

type Uint64 struct {
	core uint64
}

// ----------------------------------------------------------------------------

func (obj *Uint64) Load() uint64 {
	return atomic.LoadUint64(&obj.core)
}

func (obj *Uint64) Store(val uint64) {
	atomic.StoreUint64(&obj.core, val)
}

func (obj *Uint64) Add(delta uint64) (new uint64) {
	return atomic.AddUint64(&obj.core, delta)
}

// @param delta 举例，传入3，则减3
func (obj *Uint64) Sub(delta uint64) (new uint64) {
	return atomic.AddUint64(&obj.core, ^uint64(delta-1))
}

func (obj *Uint64) Increment() (new uint64) {
	return atomic.AddUint64(&obj.core, 1)
}

func (obj *Uint64) Decrement() (new uint64) {
	return atomic.AddUint64(&obj.core, ^uint64(0))
}

func (obj *Uint64) CompareAndSwap(old uint64, new uint64) (swapped bool) {
	return atomic.CompareAndSwapUint64(&obj.core, old, new)
}

func (obj *Uint64) Swap(new uint64) (old uint64) {
	return atomic.SwapUint64(&obj.core, new)
}

// ----------------------------------------------------------------------------

func (obj *Int64) Load() int64 {
	return atomic.LoadInt64(&obj.core)
}

func (obj *Int64) Store(val int64) {
	atomic.StoreInt64(&obj.core, val)
}

func (obj *Int64) Add(delta int64) (new int64) {
	return atomic.AddInt64(&obj.core, delta)
}

// @param delta 举例，传入3，则减3
func (obj *Int64) Sub(delta int64) (new int64) {
	return atomic.AddInt64(&obj.core, -delta)
}

func (obj *Int64) Increment() (new int64) {
	return atomic.AddInt64(&obj.core, 1)
}

func (obj *Int64) Decrement() (new int64) {
	return atomic.AddInt64(&obj.core, -1)
}

func (obj *Int64) CompareAndSwap(old int64, new int64) (swapped bool) {
	return atomic.CompareAndSwapInt64(&obj.core, old, new)
}

func (obj *Int64) Swap(new int64) (old int64) {
	return atomic.SwapInt64(&obj.core, new)
}
