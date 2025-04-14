// Author: xiexu
// Date: 2024-05-07

package stream

import (
	"context"
	"log/slog"
	"math/rand"
	"time"

	"github.com/redis/go-redis/v9"
)

// BizHandler 业务函数，业务参数和空闲时间
type BizHandler func(map[string]any, time.Duration) error

// NoRepeatHandler 去重函数，返回当前任务的唯一标识
// 返回空串表示不去重
type NoRepeatHandler func(map[string]any) string

// RedisStream redis stream 封装
type RedisStream struct {
	cache        *redis.Client
	streamName   string
	groupName    string
	consumerName string
	ctx          context.Context
	count        int64 // 一次处理的消息数量
	log          *slog.Logger
	wait         chan struct{}
	waitDuration time.Duration // 循环与阻塞的等待时间，建议 1~10 秒，默认 10秒
	noRepeatFn   NoRepeatHandler
}

// Option ...
type Option func(*RedisStream)

// WithCount 设置每次处理消息的数量
func WithCount(count int64) Option {
	return func(rs *RedisStream) {
		rs.count = count
	}
}

// WithWaitTimeSec 设置拿到消息的等待时间
func WithWaitTimeSec(dur time.Duration) Option {
	if dur <= 0 {
		panic("waitTimeSec 必须大于 0")
	}
	return func(rs *RedisStream) {
		rs.waitDuration = dur
	}
}

// WithNoRepeatFn 注册去重函数，需要配合 count 使用
func WithNoRepeatFn(fn NoRepeatHandler) Option {
	return func(rs *RedisStream) {
		if fn != nil {
			rs.noRepeatFn = fn
		}
	}
}

// NewRedisStream 初始化
func NewRedisStream(ctx context.Context, c *redis.Client, streamName, groupName, consumerName string, opts ...Option) *RedisStream {
	if streamName != "" && groupName != "" && consumerName != "" {
		_ = c.XGroupCreateMkStream(ctx, streamName, groupName, "0").Err()
	}
	r := RedisStream{
		cache:        c,
		streamName:   streamName,
		groupName:    groupName,
		consumerName: consumerName,
		count:        50,
		ctx:          ctx,
		log: slog.With(
			slog.String("stream", streamName),
			slog.String("group", groupName),
			slog.String("consumer", consumerName),
		),
		wait:         make(chan struct{}, 1),
		waitDuration: 20 * time.Second,
		noRepeatFn:   func(m map[string]any) string { return "" },
	}
	for i := range opts {
		opts[i](&r)
	}
	return &r
}

// WaitForClose 等待关闭
func (c *RedisStream) WaitForClose(ctx context.Context) {
	select {
	case <-ctx.Done():
		return
	case <-c.wait:
		return
	}
}

type xMessage struct {
	redis.XMessage
	Idle time.Duration
}

// NoAckHandler 消息异常处理
func (c *RedisStream) NoAckHandler(fn BizHandler) error {
	result := c.cache.XPendingExt(c.ctx, &redis.XPendingExtArgs{
		Stream: c.streamName,
		Group:  c.groupName,
		Start:  "-",
		End:    "+",
		Count:  c.count,
	}).Val()
	var err error

	out := make([]xMessage, 0, len(result))
	cache := make(map[string]struct{})

	var i int
	for _, v := range result {
		msgs := c.cache.XRange(c.ctx, c.streamName, v.ID, v.ID).Val()
		c.log.Info("NoAckHandler retry", "result", v, "msgs", msgs)
		for _, v2 := range msgs {
			i++
			k := c.noRepeatFn(v2.Values)
			if _, exist := cache[k]; exist && k != "" {
				_ = c.cache.XAck(c.ctx, c.streamName, c.groupName, v2.ID)
				// _ = c.cache.XDel(c.ctx, c.streamName, c.groupName, v2.ID)
				continue
			}
			cache[k] = struct{}{}
			// 重试次数并未递增，建议使用等待时间
			out = append(out, xMessage{
				XMessage: v2,
				Idle:     v.Idle,
			})
		}
	}

	if i > 0 {
		c.log.Debug("NoAckHandler", "count", i, "noRepeat", len(out))
	}

	success := make([]string, 0, len(out))
	for _, v := range out {
		if e1 := fn(v.Values, v.Idle); e1 != nil {
			c.log.Error("NoAckHandler", "err", e1, "value", v.Values)
			err = e1
			continue
		}
		success = append(success, v.ID)
	}
	if len(success) > 0 {
		if err := c.cache.XAck(c.ctx, c.streamName, c.groupName, success...).Err(); err != nil {
			c.log.Error("NoAckHandler XAck", "err", err)
		}
		// if err := c.cache.XDel(c.ctx, c.streamName, success...).Err(); err != nil {
		// 	c.log.Error("NoAckHandler XDel", "err", err)
		// }
	}

	return err
}

