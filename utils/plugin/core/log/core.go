package log

import (
	"log/slog"
	"time"
)

// Storer 数据存储
type Storer interface {
	Create([]*Log) error       // 批量创建
	Clear(time.Duration) error // 按保留天数清理
	Find(out *[]*Log, input FindLogsInput) (total int64, err error)
}

// Core 日志业务核心
type Core struct {
	store Storer
	ch    chan *Log
	log   *slog.Logger
}

// NewCore ...
func NewCore(store Storer, log *slog.Logger) Core {
	c := Core{store: store, log: log, ch: make(chan *Log, 100)}
	go c.start()
	// go c.clean()
	return c
}

// Write 记录日志
func (c Core) Write(v *Log) {
	c.ch <- v
}

func (c Core) start() {
	ticker := time.NewTicker(10 * time.Second)
	defer ticker.Stop()
	const maxCap = 2 * 8
	cache := make([]*Log, 0, maxCap)
	var err error
	for {
		select {
		case <-ticker.C:
			if len(cache) == 0 {
				continue
			}
			err = c.store.Create(cache)
			clear(cache)
			cache = cache[:0]
		case v := <-c.ch:
			cache = append(cache, v)
			if len(cache) >= maxCap {
				err = c.store.Create(cache)
				clear(cache)
				cache = cache[:0]
			}
		}
		if err != nil {
			c.log.Error(`err := c.store.Create(cache)`, "err", err)
		}
	}
}

// Find 查询日志列表
func (c Core) Find(input FindLogsInput) ([]*Log, int64, error) {
	out := make([]*Log, 0, input.Limit())
	total, err := c.store.Find(&out, input)
	return out, total, err
}
