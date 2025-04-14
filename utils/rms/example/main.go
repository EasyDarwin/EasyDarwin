package main

import (
	"context"
	"fmt"
	"time"

	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/rms/core/record"
	"easydarwin/lnton/rms/core/record/store/recorddb"
	"easydarwin/lnton/rms/recordapi"
	"github.com/gin-gonic/gin"
	"github.com/redis/go-redis/v9"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

func main() {
	dsn := "postgres://postgres:7418AD28BBF54196@212.64.34.165:20001/saida?sslmode=disable"
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		panic(err)
	}

	// rdb := redis.NewClient(&redis.Options{
	// 	Addr:     "localhost:6379",
	// 	Password: "", // 密码
	// 	DB:       0,  // 数据库
	// 	PoolSize: 20, // 连接池大小
	// })

	rdb := SetupCache("localhost:6379", "", 0)
	core := record.NewCore(recorddb.NewDB(db).AutoMigrate(false), rdb, func() (string, error) { return "", nil })
	engin := gin.Default()

	recordapi.Register(engin, core, &logg{}, func(ctx *gin.Context) {
		web.SetTraceID(ctx, "qweqwewq")
	})

	engin.Static("/static", "./")

	_ = engin.Run(":8000")
}

type logg struct{}

func (l logg) RecordLog(remark string) gin.HandlerFunc {
	return func(c *gin.Context) {
		c.Next()
	}
}

func SetupCache(addr, password string, db int) *redis.Client {
	return NewRedisCache(addr, password, db)
}

func NewRedisCache(addr, password string, db int) *redis.Client {
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
	if err != nil {
		panic(fmt.Errorf("redis 连接失败 %w", err))
	}
	return cli
}