func (c *RedisStream) noRepeat(result []redis.XStream) []redis.XMessage {
	out := make([]redis.XMessage, 0, 10)
	cache := make(map[string]struct{})
	var i int
	for _, v := range result {
		for _, v2 := range v.Messages {
			i++
			k := c.noRepeatFn(v2.Values)
			if _, exist := cache[k]; exist && k != "" {
				c.log.Debug("发现重复项，跳过", "key", k)
				_ = c.cache.XAck(c.ctx, c.streamName, c.groupName, v2.ID)
				// _ = c.cache.XDel(c.ctx, c.streamName, c.groupName, v2.ID)
				continue
			}
			cache[k] = struct{}{}
			out = append(out, v2)
		}
	}
	if i > 0 {
		c.log.Debug("XReadGroup", "count", i, "noRepeat", len(out))
	}
	return out
}

// XAdd 消息队列添加数据
func (c *RedisStream) XAdd(value any) error {
	return c.cache.XAdd(c.ctx, &redis.XAddArgs{
		Stream: c.streamName,
		MaxLen: 7000,
		ID:     "*",
		Values: value,
		Approx: true,
	}).Err()
}

// Del 删除 stream
func (c *RedisStream) Del(name string) error {
	return c.cache.Del(c.ctx, name).Err()
}

// XReadGroup 消费组处理业务
func (c *RedisStream) XReadGroup(bizFn BizHandler) error {
	result := c.cache.XReadGroup(c.ctx, &redis.XReadGroupArgs{
		Group:    c.groupName,
		Consumer: c.consumerName,
		Streams:  []string{c.streamName, ">"},
		Count:    c.count,
		Block:    c.waitDuration,
		NoAck:    false,
	}).Val()

	var err error

	success := make([]string, 0, len(result))
	for _, v := range c.noRepeat(result) {
		// 第二个参数在此处并不重要
		if e1 := bizFn(v.Values, time.Second); e1 != nil {
			err = e1
			continue
		}
		success = append(success, v.ID)
	}
	if len(success) > 0 {
		if err := c.cache.XAck(c.ctx, c.streamName, c.groupName, success...).Err(); err != nil {
			return err
		}
		// if err := c.cache.XDel(c.ctx, c.streamName, success...).Err(); err != nil {
		// 	return err
		// }
	}
	return err
}

// Consume 此函数会阻塞
// 此函数将会保证优先取最新的消息处理，同时间隔一段时间处理旧消息
// 消息有概率发生重复，由业务本身做幂等处理
func (c *RedisStream) Consume(bizFn BizHandler) {
	sec := time.Duration(rand.Intn(10))*time.Second + c.waitDuration // nolint
	ticker := time.NewTicker(sec)
	defer ticker.Stop()
	var err error
	for {
		select {
		case <-c.ctx.Done():
			c.wait <- struct{}{}
			return
		case <-ticker.C:
			err = c.NoAckHandler(bizFn)
		default:
			err = c.XReadGroup(bizFn)
		}
		if err != nil {
			c.log.Error("处理 redis 消息", "err", err)
		}
	}
}

// TimeoutHandle 超时处理
func (c *RedisStream) TimeoutHandle(expire time.Duration, fn BizHandler) BizHandler {
	return func(m map[string]any, d time.Duration) error {
		if err := fn(m, d); err != nil {
			if d > expire {
				// 任务长时间未完成，已过期，丢弃
				c.log.Error("任务过期", "idle", d.String(), "err", err, "value", m)
				return nil
			}
			return err
		}
		return nil
	}
}
