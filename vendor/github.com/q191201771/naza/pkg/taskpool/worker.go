// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package taskpool

type worker struct {
	taskChan chan taskWrapper
	p        *pool
}

func NewWorker(p *pool) *worker {
	return &worker{
		taskChan: make(chan taskWrapper, 1),
		p:        p,
	}
}

func (w *worker) Start() {
	go func() {
		for {
			task := <-w.taskChan
			if task.disposeFlag {
				w.p.onDispose(w)
				break
			}
			task.taskFn(task.param...)
			w.p.onIdle(w)
		}
	}()
}

func (w *worker) Stop() {
	w.taskChan <- taskWrapper{
		disposeFlag: true,
	}
}

func (w *worker) Go(t taskWrapper) {
	w.taskChan <- t
}
