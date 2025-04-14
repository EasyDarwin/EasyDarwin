package server

import (
	"net/http"

	"github.com/gin-gonic/gin"
	"github.com/q191201771/lal/pkg/base"
)

func (s *LalMaxServer) Cors() gin.HandlerFunc {
	return func(c *gin.Context) {
		method := c.Request.Method
		origin := c.GetHeader("Origin")
		if len(origin) == 0 {
			c.Header("Access-Control-Allow-Origin", "*")
		} else {
			c.Header("Access-Control-Allow-Origin", origin)
		}
		//服务器支持的所有跨域请求的方法
		c.Header("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE,UPDATE")
		//允许跨域设置可以返回其他子段，可以自定义字段
		c.Header("Access-Control-Allow-Headers", "*")
		c.Header("Access-Control-Allow-Headers", "Content-Type,Access-Token")
		c.Header("Access-Control-Allow-Credentials", "true")
		c.Header("Cross-Origin-Resource-Policy", "cross-origin")

		//允许类型校验
		if method == "OPTIONS" {
			c.AbortWithStatus(http.StatusNoContent)
		}
		c.Next()
	}
}

// Authentication 接口鉴权
func Authentication(secrets, ips []string) gin.HandlerFunc {
	out := base.ApiRespBasic{
		ErrorCode: http.StatusUnauthorized,
		Desp:      http.StatusText(http.StatusUnauthorized),
	}
	return func(c *gin.Context) {
		if !authentication(c.Query("token"), c.ClientIP(), secrets, ips) {
			c.JSON(200, out)
			return
		}
		c.Next()
	}
}

// authentication 判断是否符合要求，返回 false 表示鉴权失败
func authentication(reqToken, clientIP string, secrets, ips []string) bool {
	// 秘钥过滤
	if len(secrets) > 0 && !containFn(secrets, reqToken) {
		return false
	}
	// ip 白名单过滤
	if len(ips) > 0 && !containFn(ips, clientIP) {
		return false
	}
	return true
}

func containFn[T comparable](ts []T, t T) bool {
	for _, v := range ts {
		if v == t {
			return true
		}
	}
	return false
}
