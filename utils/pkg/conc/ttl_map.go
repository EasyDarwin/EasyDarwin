package conc

import (
	"context"
	"time"
)

// TTLMap 带有过期时间的 map
type TTLMap[K comparable, V any] struct {
	data Map[K, V]
	exp  Map[K, time.Time]

	cancel context.CancelFunc
}

// NewTTLMap 提供默认的过期删除
// 也可以使用 SwichFixedTimeCleanup 开启定时清空
func NewTTLMap[K comparable, V any]() *TTLMap[K, V] {
	c := TTLMap[K, V]{}
	ctx, cancel := context.WithCancel(context.Background())
	c.cancel = cancel
	go c.tickerCleanup(ctx)
	return &c
}

// SwichFixedTimeCleanup 固定时间清除全部数据
// 参数 afterFn 用于获取间隔多久以后执行
func (c *TTLMap[K, V]) SwichFixedTimeClear(afterFn func() time.Duration) *TTLMap[K, V] {
	c.cancel()
	ctx, cancel := context.WithCancel(context.Background())
	c.cancel = cancel
	go c.fixedTimeCleanup(ctx, afterFn)
	return c
}

func (c *TTLMap[K, V]) fixedTimeCleanup(ctx context.Context, fn func() time.Duration) {
	timer := time.NewTimer(fn())
	defer timer.Stop()
	for {
		select {
		case <-ctx.Done():
			return
		case <-timer.C:
			c.Clear()
			timer.Reset(fn())
		}
	}
}

func (c *TTLMap[K, V]) tickerCleanup(ctx context.Context) {
	ticker := time.NewTicker(time.Second)
	defer ticker.Stop()
	for {
		select {
		case <-ctx.Done():
			return
		case <-ticker.C:
			now := time.Now()
			c.exp.Range(func(key K, value time.Time) bool {
				if now.After(value) {
					c.data.Delete(key)
					c.exp.Delete(key)
				}
				return true
			})
		}
	}
}

// Store 将在 ttl 后自动删除 k/v
func (c *TTLMap[K, V]) Store(key K, value V, ttl time.Duration) {
	c.data.Store(key, value)
	c.exp.Store(key, time.Now().Add(ttl))
}

// Load 获取未过期的 k/v
func (c *TTLMap[K, V]) Load(key K) (V, bool) {
	var v V
	expAt, ok := c.exp.Load(key)
	if !ok {
		return v, false
	}
	if time.Now().After(expAt) {
		c.data.Delete(key)
		c.exp.Delete(key)
		return v, false
	}
	return c.data.Load(key)
}

// LoadOrStore 第二个参数，true:获取 load 的数据; false:刚存储的数据
func (c *TTLMap[K, V]) LoadOrStore(key K, value V, ttl time.Duration) (V, bool) {
	c.exp.Store(key, time.Now().Add(ttl))
	return c.data.LoadOrStore(key, value)
}

// Delete 删除 k/v
func (c *TTLMap[K, V]) Delete(key K) {
	c.data.Delete(key)
	c.exp.Delete(key)
}

// Len map 长度
func (c *TTLMap[K, V]) Len() int {
	return c.data.Len()
}

// Range 遍历 map
func (c *TTLMap[K, V]) Range(fn func(key K, value V) bool) {
	c.data.Range(fn)
}

// Clear 清空数据
func (c *TTLMap[K, V]) Clear() {
	c.data.Clear()
	c.exp.Clear()
}
