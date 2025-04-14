package conc

import (
	"slices"
	"time"
)

// Collector .
// 1. 收集器
// 2. 分门别类
// 3. 定时同步，超时删除，删除之前再同步一次
// 4. 不会去重
type Collector[T any] struct {
	data       map[string]*content[T]
	msg        chan *CollectorMsg[T]
	createCh   chan string
	noRepeatFn NoRepeatFn[T]
}

type CollectorMsg[T any] struct {
	Key   string
	Data  *T
	Total int
}

type NoRepeatFn[T any] func(*T, *T) bool

// newCollector 创建一个新的收集器
func newCatalogRecv[T any](noRepeatFn NoRepeatFn[T]) *Collector[T] {
	return &Collector[T]{
		data:       make(map[string]*content[T]),
		msg:        make(chan *CollectorMsg[T], 10),
		createCh:   make(chan string, 10),
		noRepeatFn: noRepeatFn,
	}
}

type content[T any] struct {
	lastUpdateAt time.Time
	data         []*T
	total        int
}

// start 启动定时任务检查和保存数据
func (c *Collector[T]) start(save func(string, []*T)) {
	// 创建一个每 2 秒触发一次的定时器
	check := time.NewTicker(time.Second * 2)
	// 在函数结束时停止定时器
	defer check.Stop()
	for {
		select {
		// 每 2 秒触发一次，检查数据
		case <-check.C:
			for k, v := range c.data {
				// 如果数据最后更新的时间超过 10 秒，保存并删除该数据
				if time.Since(v.lastUpdateAt) > 10*time.Second {
					save(k, v.data) // 兼容数据不全的情况，有些下级可能发送重复的通道
					delete(c.data, k)
					continue
				}
				// 如果数据的总量大于 0 且当前数据的长度达到或超过总量，保存并删除该数据
				if v.total > 0 && len(v.data) >= v.total {
					save(k, v.data)
					delete(c.data, k)
					continue
				}
			}
		// 从 createCh 通道接收数据并创建新的条目
		case v := <-c.createCh:
			c.data[v] = &content[T]{lastUpdateAt: time.Now(), data: make([]*T, 0, 1), total: -1}
		// 从 msg 通道接收数据并更新条目
		case msg := <-c.msg:
			data, exist := c.data[msg.Key]
			// 如果数据不存在，跳过该消息
			if !exist {
				continue
			}
			// 如果数据已存在且无重复，跳过该消息
			if slices.ContainsFunc(data.data, func(v *T) bool {
				return c.noRepeatFn(v, msg.Data)
			}) {
				continue
			}
			// 添加数据到对应的条目并更新最后更新时间和总量
			data.data = append(data.data, msg.Data)
			data.lastUpdateAt = time.Now()
			data.total = msg.Total
		}
	}
}
