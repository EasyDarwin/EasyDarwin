// Author: xiexu
// Date: 2024-05-07

package stream

import (
	"context"
	"fmt"
	"log/slog"
	"os"
	"sync"
	"testing"
	"time"

	"github.com/redis/go-redis/v9"
)

func newRedis(addr, password string) *redis.Client {
	cli := redis.NewClient(&redis.Options{
		Addr:     addr,
		DB:       0,
		Password: password,
	})
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	_, err := cli.Ping(ctx).Result()
	if err != nil {
		panic(fmt.Errorf("redis 连接失败 %w", err))
	}
	return cli
}

func TestXReadGroup(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.TODO(), 15*time.Second)
	defer cancel()

	cli := newRedis("127.0.0.1:6379", "")

	rs := NewRedisStream(ctx, cli, "testc", "g1", "c1", WithWaitTimeSec(time.Second))
	if err := rs.XAdd(map[string]any{"b": 1}); err != nil {
		panic(err)
	}
	if err := rs.XAdd(map[string]any{"b": 2}); err != nil {
		panic(err)
	}
	if err := rs.XAdd(map[string]any{"b": 3}); err != nil {
		panic(err)
	}

	a := "2"
	go func() {
		time.Sleep(5 * time.Second)
		fmt.Println("chanage")
		a = "*"
	}()

	rs.Consume(func(m map[string]any, idle time.Duration) error {
		fmt.Println("one", m)
		if idle > 30*time.Minute {
			return nil
		}
		if m["b"].(string) == a {
			return fmt.Errorf("retry" + a)
		}
		return nil
	})
}

func TestStream(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.TODO(), 15*time.Second)
	defer cancel()

	cli := newRedis("127.0.0.1:6379", "")

	rs := NewRedisStream(ctx, cli, "testa", "g1", "c1", WithWaitTimeSec(time.Second))
	if err := rs.XAdd(map[string]any{"a": 1}); err != nil {
		panic(err)
	}

	go func() {
		time.Sleep(time.Second)
		if err := rs.XAdd(map[string]any{"a": 2}); err != nil {
			panic(err)
		}
	}()

	var i int
	go rs.Consume(func(m map[string]any, idle time.Duration) error {
		i++
		// 模拟消息处理失败
		if i == 1 {
			return fmt.Errorf("test")
		}
		fmt.Println(m)
		return nil
	})
	rs.WaitForClose(context.TODO())
	fmt.Println("end")
}

func TestNoRepeat(t *testing.T) {
	slog.SetDefault(slog.New(slog.NewTextHandler(os.Stdout, &slog.HandlerOptions{AddSource: true, Level: slog.LevelDebug})))
	ctx, cancel := context.WithTimeout(context.TODO(), 55*time.Second)
	defer cancel()

	cli := newRedis("127.0.0.1:6379", "")

	rs := NewRedisStream(ctx, cli, "testa", "g1", "c1", WithWaitTimeSec(time.Second), WithNoRepeatFn(func(m map[string]any) string {
		return m["a"].(string)
	}))
	if err := rs.XAdd(map[string]any{"a": 1}); err != nil {
		panic(err)
	}
	if err := rs.XAdd(map[string]any{"a": 1}); err != nil {
		panic(err)
	}
	if err := rs.XAdd(map[string]any{"a": 1}); err != nil {
		panic(err)
	}
	if err := rs.XAdd(map[string]any{"a": 1}); err != nil {
		panic(err)
	}

	go func() {
		time.Sleep(time.Second)
		if err := rs.XAdd(map[string]any{"a": 2}); err != nil {
			panic(err)
		}
		if err := rs.XAdd(map[string]any{"a": 1}); err != nil {
			panic(err)
		}
		if err := rs.XAdd(map[string]any{"a": 1}); err != nil {
			panic(err)
		}
	}()

	var i int
	go rs.Consume(func(m map[string]any, idle time.Duration) error {
		i++
		// 模拟消息处理失败
		if i == 1 {
			return fmt.Errorf("test")
		}
		fmt.Println(m)
		return nil
	})
	rs.WaitForClose(context.TODO())
	fmt.Println("end")
}

func TestDoubleGroupWithXDel(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.TODO(), 10*time.Second)
	defer cancel()

	cli := newRedis("127.0.0.1:6379", "")
	cli2 := newRedis("127.0.0.1:6379", "")

	rs1 := NewRedisStream(ctx, cli, "testa3", "g1", "c1", WithWaitTimeSec(time.Second), WithNoRepeatFn(func(m map[string]any) string {
		return m["a"].(string)
	}))
	rs2 := NewRedisStream(ctx, cli2, "testa3", "g2", "c2", WithWaitTimeSec(time.Second), WithNoRepeatFn(func(m map[string]any) string {
		return m["a"].(string)
	}))

	g1 := make([]map[string]any, 0, 10)
	go rs1.Consume(func(m map[string]any, d time.Duration) error {
		g1 = append(g1, m)
		fmt.Println("g1:", m)
		return nil
	})
	g2 := make([]map[string]any, 0, 10)

	var once sync.Once
	go rs2.Consume(func(m map[string]any, d time.Duration) error {
		g2 = append(g2, m)
		once.Do(func() {
			time.Sleep(time.Second)
		})
		fmt.Println("g2:", m)
		return nil
	})
	for i := range 10 {
		if err := rs1.XAdd(map[string]any{"a": i}); err != nil {
			panic(err)
		}
	}
	rs1.WaitForClose(ctx)
	rs2.WaitForClose(ctx)

	if len(g1) != len(g2) {
		t.Fatal("期望相等，结果不相等")
	}

	fmt.Println("end")
}

func TestStatus(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.TODO(), 20*time.Second)
	defer cancel()
	cli := newRedis("127.0.0.1:3444", "46EF9C5A289942C3")
	rs := NewRedisStream(ctx, cli, "device:status:change", "main", "c1", WithWaitTimeSec(2*time.Second))
	rs.NoAckHandler(rs.TimeoutHandle(5*time.Hour, func(m map[string]any, d time.Duration) error {
		return nil
	}))
}
