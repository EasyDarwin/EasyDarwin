// Author: xiexu
// Date: 2024-05-01

package pool

import (
	"log/slog"
	"sync"
)

// Pool 限制最大并发数量，并等待任务结束
type Pool struct {
	wg      sync.WaitGroup
	limiter chan struct{}
}

// NewPool 传递参数为最大并发数量
func NewPool(taskNums int) *Pool {
	return &Pool{
		limiter: make(chan struct{}, taskNums),
	}
}

func (p *Pool) Go(fn func()) {
	p.limiter <- struct{}{}
	p.wg.Add(1)
	go func() {
		defer func() {
			if err := recover(); err != nil {
				slog.Error("pool", "err", err)
			}
		}()
		defer func() {
			p.wg.Done()
			<-p.limiter
		}()
		fn()
	}()
}

func (p *Pool) Wait() {
	p.wg.Wait()
}
