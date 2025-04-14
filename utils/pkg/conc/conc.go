// Author: xiexu
// Date: 2024-05-01

// concurrent
// 主要提供 3 个函数
// GoRun，异步执行函数
// Wait，等待所有任务执行完毕
// UnsafeWaitWithContext，包含超时机制的等待所有任务执行完毕
package conc

import (
	"context"
	"fmt"
	"log/slog"
	"runtime"
	"runtime/debug"
	"sync"
	"time"
)

type G struct {
	wg    sync.WaitGroup
	trace Tracer
}

type Tracer interface {
	Error(msg string, args ...any)
}

func New(l Tracer) *G {
	if l == nil {
		l = DefaultTracer{}
	}
	return &G{trace: l}
}

func (g *G) Wait() {
	g.wg.Wait()
}

// UnsafeWaitWithContext wait 会一直等，此函数会有个超时限制。
func (g *G) UnsafeWaitWithContext(ctx context.Context) error {
	done := make(chan struct{}, 1)
	go func() {
		defer close(done)
		g.Wait()
	}()
	select {
	case <-done:
		return nil
	case <-ctx.Done():
		return ctx.Err()
	}
}

// GoRun 异步执行任务
// 会记录数量，并且可以 wait 等待结束
func (g *G) GoRun(fn func()) {
	g.wg.Add(1)
	go func() {
		defer g.wg.Done()
		defer func() {
			if err := recover(); err != nil {
				err := fmt.Errorf("PANIC[%v] TRACE[%s]", err, debug.Stack())
				g.trace.Error(err.Error())
			}
		}()
		fn()
	}()
}

func GoSafe(fn func()) {
	go func() {
		defer func() {
			pc, _, _, _ := runtime.Caller(1)
			funcName := runtime.FuncForPC(pc).Name()
			slog.Info("goroutine exit", "func", funcName)
			if err := recover(); err != nil {
				err := fmt.Errorf("PANIC[%v] TRACE[%s]", err, debug.Stack())
				slog.Error(err.Error())
			}
		}()
		fn()
	}()
}

type DefaultTracer struct{}

func (DefaultTracer) Error(msg string, args ...any) {
	slog.Error(msg, args...)
}

// DefaultTimer 首次是 3 秒后执行，每隔 every 执行一次
func DefaultTimer(ctx context.Context, every time.Duration, fn func()) {
	Timer(ctx, 3*time.Second, every, fn)
}

// Timer 轮询任务
func Timer(ctx context.Context, first, every time.Duration, fn func()) {
	timer := time.NewTimer(first)
	defer timer.Stop()
	for {
		select {
		case <-ctx.Done():
			return
		case <-timer.C:
			fn()
			timer.Reset(every)
		}
	}
}
