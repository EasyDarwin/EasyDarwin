package plugin

import (
	"log/slog"
	"os"
	"testing"

	"easydarwin/lnton/plugin/core/log"
	"easydarwin/lnton/plugin/core/log/store/logdb"
	"github.com/gin-gonic/gin"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

func TestFind(t *testing.T) {
	g := gin.Default()
	// 初始化数据库连接
	dsn := "host=localhost port=5432 user=postgres password= dbname= sslmode=disable"
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		panic(err)
		return
	}
	// Core的Storer
	DB := logdb.NewDB(db)
	// Core.Slog.Logger
	logger := slog.New(slog.NewTextHandler(os.Stdout, nil))
	// 创建内核
	lCore := log.NewCore(DB, logger)
	// 创建上层对象
	l := NewLog(lCore)
	g.GET("/logs", l.find)
	if err := g.Run(":8080").Error; err != nil {
		return
	}
	return
}
