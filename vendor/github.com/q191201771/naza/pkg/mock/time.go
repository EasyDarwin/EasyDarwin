// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package mock

import (
	"sort"
	"sync"
	"time"
)

// TODO(chef): [feat] 增加Clock::NewTicker

type Clock interface {
	// Now NewTimer ...
	//
	// 标准库中的操作集合, stdClock和mockClock都有对应的实现
	//
	Now() time.Time
	NewTimer(d time.Duration) *Timer
	Sleep(d time.Duration)

	// Add Set ...
	//
	// mockClock使用以下这些函数来修改当前时间
	// 注意，如果是stdClock，则没有必要调用以下函数（调用以下函数为空实现）
	//
	Add(d time.Duration)
	Set(t time.Time)
}

func NewStdClock() Clock {
	return &stdClock{}
}

func NewFakeClock() Clock {
	return &fakeClock{}
}

// ---------------------------------------------------------------------------------------------------------------------

type stdClock struct {
}

func (c *stdClock) Now() time.Time {
	return time.Now()
}

func (c *stdClock) NewTimer(d time.Duration) *Timer {
	stdTimer := time.NewTimer(d)
	return &Timer{
		C:        stdTimer.C,
		stdTimer: stdTimer,
	}
}

func (c *stdClock) Sleep(d time.Duration) {
	time.Sleep(d)
}

func (c *stdClock) Add(d time.Duration) {
	// noop
}

func (c *stdClock) Set(t time.Time) {
	// noop
}

// ---------------------------------------------------------------------------------------------------------------------

type fakeClock struct {
	mu     sync.Mutex
	now    time.Time
	timers timers
}

func (fc *fakeClock) Now() time.Time {
	fc.mu.Lock()
	defer fc.mu.Unlock()
	return fc.now
}

func (fc *fakeClock) NewTimer(d time.Duration) *Timer {
	fc.mu.Lock()
	defer fc.mu.Unlock()
	ch := make(chan time.Time, 1)
	t := &Timer{
		C:      ch,
		c:      ch,
		fc:     fc,
		timing: fc.now.Add(d),
	}
	fc.addTimer(t)
	return t
}

func (fc *fakeClock) Sleep(d time.Duration) {
	// TODO(chef): [feat] 实现和add、set挂钩的Sleep，内部用Timer实现等待
	// 当前的使用场景都是测试场景，直接快速跳过Sleep以及能够满足需求
}

func (fc *fakeClock) Add(d time.Duration) {
	fc.mu.Lock()
	fc.now = fc.now.Add(d)
	fc.ringTimersIfNeeded()
	fc.mu.Unlock()

	time.Sleep(1 * time.Millisecond)
}

func (fc *fakeClock) Set(t time.Time) {
	fc.mu.Lock()
	fc.now = t
	fc.ringTimersIfNeeded()
	fc.mu.Unlock()

	time.Sleep(1 * time.Millisecond)
}

func (fc *fakeClock) resetTimerWithLock(t *Timer, d time.Duration) bool {
	fc.mu.Lock()
	defer fc.mu.Unlock()
	if t.expired || t.stopped {
		return false
	}

	fc.delTimer(t)
	t.timing = fc.now.Add(d)
	fc.addTimer(t)
	return true
}

func (fc *fakeClock) stopTimerWithLock(t *Timer) bool {
	fc.mu.Lock()
	defer fc.mu.Unlock()
	if t.expired || t.stopped {
		return false
	}

	fc.delTimer(t)
	t.stopped = true
	return true
}

func (fc *fakeClock) addTimer(t *Timer) {
	fc.timers = append(fc.timers, t)
	sort.Sort(fc.timers)
}

func (fc *fakeClock) delTimer(t *Timer) {
	i := 0
	for ; i < len(fc.timers); i++ {
		if fc.timers[i] == t {
			fc.timers = append(fc.timers[:i], fc.timers[i+1:]...)
		}
	}
}

func (fc *fakeClock) ringTimersIfNeeded() {
	i := 0
	for ; i < len(fc.timers); i++ {
		if fc.timers[i].timing.After(fc.now) {
			break
		}
		fc.timers[i].c <- fc.now
		fc.timers[i].expired = true
	}
	fc.timers = fc.timers[i:]
}

// TODO(chef): [perf] 用有序map
type timers []*Timer

func (t timers) Len() int {
	return len(t)
}

func (t timers) Less(i, j int) bool {
	return t[i].timing.Before(t[j].timing)
}

func (t timers) Swap(i, j int) {
	t[i], t[j] = t[j], t[i]
}

// ---------------------------------------------------------------------------------------------------------------------

type Timer struct {
	C <-chan time.Time

	stdTimer *time.Timer

	fc      *fakeClock
	timing  time.Time
	c       chan time.Time
	expired bool
	stopped bool
}

func (t *Timer) Reset(d time.Duration) bool {
	if t.stdTimer != nil {
		return t.stdTimer.Reset(d)
	}

	return t.fc.resetTimerWithLock(t, d)
}

func (t *Timer) Stop() bool {
	if t.stdTimer != nil {
		return t.stdTimer.Stop()
	}

	return t.fc.stopTimerWithLock(t)
}
