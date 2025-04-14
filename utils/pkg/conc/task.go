// Author: xiexu
// Date: 2024-05-01

package conc

import (
	"time"
)

// DelayTasks 延迟任务，间隔时间内，任务只会被执行一次
// 任务队列已满时，任务会被丢弃
// 任务失败后不会重试
// 超过间隔时间内的多次任务，会
// type DelayTasks struct {
// 	duration time.Duration
// 	data     Map[string, time.Time]
// 	taskCh   chan task
// }

// AssemblyLine 这是一个间隔任务的流水线
// 相同间隔时间内，任务只会被执行一次，任务失败后不会重试，同时也不会触发间隔时间
type AssemblyLine struct {
	duration time.Duration
	data     Map[string, time.Time]
	taskCh   chan task
}

// NewAssemblyLine 传入间隔时间和并发任务数量，队列缓存数量
// 任务队列已满时，任务会被丢弃
func NewAssemblyLine(duration time.Duration, count int, cacheCount int) *AssemblyLine {
	a := AssemblyLine{
		duration: duration,
		// data:      NewMap[string, time.Time](),
		taskCh: make(chan task, cacheCount),
	}
	if count <= 0 {
		count = 1
	}
	go a.clean()
	for range count {
		go a.start()
	}
	return &a
}

func (a *AssemblyLine) clean() {
	ticker := time.NewTicker(5 * time.Second)
	defer ticker.Stop()
	for {
		<-ticker.C
		a.data.Range(func(key string, value time.Time) bool {
			if time.Since(value) > a.duration {
				a.data.Delete(key)
			}
			return true
		})
	}
}

func (a *AssemblyLine) start() {
	for {
		task := <-a.taskCh
		_, exist := a.data.LoadOrStore(task.key, time.Now())
		if exist {
			continue
		}
		if err := task.fn(); err != nil {
			a.data.Delete(task.key)
		}
	}
}

type task struct {
	key string
	fn  func() error
}

// Add 添加任务，队列已满时，会阻塞等待
func (a *AssemblyLine) Add(key string, fn func() error) {
	a.taskCh <- task{key: key, fn: fn}
}

// AddOrDropIfFull 添加任务，队列已满时，丢弃
func (a *AssemblyLine) AddOrDropIfFull(key string, fn func() error) {
	select {
	case a.taskCh <- task{key: key, fn: fn}:
	default:
	}
}
