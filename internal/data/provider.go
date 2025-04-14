package data

import (
	"easydarwin/internal/conf"
	"github.com/redis/go-redis/v9"
	"path/filepath"
	"strings"
	"time"

	"easydarwin/utils/pkg/orm"
	"easydarwin/utils/pkg/system"

	"github.com/glebarez/sqlite"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

var DB *gorm.DB

var config *conf.Bootstrap

// SetupDB 初始化数据存储
func SetupDB(c *conf.Bootstrap) (*gorm.DB, error) {
	cfg := c.Data
	dial, isSQLite := getDialector(cfg.Dsn)
	if isSQLite {
		cfg.MaxIdleConns = 1
		cfg.MaxOpenConns = 1
	}

	db, err := orm.New(true, dial, orm.Config{
		MaxIdleConns:    int(cfg.MaxIdleConns),
		MaxOpenConns:    int(cfg.MaxOpenConns),
		ConnMaxLifetime: time.Duration(cfg.ConnMaxLifetime) * time.Second,
		SlowThreshold:   time.Duration(cfg.SlowThreshold) * time.Millisecond,
	})
	DB = db
	return db, err
}

func GetDatabase() *gorm.DB {
	return DB
}

func GetConfig() *conf.Bootstrap {
	return config
}
func SetConfig(c *conf.Bootstrap) {
	config = c
}

// SetupCache 初始化缓存
func SetupCache() *redis.Client {
	return RedisCli
}

// getDialector 返回 dial 和 是否 sqlite
func getDialector(dsn string) (gorm.Dialector, bool) {
	if strings.HasPrefix(dsn, "postgres") {
		return postgres.New(postgres.Config{
			DriverName: "pgx",
			DSN:        dsn,
		}), false
	}
	return sqlite.Open(filepath.Join(system.GetCWD(), dsn)), true
}
