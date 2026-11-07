// Package api Copyright 2025 EasyDarwin.
// http://www.easydarwin.org
// 管理接口鉴权
// 修复 CNVD-2026-42897 / CNVD-2026-42898 未授权访问漏洞：
// /api/v1 路由组统一强制 JWT 鉴权，仅白名单接口允许匿名访问
package api

import (
	"easydarwin/utils/pkg/web"
	"net"
	"net/http"

	"github.com/gin-gonic/gin"
)

// authWithWhitelist /api/v1 统一鉴权中间件
// 白名单：登录/登出、分享页点播查询（处理函数内限制仅返回分享中的视频）
// 流媒体内部回调（on_pub_start 等）仅允许本机回环地址访问
func authWithWhitelist(secret string) gin.HandlerFunc {
	auth := web.AuthMiddleware(secret)
	return func(c *gin.Context) {
		switch c.FullPath() {
		case "/api/v1/login", "/api/v1/logout":
			c.Next()
			return
		case "/api/v1/vod/get":
			// 分享播放页 share.html 匿名访问，get 处理函数内校验分享状态
			c.Next()
			return
		case "/api/v1/push/on_pub_start", "/api/v1/push/on_pub_stop", "/api/v1/push/on_rtmp_connect":
			// 内部回调由流媒体服务从 127.0.0.1 发起
			// 使用 RemoteAddr 而非 ClientIP，防止 X-Forwarded-For 伪造绕过
			if isLoopbackRemote(c.Request.RemoteAddr) {
				c.Next()
				return
			}
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{"msg": "无权访问"})
			return
		}
		auth(c)
	}
}

// isLoopbackRemote 判断 TCP 连接对端是否为本机回环地址
func isLoopbackRemote(remoteAddr string) bool {
	host, _, err := net.SplitHostPort(remoteAddr)
	if err != nil {
		host = remoteAddr
	}
	ip := net.ParseIP(host)
	return ip != nil && ip.IsLoopback()
}
