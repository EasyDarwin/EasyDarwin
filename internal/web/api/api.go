package api

import (
	"easydarwin/internal/conf"
	"easydarwin/utils/pkg/web"
	"github.com/gin-gonic/gin"
	"log/slog"
	"net/http"
	"runtime/debug"
)

func setupRouter(router *gin.Engine, uc *conf.Bootstrap) {

	router.Use(
		// 格式化输出到控制台，然后记录到日志
		// 此处不做 recover，底层 http.server 也会 recover，但不会输出方便查看的格式
		gin.CustomRecovery(func(c *gin.Context, err any) {
			slog.Error("panic", "err", err, "stack", string(debug.Stack()))
			c.AbortWithStatus(http.StatusInternalServerError)
		}),
		//web.Mertics(),
		web.Logger(slog.Default(), func(_ *gin.Context) bool {
			// true:记录请求响应报文
			return uc.Debug
		}),
	)
	path := "/api/v1"
	r := router.Group(path)
	registerApp(r)
	//registerConfig(r, ConfigAPI{cfg: uc.Conf, uc: uc, app: app}, auth)
	//registerVersion(r, uc.Version, auth)
	registerLiveStream(r)
	registerReverseProxy(router)
}
