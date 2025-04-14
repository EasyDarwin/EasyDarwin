package data

import (
	"context"
	"encoding"
	"encoding/json"
	"fmt"
	"log/slog"
	"time"

	"github.com/go-kratos/kratos/v2/log"

	"github.com/redis/go-redis/v9"
)

var RedisCli *redis.Client = &redis.Client{}

// NewRedisCache redis
func NewRedisCache(addr, password string, db int) (*redis.Client, error) {
	// 创建redis客户端
	cli := redis.NewClient(&redis.Options{
		Addr:     addr,
		Password: password,
		DB:       db,
	})
	// 设置连接超时时间为5秒
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	// ping测试连接是否成功
	_, err := cli.Ping(ctx).Result()
	if err == nil {
		*RedisCli = *cli
	}
	if err != nil {
		// 调试过程中发现如果redis没有连接上，前端创建GB级联无响应，后端报空指针异常
		// 如果没有连接上redis应该及时打印错误日志，根据错误修改配置文件或处理相应错误
		// 配置文件端口、密码填写不对都会发生上述现象
		log.Error("redis连接失败", slog.Any("err", err))
	}
	return cli, err
}

// Cache 缓存
type Cache struct {
	db *redis.Client
}

// NewCache 创建缓存
func NewCache(db *redis.Client) *Cache {
	return &Cache{db: db}
}

// ServiceLoad 服务详情
type ServiceLoad struct {
	Len int `redis:"len"` // 实时负载数量
	Cap int `redis:"cap"` // 允许负载总量
}

// UnmarshalBinary implements encoding.BinaryUnmarshaler.
func (s *ServiceLoad) UnmarshalBinary(data []byte) error {
	// 将二进制数据解析为ServiceLoad结构
	return json.Unmarshal(data, s)
}

// MarshalBinary implements encoding.BinaryMarshaler.
func (s *ServiceLoad) MarshalBinary() (data []byte, err error) {
	// 将ServiceLoad结构转换为二进制数据
	return json.Marshal(s)
}

var (
	_ encoding.BinaryMarshaler   = (*ServiceLoad)(nil)
	_ encoding.BinaryUnmarshaler = (*ServiceLoad)(nil)
)

func serverKey(id string) string {
	// 生成服务key
	return fmt.Sprintf("service:%s:load", id)
}

// SetServiceLoad 设置服务负载
func (c *Cache) SetServiceLoad(ctx context.Context, id string, detail ServiceLoad) error {
	// 设置服务负载
	return c.db.HSet(ctx, serverKey(id), detail).Err()
}

// AddServiceLoad 负载数量递增递减
func (c *Cache) AddServiceLoad(ctx context.Context, id string, num int64) error {
	// 负载数量递增递减
	return c.db.HIncrBy(ctx, serverKey(id), "len", num).Err()
}

// SetServiceLoadCap 更新指定的值
func (c *Cache) SetServiceLoadCap(ctx context.Context, id string, num int64) error {
	// 更新指定的值
	return c.db.HSet(ctx, serverKey(id), "cap", num).Err()
}

// ServiceLoad 查看服务负载
func (c *Cache) ServiceLoad(ctx context.Context, id string) (ServiceLoad, error) {
	// 查看服务负载
	var load ServiceLoad
	err := c.db.HGetAll(ctx, serverKey(id)).Scan(&load)
	return load, err
}

// PushDeviceStatus 推送设备状态变更消息
func (c *Cache) PushDeviceStatus(deviceID string, status bool, model, version string) error {
	// 推送设备状态变更消息
	return c.db.XAdd(context.Background(), &redis.XAddArgs{
		Stream: "device:status:change",
		MaxLen: 5000,
		Approx: true,
		ID:     "*",
		Values: []any{
			"device_id", deviceID,
			"status", status,
			"updated_at", time.Now().Unix(),
			"model", model,
			"version", version,
			"from", "HOME",
		},
	}).Err()
}

// SetDeviceWithService 设置设备与服务绑定
func (c *Cache) SetDeviceWithService(did, _, sid string) error {
	// 设置设备与服务绑定
	return c.db.Set(context.Background(), fmt.Sprintf("devices:%s:smsserver", did), sid, 6*time.Hour).Err()
}

// GetDeviceWithService 获取设备与服务绑定
func (c *Cache) GetDeviceWithService(did string) (string, error) {
	// 获取设备与服务绑定
	tx := c.db.Get(context.Background(), fmt.Sprintf("devices:%s:smsserver", did))
	err := tx.Err()
	if err == redis.Nil {
		return "", nil
	}
	return tx.Val(), err
}

func (c *Cache) HGet(ctx context.Context, key string, field string) (string, error) {
	val, err := c.db.HGet(ctx, key, field).Result()
	return val, err
}

func (c *Cache) HGetAll(ctx context.Context, id string) (map[string]string, error) {
	return c.db.HGetAll(ctx, id).Result()
}

func (c *Cache) Get(ctx context.Context, id string) (val string, err error) {
	return c.db.Get(ctx, id).Result()
}

func (c *Cache) Set(ctx context.Context, key string, value interface{}, expiration time.Duration) error {
	return c.db.Set(ctx, key, value, expiration).Err()
}

func (c *Cache) SetEx(ctx context.Context, key string, value interface{}, expiration time.Duration) error {
	return c.db.SetEx(ctx, key, value, expiration).Err()
}

//func (c *Cache) XAdd(ctx context.Context, i any) error {
//	return c.db.XAdd(ctx, i).Err()
//}

func (c *Cache) HSet(ctx context.Context, key string, values ...interface{}) (int64, error) {
	return c.db.HSet(ctx, key, values).Result()
}

func (c *Cache) HDel(ctx context.Context, key string, fields ...string) (int64, error) {
	return c.db.HDel(ctx, key, fields...).Result()
}
