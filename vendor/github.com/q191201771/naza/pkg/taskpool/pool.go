// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package taskpool

import (
	"sync"
)

type taskWrapper struct {
	taskFn      TaskFn
	param       []interface{}
	disposeFlag bool
}

type pool struct {
	maxWorkerNum int

	m sync.Mutex
	//totalWorkerNum int
	idleWorkerList []*worker
	blockTaskList  []taskWrapper
	allWorkerList  []*worker
	disposeFlag    bool
}

func newPool(option Option) *pool {
	p := pool{
		maxWorkerNum: option.MaxWorkerNum,
	}
	for i := 0; i < option.InitWorkerNum; i++ {
		p.newWorker()
	}
	return &p
}

func (p *pool) Go(task TaskFn, param ...interface{}) {
	p.m.Lock()
	defer p.m.Unlock()
	if p.disposeFlag {
		return
	}

	tw := taskWrapper{
		taskFn: task,
		param:  param,
	}
	var w *worker

	if len(p.idleWorkerList) != 0 {
		// 还有空闲worker

		w = p.idleWorkerList[len(p.idleWorkerList)-1]
		p.idleWorkerList = p.idleWorkerList[0 : len(p.idleWorkerList)-1]
		w.Go(tw)
	} else {
		// 无空闲worker

		if p.maxWorkerNum == 0 ||
			(p.maxWorkerNum > 0 && len(p.allWorkerList) < p.maxWorkerNum) {
			// 无最大worker限制，或还未达到限制

			p.newWorkerWithTask(tw)
		} else {
			// 已达到限制

			p.blockTaskList = append(p.blockTaskList, tw)
		}
	}
}

func (p *pool) KillIdleWorkers() {
	p.m.Lock()
	defer p.m.Unlock()
	if p.disposeFlag {
		return
	}

	for i := range p.idleWorkerList {
		p.idleWorkerList[i].Stop()
	}
	p.idleWorkerList = p.idleWorkerList[0:0]
}

func (p *pool) Dispose(t DisposeType) {
	p.m.Lock()
	defer p.m.Unlock()
	if p.disposeFlag {
		return
	}

	p.disposeFlag = true

	if t == DisposeTypeAsap {
		p.blockTaskList = nil
	} else if t == DisposeTypeRunAllBlockTask {
		// noop
	}

	for i := range p.idleWorkerList {
		p.idleWorkerList[i].Stop()
	}
	p.idleWorkerList = p.idleWorkerList[0:0]
}

func (p *pool) GetCurrentStatus() Status {
	p.m.Lock()
	defer p.m.Unlock()
	return Status{
		TotalWorkerNum: len(p.allWorkerList),
		IdleWorkerNum:  len(p.idleWorkerList),
		BlockTaskNum:   len(p.blockTaskList),
	}
}

func (p *pool) newWorker() *worker {
	w := NewWorker(p)
	w.Start()
	p.idleWorkerList = append(p.idleWorkerList, w)
	p.allWorkerList = append(p.allWorkerList, w)
	return w
}

func (p *pool) newWorkerWithTask(task taskWrapper) {
	w := NewWorker(p)
	w.Start()
	w.Go(task)
	p.allWorkerList = append(p.allWorkerList, w)
}

func (p *pool) onIdle(w *worker) {
	p.m.Lock()
	defer p.m.Unlock()
	if len(p.blockTaskList) == 0 {
		// 没有等待执行的任务

		if p.disposeFlag {
			w.Stop()
			return
		}

		p.idleWorkerList = append(p.idleWorkerList, w)
	} else {
		t := p.blockTaskList[0]
		p.blockTaskList = p.blockTaskList[1:]
		w.Go(t)
	}
}

func (p *pool) onDispose(w *worker) {
	p.m.Lock()
	defer p.m.Unlock()
	for i := range p.allWorkerList {
		if p.allWorkerList[i] == w {
			p.allWorkerList = append(p.allWorkerList[0:i], p.allWorkerList[i+1:]...)
			break
		}
	}
}
