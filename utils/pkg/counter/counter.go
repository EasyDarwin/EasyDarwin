package counter

import "github.com/redis/go-redis/v9"

// Engine 计数器
type Engine struct {
	cli *redis.Client
}

// New ...
func New(cli *redis.Client) *Engine {
	return &Engine{
		cli: cli,
	}
}
