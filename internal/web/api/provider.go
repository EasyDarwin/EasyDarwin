package api

import (
	"crypto/sha256"
	"easydarwin/internal/conf"
	"easydarwin/internal/core/livestream"
	"easydarwin/internal/core/livestream/store/livestreamdb"
	"easydarwin/internal/core/svr"
	"easydarwin/internal/core/video"
	"easydarwin/internal/core/video/store/voddb"
	"easydarwin/internal/data"
	"encoding/hex"
	"fmt"
	"github.com/gin-contrib/static"
	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
	"log/slog"
	"net/http"
	"os"
	"path/filepath"
	"time"
)

func NewLiveStream(db *gorm.DB) *livestream.Core {
	vdb := livestreamdb.NewDB(db).AutoMigrate(true)

	core := livestream.NewCore(vdb)
	err := core.UpdateOnlineAll(0)
	if err != nil {
		fmt.Printf("initLive error:%v\n", err.Error())
	}
	return core
}

func NewVodCore(db *gorm.DB) *video.Core {
	vodDb := voddb.NewDB(db).AutoMigrate(true)
	vodCore := video.NewCore(vodDb)
	return vodCore
}

func NewUserCore(db *gorm.DB) {
	// 初始化用户
	var u data.User
	db.AutoMigrate(&data.User{})

	if err := db.Where("username=?", "admin").First(&u).Error; err != nil {
		slog.Info("初始化数据库用户表结构")
		if err := initUser("admin", "admin"); err != nil {
			slog.Error("initUser", "err", err)
		}
	}
}

func initUser(username, password string) error {
	// 创建系统管理员用户
	u := data.User{
		ID:       1,        // 用户ID
		Name:     "系统管理员",  // 用户名称
		Remark:   "系统创建",   // 用户备注
		Username: username, // 用户名
		Role:     "admin",  // 用户角色
	}
	// 对密码进行加密
	s := sha256.Sum256([]byte(password))
	u.Password = hex.EncodeToString(s[:])
	data.GetDatabase().Where(data.User{}).FirstOrCreate(&u)
	return nil
}

func NewHTTPHandler(uc *conf.Bootstrap) http.Handler {

	// 如果不处于调试模式，将 Gin 设置为发布模式
	if !uc.Debug {
		gin.SetMode(gin.ReleaseMode) // 将 Gin 设置为发布模式
	}
	//g = gin.New()
	g := svr.Lals.GetRouter()
	if g == nil {
		for i := 0; i < 5; i++ {
			g = svr.Lals.GetRouter()
			if g != nil {
				break
			}
			time.Sleep(500 * time.Millisecond)
		}
		if g == nil {
			return g
		}
	}
	//g := gin.Default() // 创建一个新的 Gin 实例
	dir, _ := os.Getwd()
	//前端分发
	wwwDir := filepath.Join(dir, "web")
	g.Use(static.Serve("/", static.LocalFile(wwwDir, true)))
	//快照
	snapDir := filepath.Join(dir, "snap")
	g.Use(static.Serve("/snap", static.LocalFile(snapDir, true)))

	// 处理未找到路由的情况，返回 JSON 格式的 404 错误信息
	g.NoRoute(func(c *gin.Context) {
		c.JSON(404, "来到了无人的荒漠") // 返回 JSON 格式的 404 错误信息
	})
	// 如果启用了 Pprof，设置 Pprof 监控
	//if cfg.Pprof {
	//	web.SetupPProf(g, &cfg.AcessIps) // 设置 Pprof 监控
	//}

	setupRouter(g, uc) // 设置路由处理函数

	return g // 返回配置好的 Gin 实例作为 http.Handler
}
